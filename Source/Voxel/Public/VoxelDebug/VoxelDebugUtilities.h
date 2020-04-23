// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelConfigEnums.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelDebugUtilities.generated.h"

class FVoxelData;
class AVoxelWorld;
class AVoxelWorldInterface;

UCLASS()
class VOXEL_API UVoxelDebugUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug", meta = (DefaultToSelf = "World", AdvancedDisplay = "Transform"))
	static void DrawDebugIntBox(
		AVoxelWorld* World, 
		FIntBox Bounds,
		FTransform Transform, 
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	static void DrawDebugIntBox(
		const AVoxelWorldInterface* World,
		FIntBox Box,
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug", meta = (DefaultToSelf = "World"))
	static void DebugVoxelsInsideBounds(
		AVoxelWorld* World,
		FIntBox Bounds, 
		FLinearColor Color = FLinearColor::Red, 
		float Lifetime = 1, 
		float Thickness = 1, 
		bool bDebugDensities = true, 
		FLinearColor TextColor = FLinearColor::Black);

	struct FDrawDataOctreeSettings
	{
		AVoxelWorld* World = nullptr;
		float Lifetime = 0;
		bool bShowSingle = false;
		bool bShowCached = false;
		FColor SingleColor = FColor::Red;
		FColor SingleDirtyColor = FColor::Red;
		FColor CachedColor = FColor::Red;
		FColor DirtyColor = FColor::Red;

	};
	template<typename T>
	static void DrawDataOctreeImpl(const FVoxelData& Data, const FDrawDataOctreeSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug", meta = (DefaultToSelf = "World"))
	static void DrawDataOctree(
		AVoxelWorld* World,
		EVoxelDataType DataType,
		float Lifetime = 0,
		bool bShowSingle = false,
		bool bShowCached = false,
		FColor SingleColor = FColor::Red,
		FColor SingleDirtyColor = FColor::Red,
		FColor CachedColor = FColor::Red,
		FColor DirtyColor = FColor::Red);
};