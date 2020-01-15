// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VoxelSettings.generated.h"

/**
 * Usage example: In DefaultEngine.ini
 * [/Script/Voxel.VoxelSettings]
 * bDisableAutoPreview=True
 */

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="Voxel Plugin"))
class VOXEL_API UVoxelSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UVoxelSettings();

    UPROPERTY(Config, EditAnywhere, Category="Config")
    bool bShowNotifications = true;

	UPROPERTY(Config, EditAnywhere, Category="Config")
	bool bDisableAutoPreview = false;

	// Round voxels that do not affect surface nor normals to improve compression
	// Takes a while when saving
	UPROPERTY(Config, EditAnywhere, Category="Config")
	bool bRoundBeforeSaving = false;
	
    virtual FName GetContainerName() const override;
    virtual void PostInitProperties() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};