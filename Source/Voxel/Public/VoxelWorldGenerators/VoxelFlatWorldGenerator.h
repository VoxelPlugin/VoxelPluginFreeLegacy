// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "VoxelFlatWorldGenerator.generated.h"

class VOXEL_API FVoxelFlatWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	FVoxelFlatWorldGeneratorInstance() = default;

	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const final
	{
		for (int32 Z : QueryZone.ZIt())
		{
			FVoxelValue Value = Z >= 0 ? FVoxelValue::Empty : FVoxelValue::Full;

			for (int32 Y : QueryZone.YIt())
			{
				for (int32 X : QueryZone.XIt())
				{
					SetValueAndMaterial(Values, Materials, QueryZone, QueryLOD, ItemHolder, X, Y, Z, Value, FVoxelMaterial());
				}
			}
		}
	}
	void GetValueAndMaterialInternal(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const final
	{
		if (OutValue)
		{
			*OutValue = Z >= 0 ? FVoxelValue::Empty : FVoxelValue::Full;
		}
		if (OutMaterial)
		{
			*OutMaterial = FVoxelMaterial();
		}
	}
	EVoxelEmptyState IsEmpty(const FIntBox& Bounds, int32 LOD) const final
	{
		return (Bounds.Max.Z) <= 0 ? EVoxelEmptyState::AllFull : (0 <= Bounds.Min.Z) ? EVoxelEmptyState::AllEmpty : EVoxelEmptyState::Unknown;
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
	TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator() override
	{
		return MakeShared<FVoxelFlatWorldGeneratorInstance, ESPMode::ThreadSafe>();
	}
	//~ End UVoxelWorldGenerator Interface
};