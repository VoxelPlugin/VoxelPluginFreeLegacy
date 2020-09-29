// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Gen/VoxelToolsBase.h"
#include "VoxelSphereTools.generated.h"

UCLASS()
class VOXEL_API UVoxelSphereTools : public UVoxelToolsBase
{
	GENERATED_BODY()
	
public:
	/**
	 * Set the density in a sphere
	 * @see SetValueSphere, SetValueSphereAsync and FVoxelSphereToolsImpl::SetValueSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Value                	The density to set
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void SetValueSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Value,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Set the density in a sphere
	 * Runs asynchronously in a background thread
	 * @see SetValueSphere, SetValueSphereAsync and FVoxelSphereToolsImpl::SetValueSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Value                	The density to set
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SetValueSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Value,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Set the density in a sphere
	 * @see SetValueSphere, SetValueSphereAsync and FVoxelSphereToolsImpl::SetValueSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Value               	The density to set
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetValueSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Value,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Set the density in a sphere
	 * Runs asynchronously in a background thread
	 * @see SetValueSphere, SetValueSphereAsync and FVoxelSphereToolsImpl::SetValueSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Value                	The density to set
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetValueSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Value,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Remove a sphere
	 * @see RemoveSphere, RemoveSphereAsync and FVoxelSphereToolsImpl::RemoveSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void RemoveSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Remove a sphere
	 * Runs asynchronously in a background thread
	 * @see RemoveSphere, RemoveSphereAsync and FVoxelSphereToolsImpl::RemoveSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void RemoveSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Remove a sphere
	 * @see RemoveSphere, RemoveSphereAsync and FVoxelSphereToolsImpl::RemoveSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RemoveSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Remove a sphere
	 * Runs asynchronously in a background thread
	 * @see RemoveSphere, RemoveSphereAsync and FVoxelSphereToolsImpl::RemoveSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RemoveSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Add a sphere
	 * @see AddSphere, AddSphereAsync and FVoxelSphereToolsImpl::AddSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void AddSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a sphere
	 * Runs asynchronously in a background thread
	 * @see AddSphere, AddSphereAsync and FVoxelSphereToolsImpl::AddSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void AddSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Add a sphere
	 * @see AddSphere, AddSphereAsync and FVoxelSphereToolsImpl::AddSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void AddSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a sphere
	 * Runs asynchronously in a background thread
	 * @see AddSphere, AddSphereAsync and FVoxelSphereToolsImpl::AddSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void AddSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Paint material in a sphere shape
	 * @see SetMaterialSphere, SetMaterialSphereAsync and FVoxelSphereToolsImpl::SetMaterialSphere
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	PaintMaterial           	The material to paint
	 * @param	Strength                	The strength of the painting (preferably between 0 and 1)
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender"))
	static void SetMaterialSphere(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		float Strength = 1.f,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Paint material in a sphere shape
	 * Runs asynchronously in a background thread
	 * @see SetMaterialSphere, SetMaterialSphereAsync and FVoxelSphereToolsImpl::SetMaterialSphere
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	PaintMaterial           	The material to paint
	 * @param	Strength                	The strength of the painting (preferably between 0 and 1)
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings     	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SetMaterialSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		float Strength = 1.f,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Paint material in a sphere shape
	 * @see SetMaterialSphere, SetMaterialSphereAsync and FVoxelSphereToolsImpl::SetMaterialSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	PaintMaterial       	The material to paint
	 * @param	Strength            	The strength of the painting (preferably between 0 and 1)
	 * @param	FalloffType         	The type of falloff
	 * @param	Falloff             	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	OutModifiedMaterials	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetMaterialSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		float Strength = 1.f,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Paint material in a sphere shape
	 * Runs asynchronously in a background thread
	 * @see SetMaterialSphere, SetMaterialSphereAsync and FVoxelSphereToolsImpl::SetMaterialSphere
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	PaintMaterial           	The material to paint
	 * @param	Strength                	The strength of the painting (preferably between 0 and 1)
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	Callback                	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds         	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SetMaterialSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		float Strength = 1.f,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Apply a 3x3x3 kernel
	 * @see ApplyKernelSphere, ApplyKernelSphereAsync and FVoxelSphereToolsImpl::ApplyKernelSphere
	 * @param	ModifiedValues                	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds                  	Returns the bounds edited by this function
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues         	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void ApplyKernelSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply a 3x3x3 kernel
	 * Runs asynchronously in a background thread
	 * @see ApplyKernelSphere, ApplyKernelSphereAsync and FVoxelSphereToolsImpl::ApplyKernelSphere
	 * @param	ModifiedValues                	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds                  	Returns the bounds edited by this function
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues         	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings           	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void ApplyKernelSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Apply a 3x3x3 kernel
	 * @see ApplyKernelSphere, ApplyKernelSphereAsync and FVoxelSphereToolsImpl::ApplyKernelSphere
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	OutModifiedValues             	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds               	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void ApplyKernelSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply a 3x3x3 kernel
	 * Runs asynchronously in a background thread
	 * @see ApplyKernelSphere, ApplyKernelSphereAsync and FVoxelSphereToolsImpl::ApplyKernelSphere
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Callback                      	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds               	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues         	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void ApplyKernelSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Apply a 3x3x3 kernel to the materials
	 * @see ApplyMaterialKernelSphere, ApplyMaterialKernelSphereAsync and FVoxelSphereToolsImpl::ApplyMaterialKernelSphere
	 * @param	ModifiedMaterials             	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds                  	Returns the bounds edited by this function
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Mask                          	The material channels to affect
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials      	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender"))
	static void ApplyMaterialKernelSphere(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		UPARAM(meta = (Bitmask, BitmaskEnum = EVoxelMaterialMask_BP)) int32 Mask = 4095,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply a 3x3x3 kernel to the materials
	 * Runs asynchronously in a background thread
	 * @see ApplyMaterialKernelSphere, ApplyMaterialKernelSphereAsync and FVoxelSphereToolsImpl::ApplyMaterialKernelSphere
	 * @param	ModifiedMaterials             	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds                  	Returns the bounds edited by this function
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Mask                          	The material channels to affect
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials      	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings           	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void ApplyMaterialKernelSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		UPARAM(meta = (Bitmask, BitmaskEnum = EVoxelMaterialMask_BP)) int32 Mask = 4095,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Apply a 3x3x3 kernel to the materials
	 * @see ApplyMaterialKernelSphere, ApplyMaterialKernelSphereAsync and FVoxelSphereToolsImpl::ApplyMaterialKernelSphere
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Mask                          	The material channels to affect
	 * @param	OutModifiedMaterials          	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds               	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void ApplyMaterialKernelSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Apply a 3x3x3 kernel to the materials
	 * Runs asynchronously in a background thread
	 * @see ApplyMaterialKernelSphere, ApplyMaterialKernelSphereAsync and FVoxelSphereToolsImpl::ApplyMaterialKernelSphere
	 * @param	VoxelWorld                    	The voxel world to do the edit to
	 * @param	Position                      	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                        	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	CenterMultiplier              	Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier 	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier 	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations                 	The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Mask                          	The material channels to affect
	 * @param	Callback                      	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds               	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded                	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials      	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace          	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender                 	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void ApplyMaterialKernelSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Smooth a sphere
	 * @see SmoothSphere, SmoothSphereAsync and FVoxelSphereToolsImpl::SmoothSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength             	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations        	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	FalloffType          	The type of falloff
	 * @param	Falloff              	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void SmoothSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Smooth a sphere
	 * Runs asynchronously in a background thread
	 * @see SmoothSphere, SmoothSphereAsync and FVoxelSphereToolsImpl::SmoothSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength             	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations        	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	FalloffType          	The type of falloff
	 * @param	Falloff              	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SmoothSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Smooth a sphere
	 * @see SmoothSphere, SmoothSphereAsync and FVoxelSphereToolsImpl::SmoothSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength            	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations       	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	FalloffType         	The type of falloff
	 * @param	Falloff             	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SmoothSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Smooth a sphere
	 * Runs asynchronously in a background thread
	 * @see SmoothSphere, SmoothSphereAsync and FVoxelSphereToolsImpl::SmoothSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength             	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations        	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	FalloffType          	The type of falloff
	 * @param	Falloff              	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SmoothSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Smooth materials in a sphere
	 * @see SmoothMaterialSphere, SmoothMaterialSphereAsync and FVoxelSphereToolsImpl::SmoothMaterialSphere
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength                	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations           	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	Mask                    	The material channels to affect
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender"))
	static void SmoothMaterialSphere(
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		UPARAM(meta = (Bitmask, BitmaskEnum = EVoxelMaterialMask_BP)) int32 Mask = 4095,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = true,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Smooth materials in a sphere
	 * Runs asynchronously in a background thread
	 * @see SmoothMaterialSphere, SmoothMaterialSphereAsync and FVoxelSphereToolsImpl::SmoothMaterialSphere
	 * @param	ModifiedMaterials       	Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds            	Returns the bounds edited by this function
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength                	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations           	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	Mask                    	The material channels to affect
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings     	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedMaterials, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void SmoothMaterialSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		UPARAM(meta = (Bitmask, BitmaskEnum = EVoxelMaterialMask_BP)) int32 Mask = 4095,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Smooth materials in a sphere
	 * @see SmoothMaterialSphere, SmoothMaterialSphereAsync and FVoxelSphereToolsImpl::SmoothMaterialSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength            	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations       	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	Mask                	The material channels to affect
	 * @param	FalloffType         	The type of falloff
	 * @param	Falloff             	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	OutModifiedMaterials	Optional. Record the Materials modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SmoothMaterialSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		TArray<FModifiedVoxelMaterial>* OutModifiedMaterials = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Smooth materials in a sphere
	 * Runs asynchronously in a background thread
	 * @see SmoothMaterialSphere, SmoothMaterialSphereAsync and FVoxelSphereToolsImpl::SmoothMaterialSphere
	 * @param	VoxelWorld              	The voxel world to do the edit to
	 * @param	Position                	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius                  	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Strength                	The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations           	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	Mask                    	The material channels to affect
	 * @param	FalloffType             	The type of falloff
	 * @param	Falloff                 	The falloff, between 0 and 1. Set to 0 to disable.
	 * @param	Callback                	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds         	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded          	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedMaterials	If false, will not fill ModifiedMaterials, making the edit faster.
	 * @param	bConvertToVoxelSpace    	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender           	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void SmoothMaterialSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f,
		const FOnVoxelToolComplete_WithModifiedMaterials& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * @see TrimSphere, TrimSphereAsync and FVoxelSphereToolsImpl::TrimSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Normal               	The normal
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff, between 0 and 1
	 * @param	bAdditive            	Whether to add or remove voxels
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void TrimSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * Runs asynchronously in a background thread
	 * @see TrimSphere, TrimSphereAsync and FVoxelSphereToolsImpl::TrimSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Normal               	The normal
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff, between 0 and 1
	 * @param	bAdditive            	Whether to add or remove voxels
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void TrimSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * @see TrimSphere, TrimSphereAsync and FVoxelSphereToolsImpl::TrimSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Normal              	The normal
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff             	The falloff, between 0 and 1
	 * @param	bAdditive           	Whether to add or remove voxels
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void TrimSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * Runs asynchronously in a background thread
	 * @see TrimSphere, TrimSphereAsync and FVoxelSphereToolsImpl::TrimSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Normal               	The normal
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	Falloff              	The falloff, between 0 and 1
	 * @param	bAdditive            	Whether to add or remove voxels
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void TrimSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @see RevertSphere, RevertSphereAsync and FVoxelSphereToolsImpl::RevertSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	HistoryPosition      	The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void RevertSphere(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * Runs asynchronously in a background thread
	 * @see RevertSphere, RevertSphereAsync and FVoxelSphereToolsImpl::RevertSphere
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	HistoryPosition      	The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void RevertSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @see RevertSphere, RevertSphereAsync and FVoxelSphereToolsImpl::RevertSphere
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	HistoryPosition     	The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues       	Whether to revert values
	 * @param	bRevertMaterials    	Whether to revert materials
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RevertSphere(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * Runs asynchronously in a background thread
	 * @see RevertSphere, RevertSphereAsync and FVoxelSphereToolsImpl::RevertSphere
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	HistoryPosition      	The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RevertSphereAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
public:
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @see RevertSphereToGenerator, RevertSphereToGeneratorAsync and FVoxelSphereToolsImpl::RevertSphereToGenerator
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender"))
	static void RevertSphereToGenerator(
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials,
		bool bMultiThreaded = true,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * Runs asynchronously in a background thread
	 * @see RevertSphereToGenerator, RevertSphereToGeneratorAsync and FVoxelSphereToolsImpl::RevertSphereToGenerator
	 * @param	ModifiedValues       	Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging
	 * @param	EditedBounds         	Returns the bounds edited by this function
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	 * @param	bHideLatentWarnings  	Hide latent warnings caused by calling a node before its previous call completion.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "VoxelWorld", AdvancedDisplay = "bMultiThreaded, bRecordModifiedValues, bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void RevertSphereToGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedValues,
		FVoxelIntBox& EditedBounds,
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @see RevertSphereToGenerator, RevertSphereToGeneratorAsync and FVoxelSphereToolsImpl::RevertSphereToGenerator
	 * @param	VoxelWorld          	The voxel world to do the edit to
	 * @param	Position            	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius              	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bRevertValues       	Whether to revert values
	 * @param	bRevertMaterials    	Whether to revert materials
	 * @param	OutModifiedValues   	Optional. Record the Values modified by this function. Useful to track the amount of edit done, for instance to give resources when digging. Will append to existing values.
	 * @param	OutEditedBounds     	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded      	If true, multiple threads will be used to make the edit faster.
	 * @param	bConvertToVoxelSpace	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender       	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RevertSphereToGenerator(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials,
		TArray<FModifiedVoxelValue>* OutModifiedValues = nullptr,
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * Runs asynchronously in a background thread
	 * @see RevertSphereToGenerator, RevertSphereToGeneratorAsync and FVoxelSphereToolsImpl::RevertSphereToGenerator
	 * @param	VoxelWorld           	The voxel world to do the edit to
	 * @param	Position             	The position of the center. In world space (unreal units) if bConvertToVoxelSpace is true. In voxel space if false.
	 * @param	Radius               	The radius. In unreal units if bConvertToVoxelSpace is true. In voxels if false.
	 * @param	bRevertValues        	Whether to revert values
	 * @param	bRevertMaterials     	Whether to revert materials
	 * @param	Callback             	Called on the game thread when the function is completed. Will not be called if the async function completes after the voxel world is destroyed.
	 * @param	OutEditedBounds      	Optional. Returns the bounds edited by this function
	 * @param	bMultiThreaded       	If true, multiple threads will be used to make the edit faster. Not recommended on async functions, as it might cause lag.
	 * @param	bRecordModifiedValues	If false, will not fill ModifiedValues, making the edit faster.
	 * @param	bConvertToVoxelSpace 	If true, Position and Radius will be converted to voxel space. Else they will be used directly.
	 * @param	bUpdateRender        	If false, will only edit the data and not update the render. Rarely needed.
	*/
	static void RevertSphereToGeneratorAsync(
		AVoxelWorld* VoxelWorld,
		const FVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials,
		const FOnVoxelToolComplete_WithModifiedValues& Callback = {},
		FVoxelIntBox* OutEditedBounds = nullptr,
		bool bMultiThreaded = false,
		bool bRecordModifiedValues = true,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
};