// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelTickable.h"

struct FVoxelChunkMesh;
class AVoxelWorld;
class AVoxelWorldInterface;
class UVoxelInvokerComponentBase;

DECLARE_DELEGATE_OneParam(FChunkDelegate, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_OneParam(FChunkMulticastDelegate, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnMeshCreatedDelegate, int32, const FVoxelIntBox&, const FVoxelChunkMesh&);

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Events Memory"), STAT_VoxelEventsMemory, STATGROUP_VoxelMemory, VOXEL_API);

struct VOXEL_API FVoxelEventManagerSettings
{
	const float UpdateRate;
	const TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface;
	const TWeakObjectPtr<UWorld> World;
	const FVoxelIntBox WorldBounds;

	FVoxelEventManagerSettings(const AVoxelWorld* World, EVoxelPlayType PlayType);
};

struct FVoxelEventHandle
{
	FDelegateHandle OnActivateHandle;
	FDelegateHandle OnDeactivateHandle;

	int32 ChunkSize = -1;
	int32 DistanceInChunks = -1;
	uint32 Flags;
	
	inline bool IsValid() const
	{
		return OnActivateHandle.IsValid() || OnDeactivateHandle.IsValid();
	}
};

namespace EVoxelEventFlags
{
	enum Type : uint32
	{
		None = 0,
		GenerationEvent = 0x1,
		LocalInvokerOnly = 0x2
	};
}

class VOXEL_API FVoxelEventManager : public FVoxelTickable, public TVoxelSharedFromThis<FVoxelEventManager>
{
public:
	FVoxelEventManagerSettings Settings;

	static TVoxelSharedRef<FVoxelEventManager> Create(const FVoxelEventManagerSettings& Settings);
	void Destroy();
	~FVoxelEventManager();

private:
	explicit FVoxelEventManager(const FVoxelEventManagerSettings& Settings);
	UE_NONCOPYABLE(FVoxelEventManager);
	
public:
	FVoxelEventHandle BindEvent(
		bool bFireExistingOnes, 
		int32 ChunkSize, 
		int32 DistanceInChunks, 
		const FChunkDelegate& OnActivate, 
		const FChunkDelegate& OnDeactivate,
		EVoxelEventFlags::Type Flags = EVoxelEventFlags::None);
	FVoxelEventHandle BindGenerationEvent(
		bool bFireExistingOnes,
		int32 ChunkSize,
		int32 DistanceInChunks,
		const FChunkDelegate& OnGenerate,
		EVoxelEventFlags::Type Flags = EVoxelEventFlags::None);
	
	void UnbindEvent(FVoxelEventHandle Handle);

public:
	template<typename T>
	void IterateActiveChunks(int32 ChunkSize, int32 DistanceInChunks, uint32 Flags, T Lambda) const
	{
		const FEventKey Key{ ChunkSize, DistanceInChunks, Flags };
		if (auto* EventPtr = Events.Find(Key))
		{
			auto& Event = **EventPtr;
			for (auto& P : Event.ActiveOrGeneratedChunks)
			{
				Lambda(FVoxelIntBox(P * Event.ChunkSize, (P + 1) * Event.ChunkSize));
			}
		}
	}

protected:
	//~ Begin FVoxelTickable Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	//~ End FVoxelTickable Interface
	
private:
	struct FEventKey
	{
		int32 ChunkSize = -1;
		int32 Distance = -1;
		uint32 Flags = 0;

		FEventKey() = default;
		FEventKey(int32 ChunkSize, int32 Distance, uint32 Flags)
			: ChunkSize(ChunkSize)
			, Distance(Distance)
			, Flags(Flags)
		{
		}
		
		inline friend uint32 GetTypeHash(FEventKey Key)
		{
			return uint32(Key.ChunkSize) + uint32(Key.Distance * 23) + 93 * Key.Flags;
		}
		inline bool operator==(const FEventKey& Other) const
		{
			return
				ChunkSize == Other.ChunkSize &&
				Distance == Other.Distance &&
				Flags == Other.Flags;
		}
	};
	struct FEventInvoker
	{
		uint32 ChunkSize = -1;
		FIntVector Position;
		TWeakObjectPtr<UVoxelInvokerComponentBase> InvokerComponent;

		FEventInvoker() = default;
		FEventInvoker(uint32 ChunkSize, const FIntVector& Position, TWeakObjectPtr<UVoxelInvokerComponentBase> InvokerComponent)
			: ChunkSize(ChunkSize)
			, Position(Position)
			, InvokerComponent(InvokerComponent)
		{
		}
	};
	struct FEventInfo
	{
		const int32 ChunkSize;
		const int32 DistanceInChunks;
		const uint32 Flags;
		FChunkMulticastDelegate OnActivate;
		FChunkMulticastDelegate OnDeactivate;
		TSet<FIntVector> ActiveOrGeneratedChunks; // If generation event this is the list of already generated chunks

		FEventInfo(int32 ChunkSize, int32 Distance, uint32 Flags)
			: ChunkSize(ChunkSize)
			, DistanceInChunks(Distance)
			, Flags(Flags)
		{
		}

		inline bool IsBound() const { return OnActivate.IsBound() || OnDeactivate.IsBound(); }
		inline uint32 GetAllocatedSize() const { return sizeof(*this) + ActiveOrGeneratedChunks.GetAllocatedSize(); }
	};

	double LastUpdateTime = 0;
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> OldInvokerComponents;
	TMap<FEventKey, TArray<FEventInvoker>> EventsInvokers;
	TMap<FEventKey, TUniquePtr<FEventInfo>> Events;

	void Update();
	void UpdateInvokers(const TArray<TPair<FEventKey, TArray<FIntVector>>>& InvokersToUpdate);
	void ClearOldInvokerComponents();

private:
	uint32 EventsAllocatedSize = 0;
	uint32 NumEvents = 0;
	uint32 NumActiveOrGeneratedChunks = 0;

	void UpdateEventsAllocatedSize();
};