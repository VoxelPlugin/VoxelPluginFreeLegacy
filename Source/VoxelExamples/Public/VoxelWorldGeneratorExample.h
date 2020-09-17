// Copyright 2020 Phyronnaz

#pragma once

#if 0

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGeneratorExample.generated.h"

UCLASS(Blueprintable)
class UVoxelWorldGeneratorExample : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Generator")
	float NoiseHeight = 10.f;

	//~ Begin UVoxelWorldGenerator Interface
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	//~ End UVoxelWorldGenerator Interface
};

class FVoxelWorldGeneratorExampleInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelWorldGeneratorExampleInstance, UVoxelWorldGeneratorExample>
{
public:
    using Super = TVoxelWorldGeneratorInstanceHelper<FVoxelWorldGeneratorExampleInstance, UVoxelWorldGeneratorExample>;

	explicit FVoxelWorldGeneratorExampleInstance(const UVoxelWorldGeneratorExample& MyGenerator);

	//~ Begin FVoxelWorldGeneratorInstance Interface
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) override;

	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;

	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;

	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final;
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const float NoiseHeight;
	FVoxelFastNoise Noise;
};

#endif