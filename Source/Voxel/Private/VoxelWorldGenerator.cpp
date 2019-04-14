// Copyright 2019 Phyronnaz

#include "VoxelWorldGenerator.h"

void FVoxelWorldGeneratorInstance::GetValueAndMaterialInternal(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const
{
	GetValuesAndMaterials(OutValue, OutMaterial, FVoxelWorldGeneratorQueryZone(X, Y, Z), QueryLOD, ItemHolder);
}

void FVoxelWorldGeneratorInstance::ApplyVoxelPlaceableItems(const FVoxelPlaceableItemHolder& ItemHolder, const int32 X, const int32 Y, const int32 Z, FVoxelValue& Value, FVoxelMaterial& Material)
{
}