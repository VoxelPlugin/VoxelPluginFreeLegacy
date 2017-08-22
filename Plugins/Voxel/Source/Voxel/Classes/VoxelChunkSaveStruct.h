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

	UPROPERTY(EditAnywhere, Category = Voxel)
		TArray<int> Values;

	UPROPERTY(EditAnywhere, Category = Voxel)
		TArray<FColor> Colors;

	FVoxelChunkSaveStruct() : Depth(-1)
	{
	}

	FVoxelChunkSaveStruct(FIntVector Position, int Depth, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors) : Position(Position), Depth(Depth), Values(Values), Colors(Colors)
	{
	}
};