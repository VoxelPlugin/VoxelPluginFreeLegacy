#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.generated.h"


USTRUCT(BlueprintType)
struct PROCEDURAL_API FVoxelChunkSaveStruct
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

	FVoxelChunkSaveStruct() {}
	FVoxelChunkSaveStruct(FIntVector Position, int Depth, TArray<int> Values, TArray<FColor> Colors) : Position(Position), Depth(Depth), Values(Values), Colors(Colors) {}
	FVoxelChunkSaveStruct(FIntVector Position, int Depth, TArray<signed char> Values, TArray<FColor> Colors) : Position(Position), Depth(Depth), Colors(Colors)
	{
		this->Values.SetNumUninitialized(Values.Num());
		for (int i = 0; i < Values.Num(); i++)
		{
			this->Values[i] = Values[i];
		}
	}
};