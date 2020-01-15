// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelFlatWorldGenerator.generated.h"

class UVoxelFlatWorldGenerator;

class VOXEL_API FVoxelFlatWorldGeneratorInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelFlatWorldGeneratorInstance, UVoxelFlatWorldGenerator>
{
public:
	FVoxelFlatWorldGeneratorInstance() = default;

	//~ Begin FVoxelWorldGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Z + 0.001f; // Try to avoid having 0 as density, as it behaves weirdly
	}
	inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return FVoxelMaterial::Default();
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z);
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface
};

/**
 * Flat world
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGenerator Interface
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override
	{
		return MakeVoxelShared<FVoxelFlatWorldGeneratorInstance>();
	}
	//~ End UVoxelWorldGenerator Interface
};