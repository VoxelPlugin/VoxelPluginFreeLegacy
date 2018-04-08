// Copyright 2018 Phyronnaz

#include "VoxelModule.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "VoxelPrivate.h"

class FVoxel : public IVoxel
{
public:
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_MODULE(FVoxel, Voxel)
DEFINE_LOG_CATEGORY(LogVoxel);