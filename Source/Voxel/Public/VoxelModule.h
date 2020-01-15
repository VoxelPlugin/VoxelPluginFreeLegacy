// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class VOXEL_API FVoxelModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
