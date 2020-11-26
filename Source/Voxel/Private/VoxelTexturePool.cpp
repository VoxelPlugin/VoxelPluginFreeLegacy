// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelTexturePool.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelMessages.h"
#include "VoxelWorld.h"

#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#if ENGINE_MINOR_VERSION >= 26
#include "Rendering/Texture2DResource.h"
#endif

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Free Slots"), STAT_VoxelTexturePool_FreeSlots, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Used Slots"), STAT_VoxelTexturePool_UsedSlots, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Texture"), STAT_VoxelTexturePool_NumTextures, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Entries"), STAT_VoxelTexturePool_NumEntries, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Num Texture Updates"), STAT_VoxelTexturePool_NumTextureUpdates, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Num RHIUpdateTexture2D"), STAT_VoxelTexturePool_NumRHIUpdateTexture2D, STATGROUP_VoxelTexturePool);
DECLARE_DWORD_COUNTER_STAT(TEXT("Texture Updates Size"), STAT_VoxelTexturePool_TextureUpdatesSize, STATGROUP_VoxelTexturePool);

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelTexturePool_UsedData);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelTexturePool_WastedData);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelTexturePool_TextureData);

FVoxelTexturePoolSettings::FVoxelTexturePoolSettings(const AVoxelWorld* World, EVoxelPlayType PlayType)
	: DebugVoxelWorld(World)
	, TextureSize(FMath::Clamp(World->TexturePoolTextureSize, 128, 16384))
{
}

TVoxelSharedRef<FVoxelTexturePool> FVoxelTexturePool::Create(const FVoxelTexturePoolSettings& Settings)
{
	return MakeShareable(new FVoxelTexturePool(Settings));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelTexturePoolEntry> FVoxelTexturePool::AddEntry(
	const TVoxelSharedRef<FVoxelTexturePoolTextureData>& ColorData, 
	const TVoxelSharedRef<FVoxelMaterialInterface>& MaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();
	
	ensure(ColorData->Num() > 0);
	ensure(MaterialInstance->IsMaterialInstance());

	const auto Entry = MakeVoxelShared<FEntry>(ColorData, MaterialInstance);
	Entry->AllocateSlot(*this);
	
	const FEntryId EntryId = Entries.Add(Entry);
	return MakeShareable(new FVoxelTexturePoolEntry(EntryId, Entry->Id, AsShared()));
}

void FVoxelTexturePool::RemoveEntry(FEntryId EntryId, FEntryUniqueId UniqueId)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!ensure(Entries.IsValidIndex(EntryId)))
	{
		return;
	}

	FEntry& Entry = *Entries[EntryId];
	if (ensure(Entry.Id == UniqueId))
	{
		Entry.FreeSlot();
		Entries.RemoveAt(EntryId);
	}
}

void FVoxelTexturePool::Compact()
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& Entry : Entries)
	{
		Entry->FreeSlot();
	}
	for (auto& Entry : Entries)
	{
		Entry->AllocateSlot(*this);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelTexturePool::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	for (auto& TextureInfo : TextureInfos)
	{
		TextureInfo->Check();
		ensure(TextureInfo->Texture);
		Collector.AddReferencedObject(TextureInfo->Texture);
		ensure(TextureInfo->Texture);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePool::FTextureInfo::FTextureInfo()
{
	INC_DWORD_STAT(STAT_VoxelTexturePool_NumTextures);
}

FVoxelTexturePool::FTextureInfo::~FTextureInfo()
{
	DEC_DWORD_STAT(STAT_VoxelTexturePool_NumTextures);
	
	DEC_DWORD_STAT_BY(STAT_VoxelTexturePool_FreeSlots, FreeSlots.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelTexturePool_UsedSlots, UsedSlots.Num());

	for (auto& Slot : FreeSlots)
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_WastedData, sizeof(FColor) * Slot.Num);
	}
	for (auto& Slot : UsedSlots)
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_UsedData, sizeof(FColor) * Slot.Num);
	}
}

