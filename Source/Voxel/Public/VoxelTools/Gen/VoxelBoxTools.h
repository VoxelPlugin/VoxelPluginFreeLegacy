// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Gen/VoxelToolsBase.h"
#include "VoxelBoxTools.generated.h"

UCLASS()
class VOXEL_API UVoxelBoxTools : public UVoxelToolsBase
{
	GENERATED_BODY()
	
public:
	/**
	 * Set the density in a box
	 * @see SetValueBox, SetValueBoxAsync and FVoxelBoxToolsImpl::SetValueBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	Value                	The density to set
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender"))
	static void SetValueBox(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		float Value,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
	/**
	 * Set the density in a box
	 * Runs asynchronously in a background thread
	 * @see SetValueBox, SetValueBoxAsync and FVoxelBoxToolsImpl::SetValueBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	Value                	The density to set
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SetValueBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		float Value,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Set the density in a box
	 * @see SetValueBox, SetValueBoxAsync and FVoxelBoxToolsImpl::SetValueBox
	 * @param	VoxelWorld       	The voxel world to do the edit to
	 * @param	Bounds           	The bounds of the box
	 * @param	Value            	The density to set
	 * @param	OutModifiedValues	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds  	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded   	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender    	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetValueBox(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		float Value,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Set the density in a box
	 * Runs asynchronously in a background thread
	 * @see SetValueBox, SetValueBoxAsync and FVoxelBoxToolsImpl::SetValueBox
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	Value                	The density to set
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetValueBoxAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		float Value,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Add a box shape
	 * @see AddBox, AddBoxAsync and FVoxelBoxToolsImpl::AddBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender"))
	static void AddBox(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a box shape
	 * Runs asynchronously in a background thread
	 * @see AddBox, AddBoxAsync and FVoxelBoxToolsImpl::AddBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void AddBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Add a box shape
	 * @see AddBox, AddBoxAsync and FVoxelBoxToolsImpl::AddBox
	 * @param	VoxelWorld       	The voxel world to do the edit to
	 * @param	Bounds           	The bounds of the box
	 * @param	OutModifiedValues	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds  	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded   	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender    	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void AddBox(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a box shape
	 * Runs asynchronously in a background thread
	 * @see AddBox, AddBoxAsync and FVoxelBoxToolsImpl::AddBox
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void AddBoxAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Remove a box shape
	 * @see RemoveBox, RemoveBoxAsync and FVoxelBoxToolsImpl::RemoveBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender"))
	static void RemoveBox(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
	/**
	 * Remove a box shape
	 * Runs asynchronously in a background thread
	 * @see RemoveBox, RemoveBoxAsync and FVoxelBoxToolsImpl::RemoveBox
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void RemoveBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Remove a box shape
	 * @see RemoveBox, RemoveBoxAsync and FVoxelBoxToolsImpl::RemoveBox
	 * @param	VoxelWorld       	The voxel world to do the edit to
	 * @param	Bounds           	The bounds of the box
	 * @param	OutModifiedValues	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds  	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded   	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender    	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RemoveBox(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Remove a box shape
	 * Runs asynchronously in a background thread
	 * @see RemoveBox, RemoveBoxAsync and FVoxelBoxToolsImpl::RemoveBox
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Bounds               	The bounds of the box
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RemoveBoxAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Paint a box shape
	 * @see SetMaterialBox, SetMaterialBoxAsync and FVoxelBoxToolsImpl::SetMaterialBox
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Bounds                  	The bounds of the box
	 * @param	PaintMaterial           	The material to set
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender"))
	static void SetMaterialBox(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
	
	/**
	 * Paint a box shape
	 * Runs asynchronously in a background thread
	 * @see SetMaterialBox, SetMaterialBoxAsync and FVoxelBoxToolsImpl::SetMaterialBox
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Bounds                  	The bounds of the box
	 * @param	PaintMaterial           	The material to set
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings     	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SetMaterialBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Paint a box shape
	 * @see SetMaterialBox, SetMaterialBoxAsync and FVoxelBoxToolsImpl::SetMaterialBox
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Bounds              	The bounds of the box
	 * @param	PaintMaterial       	The material to set
	 * @param	OutModifiedMaterials	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetMaterialBox(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bUpdateRender = true);
	
	/**
	 * Paint a box shape
	 * Runs asynchronously in a background thread
	 * @see SetMaterialBox, SetMaterialBoxAsync and FVoxelBoxToolsImpl::SetMaterialBox
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Bounds                  	The bounds of the box
	 * @param	PaintMaterial           	The material to set
	 * @param	Callback                	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds         	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetMaterialBoxAsync(
		AVoxelWorld* VoxelWorld,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bUpdateRender = true);
};