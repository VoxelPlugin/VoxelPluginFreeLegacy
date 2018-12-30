// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class UMaterialFunction;
class UPackage;
class UVoxelMaterialCollection;

class FVoxelMaterialCollectionHelpers
{
public:
	static bool GenerateSingleMaterials(UVoxelMaterialCollection* Collection, FString& OutError);
	static bool GenerateDoubleMaterials(UVoxelMaterialCollection* Collection, FString& OutError);
	static bool GenerateTripleMaterials(UVoxelMaterialCollection* Collection, FString& OutError);
};