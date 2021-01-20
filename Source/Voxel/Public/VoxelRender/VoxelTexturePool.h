// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGCObject.h"
#include "VoxelSubsystem.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "VoxelTexturePool.generated.h"

enum class EVoxelPlayType;
class AVoxelWorld;
class UTexture2D;
class FVoxelTexturePoolEntry;
class FVoxelMaterialInterface;

DECLARE_STATS_GROUP(TEXT("Voxel Texture Pool"), STATGROUP_VoxelTexturePool, STATCAT_Advanced);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Used Data"), STAT_VoxelTexturePool_UsedData, STATGROUP_VoxelTexturePool, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Wasted Data"), STAT_VoxelTexturePool_WastedData, STATGROUP_VoxelTexturePool, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Texture Data"), STAT_VoxelTexturePool_TextureData, STATGROUP_VoxelTexturePool, VOXEL_API);

class FVoxelTexturePoolTextureData
{
public:
	explicit FVoxelTexturePoolTextureData(uint32 Stride, TArray<uint8>&& InData)
		: Stride(Stride)
		, Data(MoveTemp(InData))
	{
		check(Data.Num() % Stride == 0);
		ensure(Data.Num() > 0);
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_TextureData, Data.GetAllocatedSize());
	}
	~FVoxelTexturePoolTextureData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_TextureData, Data.GetAllocatedSize());
	}

	uint32 GetStride() const { return Stride; }
	const uint8* RESTRICT GetData() const { return Data.GetData(); }
	int32 Num() const { return Data.Num() / Stride; }

private:
	const uint32 Stride;
	const TArray<uint8> Data;
};

UCLASS()
class VOXEL_API UVoxelTexturePoolSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelTexturePool);
};

DECLARE_UNIQUE_VOXEL_ID(FVoxelTexturePoolEntryUniqueId);

class VOXEL_API FVoxelTexturePool : public IVoxelSubsystem, public FVoxelGCObject
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelTexturePoolSubsystemProxy);
	DECLARE_TYPED_VOXEL_SPARSE_ARRAY_ID(FEntryId);
	using FEntryUniqueId = FVoxelTexturePoolEntryUniqueId;
	
public:
	FVoxelTexturePool(FVoxelRuntime& Runtime, const FVoxelRuntimeSettings& Settings);
	
	TVoxelSharedPtr<FVoxelTexturePoolEntry> AddEntry(
		const TVoxelSharedRef<FVoxelTexturePoolTextureData>& ColorData,
		const TVoxelSharedRef<FVoxelMaterialInterface>& MaterialInstance);
	
	void RemoveEntry(FEntryId EntryId, FEntryUniqueId UniqueId);

	// Reallocate all the entries, reducing fragmentation
	void Compact();

protected:
	// Begin FVoxelGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return "FVoxelTexturePool"; }
	// End FVoxelGCObject Interface

	//~ Begin IVoxelSubsystem Interface
	virtual void PreDestructor() override;
	//~ End IVoxelSubsystem Interface

private:
	const uint32 Stride;
	
private:
	struct FTextureSlot
	{
		int32 StartIndex = 0;
		int32 Num = 0;

		int32 EndIndex() const { return StartIndex + Num; }
	};
	struct FUsedTextureSlot : FTextureSlot
	{
		FEntryUniqueId EntryId;
	};

	DECLARE_TYPED_VOXEL_SPARSE_ARRAY_ID(FTextureSlotId);
	
	class FTextureInfo
	{
	public:
		const uint32 Stride;
		UTexture2D* Texture = nullptr;

		TArray<FTextureSlot> FreeSlots;
		TVoxelTypedSparseArray<FTextureSlotId, FUsedTextureSlot> UsedSlots;

		explicit FTextureInfo(uint32 Stride);
		~FTextureInfo();
		UE_NONCOPYABLE(FTextureInfo);
		
		void Check() const;
		void FreeSlot(FTextureSlotId SlotId, FEntryUniqueId EntryId);

	private:
		void CompactFreeSlots();
	};
	TArray<TVoxelSharedPtr<FTextureInfo>> TextureInfos;

private:
	struct FTextureSlotRef
	{
		TVoxelWeakPtr<FTextureInfo> TextureInfo;
		FTextureSlotId SlotId;

		bool IsValid() const
		{
			return SlotId.IsValid();
		}
		const FTextureInfo* GetSlot(FUsedTextureSlot& OutSlot) const;
	};
	
	FTextureSlotRef AllocateSlot(int32 Size, FEntryUniqueId EntryId);

	UTexture2D* CreateTexture() const;

private:
	struct FEntry
	{
		const FEntryUniqueId Id = FEntryUniqueId::New();
		
		const TVoxelSharedRef<FVoxelTexturePoolTextureData> ColorData;
		const TVoxelWeakPtr<FVoxelMaterialInterface> MaterialInstance;

		FTextureSlotRef SlotRef;

		FEntry(const TVoxelSharedRef<FVoxelTexturePoolTextureData>& ColorData, const TVoxelWeakPtr<FVoxelMaterialInterface>& MaterialInstance);
		~FEntry();
		UE_NONCOPYABLE(FEntry);
		
		void CopyDataToTexture(bool bJustClearData = false) const;
		void SetupMaterialInstance() const;

		void AllocateSlot(FVoxelTexturePool& Pool);
		void FreeSlot();
	};
	TVoxelTypedSparseArray<FEntryId, TVoxelSharedPtr<FEntry>> Entries;
};

class VOXEL_API FVoxelTexturePoolEntry
{
public:
	~FVoxelTexturePoolEntry();

private:
	const FVoxelTexturePool::FEntryId EntryId;
	const FVoxelTexturePool::FEntryUniqueId UniqueId;
	const TVoxelWeakPtr<FVoxelTexturePool> Pool;

	explicit FVoxelTexturePoolEntry(
		FVoxelTexturePool::FEntryId EntryId,
		FVoxelTexturePool::FEntryUniqueId UniqueId,
		const TVoxelWeakPtr<FVoxelTexturePool>& Pool)
		: EntryId(EntryId)
		, UniqueId(UniqueId)
		, Pool(Pool)
	{
	}

	friend class FVoxelTexturePool;
};