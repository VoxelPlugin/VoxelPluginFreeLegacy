// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VoxelOpenAssetsOnStartup.generated.h"

class FUICommandList;

UCLASS(config=EditorPerProjectUserSettings, defaultconfig, meta=(DisplayName="Open Assets On Startup"))
class UVoxelOpenAssetsOnStartup : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	static void Init();

private:
	void ActualInit();
	
public:
	// If true, will add a Open Asset On Startup option to all assets context menus in the content browser
	// Assets marked as such will automatically open on engine startup
	// Useful to iterate quickly when restarting the editor
    UPROPERTY(Config, EditAnywhere, Category="Config")
    bool bEnableOpenAssetsOnStartup = false;

	// If true, will show a context menu option to change the project editor startup map in the context menu
    UPROPERTY(Config, EditAnywhere, Category="Config")
    bool bShowSetAsStartupMap = false;

	UPROPERTY()
	TMap<FName, bool> AssetsToOpenOnStartup;
};