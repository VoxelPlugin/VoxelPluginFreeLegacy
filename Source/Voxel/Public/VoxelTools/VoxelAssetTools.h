// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssetTools.generated.h"

class AVoxelWorld;
class FVoxelData;
class UVoxelTransformableGeneratorInstanceWrapper;

template<typename T>
class TVoxelDataItemWrapper;

struct FVoxelAssetItem;
struct FVoxelDisableEditsBoxItem;

UENUM(BlueprintType)
enum class EVoxelAssetMergeMode : uint8
{
	// Import all values. Import no materials
	AllValues,
	// Import all materials. Import no values
	AllMaterials,
	// Import all values and all materials
	AllValuesAndAllMaterials,
	// Import values that are "inside" the asset. Import no materials.
	InnerValues,
	// Import materials that are "inside" the asset. Import no values.
	InnerMaterials,
	// Import values and materials that are "inside" the asset.
	InnerValuesAndInnerMaterials,
};

USTRUCT(BlueprintType)
struct FVoxelAssetItemReference
{
	GENERATED_BODY()

	FVoxelIntBox Bounds;
	TVoxelWeakPtr<const TVoxelDataItemWrapper<FVoxelAssetItem>> Item;
};

USTRUCT(BlueprintType)
struct FVoxelDisableEditsBoxItemReference
{
	GENERATED_BODY()

	FVoxelIntBox Bounds;
	TVoxelWeakPtr<const TVoxelDataItemWrapper<FVoxelDisableEditsBoxItem>> Item;
};

UCLASS()
class VOXEL_API UVoxelAssetTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Cheap, unless there are edited voxels in the zone the asset is imported in
	 * Will provide the PreviousGenerator to voxel graphs
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	Priority				Priority of the asset: the higher priority ones will be on top of lower priority ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 * @param	bUpdateRender			If the render should be updated. Not needed if done right after creating the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace, bUpdateRender"))
	static void ImportAssetAsReference(
		FVoxelAssetItemReference& Reference,
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		int32 Priority,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Cheap, unless there are edited voxels in the zone the asset is imported in
	 * Will provide the PreviousGenerator to voxel graphs
	 * Runs asynchronously
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	Priority				Priority of the asset: the higher priority ones will be on top of lower priority ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 * @param	bUpdateRender			If the render should be updated. Not needed if done right after creating the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings"))
	static void ImportAssetAsReferenceAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelAssetItemReference& Reference,
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		int32 Priority,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
public:
	static void ImportModifierAssetImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const FTransform& Transform,
		const FVoxelTransformableGeneratorInstance& Instance,
		bool bModifyValues,
		bool bModifyMaterials);

	/**
	 * Add a voxel modifier asset to the world. Should be a graph asset.
	 * Unlike ImportAsset, this WILL provide the Previous Generator to the graph, so you can access the existing voxel value
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bLockEntireWorld, bConvertToVoxelSpace"))
	static void ImportModifierAsset(
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		bool bModifyValues = true,
		bool bModifyMaterials = true,
		bool bLockEntireWorld = true,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bLockEntireWorld, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ImportModifierAssetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		bool bModifyValues = true,
		bool bModifyMaterials = true,
		bool bLockEntireWorld = true,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);

public:
	static void ImportAssetImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const FTransform& Transform,
		const FVoxelTransformableGeneratorInstance& Instance,
		bool bSubtractive,
		EVoxelAssetMergeMode MergeMode,
		uint32 MaterialMask);
	static void ImportDataAssetImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelVector& Position,
		const FVoxelDataAssetData& AssetData,
		bool bSubtractive,
		EVoxelAssetMergeMode MergeMode,
		uint32 MaterialMask);

	template<typename TData, typename T1, typename T2>
	static void ImportDataAssetImpl(
		TData& Data,
		const FVoxelVector& Position,
		const FVoxelDataAssetData& AssetData,
		// To set the default value when querying the asset
		bool bSubtractive,
		T1 GetValue = [] (float OldValue, float InstanceValue) { return OldValue; },
		bool bSetMaterials = false,
		T2 GetMaterial = [](float OldValue, float NewValue, FVoxelMaterial OldMaterial, float InstanceValue, FVoxelMaterial InstanceMaterial) { return OldMaterial; });
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bSubtractive			If false, the inner values are the full ones. If true, the inner values are the empty ones.
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace"))
	static void ImportAsset(
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		bool bSubtractive = false,
		EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bSubtractive			If false, the inner values are the full ones. If true, the inner values are the empty ones.
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ImportAssetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		UVoxelTransformableGeneratorInstanceWrapper* Asset,
		FTransform Transform,
		FVoxelIntBox Bounds,
		bool bSubtractive = false,
		EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);

public:
	/**
	 * Specialization of ImportAsset for data assets with no scale nor rotation
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Position				The position of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace"))
	static void ImportDataAssetFast(
		AVoxelWorld* World,
		UVoxelDataAsset* Asset,
		FVector Position,
		EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials,
		bool bConvertToVoxelSpace = true);

	/**
	 * Specialization of ImportAsset for data assets with no scale nor rotation
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Position				The position of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ImportDataAssetFastAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		UVoxelDataAsset* Asset,
		FVector Position,
		EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
public:
	static void InvertDataAssetImpl(
		const FVoxelDataAssetData& AssetData,
		FVoxelDataAssetData& InvertedAssetData);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools")
	static void InvertDataAsset(UVoxelDataAsset* Asset, UVoxelDataAsset*& InvertedAsset);
	
public:
	static void SetDataAssetMaterialImpl(
		const FVoxelDataAssetData& AssetData,
		FVoxelDataAssetData& NewAssetData,
		FVoxelMaterial Material);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools")
	static void SetDataAssetMaterial(UVoxelDataAsset* Asset, UVoxelDataAsset*& NewAsset, FVoxelMaterial Material);

public:
	static void CreateDataAssetFromWorldSectionImpl(
		const FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		bool bCopyMaterials,
		FVoxelDataAssetData& AssetData);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World"))
	static UVoxelDataAsset* CreateDataAssetFromWorldSection(
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bCopyMaterials);

public:
	/**
	 * Add a disable edits box to the world
	 * @param	World			The voxel world to edit
	 * @param	Bounds			The bounds of the box in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World"))
	static void AddDisableEditsBox(
		FVoxelDisableEditsBoxItemReference& Reference,
		AVoxelWorld* World,
		FVoxelIntBox Bounds);
	
	/**
	 * Add a disable edits box to the world
	 * Runs asynchronously
	 * @param	World	The voxel world to edit
	 * @param	Bounds	The bounds of the box in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void AddDisableEditsBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelDisableEditsBoxItemReference& Reference,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);
};