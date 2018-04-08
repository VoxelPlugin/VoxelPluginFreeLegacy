// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "EmptyWorldGenerator.generated.h"

/**
 * Empty World
 */
UCLASS(Blueprintable)
class VOXEL_API UEmptyWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()
	
public:
	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() override;
	//~ End UVoxelWorldGenerator Interface
};

class VOXEL_API FEmptyWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override;
	//~ End FVoxelWorldGeneratorInstance Interface
};