// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelSubsystem.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"
#include "VoxelChunkGeneration.generated.h"

struct FVoxelChunkConfig
{
	static constexpr int32 ChunkSizeXY = 32;
	static constexpr int32 ChunkSizeZ = 256;
	static constexpr int32 NumVoxels = ChunkSizeXY * ChunkSizeXY * ChunkSizeZ;

	static FIntVector GetChunkSize()
	{
		return FIntVector(ChunkSizeXY, ChunkSizeXY, ChunkSizeZ);
	}
	static FIntPoint GetChunkPosition(const FVoxelVector2D& Position)
	{
		return FVoxelUtilities::FloorToInt(Position / ChunkSizeXY) * ChunkSizeXY;
	}
};

class IVoxelChunkChannel
{
public:
};

class IVoxelChunkChannelData
{
public:
};

class FVoxelChunkData
{
public:
	template<typename T>
	using TData = TVoxelStaticArray<T, FVoxelChunkConfig::NumVoxels>;
	
	FCriticalSection CriticalSection;
	TUniquePtr<TData<FVoxelValue>> Values;
	TUniquePtr<TData<FVoxelMaterial>> Materials;

	TMap<FName, TUniquePtr<IVoxelChunkChannelData>> ChannelsData;
	
	template<typename T>
	const TData<T>* GetData() const
	{
		return FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*this).Get();
	}
	template<typename T>
	void SetData(const TData<T>& Data)
	{
		FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*this) = MakeUnique<TData<T>>(Data);
	}
};

class FVoxelChunksData : public TVoxelSharedFromThis<FVoxelChunksData>
{
public:
	TVoxelSharedPtr<FVoxelChunkData> FindChunk(const FIntPoint& Position) const
	{
		FScopeLock Lock(&CriticalSection);
		return FindChunk_AssumeLocked(Position);
	}
	TVoxelSharedRef<FVoxelChunkData> FindOrAddChunk(const FIntPoint& Position)
	{
		FScopeLock Lock(&CriticalSection);
		return FindOrAddChunk_AssumeLocked(Position);
	}
	
	TVoxelSharedPtr<FVoxelChunkData> FindChunk_AssumeLocked(const FIntPoint& Position) const
	{
		return Chunks.FindRef(Position);
	}
	TVoxelSharedRef<FVoxelChunkData> FindOrAddChunk_AssumeLocked(const FIntPoint& Position)
	{
		TVoxelSharedPtr<FVoxelChunkData>& Result = Chunks.FindOrAdd(Position);
		if (!Result)
		{
			Result = MakeVoxelShared<FVoxelChunkData>();
		}
		return Result.ToSharedRef();
	}

	FCriticalSection& GetSection() const
	{
		return CriticalSection;
	}

private:
	mutable FCriticalSection CriticalSection;
	TMap<FIntPoint, TVoxelSharedPtr<FVoxelChunkData>> Chunks;
};

UCLASS()
class VOXEL_API UVoxelChunksSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelChunksSubsystem);
};

class VOXEL_API FVoxelChunksSubsystem : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelChunksSubsystemProxy);
	
	const TVoxelSharedRef<FVoxelChunksData> Data = MakeVoxelShared<FVoxelChunksData>();

	//~ Begin IVoxelSubsystem Interface
	virtual void Create() override;
	//~ End IVoxelSubsystem Interface
};