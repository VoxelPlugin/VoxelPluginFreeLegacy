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

	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, float FadeDistance = 3, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);



	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, int Stength, bool bAdd, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetColorProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, FLinearColor Color, float FadeDistance = 3, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);

	// Does not work
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SmoothValue(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, float Speed = 0.01f, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
};
