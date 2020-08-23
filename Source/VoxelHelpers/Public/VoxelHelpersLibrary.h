// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelHelpersLibrary.generated.h"

class UProceduralMeshComponent;

UCLASS()
class VOXELHELPERS_API UVoxelHelpersLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel Helpers")
	static void CreateProcMeshPlane(UProceduralMeshComponent* Mesh, int32 SizeX = 512, int32 SizeY = 512, float Step = 100);
};
