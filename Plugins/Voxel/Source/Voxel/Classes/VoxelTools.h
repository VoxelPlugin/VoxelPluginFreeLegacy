// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMeshAsset.h"
#include "VoxelMaterial.h"
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
	 * @param	ValueMultiplier		-ValueMultiplier will be set inside the sphere and ValueMultiplier outside
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
		static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bAsync = false, float ValueMultiplier = 1);

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
		static void SetMaterialSphere(AVoxelWorld* World, FVector Position, float Radius, uint8 MaterialIndex, bool bUseLayer1 = true, float FadeDistance = 3, bool bAsync = false);

	/**
	 * Add or remove continuously
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Direction		Direction of the projection in world space
	 * @param	Radius			Radius in world space
	 * @param	Stength			Speed of modification
	 * @param	bAdd			Add or remove?
	 * @param	MaxDistance		Max distance to modify
	 * @param	bAsync			Update async
	 * @param	MinValue		New values are clamped between MinValue and MaxValue
	 * @param	MaxValue		New values are clamped between MinValue and MaxValue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
		static void SetValueProjection(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, float Stength, bool bAdd, float MaxDistance = 1000,
									   bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true, float MinValue = -1, float MaxValue = 1);

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
		static void SetMaterialProjection(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, uint8 MaterialIndex, bool bUseLayer1 = true,
										  float FadeDistance = 3, float MaxDistance = 1000, bool bAsync = false, bool bDebugLines = false, bool bDebugPoints = true);
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
	 * Import a mesh into voxels. WARNING: Slow (use raycasts)
	 * @param	World			Voxel world
	 * @param	MeshToImport	Subclass of VoxelMeshAsset
	 * @param	Position		Position in world space
	 * @param	bAsync			Update async?
	 * @param	bDebugPoints	Draw debug points?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "3"))
		static void ImportMesh(AVoxelWorld* World, TSubclassOf<AVoxelMeshAsset> MeshToImport, FVector Position, bool bAsync = false, bool bDebugPoints = false);

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
		static void ApplyWaterEffect(AVoxelWorld* WaterWorld, AVoxelWorld* SolidWorld, FVector Position, float Radius, float DownSpeed, float LateralSpeed, bool bAsync = false, float ValueMultiplier = 1);

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