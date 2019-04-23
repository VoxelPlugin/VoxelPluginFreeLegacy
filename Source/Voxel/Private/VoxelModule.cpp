// Copyright 2019 Phyronnaz

#include "VoxelModule.h"
#include "VoxelGlobals.h"

#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"

#if USE_STEAM
THIRD_PARTY_INCLUDES_START
#include "steam/isteamnetworking.h"
#include "steam/steam_api.h"
#include "steam/isteamgameserver.h"
THIRD_PARTY_INCLUDES_END

#define STEAM_SDK_ROOT_PATH TEXT("Binaries/ThirdParty/Steamworks")

static FString GetSteamModulePath()
{
#if PLATFORM_WINDOWS
	#if PLATFORM_64BITS
		return FPaths::EngineDir() / STEAM_SDK_ROOT_PATH / STEAM_SDK_VER_PATH / TEXT("Win64/");
	#else
		return FPaths::EngineDir() / STEAM_SDK_ROOT_PATH / STEAM_SDK_VER_PATH / TEXT("Win32/");
	#endif	//PLATFORM_64BITS
#elif PLATFORM_LINUX
	#if PLATFORM_64BITS
		return FPaths::EngineDir() / STEAM_SDK_ROOT_PATH / STEAM_SDK_VER_PATH / TEXT("x86_64-unknown-linux-gnu/");
	#else
		return FPaths::EngineDir() / STEAM_SDK_ROOT_PATH / STEAM_SDK_VER_PATH / TEXT("i686-unknown-linux-gnu/");
	#endif	//PLATFORM_64BITS
#else
	return FString();
#endif	//PLATFORM_WINDOWS
}

void FVoxelModule::LoadSteamModules()
{
	UE_LOG(LogVoxel, Log, TEXT("Loading Steam SDK %s"), STEAM_SDK_VER);

#if PLATFORM_WINDOWS

#if PLATFORM_64BITS
	FString Suffix("64");
#else
	FString Suffix;
#endif

	FString RootSteamPath = GetSteamModulePath();
	FPlatformProcess::PushDllDirectory(*RootSteamPath);
	SteamDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + "steam_api" + Suffix + ".dll"));
	if (IsRunningDedicatedServer() && FCommandLine::IsInitialized() && FParse::Param(FCommandLine::Get(), TEXT("force_steamclient_link")))
	{
		UE_LOG(LogVoxel, Log, TEXT("Force linking the steam client dlls."));
		bForceLoadSteamClientDll = true;
		SteamServerDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + "steamclient" + Suffix + ".dll"));
	}
	FPlatformProcess::PopDllDirectory(*RootSteamPath);
#elif PLATFORM_MAC
	SteamDLLHandle = FPlatformProcess::GetDllHandle(TEXT("libsteam_api.dylib"));
#elif PLATFORM_LINUX

#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
	UE_LOG(LogVoxel, Log, TEXT("Loading system libsteam_api.so."));
	SteamDLLHandle = FPlatformProcess::GetDllHandle(TEXT("libsteam_api.so"));
	if (SteamDLLHandle == nullptr)
	{
		// try bundled one
		UE_LOG(LogVoxel, Warning, TEXT("Could not find system one, loading bundled libsteam_api.so."));
		FString RootSteamPath = GetSteamModulePath();
		SteamDLLHandle = FPlatformProcess::GetDllHandle(*(RootSteamPath + "libsteam_api.so"));
	}

	if (SteamDLLHandle)
	{
		UE_LOG(LogVoxel, Display, TEXT("Loaded libsteam_api.so at %p"), SteamDLLHandle);
	}
	else
	{
		UE_LOG(LogVoxel, Warning, TEXT("Unable to load libsteam_api.so, Steam functionality will not work"));
	}
#else
	UE_LOG_ONLINE(Log, TEXT("libsteam_api.so is linked explicitly and should be already loaded."));
#endif // LOADING_STEAM_LIBRARIES_DYNAMICALLY

#endif	//PLATFORM_WINDOWS
}

