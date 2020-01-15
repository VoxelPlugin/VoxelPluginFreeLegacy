// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelConfigEnums.h"
#include "VoxelTickable.h"

struct FVoxelChunkMesh;
class AVoxelWorld;
class AVoxelWorldInterface;
class UVoxelInvokerComponent;

DECLARE_DELEGATE_OneParam(FChunkDelegate, FIntBox);
DECLARE_MULTICAST_DELEGATE_OneParam(FChunkMulticastDelegate, FIntBox);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnMeshCreatedDelegate, int32, const FIntBox&, const FVoxelChunkMesh&);

struct FVoxelProcGenSettings
{
	const float UpdateRate;
	const TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface;
	const TWeakObjectPtr<UWorld> World;
	const FIntBox WorldBounds;

	FVoxelProcGenSettings(const AVoxelWorld* World, EVoxelPlayType PlayType);
};

struct FVoxelProcGenEventHandle
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

namespace EVoxelProcGenFlags
{
	enum Type : uint32
	{
		None = 0,
		GenerationEvent = 0x1,
		LocalInvokerOnly = 0x2
	};
}

class FVoxelProcGenManager : public FVoxelTickable, public TVoxelSharedFromThis<FVoxelProcGenManager>
{
public:
	FVoxelProcGenSettings Settings;

	static TVoxelSharedRef<FVoxelProcGenManager> Create(const FVoxelProcGenSettings& Settings);
	void Destroy();
	~FVoxelProcGenManager();

private:
	explicit FVoxelProcGenManager(const FVoxelProcGenSettings& Settings);

public:
	FVoxelProcGenEventHandle BindEvent(
		bool bFireExistingOnes, 
		int32 ChunkSize, 
		int32 DistanceInChunks, 
		const FChunkDelegate& OnActivate, 
		const FChunkDelegate& OnDeactivate,
		EVoxelProcGenFlags::Type Flags = EVoxelProcGenFlags::None);
	FVoxelProcGenEventHandle BindGenerationEvent(
		bool bFireExistingOnes,
		int32 ChunkSize,
		int32 DistanceInChunks,
		const FChunkDelegate& OnGenerate,
		EVoxelProcGenFlags::Type Flags = EVoxelProcGenFlags::None);
	
	void UnbindEvent(FVoxelProcGenEventHandle Handle);

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
				Lambda(FIntBox(P * Event.ChunkSize, (P + 1) * Event.ChunkSize));
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
		FVector Position; // TODO: store an IntVector for more precision?
		TWeakObjectPtr<UVoxelInvokerComponent> InvokerComponent;

		FEventInvoker() = default;
		FEventInvoker(uint32 ChunkSize, const FVector& Position, TWeakObjectPtr<UVoxelInvokerComponent> InvokerComponent)
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
	
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> OldInvokerComponents;
	TMap<FEventKey, TArray<FEventInvoker>> EventsInvokers;
	TMap<FEventKey, TUniquePtr<FEventInfo>> Events;

	void Update();
	void UpdateInvokers(const TArray<TPair<FEventKey, TArray<FVector>>>& InvokersToUpdate);

private:
	uint32 EventsAllocatedSize = 0;
	uint32 NumEvents = 0;
	uint32 NumActiveOrGeneratedChunks = 0;

	void UpdateEventsAllocatedSize();
};