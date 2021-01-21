// Copyright 2021 Phyronnaz

#include "VoxelFoliageModule.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelSystemUtilities.h"

#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

void FVoxelFoliageModule::StartupModule()
{
	IPlugin& Plugin = FVoxelSystemUtilities::GetPlugin();

	const FString PluginBaseDir = FPaths::ConvertRelativePathToFull(Plugin.GetBaseDir());

}

IMPLEMENT_MODULE(FVoxelFoliageModule, VoxelFoliage)