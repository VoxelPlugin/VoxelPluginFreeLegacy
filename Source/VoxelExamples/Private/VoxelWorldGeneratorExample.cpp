// Copyright 2020 Phyronnaz

#if 0

#include "VoxelWorldGeneratorExample.h"
#include "FastNoise/VoxelFastNoise.inl"
#include "VoxelMaterialBuilder.h"

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelWorldGeneratorExample::GetInstance()
{
	return MakeVoxelShared<FVoxelWorldGeneratorExampleInstance>(*this);
}

///////////////////////////////////////////////////////////////////////////////

FVoxelWorldGeneratorExampleInstance::FVoxelWorldGeneratorExampleInstance(const UVoxelWorldGeneratorExample& MyGenerator)
	: Super(&MyGenerator)
	, NoiseHeight(MyGenerator.NoiseHeight)
{
}

void FVoxelWorldGeneratorExampleInstance::Init(const FVoxelWorldGeneratorInit& InitStruct)
{
	Noise.SetSeed(InitStruct.Seeds.Contains("MySeed") ? InitStruct.Seeds["MySeed"] : 1337);
}

v_flt FVoxelWorldGeneratorExampleInstance::GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	const float Height = Noise.GetPerlin_2D(X, Y, 0.01f) * NoiseHeight;
	
	// Positive value -> empty voxel
	// Negative value -> full voxel
	// Value positive when Z > Height, and negative Z < Height
	float Value = Z - Height;
	
	// The voxel value is clamped between -1 and 1. That can result in a bad gradient/normal. To solve that we divide it
	Value /= 5;

	return Value;
}

FVoxelMaterial FVoxelWorldGeneratorExampleInstance::GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	FVoxelMaterialBuilder Builder;

	// RGB
	Builder.SetMaterialConfig(EVoxelMaterialConfig::RGB);
	Builder.SetColor(FColor::Red);

	// Single index
	//Builder.SetMaterialConfig(EVoxelMaterialConfig::SingleIndex);
	//Builder.SetSingleIndex(0); 

	// Multi index
	//Builder.SetMaterialConfig(EVoxelMaterialConfig::MultiIndex);
	//Builder.AddMultiIndex(0, 0.5f);
	//Builder.AddMultiIndex(1, 0.5f);
	
	return Builder.Build();
}

TVoxelRange<v_flt> FVoxelWorldGeneratorExampleInstance::GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	// Return the values that GetValueImpl can return in Bounds
	// Used to skip chunks where the value does not change
	// Be careful, if wrong your world will have holes!
	// By default return infinite range to be safe
	return TVoxelRange<v_flt>::Infinite();

	// Example for the GetValueImpl above

	// Noise is between -1 and 1
	const TVoxelRange<v_flt> Height = TVoxelRange<v_flt>(-1, 1) * NoiseHeight;

	// Z can go from min to max
	TVoxelRange<v_flt> Value = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z) - Height;

	Value /= 5;

	return Value;
}

FVector FVoxelWorldGeneratorExampleInstance::GetUpVector(v_flt X, v_flt Y, v_flt Z) const
{
	// Used by spawners
	return FVector::UpVector;
}

#endif