void FVoxelTexturePool::FTextureInfo::Check() const
{
#if VOXEL_DEBUG
	for (int32 Index = 1; Index < FreeSlots.Num(); Index++)
	{
		auto& SlotA = FreeSlots[Index - 1];
		auto& SlotB = FreeSlots[Index];

		ensure(SlotA.Num > 0);
		ensure(SlotB.Num > 0);
		
		ensure(SlotA.StartIndex < SlotB.StartIndex);
		// If equal, should be merged
		ensure(SlotA.EndIndex() < SlotB.StartIndex);
	}
#endif
}

void FVoxelTexturePool::FTextureInfo::FreeSlot(FTextureSlotId SlotId, FEntryUniqueId EntryId)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());
	ensure(EntryId.IsValid());
	Check();
	
	if (!ensure(UsedSlots.IsValidIndex(SlotId)))
	{
		return;
	}

	const FUsedTextureSlot UsedSlot = UsedSlots[SlotId];
	UsedSlots.RemoveAt(SlotId);
	DEC_DWORD_STAT(STAT_VoxelTexturePool_UsedSlots);
	ensure(UsedSlot.EntryId == EntryId);
	
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_WastedData, sizeof(FColor) * UsedSlot.Num);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_UsedData, sizeof(FColor) * UsedSlot.Num);

	int32 Index;
	for (Index = 0; Index < FreeSlots.Num(); Index++)
	{
		if (FreeSlots[Index].StartIndex > UsedSlot.StartIndex)
		{
			break;
		}
	}
	
	INC_DWORD_STAT(STAT_VoxelTexturePool_FreeSlots);
	FreeSlots.Insert(FTextureSlot(UsedSlot), Index);

	CompactFreeSlots();
	Check();
}

void FVoxelTexturePool::FTextureInfo::CompactFreeSlots()
{
	VOXEL_FUNCTION_COUNTER();
	check(FreeSlots.Num() > 0);
	
	TArray<FTextureSlot> NewSlots;
	NewSlots.Add(FreeSlots[0]);
	
	for (int32 Index = 1; Index < FreeSlots.Num(); Index++)
	{
		const FTextureSlot& Slot = FreeSlots[Index];
		FTextureSlot& LastSlot = NewSlots.Last();

		if (LastSlot.EndIndex() == Slot.StartIndex)
		{
			LastSlot.Num += Slot.Num;
		}
		else
		{
			NewSlots.Add(Slot);
		}
	}

	DEC_DWORD_STAT_BY(STAT_VoxelTexturePool_FreeSlots, FreeSlots.Num());
	FreeSlots = MoveTemp(NewSlots);
	INC_DWORD_STAT_BY(STAT_VoxelTexturePool_FreeSlots, FreeSlots.Num());
}

