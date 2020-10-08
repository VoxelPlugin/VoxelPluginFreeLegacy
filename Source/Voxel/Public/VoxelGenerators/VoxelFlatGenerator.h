// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelUtilities/VoxelDataItemUtilities.h"
#include "VoxelFlatGenerator.generated.h"

class UVoxelFlatGenerator;

USTRUCT(BlueprintType)
struct FVoxelFlatGeneratorDataItemConfig
{
	GENERATED_BODY()

	// In voxels, how smooth the intersection with the existing terrain and these items should be
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float Smoothness = 10;

	// Only items matching this mask will be added
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (Bitmask, BitmaskEnum = EVoxel32BitMask))
	int32 Mask = 0;

	// If true, will subtract the items from the world and will invert their values
	// If false, will add the items to the world and will not invert their values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bSubtractItems = false;
};

/**
 * Flat world
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelFlatGenerator : public UVoxelGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FLinearColor Color = FLinearColor::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<FVoxelFlatGeneratorDataItemConfig> DataItemConfigs = 
	{
		{
			2,
			1 << 0,
			true
		},
		{
			2,
			1 << 1,
			false
		},
	};
		
	//~ Begin UVoxelGenerator Interface
	TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	//~ End UVoxelGenerator Interface
};

class FVoxelFlatGeneratorInstance : public TVoxelGeneratorInstanceHelper<FVoxelFlatGeneratorInstance, UVoxelFlatGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelFlatGeneratorInstance, UVoxelFlatGenerator>;
	
	const FVoxelMaterial Material;
	const TArray<FVoxelFlatGeneratorDataItemConfig> DataItemConfigs;
	
	explicit FVoxelFlatGeneratorInstance(UVoxelFlatGenerator& Object)
		: Super(&Object)
		, Material(FVoxelMaterial::CreateFromColor(Object.Color))
		, DataItemConfigs(Object.DataItemConfigs)
	{
	}

	//~ Begin FVoxelGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		v_flt Density = Z + 0.001f; // Try to avoid having 0 as density, as it behaves weirdly

		if (Items.ItemHolder.GetDataItems().Num() > 0)
		{
			for (auto& DataItemConfig : DataItemConfigs)
			{
				if (DataItemConfig.bSubtractItems)
				{
					Density = FVoxelUtilities::CombineDataItemDistance<true>(Density, Items.ItemHolder, X, Y, Z, DataItemConfig.Smoothness, DataItemConfig.Mask, EVoxelDataItemCombineMode::Max);
				}
				else
				{
					Density = FVoxelUtilities::CombineDataItemDistance<false>(Density, Items.ItemHolder, X, Y, Z, DataItemConfig.Smoothness, DataItemConfig.Mask, EVoxelDataItemCombineMode::Min);
				}
			}
		}
		
		return Density;
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		const auto X = TVoxelRange<v_flt>(Bounds.Min.X, Bounds.Max.X);
		const auto Y = TVoxelRange<v_flt>(Bounds.Min.Y, Bounds.Max.Y);
		const auto Z = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z);

		auto Density = Z + 0.001f;

		if (Items.ItemHolder.GetDataItems().Num() > 0)
		{
			for (auto& DataItemConfig : DataItemConfigs)
			{
				if (DataItemConfig.bSubtractItems)
				{
					Density = FVoxelUtilities::CombineDataItemDistanceRange<true>(Density, Items.ItemHolder, X, Y, Z, DataItemConfig.Smoothness, DataItemConfig.Mask, EVoxelDataItemCombineMode::Max);
				}
				else
				{
					Density = FVoxelUtilities::CombineDataItemDistanceRange<false>(Density, Items.ItemHolder, X, Y, Z, DataItemConfig.Smoothness, DataItemConfig.Mask, EVoxelDataItemCombineMode::Min);
				}
			}
		}

		return Density;
	}
	inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Material;
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelGeneratorInstance Interface
};

inline TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelFlatGenerator::GetInstance()
{
	return MakeVoxelShared<FVoxelFlatGeneratorInstance>(*this);
}