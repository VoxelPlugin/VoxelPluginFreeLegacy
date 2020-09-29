// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelToolLibary.generated.h"

class UVoxelToolBase;
class UMaterialInstanceDynamic;

UCLASS()
class VOXEL_API UVoxelToolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel Tool Library")
	static void UpdateSphereOverlayMaterial(UVoxelToolBase* Tool, UMaterialInstanceDynamic* OverlayMaterialInstance, EVoxelFalloff FalloffType, float Falloff);
};