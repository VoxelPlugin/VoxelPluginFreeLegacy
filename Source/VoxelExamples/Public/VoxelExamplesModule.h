// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "VoxelExamplesModule.generated.h"

class FVoxelExamples : public IModuleInterface
{
};

// UBT doesn't like not having any UObject in a module
UCLASS()
class UVoxelExamplesModuleDummy : public UObject
{
	GENERATED_BODY()
};