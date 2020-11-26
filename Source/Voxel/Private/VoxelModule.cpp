// Copyright 2020 Phyronnaz

#include "VoxelModule.h"

#include "VoxelMessages.h"
#include "VoxelTests.h"
#include "VoxelThreadPool.h"
#include "VoxelStartupPopup.h"
#include "VoxelDebug/VoxelDebugManager.h"

#include "Interfaces/IPluginManager.h"
#include "ShaderCore.h"
#include "Misc/PackageName.h"
#include "Misc/MessageDialog.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

void FVoxelModule::StartupModule()
{
	LOG_VOXEL(Log, TEXT("VOXEL_DEBUG=%d"), VOXEL_DEBUG);
	
	if (VOXEL_DEBUG || !UE_BUILD_SHIPPING)
	{
		FVoxelTests::Test();
	}

	check(!GVoxelThreadPool);
	GVoxelThreadPool = new FVoxelThreadPool();

	check(!GVoxelDebugManager);
	GVoxelDebugManager = new FVoxelGlobalDebugManager();

	FVoxelStartupPopup::OnModuleStartup();
	
	ApplyCVarSettingsFromIni(TEXT("/Script/Voxel.VoxelSettings"), *GEngineIni, ECVF_SetByProjectSetting);
	
	{
		const auto Plugin = IPluginManager::Get().FindPlugin(VOXEL_PLUGIN_NAME);

		// This is needed to correctly share content across Pro and Free
		FPackageName::UnRegisterMountPoint(TEXT("/") VOXEL_PLUGIN_NAME TEXT("/"), Plugin->GetContentDir());
		FPackageName::RegisterMountPoint("/Voxel/", Plugin->GetContentDir());

		const FString PluginBaseDir = Plugin.IsValid() ? FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()) : "";

		const FString PluginShaderDir = FPaths::Combine(PluginBaseDir, TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/Voxel"), PluginShaderDir);

	}
}

void FVoxelModule::ShutdownModule()
{
	check(GVoxelThreadPool);
	delete GVoxelThreadPool;
	GVoxelThreadPool = nullptr;

	check(GVoxelDebugManager);
	delete GVoxelDebugManager;
	GVoxelDebugManager = nullptr;
}

IMPLEMENT_MODULE(FVoxelModule, Voxel)