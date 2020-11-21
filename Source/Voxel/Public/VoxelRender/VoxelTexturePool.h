// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "UObject/GCObject.h"

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
	explicit FVoxelTexturePoolTextureData(TArray<FColor>&& InData)
		: Data(MoveTemp(InData))
	{
		ensure(Data.Num() > 0);
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_TextureData, Data.GetAllocatedSize());
	}
	~FVoxelTexturePoolTextureData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTexturePool_TextureData, Data.GetAllocatedSize());
	}

	const FColor* RESTRICT GetData() const { return Data.GetData(); }
	int32 Num() const { return Data.Num(); }

private:
	const TArray<FColor> Data;
};

struct VOXEL_API FVoxelTexturePoolSettings
{
	const TWeakObjectPtr<const AVoxelWorld> DebugVoxelWorld;
	const int32 TextureSize = 512;

	explicit FVoxelTexturePoolSettings(const AVoxelWorld* World, EVoxelPlayType PlayType);
};

class VOXEL_API FVoxelTexturePool : public FGCObject, public TVoxelSharedFromThis<FVoxelTexturePool>
{
public:
	DECLARE_TYPED_VOXEL_SPARSE_ARRAY_ID(FEntryId);
	DECLARE_UNIQUE_VOXEL_ID(FEntryUniqueId);
	
	const FVoxelTexturePoolSettings Settings;

	static TVoxelSharedRef<FVoxelTexturePool> Create(const FVoxelTexturePoolSettings& Settings);
	
private:
	explicit FVoxelTexturePool(const FVoxelTexturePoolSettings& Settings)
		: Settings(Settings)
	{
	}

public:
	TVoxelSharedRef<FVoxelTexturePoolEntry> AddEntry(
		const TVoxelSharedRef<FVoxelTexturePoolTextureData>& ColorData,
		const TVoxelSharedRef<FVoxelMaterialInterface>& MaterialInstance);
	
	void RemoveEntry(FEntryId EntryId, FEntryUniqueId UniqueId);

	// Reallocate all the entries, reducing fragmentation
	void Compact();

protected:
	// Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End FGCObject Interface
	
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
		UTexture2D* Texture = nullptr;

		TArray<FTextureSlot> FreeSlots;
		TVoxelTypedSparseArray<FTextureSlotId, FUsedTextureSlot> UsedSlots;

		FTextureInfo();
		~FTextureInfo();
		UE_NONCOPYABLE(FTextureInfo);
		
		void Check() const;
		void FreeSlot(FTextureSlotId SlotId, FEntryUniqueId EntryId);

	private:
		void CompactFreeSlots();
	};
	TArray<TVoxelSharedPtr<FTextureInfo>> TextureInfos;

public:
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

public:
	struct FEntry
	{
		const FEntryUniqueId Id = VOXEL_UNIQUE_ID();
		
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