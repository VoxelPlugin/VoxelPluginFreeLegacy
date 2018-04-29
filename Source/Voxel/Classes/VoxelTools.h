// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelProceduralMeshComponent.h"
#include "VoxelPart.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;
class UVoxelAsset;

UENUM(BlueprintType)
enum class EBlueprintSuccess : uint8
{
	Success,
	Failed
};

UENUM(BlueprintType)
enum class EVoxelAssetPositionOffset : uint8
{
	Default,
	PositionIsBottom,
	PositionIsMiddle,
	PositionIsTop
};

UENUM(BlueprintType)
enum class EVoxelLayer : uint8
{
	Layer1,
	Layer2
};

/**
 * Blueprint tools for voxels
 */
UCLASS()
class VOXEL_API UVoxelTools : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="Math|IntBox")
	static FIntBox TranslateBox(const FIntBox& Box, const FIntVector& Position) { return Box.TranslateBy(Position); }
	
	UFUNCTION(BlueprintPure, meta=(DisplayName = "IntVector + IntVector", CompactNodeTitle = "+", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category="Math|IntVector")
	static FIntVector Add_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right) { return Left + Right; }
	
	UFUNCTION(BlueprintPure, meta=(DisplayName = "IntVector - IntVector", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category="Math|IntVector")
	static FIntVector Substract_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right) { return Left - Right; }
	

	/**
	 * Add or remove a sphere shape
	 * @param	World				Voxel world
	 * @param	Position			Position in world space
	 * @param	Radius				Radius of the sphere in world space
	 * @param	bAdd				Add or remove?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
	static void SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd);

	/**
	* Add or remove a box shape
	* @param	World				Voxel world
	* @param	Position			Position in world space. Lower left corner of the box
	* @param	Size				Size in voxel space
	* @param	bAdd				Add or remove?
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetValueBox(AVoxelWorld*  World, FVector Position, FIntVector Size, bool bAdd);
	
	/**
	 * Paint a box shape
	 * @param	World				Voxel world
	 * @param	Position			Position in world space. Lower left corner of the box
	 * @param	Size			The size of the box in voxel space
	 * @param	MaterialIndex	The material to paint
	 * @param	Layer			Layer to paint on
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	 static void SetMaterialBox(AVoxelWorld*  World, FVector Position, FIntVector Size, uint8 MaterialIndex, EVoxelLayer Layer = EVoxelLayer::Layer1);

	/**
	 * Paint a sphere shape
	 * @param	World			Voxel world
	 * @param	Position		Position in world space
	 * @param	Radius			Radius of the sphere in world space
	 * @param	MaterialIndex	Material to set
	 * @param	Layer			Layer to use
	 * @param	FadeDistance	Size, in world space, of external band where color is interpolated with existing one
	 * @param	Exponent		The exponent to use for the falloff
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
	static void SetMaterialSphere(AVoxelWorld* World, FVector Position, float Radius, uint8 MaterialIndex, EVoxelLayer Layer = EVoxelLayer::Layer1, float FadeDistance = 5, float Exponent = 2);

	/**
	 * Edit the surface of the world
	 * @param	World				Voxel world
	 * @param	StartPosition		Start Position in world space
	 * @param	Direction			Direction of the projection in world space
	 * @param	Radius				Radius in world space
	 * @param	Strength			Speed of modification
	 * @param	bAdd				Add or remove?
	 * @param	ToolHeight			Tool height (projected on normal)
	 * @param	Precision			How close are raycasts (relative to VoxelSize)
	 * @return	ModifiedPositions	The edited positions
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
	static void SetValueProjection(TArray<FIntVector>& ModifiedPositions, AVoxelWorld* World, FVector StartPosition, FVector Direction, float Radius, float Strength, bool bAdd, float ToolHeight = 200,
								   float Precision = 0.5f, bool bShowRaycasts = false, bool bShowHitPoints = false, bool bShowModifiedVoxels = false, bool bEnableMultiTrace = true);

	/**
	 * Paint the surface of the world
	 * @param	World			Voxel world
	 * @param	Start Position	Start Position in world space
	 * @param	Direction		Direction of the projection in world space
	 * @param	Radius			Radius in world space
	 * @param	MaterialIndex	Material to set
	 * @param	Layer			Layer to use
	 * @param	FadeDistance	Size in world space of external band where color is interpolated with existing one
	 * @param	Exponent		The exponent to use for the falloff
	 * @param	ToolHeight		Tool height (projected on normal)
	 * @param	Precision		How close are raycasts (relative to VoxelSize)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "6"))
	static void SetMaterialProjection(AVoxelWorld* World, FVector StartPosition, FVector Direction, float Radius, uint8 MaterialIndex, EVoxelLayer Layer = EVoxelLayer::Layer1, float FadeDistance = 3, float Exponent = 2,
								      float ToolHeight = 200, float Precision = 0.5f, bool bShowRaycasts = false, bool bShowHitPoints = false, bool bShowModifiedVoxels = false, bool bEnableMultiTrace = true);

	/**
	 * Flatten the surface of the world
	 * @param	World								Voxel World
	 * @param	Position							Position in world space
	 * @param	Normal								Normal at Position
	 * @param	Radius								Radius in world space
	 * @param	Strength							Speed of the edit
	 * @param	bDontModifyVoxelsAroundPosition		Don't edit the voxels around Position. Use if the tool is moving when editing
	 * @param	bDontModifyEmptyVoxels				Can't add on voxels totally empty
	 * @param	bDontModifyFullVoxels				Can't remove on voxels totally full
	 * @param	bShowModifiedVoxels					Draw debug points on modified voxels
	 * @param	bShowTestedVoxels					Draw debug points on voxels tested for validity
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
	 static void Flatten(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, float Strength = 0.1, bool bDontModifyVoxelsAroundPosition = false, bool bDontModifyEmptyVoxels = false, bool bDontModifyFullVoxels = false, bool bShowModifiedVoxels = false, bool bShowTestedVoxels = false);
	

	/**
	 * Get Voxel World from mouse world position and direction given by GetMouseWorldPositionAndDirection. This is a simple Raycast
	 * @param	WorldPosition		Mouse world position
	 * @param	WorldDirection		Mouse world direction
	 * @param	MaxDistance			Raycast distance limit
	 * @param	PlayerController	To get world
	 * @param	bMultipleHits		Use Multi line trace?
	 * @return	World				Voxel world
	 * @return	HitPosition			Position in world space of the hit
	 * @return	HitNormal			Normal of the hit (given by the mesh)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController, bool bMultipleHits,
							  AVoxelWorld*& World, FVector& HitPosition, FVector& HitNormal);

	/**
	 * Get mouse world position and direction
	 * @param	PlayerController	Player owning the mouse
	 * @return	WorldPosition		World position of the mouse
	 * @return	WorldDirection		World direction the mouse is facing
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", Meta = (ExpandEnumAsExecs = "Branches"))
	static void GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches);
	
};