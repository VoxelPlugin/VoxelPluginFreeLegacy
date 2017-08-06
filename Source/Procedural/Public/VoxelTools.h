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
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		static void SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, bool bQueueUpdate = true, bool bApplyUpdates = true);
};
