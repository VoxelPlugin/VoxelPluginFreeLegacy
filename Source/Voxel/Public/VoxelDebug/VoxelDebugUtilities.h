// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelDebugUtilities.generated.h"

class FVoxelData;
class AVoxelWorld;
class AVoxelWorldInterface;
class IVoxelWorldInterface;
class UVoxelLineBatchComponent;

UCLASS()
class VOXEL_API UVoxelDebugUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug", meta = (DefaultToSelf = "World", AdvancedDisplay = "Transform"))
	static void DrawDebugIntBox(
		AVoxelWorld* World, 
		FVoxelIntBox Bounds,
		FTransform Transform, 
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	static void DrawDebugIntBox(
		const AVoxelWorldInterface* World,
		FVoxelIntBox Box,
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	static void DrawDebugIntBox(
		const IVoxelWorldInterface& World,
		UVoxelLineBatchComponent& LineBatchComponent,
		FTransform Transform,
		FVoxelIntBox Box,
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Debug", meta = (DefaultToSelf = "World"))
	static void DebugVoxelsInsideBounds(
		AVoxelWorld* World,
		FVoxelIntBox Bounds, 
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