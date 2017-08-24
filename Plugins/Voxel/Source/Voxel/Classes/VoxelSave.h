#pragma once

#include "CoreMinimal.h"
#include <list>
#include "VoxelSave.generated.h"


USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelChunkSave
{
	GENERATED_BODY()

public:
	UPROPERTY()
		uint32 Id;

	UPROPERTY(VisibleAnywhere)
		FIntVector Position;

	UPROPERTY(VisibleAnywhere)
		TArray<float> Values;

	UPROPERTY(VisibleAnywhere)
		TArray<FColor> Colors;

	FVoxelChunkSave() : Id(-1)
	{
	}

	FVoxelChunkSave(uint32 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values,
					TArray<FColor, TFixedAllocator<16 * 16 * 16>>& Colors) : Id(Id), Position(Position), Values(Values), Colors(Colors)
	{
	}
};

USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelWorldSave
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
		int Depth;

	UPROPERTY(VisibleAnywhere)
		TArray<FVoxelChunkSave> Chunks;


	FVoxelWorldSave() : Depth(-1)
	{
	}

	FVoxelWorldSave(int Depth, std::list<FVoxelChunkSave> ChunksList) : Depth(Depth)
	{
		Chunks.SetNum(ChunksList.size());

		for (int i = 0; i < Chunks.Num(); i++)
		{
			Chunks[i] = ChunksList.back();
			ChunksList.pop_back();
		}
	}

	std::list<FVoxelChunkSave> GetChunksList()
	{
		std::list<FVoxelChunkSave> ChunksList;
		for (int i = 0; i < Chunks.Num(); i++)
		{
			ChunksList.push_front(Chunks[i]);
		}
		return ChunksList;
	}
};