// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelFlatWorldGenerator.generated.h"

class UVoxelFlatWorldGenerator;

class VOXEL_API FVoxelFlatWorldGeneratorInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelFlatWorldGeneratorInstance, UVoxelFlatWorldGenerator>
{
public:
	const FVoxelMaterial Material;
	
	explicit FVoxelFlatWorldGeneratorInstance(const FVoxelMaterial& Material = FVoxelMaterial::Default())
		: Material(Material)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Z + 0.001f; // Try to avoid having 0 as density, as it behaves weirdly
	}
	inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Material;
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FLinearColor Color = FLinearColor::Transparent;
		
	//~ Begin UVoxelWorldGenerator Interface
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override
	{
		return MakeVoxelShared<FVoxelFlatWorldGeneratorInstance>(FVoxelMaterial::CreateFromColor(Color));
	}
	//~ End UVoxelWorldGenerator Interface
};