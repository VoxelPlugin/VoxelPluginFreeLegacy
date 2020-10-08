// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"

class FVoxelDataAssetInstance : public TVoxelGeneratorInstanceHelper<FVoxelDataAssetInstance, UVoxelDataAsset>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelDataAssetInstance, UVoxelDataAsset>;
	
	const TVoxelSharedRef<const FVoxelDataAssetData> Data;
	const bool bSubtractiveAsset;
	const FIntVector PositionOffset;
	float Tolerance = 0.f;

public:
	FVoxelDataAssetInstance(UVoxelDataAsset& Asset)
		: Super(&Asset)
		, Data(Asset.GetData())
		, bSubtractiveAsset(Asset.bSubtractiveAsset)
		, PositionOffset(Asset.PositionOffset)
		, Tolerance(Asset.Tolerance)
	{
	}

	//~ Begin FVoxelGeneratorInstance Interface
	virtual void Init(const FVoxelGeneratorInit& InitStruct) override
	{
		if (InitStruct.RenderType == EVoxelRenderType::Cubic)
		{
			Tolerance = FMath::Max(Tolerance, 0.1f);
		}
	}
	
	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		X -= PositionOffset.X;
		Y -= PositionOffset.Y;
		Z -= PositionOffset.Z;
		
		return Data->GetInterpolatedValue(X, Y, Z, bSubtractiveAsset ? FVoxelValue::Full() : FVoxelValue::Empty(), Tolerance);
	}
	
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		X -= PositionOffset.X;
		Y -= PositionOffset.Y;
		Z -= PositionOffset.Z;
		
		if (Data->HasMaterials())
		{
			return Data->GetInterpolatedMaterial(X, Y, Z, Tolerance);
		}
		else
		{
			return FVoxelMaterial::Default();
		}
	}
	
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Bounds.Intersect(GetLocalBounds()))
		{
			return { -1, 1 };
		}
		else
		{
			return bSubtractiveAsset ? -1 : 1;
		}
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelGeneratorInstance Interface

private:
	FORCEINLINE FVoxelIntBox GetLocalBounds() const
	{
		return FVoxelIntBox(PositionOffset, PositionOffset + Data->GetSize());
	}
};