const FVoxelTexturePool::FTextureInfo* FVoxelTexturePool::FTextureSlotRef::GetSlot(FUsedTextureSlot& OutSlot) const
{
	if (!ensure(IsValid()))
	{
		return nullptr;
	}

	const auto PinnedTextureInfo = TextureInfo.Pin();
	if (!ensure(PinnedTextureInfo))
	{
		return nullptr;
	}

	if (!ensure(PinnedTextureInfo->UsedSlots.IsValidIndex(SlotId)))
	{
		return nullptr;
	}

	OutSlot = PinnedTextureInfo->UsedSlots[SlotId];
	return PinnedTextureInfo.Get();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePool::FTextureSlotRef FVoxelTexturePool::AllocateSlot(int32 Size, FEntryUniqueId EntryId)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());
	ensure(EntryId.IsValid());

	if (Size > FMath::Square(Settings.TextureSize))
	{
		FVoxelMessages::Error(FString::Printf(
			TEXT("The voxel world texture pool texture size is too small: tried to allocate %d values, but textures are max %dx%d!"),
			Size,
			Settings.TextureSize,
			Settings.TextureSize));
		return {};
	}

	// Try to find a free slot
	for (auto& TextureInfo : TextureInfos)
	{
		auto& FreeSlots = TextureInfo->FreeSlots;
		for (int32 Index = 0; Index < FreeSlots.Num(); Index++)
		{
			auto& FreeSlot = FreeSlots[Index];
			if (FreeSlot.Num == Size)
			{
				FUsedTextureSlot UsedSlot;
				UsedSlot.StartIndex = FreeSlot.StartIndex;
				UsedSlot.Num = FreeSlot.Num;
				UsedSlot.EntryId = EntryId;

				DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_WastedData, sizeof(FColor) * FreeSlot.Num);
				DEC_DWORD_STAT(STAT_VoxelTexturePool_FreeSlots);
				FreeSlots.RemoveAt(Index);

				FTextureSlotRef Ref;
				Ref.TextureInfo = TextureInfo;
				Ref.SlotId = TextureInfo->UsedSlots.Add(UsedSlot);
				INC_DWORD_STAT(STAT_VoxelTexturePool_UsedSlots);
				INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_UsedData, sizeof(FColor) * UsedSlot.Num);
				return Ref;
			}
			else if (FreeSlot.Num > Size)
			{
				FUsedTextureSlot UsedSlot;
				UsedSlot.StartIndex = FreeSlot.StartIndex;
				UsedSlot.Num = Size;
				UsedSlot.EntryId = EntryId;

				FreeSlot.StartIndex += Size;
				FreeSlot.Num -= Size;
				DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_WastedData, sizeof(FColor) * Size);

				FTextureSlotRef Ref;
				Ref.TextureInfo = TextureInfo;
				Ref.SlotId = TextureInfo->UsedSlots.Add(UsedSlot);
				INC_DWORD_STAT(STAT_VoxelTexturePool_UsedSlots);
				INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_UsedData, sizeof(FColor) * UsedSlot.Num);
				return Ref;
			}
		}
	}
	
	// Allocate a new texture
	UTexture2D* NewTexture = CreateTexture();
	if (!ensure(NewTexture))
	{
		return {};
	}

	const auto NewTextureInfo = MakeVoxelShared<FTextureInfo>();
	NewTextureInfo->Texture = NewTexture;
	TextureInfos.Add(NewTextureInfo);

	if (Size < FMath::Square(Settings.TextureSize))
	{
		// Add free slot
		FTextureSlot FreeSlot;
		FreeSlot.StartIndex = Size;
		FreeSlot.Num = FMath::Square(Settings.TextureSize) - Size;
		NewTextureInfo->FreeSlots.Add(FreeSlot);
		INC_DWORD_STAT(STAT_VoxelTexturePool_FreeSlots);
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_WastedData, sizeof(FColor) * FreeSlot.Num);
	}

	FUsedTextureSlot UsedSlot;
	UsedSlot.StartIndex = 0;
	UsedSlot.Num = Size;
	UsedSlot.EntryId = EntryId;

	FTextureSlotRef Ref;
	Ref.TextureInfo = NewTextureInfo;
	Ref.SlotId = NewTextureInfo->UsedSlots.Add(UsedSlot);
	INC_DWORD_STAT(STAT_VoxelTexturePool_UsedSlots);
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_UsedData, sizeof(FColor) * UsedSlot.Num);
	return Ref;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UTexture2D* FVoxelTexturePool::CreateTexture() const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());
	
	UTexture2D* Texture = UTexture2D::CreateTransient(Settings.TextureSize, Settings.TextureSize);
	if (!ensure(Texture))
	{
		return nullptr;
	}

	Texture->CompressionSettings = TC_VectorDisplacementmap;
	Texture->SRGB = false;
	Texture->Filter = TF_Nearest;

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	{
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memzero(Data, sizeof(FColor) * FMath::Square(Settings.TextureSize));
		Mip.BulkData.Unlock();
	}
	Texture->UpdateResource();

	if (auto* VoxelWorld = Settings.DebugVoxelWorld.Get())
	{
		VoxelWorld->DebugTextures.Remove(nullptr);
		VoxelWorld->DebugTextures.Add(Texture);
	}
	
	return Texture;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePool::FEntry::FEntry(const TVoxelSharedRef<FVoxelTexturePoolTextureData>& ColorData, const TVoxelWeakPtr<FVoxelMaterialInterface>& MaterialInstance)
	: ColorData(ColorData)
	, MaterialInstance(MaterialInstance)
{
	INC_DWORD_STAT(STAT_VoxelTexturePool_NumEntries);
}

