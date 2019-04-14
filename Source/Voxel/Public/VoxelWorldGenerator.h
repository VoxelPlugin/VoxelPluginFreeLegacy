// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "IntBox.h"
#include "VoxelQueryZone.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelWorldGenerator.generated.h"

struct FVoxelWorldGeneratorInit
{
	TMap<FString, int32> Seeds;
	float VoxelSize = 100;

	FVoxelWorldGeneratorInit() {}

	FVoxelWorldGeneratorInit(const TMap<FString, int32>& Seeds, float VoxelSize)
		: Seeds(Seeds)
		, VoxelSize(VoxelSize)
	{
	}
};

enum class EVoxelEmptyState : uint8
{
	Unknown,
	AllFull,
	AllEmpty
};

/**
 * A FVoxelWorldGeneratorInstance is a constant object created by a UVoxelWorldGenerator
 */
class VOXEL_API FVoxelWorldGeneratorInstance
{		
public:
	const TMap<FName, uint8> SingleFloatOutputsNames;
	const TMap<FName, uint8> SingleIntOutputsNames;

	FVoxelWorldGeneratorInstance(
		const TMap<FName, uint8>& SingleFloatOutputsNames = {},
		const TMap<FName, uint8>& SingleIntOutputsNames = {})
		: SingleFloatOutputsNames(SingleFloatOutputsNames)
		, SingleIntOutputsNames(SingleIntOutputsNames)
	{
	}
	virtual ~FVoxelWorldGeneratorInstance() = default;

	//~ Begin FVoxelWorldGeneratorInstance Interface
	// Initialization
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) {}
	
	// Defaults to GetValuesAndMaterials, should override with more optimized stuff if you can
	virtual void GetValueAndMaterialInternal(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const;
	// Main query function
	virtual void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const = 0;

	virtual bool HasCache2D() const { return false; }
	virtual void Cache2D(float Values[], const FVoxelWorldGeneratorQueryZone2D& QueryZone) const {}
	virtual void GetValuesAndMaterialsCache2D(FVoxelValue Values[], FVoxelMaterial Materials[], const float Cache2DValues[], const FVoxelWorldGeneratorQueryZone& QueryZone, const FVoxelPlaceableItemHolder& ItemHolder) const {}

	// World up vector at position (must be normalized). Used for spawners
	virtual FVector GetUpVector(int32 X, int32 Y, int32 Z) const { return FVector::UpVector; }
	inline FVector GetUpVector(const FIntVector& P) const { return GetUpVector(P.X, P.Y, P.Z); }

	// If true, GetValuesAndMaterials(Start, Step, Size) MUST be empty, else it MIGHT be empty
	virtual EVoxelEmptyState IsEmpty(const FIntBox& Bounds, int32 QueryLOD) const { return EVoxelEmptyState::Unknown; }

	// Custom outputs
	virtual float GetFloatOutput(int32 X, int32 Y, int32 Z, uint8 Index) const { return 0; }
	virtual int32 GetIntOutput(int32 X, int32 Y, int32 Z, uint8 Index) const { return 0; }
	//~ End FVoxelWorldGeneratorInstance Interface
	
public:
	/** Get the value and material (X, Y, Z) */
	inline void GetValueAndMaterial(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const
	{
		GetValueAndMaterialInternal(X, Y, Z, OutValue, OutMaterial, QueryLOD, ItemHolder);
		if (!ItemHolder.IsEmpty())
		{
			FVoxelValue TmpValue;
			FVoxelMaterial TmpMaterial;
			ApplyVoxelPlaceableItems(ItemHolder, X, Y, Z, OutValue ? *OutValue : TmpValue, OutMaterial ? *OutMaterial : TmpMaterial);
		}
	}

protected:
	inline static void SetValueAndMaterial(FVoxelValue* Values, FVoxelMaterial* Materials, const FVoxelWorldGeneratorQueryZone& QueryZone, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder, int32 X, int32 Y, int32 Z, FVoxelValue Value, FVoxelMaterial Material)
	{
		if (!ItemHolder.IsEmpty())
		{
			ApplyVoxelPlaceableItems(ItemHolder, X, Y, Z, Value, Material);
		}
		int32 Index = QueryZone.GetIndex(X, Y, Z);
		if (Values)
		{
			Values[Index] = Value;
		}
		if (Materials)
		{
			Materials[Index] = Material;
		}
	}

private:
	static void ApplyVoxelPlaceableItems(const FVoxelPlaceableItemHolder& ItemHolder, int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material);
};

/**
 * A UVoxelWorldGenerator is used to create a FVoxelWorldGeneratorInstance
 */
UCLASS(Blueprintable, abstract)
class VOXEL_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGenerator Interface
	virtual TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator()
	{
		unimplemented();
		return TSharedPtr<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe>().ToSharedRef();
	}
	//~ End UVoxelWorldGenerator Interface
};