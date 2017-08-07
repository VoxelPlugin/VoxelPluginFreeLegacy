// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;

/**
 *
 */
UCLASS()
class PROCEDURAL_API UVoxelTools : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);

	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);



	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, int Stength, bool bAdd, float Height = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetColorProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, FLinearColor Color, float Height = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
};
