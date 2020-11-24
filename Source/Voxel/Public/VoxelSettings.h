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

public:
	// Number of threads allocated for the voxel background processing. Setting it too high may impact performance
	// The threads are shared across all voxel worlds
	// Can be set using voxel.threading.NumThreads
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = 1, ConsoleVariable = "voxel.threading.NumThreads"))
	int32 NumberOfThreads;

	// Only used if ConstantPriorities is false
	// Time, in seconds, during which a task priority is valid and does not need to be recomputed
	// Lowering this will increase async cost to recompute priorities, but will lead to more precise scheduling
	// Increasing this will decreasing async cost to recompute priorities, but might lead to imprecise scheduling if the invokers are moving fast
	// Can be set using voxel.threading.PriorityDuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (ClampMin = 0, ConsoleVariable = "voxel.threading.PriorityDuration", EditCondition = "!bConstantPriorities"))
	float PriorityDuration;

public:
    UPROPERTY(Config, EditAnywhere, Category="Config")
    bool bShowNotifications = true;

	UPROPERTY(Config, EditAnywhere, Category="Config")
	bool bDisableAutoPreview = false;

	// Round voxels that do not affect surface nor normals to improve compression
	// Takes a while when saving
	UPROPERTY(Config, EditAnywhere, Category="Config")
	bool bRoundBeforeSaving = false;

	// -1 = ZLib default compression
	// 0 = No compression
	// 1 = Best speed
	// 9 = Best compression
	// Used when compressing voxel save, heightmaps, data assets...
	// Compression speed is written to the log
	// In my tests a compression level of 1 was very fast without compromising too much compression
	UPROPERTY(Config, EditAnywhere, Category="Compression", meta = (ClampMin = -1, ClampMax = 9, UIMin = -1, UIMax = 9))
    int32 DefaultCompressionLevel = 1;
	
    virtual FName GetContainerName() const override;
    virtual void PostInitProperties() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};