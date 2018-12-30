// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "EmptyWorldGenerator.generated.h"

class VOXEL_API FEmptyWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	FEmptyWorldGeneratorInstance(TSharedPtr<const FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> WorldUpGenerator = nullptr)
		: WorldUpGenerator(WorldUpGenerator)
	{

	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterials(
		FVoxelValue Values[],
		FVoxelMaterial Materials[],
		const FVoxelWorldGeneratorQueryZone& QueryZone,
		int QueryLOD,
		const FVoxelPlaceableItemHolder& ItemHolder) const final
	{
		for (int X : QueryZone.XIt())
		{
			for (int Y : QueryZone.YIt())
			{
				for (int Z : QueryZone.ZIt())
				{
					SetValueAndMaterial(Values, Materials, QueryZone, QueryLOD, ItemHolder, X, Y, Z, FVoxelValue::Empty, FVoxelMaterial());
				}
			}
		}
	}
	EVoxelEmptyState IsEmpty(const FIntBox& Bounds, int LOD) const final
	{
		return EVoxelEmptyState::Unknown;
	}

	FVector GetUpVector(int X, int Y, int Z) const final
	{
		return WorldUpGenerator.IsValid() ? WorldUpGenerator->GetUpVector(X, Y, Z) : FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const TSharedPtr<const FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> WorldUpGenerator;
};

/**
 * Empty World
 */
UCLASS(Blueprintable)
class VOXEL_API UEmptyWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()
	
public:
	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator() override
	{
		return MakeShared<FEmptyWorldGeneratorInstance, ESPMode::ThreadSafe>();
	}
	//~ End UVoxelWorldGenerator Interface
};