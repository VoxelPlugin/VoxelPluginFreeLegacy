// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Gen/VoxelToolsBase.h"
#include "VoxelLevelTools.generated.h"

UCLASS()
class VOXEL_API UVoxelLevelTools : public UVoxelToolsBase
{
	GENERATED_BODY()
	
public:
	/**
	 * Stamps a cylinder, to quickly level parts of the world
	 * If additive, will stamp a smooth cylinder above Position. Else will remove one below Position
	 * @see Level, LevelAsync and FVoxelLevelToolsImpl::Level
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the top (or bottom if subtractive) of the cylinder. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff between 0 and 1. The higher the smoother the cylinder edge.
	 * @param	Height               	The height of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bAdditive            	Additive or subtractive edit, see node comment
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position, Radius and Height will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Level Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void Level(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Falloff,
		float Height,
		bool bAdditive,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Stamps a cylinder, to quickly level parts of the world
	 * If additive, will stamp a smooth cylinder above Position. Else will remove one below Position
	 * Runs asynchronously in a background thread
	 * @see Level, LevelAsync and FVoxelLevelToolsImpl::Level
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the top (or bottom if subtractive) of the cylinder. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff between 0 and 1. The higher the smoother the cylinder edge.
	 * @param	Height               	The height of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bAdditive            	Additive or subtractive edit, see node comment
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position, Radius and Height will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Level Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void LevelAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Falloff,
		float Height,
		bool bAdditive,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Stamps a cylinder, to quickly level parts of the world
	 * If additive, will stamp a smooth cylinder above Position. Else will remove one below Position
	 * @see Level, LevelAsync and FVoxelLevelToolsImpl::Level
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the top (or bottom if subtractive) of the cylinder. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff             	The falloff between 0 and 1. The higher the smoother the cylinder edge.
	 * @param	Height              	The height of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bAdditive           	Additive or subtractive edit, see node comment
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position, Radius and Height will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void Level(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Falloff,
		float Height,
		bool bAdditive,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Stamps a cylinder, to quickly level parts of the world
	 * If additive, will stamp a smooth cylinder above Position. Else will remove one below Position
	 * Runs asynchronously in a background thread
	 * @see Level, LevelAsync and FVoxelLevelToolsImpl::Level
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the top (or bottom if subtractive) of the cylinder. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff between 0 and 1. The higher the smoother the cylinder edge.
	 * @param	Height               	The height of the cylinder. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bAdditive            	Additive or subtractive edit, see node comment
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position, Radius and Height will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void LevelAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Falloff,
		float Height,
		bool bAdditive,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
};