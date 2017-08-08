#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.generated.h"


USTRUCT(BlueprintType)
struct VOXEL_API FVoxelChunkSaveStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		FIntVector Position;

	UPROPERTY(EditAnywhere, Category = Voxel)
		int Depth;

	TArray<int, TFixedAllocator<16 * 16 * 16>> Values;

	TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors;

	FVoxelChunkSaveStruct()
	{
	}

	FVoxelChunkSaveStruct(FIntVector Position, int Depth, TArray<int, TFixedAllocator<16 * 16 * 16>> Values, TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors) : Position(Position), Depth(Depth), Values(Values), Colors(Colors)
	{
	}

	FVoxelChunkSaveStruct(FIntVector Position, int Depth, TArray<signed char, TFixedAllocator<16 * 16 * 16>> Values, TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors) : Position(Position), Depth(Depth), Colors(Colors)
	{
		this->Values.SetNumUninitialized(Values.Num());
		for (int i = 0; i < Values.Num(); i++)
		{
			this->Values[i] = Values[i];
		}
	}
};