// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelSurfaceEdits.h"
#include "VoxelTools/Gen/VoxelToolsBase.h"
#include "VoxelSurfaceEditTools.generated.h"

UCLASS()
class VOXEL_API UVoxelSurfaceEditTools : public UVoxelToolsBase
{
	GENERATED_BODY()
	
public:
	/**
	 * Apply processed voxels strengths to the voxel values
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelValues, EditVoxelValuesAsync and FVoxelSurfaceEditToolsImpl::EditVoxelValues
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	ProcessedVoxels      	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	DistanceDivisor      	Distance divisor: the new value will be divided by this. Useful if normals are bad because of clamped values
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "DistanceDivisor, bMultiThreaded, bRecordModifiedValues, bUpdateRender"))
	static void EditVoxelValues(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float DistanceDivisor = 1.f,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply processed voxels strengths to the voxel values
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelValues, EditVoxelValuesAsync and FVoxelSurfaceEditToolsImpl::EditVoxelValues
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	ProcessedVoxels      	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	DistanceDivisor      	Distance divisor: the new value will be divided by this. Useful if normals are bad because of clamped values
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "DistanceDivisor, bMultiThreaded, bRecordModifiedValues, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void EditVoxelValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float DistanceDivisor = 1.f,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Apply processed voxels strengths to the voxel values
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelValues, EditVoxelValuesAsync and FVoxelSurfaceEditToolsImpl::EditVoxelValues
	 * @param	VoxelWorld       	The voxel world to do the edit to
	 * @param	ProcessedVoxels  	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	DistanceDivisor  	Distance divisor: the new value will be divided by this. Useful if normals are bad because of clamped values
	 * @param	OutModifiedValues	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds  	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded   	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender    	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void EditVoxelValues(
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float DistanceDivisor = 1.f,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply processed voxels strengths to the voxel values
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelValues, EditVoxelValuesAsync and FVoxelSurfaceEditToolsImpl::EditVoxelValues
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	ProcessedVoxels      	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	DistanceDivisor      	Distance divisor: the new value will be divided by this. Useful if normals are bad because of clamped values
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void EditVoxelValuesAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float DistanceDivisor = 1.f,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Apply paint material to the processed voxels, using the processed strengths
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelMaterials, EditVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::EditVoxelMaterials
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	PaintMaterial           	The paint material to apply
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender"))
	static void EditVoxelMaterials(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelPaintMaterial& PaintMaterial,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply paint material to the processed voxels, using the processed strengths
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelMaterials, EditVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::EditVoxelMaterials
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	PaintMaterial           	The paint material to apply
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings     	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void EditVoxelMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelPaintMaterial& PaintMaterial,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Apply paint material to the processed voxels, using the processed strengths
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelMaterials, EditVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::EditVoxelMaterials
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	PaintMaterial       	The paint material to apply
	 * @param	ProcessedVoxels     	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	OutModifiedMaterials	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void EditVoxelMaterials(
		AVoxelWorld* VoxelWorld,
		const FVoxelPaintMaterial& PaintMaterial,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply paint material to the processed voxels, using the processed strengths
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see EditVoxelMaterials, EditVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::EditVoxelMaterials
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	PaintMaterial           	The paint material to apply
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	Callback                	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds         	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void EditVoxelMaterialsAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelPaintMaterial& PaintMaterial,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Propagate the materials of the voxels, so that the new surface is painted correctly
	 * Must be called BEFORE EditVoxelValues!
	 * @see ApplyStack, AddToStack
	 * @see PropagateVoxelMaterials, PropagateVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender"))
	static void PropagateVoxelMaterials(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
	
	/**
	 * Propagate the materials of the voxels, so that the new surface is painted correctly
	 * Must be called BEFORE EditVoxelValues!
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see PropagateVoxelMaterials, PropagateVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings     	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Edit Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void PropagateVoxelMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Propagate the materials of the voxels, so that the new surface is painted correctly
	 * Must be called BEFORE EditVoxelValues!
	 * @see ApplyStack, AddToStack
	 * @see PropagateVoxelMaterials, PropagateVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	ProcessedVoxels     	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	OutModifiedMaterials	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void PropagateVoxelMaterials(
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Propagate the materials of the voxels, so that the new surface is painted correctly
	 * Must be called BEFORE EditVoxelValues!
	 * Runs asynchronously in a background thread
	 * @see ApplyStack, AddToStack
	 * @see PropagateVoxelMaterials, PropagateVoxelMaterialsAsync and FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	ProcessedVoxels         	The processed voxels, usually obtained by applying a surface edit stack
	 * @param	Callback                	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds         	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void PropagateVoxelMaterialsAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
};