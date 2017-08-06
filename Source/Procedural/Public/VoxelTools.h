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
		static void SetValueSphere(AVoxelWorld* world, FVector position, float radius, bool add, bool bQueueUpdate = true, bool bApplyUpdates = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		static void SetColorSphere(AVoxelWorld* world, FVector position, float radius, FLinearColor color, bool bQueueUpdate = true, bool bApplyUpdates = true);
};
