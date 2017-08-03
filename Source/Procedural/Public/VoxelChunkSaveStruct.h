#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.generated.h"


USTRUCT()
struct PROCEDURAL_API FVoxelChunkSaveStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		FIntVector Position;

	UPROPERTY(EditAnywhere, Category = Voxel)
		TArray<int> Values;

	UPROPERTY(EditAnywhere, Category = Voxel)
		TArray<FColor> Colors;

	FVoxelChunkSaveStruct() {}
	FVoxelChunkSaveStruct(FIntVector position, TArray<int> values, TArray<FColor> colors) : Position(position), Values(values), Colors(colors) {}
	FVoxelChunkSaveStruct(FIntVector position, TArray<signed char> values, TArray<FColor> colors) : Position(position), Colors(colors)
	{
		Values.SetNumUninitialized(values.Num());
		for (int i = 0; i < values.Num(); i++)
		{
			Values[i] = values[i];
		}
	}
};