void FVoxelModule::UnloadSteamModules()
{
#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
	if (SteamDLLHandle != NULL)
	{
		FPlatformProcess::FreeDllHandle(SteamDLLHandle);
		SteamDLLHandle = NULL;
	}

	if (SteamServerDLLHandle != NULL)
	{
		FPlatformProcess::FreeDllHandle(SteamServerDLLHandle);
		SteamServerDLLHandle = NULL;
	}
#endif	//LOADING_STEAM_LIBRARIES_DYNAMICALLY
}

bool FVoxelModule::AreSteamDllsLoaded() const
{
	bool bLoadedClientDll = true;
	bool bLoadedServerDll = true;

#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
	bLoadedClientDll = (SteamDLLHandle != NULL) ? true : false;
	#if LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY
	bLoadedServerDll = IsRunningDedicatedServer() ? ((SteamServerDLLHandle != NULL || !bForceLoadSteamClientDll) ? true : false) : true;
	#endif //LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY
#endif // LOADING_STEAM_LIBRARIES_DYNAMICALLY

	return bLoadedClientDll && bLoadedServerDll;
}
#endif // USE_STEAM

void FVoxelModule::StartupModule()
{
#if USE_STEAM
	LoadSteamModules();
	if (AreSteamDllsLoaded())
	{
		bSteamworksClientInitialized = SteamAPI_Init();

		// Test all the Steam interfaces
#define GET_STEAMWORKS_INTERFACE(Interface) \
		if (Interface() == nullptr) \
		{ \
			UE_LOG(LogVoxel, Warning, TEXT("Steamworks: %s() failed!"), TEXT(#Interface)); \
			bSteamworksClientInitialized = false; \
		} \

		// GSteamUtils
		GET_STEAMWORKS_INTERFACE(SteamUtils);
		// GSteamUser
		GET_STEAMWORKS_INTERFACE(SteamUser);
		// GSteamFriends
		GET_STEAMWORKS_INTERFACE(SteamFriends);
		// GSteamRemoteStorage
		GET_STEAMWORKS_INTERFACE(SteamRemoteStorage);
		// GSteamUserStats
		GET_STEAMWORKS_INTERFACE(SteamUserStats);
		// GSteamMatchmakingServers
		GET_STEAMWORKS_INTERFACE(SteamMatchmakingServers);
		// GSteamApps
		GET_STEAMWORKS_INTERFACE(SteamApps);
		// GSteamNetworking
		GET_STEAMWORKS_INTERFACE(SteamNetworking);
		// GSteamMatchmaking
		GET_STEAMWORKS_INTERFACE(SteamMatchmaking);

#undef GET_STEAMWORKS_INTERFACE

		if (bSteamworksClientInitialized)
		{
			UE_LOG(LogVoxel, Log, TEXT("Steamworks initialization successful"));
		}
		else
		{
			UE_LOG(LogVoxel, Warning, TEXT("Steamworks initialization failed! Make sure you have enabled the Online Steam Subsystem"));
		}
	}
	else
	{
		UE_LOG(LogVoxel, Warning, TEXT("Steam SDK %s libraries not present at %s or failed to load!"), STEAM_SDK_VER, *GetSteamModulePath());
		UnloadSteamModules();
	}
#endif
}

void FVoxelModule::ShutdownModule()
{
#if USE_STEAM
	UnloadSteamModules();
#endif
}

FVoxelModule* GetVoxelModule()
{
	FModuleManager& ModuleManager = FModuleManager::Get();
	if (!ModuleManager.IsModuleLoaded("Voxel"))
	{
		// Attempt to load the module
		ModuleManager.LoadModule("Voxel");
	}
	auto* Module = ModuleManager.GetModule("Voxel");
	check(Module);

	return static_cast<FVoxelModule*>(Module);
}

bool FVoxelModule::IsSteamLoaded()
{
#if USE_STEAM	
	return GetVoxelModule()->AreSteamDllsLoaded();
#else
	return false;
#endif
}

bool FVoxelModule::IsSteamInitialized()
{
#if USE_STEAM	
	return GetVoxelModule()->bSteamworksClientInitialized;
#else
	return false;
#endif
}


IMPLEMENT_MODULE(FVoxelModule, Voxel)