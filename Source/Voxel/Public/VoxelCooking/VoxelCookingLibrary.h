// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelCookedData.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelData/VoxelSave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelCookingLibrary.generated.h"

class AVoxelWorld;

USTRUCT(BlueprintType)
struct FVoxelCookingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 ThreadCount = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 RenderOctreeDepth = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float VoxelSize = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelGeneratorPicker Generator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", AdvancedDisplay)
	bool bLogProgress = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", AdvancedDisplay)
	bool bFastCollisionCook = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", AdvancedDisplay)
	bool bCleanCollisionMesh = false;
};

UCLASS()
class VOXEL_API UVoxelCookingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FVoxelCookedData CookVoxelDataImpl(const FVoxelCookingSettings& Settings, const FVoxelUncompressedWorldSaveImpl* Save = nullptr);

	// Cook collision meshes and save the result to VoxelCookedData
	// Can then be loaded using LoadCookedVoxelData
	// Useful for servers
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cooking")
	static FVoxelCookedData CookVoxelData(FVoxelCookingSettings Settings)
	{
		return CookVoxelDataImpl(Settings, nullptr);
	}
	// Cook collision meshes and save the result to VoxelCookedData
	// Can then be loaded using LoadCookedVoxelData
	// Useful for servers
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cooking")
	static FVoxelCookedData CookVoxelDataWithSave(FVoxelCookingSettings Settings, FVoxelUncompressedWorldSave Save)
	{
		return CookVoxelDataImpl(Settings, &Save.Const());
	}
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Cooking", meta = (DefaultToSelf = "World"))
	static FVoxelCookingSettings MakeVoxelCookingSettingsFromVoxelWorld(AVoxelWorld* World, int32 ThreadCount = 2);

public:
	// Loads collision cooked with CookVoxelData
	// The voxel world must not be created: it won't ever be created, collision meshes will be loaded directly
	// Note: Only the voxel world collision settings will be applied
	// Useful for servers
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cooking", meta = (DefaultToSelf = "World"))
	static void LoadCookedVoxelData(FVoxelCookedData CookedData, AVoxelWorld* World);
};
