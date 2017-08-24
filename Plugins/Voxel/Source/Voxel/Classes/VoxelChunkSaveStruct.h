#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.generated.h"


USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelChunkSaveStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		FIntVector Position;

	UPROPERTY(EditAnywhere)
		TArray<float> Values;

	UPROPERTY(EditAnywhere)
		TArray<FColor> Colors;

	FVoxelChunkSaveStruct() : Id(0)
	{
	}

	FVoxelChunkSaveStruct(uint32 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values,
						  TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors) : Id(Id), Position(Position), Values(Values), Colors(Colors)
	{
	}
};