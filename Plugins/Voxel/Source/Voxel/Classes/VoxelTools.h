// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;

/**
 *
 */
UCLASS()
class VOXEL_API UVoxelTools : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Set value to positive or negative in a sphere
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the sphere
	 * @param	bAdd			Add or remove?
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);

	/**
	 * Set color in a sphere
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the sphere
	 * @param	Color			Color to set
	 * @param	FadeDistance	Size of external band where color is interpolated with existing one
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, float FadeDistance = 3, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);


	/**
	 * Set value to positive or negative in a cone
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the cone
	 * @param	Height			Height of the cone
	 * @param	bAdd			Add or remove?
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetValueCone(AVoxelWorld* World, FVector Position, float Radius, float Height, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false);


	/**
	 * Add or remove continuously
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Normal			Normal in world space at hit point
	 * @param	Radius			
	 * @param	Strength		Speed of modification
	 * @param	bAdd			Add or remove?
	 * @param	MaxDistance		Max distance to modify
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, int Stength, bool bAdd, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);

	/**
	 * Set color on surface
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Normal			Normal in world space at hit point
	 * @param	Radius
	 * @param	FadeDistance	Size of external band where color is interpolated with existing one
	 * @param	MaxDistance		Max distance to modify
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetColorProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, FLinearColor Color, float FadeDistance = 3, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);

	// Does not work
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SmoothValue(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, float Speed = 0.01f, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
};
