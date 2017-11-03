// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;

UENUM(BlueprintType)
enum class EBlueprintSuccess : uint8
{
	Sucess, // Correcting typo makes editor crash
	Failed
};

/**
 * Blueprint tools for voxels
 */
UCLASS()
class VOXEL_API UVoxelTools : public UObject
{
	GENERATED_BODY()
public:

	/**
	 * Set value to positive or negative in a sphere
	 * @param	World				Voxel world
	 * @param	Position			Position in world space
	 * @param	Radius				Radius of the sphere in voxel space
	 * @param	bAdd				Add or remove?
	 * @param	bAsync				Update async
	 * @param	HardnessMultiplier	-HardnessMultiplier will be set inside the sphere and HardnessMultiplier outside
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetValueSphere(AVoxelWorld* World, const FVector Position, const float Radius, const bool bAdd, const bool bAsync = false, const float HardnessMultiplier = 1);

	/**
	* Set value to positive or negative in a specified box. Box is placed on its bottom side, relative to the supplied world space position.
	* @param	World				Voxel world
	* @param	Position			Position in world space
	* @param	ExtentXInVoxel		Box X extent in voxel space
	* @param	ExtentYInVoxel		Box Y extent in voxel space
	* @param	ExtentZInVoxel		Box Z extent in voxel space
	* @param	bAdd				Add or remove?
	* @param	bAsync				Update async
	* @param	ValueMultiplier		-ValueMultiplier will be set inside and ValueMultiplier outside
	*/
	//UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
	//	static void SetValueBox(AVoxelWorld* const World, const FVector Position, const float ExtentXInVoxel, const float ExtentYInVoxel,
	//		const float ExtentZInVoxel, const bool bAdd, const bool bAsync, const float ValueMultiplier = 1);

	/**
	 * Set color in a sphere
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the sphere in voxel space
	 * @param	Color			Color to set
	 * @param	FadeDistance	Size, in world space, of external band where color is interpolated with existing one
	 * @param	bAsync				Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SetMaterialSphere(AVoxelWorld* World, const FVector Position, const float Radius, const uint8 MaterialIndex, const bool bUseLayer1 = true, const float FadeDistance = 3, const bool bAsync = false);

	/**
	 * Add or remove continuously
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction of the projection in world space
	 * @param	Radius			Radius in world space
	 * @param	Strength		Speed of modification
	 * @param	bAdd			Add or remove?
	 * @param	MaxDistance		Max distance to modify
	 * @param	Precision		How close are raycasts (relative to VoxelSize)
	 * @param	bAsync			Update async
	 * @param	MinValue		New values are clamped between MinValue and MaxValue
	 * @param	MaxValue		New values are clamped between MinValue and MaxValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, const FVector Position, const FVector Direction, const float Radius, const float Strength, const bool bAdd, const float MaxDistance = 1000,
			const float Precision = 0.25f, const bool bAsync = false, const bool bShowRaycasts = false, const bool bShowHitPoints = true, const bool bShowModifiedVoxels = false, const float MinValue = -1, const float MaxValue = 1);

	/**
	 * Set color on surface
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction of the projection in world space
	 * @param	Radius			Radius in world space
	 * @param	Color			Color to set
	 * @param	FadeDistance	Size in world space of external band where color is interpolated with existing one
	 * @param	MaxDistance		Max distance to modify
	 * @param	bAsync			Update async
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetMaterialProjection(AVoxelWorld* World, FVector Position, const FVector Direction, const float Radius, const uint8 MaterialIndex, const bool bUseLayer1 = true, const float FadeDistance = 3,
			const float MaxDistance = 1000, const float Precision = 0.25f, const bool bAsync = false, const bool bShowRaycasts = false, const bool bShowHitPoints = true, const bool bShowModifiedVoxels = false);
	/**
	 * Set color on surface
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction to align to in world space
	 * @param	Radius			Radius
	 * @param	Speed			Speed of changes
	 * @param	MaxDistance		Max distance to modify
	 * @param	bAsync			Update async
	 * @param	MinValue		New values are clamped between MinValue and MaxValue
	 * @param	MaxValue		New values are clamped between MinValue and MaxValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void SmoothValue(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, float Speed = 0.01f, float MaxDistance = 1000,
			bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true, float MinValue = -1, float MaxValue = 1);

	/**
	 * Import a VoxelAsset in the world
	 * @param	World					Voxel world
	 * @param	Asset					Asset to import
	 * @param	Position				Position in world space
	 * @param	bForceUseOfAllVoxels	Set all the voxels to the asset values?
	 * @param	bAsync					Update async?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void ImportAsset(AVoxelWorld* World, UVoxelAsset* Asset, const FVector Position, const bool bPositionZIsBottom = true, const bool bForceUseOfAllVoxels = false, const bool bAsync = false);

	/**
	 * Get Voxel World from mouse world position and direction given by GetMouseWorldPositionAndDirection
	 * @param	WorldPosition		Mouse world position
	 * @param	WorldDirection		Mouse world direction
	 * @param	MaxDistance			Raycast distance limit
	 * @param	PlayerController	To get world
	 * @return	World				Voxel world
	 * @return	Position			Position in world space of the hit
	 * @return	Normal				Normal of the hit (given by the mesh)
	 * @return	CameraDirection		Direction of the raycast (useful for smooth)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", Meta = (ExpandEnumAsExecs = "Branches"))
		static void GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController,
			AVoxelWorld*& World, FVector& Position, FVector& Normal, FVector& CameraDirection, EBlueprintSuccess& Branches);

	/**
	 * Get mouse world position and direction
	 * @param	PlayerController	Player owning the mouse
	 * @return	WorldPosition		World position of the mouse
	 * @return	WorldDirection		World direction the mouse is facing
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", Meta = (ExpandEnumAsExecs = "Branches"))
		static void GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches);

	/**
	 * Water effect
	 * @param	World				Voxel world
	 * @param	Position			Position in world space
	 * @param	Radius				Radius of the box in voxel space (-Radius < X < Radius ...)
	 * @param	DownSpeed			Speed of the water going down
	 * @param	LateralSpeed		Speed of the water going on sides
	 * @param	bAsync				Update async
	 * @param	ValueMultiplier		-ValueMultiplier will be set inside and ValueMultiplier outside
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
		static void ApplyWaterEffect(AVoxelWorld* World, const int N, const bool bInit, UPARAM(ref) TArray<float>& Dens0, UPARAM(ref) TArray<float>& U0, UPARAM(ref) TArray<float>& V0, UPARAM(ref) TArray<float>& W0, const float Visc, const float Diff, const float Dt, UPARAM(ref) TArray<float>& Dens, UPARAM(ref) TArray<float>&U, UPARAM(ref) TArray<float>& V, UPARAM(ref) TArray<float>& W);

	/**
	 * Remove all blocks not connected to Position
	 * @param	World					Voxel world
	 * @param	Position				Position in world space
	 * @param	Radius					Radius of the box in voxel space (-Radius < X < Radius ...)
	 * @param	bBordersAreConnected	Whether to consider borders connected to Position. Useful to avoid removing parts that are connected outside of the box
	 * @param	bAsync					Update async
	 * @param	ValueMultiplier			-ValueMultiplier will be set inside and ValueMultiplier outside
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "3"))
		static void RemoveNonConnectedBlocks(AVoxelWorld* World, FVector Position, float Radius, bool bBordersAreConnected = true, bool bAsync = false, float ValueMultiplier = 1);
};