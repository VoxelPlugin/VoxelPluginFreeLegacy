// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTexture.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelGraphBlueprintTools.generated.h"

class UVoxelWorldGenerator;

UCLASS()
class VOXELGRAPH_API UVoxelGraphBlueprintTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static bool SetVoxelGraphFloatParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static bool SetVoxelGraphIntParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static bool SetVoxelGraphBoolParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, bool Value);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static bool SetVoxelGraphColorParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FLinearColor Value);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static bool SetVoxelGraphVoxelTextureParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FVoxelFloatTexture Value);

	// You need to call this before any SetVoxelGraphParameter to make sure no parameter override is left from a previous session
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Graph")
	static void ClearVoxelGraphParametersOverrides(UVoxelWorldGenerator* WorldGenerator);
};