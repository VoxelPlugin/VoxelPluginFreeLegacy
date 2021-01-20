// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunks/VoxelChunkGeneration.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelChunkGenerator.generated.h"

UCLASS(Blueprintable)
class VOXEL_API UVoxelChunkGenerator : public UVoxelGenerator
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	//~ End UVoxelGenerator Interface
};

class VOXEL_API FVoxelChunkGeneratorInstance : public TVoxelGeneratorInstanceHelper<FVoxelChunkGeneratorInstance, UVoxelChunkGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelChunkGeneratorInstance, UVoxelChunkGenerator>;
	
	TVoxelSharedPtr<FVoxelChunksData> Data;
	
	explicit FVoxelChunkGeneratorInstance(const UVoxelChunkGenerator& Generator)
		: Super(&Generator)
	{
	}

	//~ Begin FVoxelGeneratorInstance Interface
	virtual void Init(const FVoxelGeneratorInit& InitStruct) override;

	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;

	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;

	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelGeneratorInstance Interface

private:
	template<typename T>
	T GetImpl(v_flt X, v_flt Y, v_flt Z, T BelowValue, T AboveValue) const;
};