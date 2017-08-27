// Copyright 2017 Phyronnaz

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
	 * @param	Radius			Radius of the sphere in voxel space
	 * @param	bAdd			Add or remove?
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 * @param	InsideValue		Value to set inside the sphere
	 * @param	OutsideValue	Value to set outside the sphere
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, float InsideValue = -1, float OutsideValue = 1);

	/**
	 * Set color in a sphere
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the sphere in voxel space
	 * @param	Color			Color to set
	 * @param	FadeDistance	Size, in world space, of external band where color is interpolated with existing one
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
	 * @param	Radius			Radius of the cone in voxel space
	 * @param	Height			Height of the cone
	 * @param	bAdd			Add or remove?
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 * @param	InsideValue		Value to set inside the cone
	 * @param	OutsideValue	Value to set outside the cone
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetValueCone(AVoxelWorld* World, FVector Position, float Radius, float Height, bool bAdd, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, float InsideValue = -1, float OutsideValue = 1);


	/**
	 * Add or remove continuously
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction of the projection in world space 
	 * @param	Radius			Radius
	 * @param	Stength			Speed of modification
	 * @param	bAdd			Add or remove?
	 * @param	MaxDistance		Max distance to modify
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 * @param	MinValue		New values are clampped between MinValue and MaxValue
	 * @param	MaxValue		New values are clampped between MinValue and MaxValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, float Stength, bool bAdd, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true, float MinValue = -1, float MaxValue = 1);

	/**
	 * Set color on surface
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction of the projection in world space 
	 * @param	Radius			Radius
	 * @param	Color			Color to set
	 * @param	FadeDistance	Size in world space of external band where color is interpolated with existing one
	 * @param	MaxDistance		Max distance to modify
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetColorProjection(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, FLinearColor Color, float FadeDistance = 3, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
	/**
	 * Set color on surface
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction to align to in world space
	 * @param	Radius			Radius
	 * @param	Speed			Speed of changes
	 * @param	MaxDistance		Max distance to modify
	 * @param	bQueueUpdate	Should modified chunks be queued for update?
	 * @param	bApplyUpdates	Should queued update be applied?
	 * @param	bAsync			Update async
	 * @param	MinValue		New values are clampped between MinValue and MaxValue
	 * @param	MaxValue		New values are clampped between MinValue and MaxValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SmoothValue(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, float Speed = 0.01f, float MaxDistance = 1000, bool bQueueUpdate = true, bool bApplyUpdates = true, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true, float MinValue = -1, float MaxValue = 1);

	/**
	 * Import a mesh into voxels. WARNING: Slow (use raycasts)
	 * @param	bAdd		Add or remove the mesh from voxels?
	 * @param	World		Voxel world
	 * @param	Component	Component with collision
	 * @param	Actor		Actor holding component (used to get bounds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		static void ImportMesh(bool bAdd, AVoxelWorld* World, UPrimitiveComponent* Component, AActor* Actor);

};
