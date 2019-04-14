// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelWorldGenerator.h"
#include "VoxelSphereWorldGenerator.generated.h"

class VOXEL_API FVoxelSphereWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	FVoxelSphereWorldGeneratorInstance(float Radius, const FVoxelMaterial& Material, bool bInverseOutsideInside)
		: Radius(Radius)
		, Material(Material)
		, bInverseOutsideInside(bInverseOutsideInside)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const final
	{
		for (int32 X : QueryZone.XIt())
		{
			for (int32 Y : QueryZone.YIt())
			{
				for (int32 Z : QueryZone.ZIt())
				{
					SetValueAndMaterial(Values, Materials, QueryZone, QueryLOD, ItemHolder, X, Y, Z, GetSphereValue(X, Y, Z), Material);
				}
			}
		}
	}
	void GetValueAndMaterialInternal(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const final
	{
		if (OutValue)
		{
			*OutValue = GetSphereValue(X, Y, Z);
		}
		if (OutMaterial)
		{
			*OutMaterial = Material;
		}
	}
	FVector GetUpVector(int32 X, int32 Y, int32 Z) const final
	{
		return FVector(X, Y, Z).GetSafeNormal();
	}
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const float Radius;
	const FVoxelMaterial Material;
	const bool bInverseOutsideInside;

	inline FVoxelValue GetSphereValue(int32 X, int32 Y, int32 Z) const
	{
		// Distance to the center
		float Distance = FVector(X, Y, Z).Size();

		// Alpha = -1 inside the sphere, 1 outside and an interpolated value on intersection
		float Alpha = FMath::Clamp(Distance - Radius, -2.f, 2.f) / 2;

		return Alpha * (bInverseOutsideInside ? -1 : 1);
	}
};

/**
 * Sphere World
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelSphereWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	// Radius of the sphere in voxels
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Radius = 10;

	// Material of the sphere
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelMaterial Material;

	// If true, sphere is a hole in a full world
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool InverseOutsideInside = false;

	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator() override
	{
		return MakeShared<FVoxelSphereWorldGeneratorInstance, ESPMode::ThreadSafe>(Radius, Material, InverseOutsideInside);
	}
	//~ End UVoxelWorldGenerator Interface
};