// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

#define LOADING_STEAM_CLIENT_LIBRARY_DYNAMICALLY				(PLATFORM_WINDOWS || PLATFORM_MAC || (PLATFORM_LINUX && !IS_MONOLITHIC))
#define LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY				((PLATFORM_WINDOWS && PLATFORM_32BITS) || (PLATFORM_LINUX && !IS_MONOLITHIC) || PLATFORM_MAC)
#define LOADING_STEAM_LIBRARIES_DYNAMICALLY						(LOADING_STEAM_CLIENT_LIBRARY_DYNAMICALLY || LOADING_STEAM_SERVER_LIBRARY_DYNAMICALLY)

class VOXEL_API FVoxelModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	static bool IsSteamLoaded();
	static bool IsSteamInitialized();

private:
#if USE_STEAM
#if LOADING_STEAM_LIBRARIES_DYNAMICALLY
	/** Handle to the STEAM API dll */
	void* SteamDLLHandle;
	/** Handle to the STEAM dedicated server support dlls */
	void* SteamServerDLLHandle;
#endif	//LOADING_STEAM_LIBRARIES_DYNAMICALLY

	/** If we force loaded the steamclient dlls due to launch flags */
	bool bForceLoadSteamClientDll;

	bool bSteamworksClientInitialized = false;

	/**
	 *	Load the required modules for Steam
	 */
	void LoadSteamModules();

	/**
	 *	Unload the required modules for Steam
	 */
	void UnloadSteamModules();
	
	/**
	 * Are the Steam support Dlls loaded
	 */
	bool AreSteamDllsLoaded() const;
#endif // USE_STEAM
};