FVoxelTexturePool::FEntry::~FEntry()
{
	DEC_DWORD_STAT(STAT_VoxelTexturePool_NumEntries);
}

void FVoxelTexturePool::FEntry::CopyDataToTexture(bool bJustClearData) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	FUsedTextureSlot Slot;
	const FTextureInfo* TextureInfo = SlotRef.GetSlot(Slot);
	if (!ensure(TextureInfo))
	{
		return;
	}

	if (!ensure(Slot.Num == ColorData->Num()))
	{
		return;
	}

	auto* Texture = TextureInfo->Texture;
	if (!ensure(Texture))
	{
		return;
	}

	INC_DWORD_STAT(STAT_VoxelTexturePool_NumTextureUpdates);
	INC_DWORD_STAT_BY(STAT_VoxelTexturePool_TextureUpdatesSize, ColorData->Num() * sizeof(FColor));

	if (ensure(Texture->Resource))
	{
		ENQUEUE_RENDER_COMMAND(UpdateVoxelTexturePoolRegionsData)(
        [
		  Resource = UE_26_SWITCH(static_cast<FTexture2DResource*>(Texture->Resource), Texture->Resource->GetTexture2DResource()),
          ColorData = ColorData, 
		  Slot, 
		  bJustClearData](FRHICommandListImmediate& RHICmdList)
        {
			VOXEL_RENDER_SCOPE_COUNTER("Update Region");
			
			const uint8* Data = reinterpret_cast<const uint8*>(ColorData->GetData());
			TArray<uint8> EmptyData;
			if (bJustClearData)
			{
				EmptyData.SetNumZeroed(Slot.Num * sizeof(FColor));
				Data = EmptyData.GetData();
			}
			
			const int32 RowSize = Resource->GetSizeX();
			const FTexture2DRHIRef TextureRHI = Resource->GetTexture2DRHI();
			const uint8* const EndData = Data + Slot.Num * sizeof(FColor);

			if (!TextureRHI)
			{
				ensure(GIsRequestingExit);
				return;
			}

			// SourcePitch - size in bytes of each row of the source image

			int32 FirstIndex = Slot.StartIndex;
			if (FirstIndex % RowSize != 0)
			{
				// Fixup any data at the start not aligned to the row
				
				const int32 StartX = FirstIndex % RowSize;
				const int32 Num = FMath::Min(RowSize - StartX, Slot.Num);
				
				FUpdateTextureRegion2D Region;
				Region.DestX = StartX;
				Region.DestY = FirstIndex / RowSize;
				Region.SrcX = 0;
				Region.SrcY = 0;
				Region.Width = Num;
				Region.Height = 1;
				
				VOXEL_RENDER_SCOPE_COUNTER("RHIUpdateTexture2D");
				INC_DWORD_STAT(STAT_VoxelTexturePool_NumRHIUpdateTexture2D);
				RHIUpdateTexture2D(TextureRHI, 0, Region, Num * sizeof(FColor), Data);

				FirstIndex += Num;
				Data += Region.Width * Region.Height * sizeof(FColor);
				check(Data <= EndData);
			}
			
			if (Data == EndData)
			{
				// Only one line
				return;
			}
			check(FirstIndex % RowSize == 0);
		
			const int32 LastIndex = Slot.EndIndex();
			const int32 FirstIndexRow = FirstIndex / RowSize;
			const int32 LastIndexRow = LastIndex / RowSize;
			if (FirstIndexRow != LastIndexRow)
			{
				FUpdateTextureRegion2D Region;
				Region.DestX = 0;
				Region.DestY = FirstIndexRow;
				Region.SrcX = 0;
				Region.SrcY = 0;
				Region.Width = RowSize;
				Region.Height = LastIndexRow - FirstIndexRow;
				
				VOXEL_RENDER_SCOPE_COUNTER("RHIUpdateTexture2D");
				INC_DWORD_STAT(STAT_VoxelTexturePool_NumRHIUpdateTexture2D);
				RHIUpdateTexture2D(TextureRHI, 0, Region, RowSize * sizeof(FColor), Data);

				Data += Region.Width * Region.Height * sizeof(FColor);
				check(Data <= EndData);
			}

			if (LastIndex % RowSize != 0)
			{
				// Fixup any data at the end not aligned to the row
				
				const int32 Num = LastIndex % RowSize;
				
				FUpdateTextureRegion2D Region;
				Region.DestX = 0;
				Region.DestY = LastIndex / RowSize;
				Region.SrcX = 0;
				Region.SrcY = 0;
				Region.Width = Num;
				Region.Height = 1;
				
				VOXEL_RENDER_SCOPE_COUNTER("RHIUpdateTexture2D");
				INC_DWORD_STAT(STAT_VoxelTexturePool_NumRHIUpdateTexture2D);
				RHIUpdateTexture2D(TextureRHI, 0, Region, Num * sizeof(FColor), Data);
				
				Data += Region.Width * Region.Height * sizeof(FColor);
			}
			check(Data == EndData);
		});
	}
	else
	{
		FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
		{
			void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
			if (ensure(Data))
			{
				void* Dst = static_cast<FColor*>(Data) + Slot.StartIndex;
				const void* Src = ColorData->GetData();
				const int32 Count = ColorData->Num() * sizeof(FColor);

				if (bJustClearData)
				{
					FMemory::Memzero(Dst, Count);
				}
				else
				{
					FMemory::Memcpy(Dst, Src, Count);
				}
			}
			Mip.BulkData.Unlock();
		}
		Texture->UpdateResource();
	}
}

void FVoxelTexturePool::FEntry::SetupMaterialInstance() const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	const TVoxelSharedPtr<FVoxelMaterialInterface> PinnedMaterialInstance = MaterialInstance.Pin();
	if (!ensure(PinnedMaterialInstance) ||
		!ensure(PinnedMaterialInstance->IsMaterialInstance()))
	{
		return;
	}

	UMaterialInstanceDynamic* MaterialInstanceObject = Cast<UMaterialInstanceDynamic>(PinnedMaterialInstance->GetMaterial());
	if (!ensure(MaterialInstanceObject))
	{
		return;
	}

	FUsedTextureSlot Slot;
	const FTextureInfo* TextureInfo = SlotRef.GetSlot(Slot);
	if (!ensure(TextureInfo) ||
		!ensure(TextureInfo->Texture))
	{
		return;
	}

	MaterialInstanceObject->SetTextureParameterValue(STATIC_FNAME("VoxelTexturePool_Texture"), TextureInfo->Texture);
	MaterialInstanceObject->SetScalarParameterValue(STATIC_FNAME("VoxelTexturePool_TextureSizeX"), TextureInfo->Texture->GetSizeX());
	MaterialInstanceObject->SetScalarParameterValue(STATIC_FNAME("VoxelTexturePool_IndexOffset"), Slot.StartIndex);
}

void FVoxelTexturePool::FEntry::AllocateSlot(FVoxelTexturePool& Pool)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!SlotRef.IsValid());
	SlotRef = Pool.AllocateSlot(ColorData->Num(), Id);

	if (SlotRef.IsValid())
	{
		// If we didn't fail to allocate
		CopyDataToTexture();
		SetupMaterialInstance();
	}
}

void FVoxelTexturePool::FEntry::FreeSlot()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!SlotRef.IsValid())
	{
		// Failed to allocate - nothing to do
		return;
	}

#if VOXEL_DEBUG
	// Clear texture to make spotting errors easier
	CopyDataToTexture(true);
#endif

	const auto TextureInfo = SlotRef.TextureInfo.Pin();
	if (!ensure(TextureInfo))
	{
		return;
	}

	TextureInfo->FreeSlot(SlotRef.SlotId, Id);

	SlotRef = {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTexturePoolEntry::~FVoxelTexturePoolEntry()
{
	const auto PinnedPool = Pool.Pin();
	if (PinnedPool.IsValid())
	{
		PinnedPool->RemoveEntry(EntryId, UniqueId);
	}
}