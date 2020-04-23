// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssetTools.generated.h"

class AVoxelWorld;
class FVoxelPlaceableItem;
class FVoxelData;

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
struct FVoxelPlaceableItemReference
{
	GENERATED_BODY()

	FIntBox Bounds;
	TVoxelWeakPtr<FVoxelPlaceableItem> Item;
};

UCLASS()
class VOXEL_API UVoxelAssetTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Cheap, unless there are edited voxels in the zone the asset is imported in
	 * Will provide the PreviousWorldGenerator to voxel graphs
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	Priority				Priority of the asset: the higher priority ones will be on top of lower priority ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 * @param	bUpdateRender			If the render should be updated. Not needed if done right after creating the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace, bUpdateRender"))
	static void ImportAssetAsReference(
		FVoxelPlaceableItemReference& Reference,
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
		int32 Priority,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Cheap, unless there are edited voxels in the zone the asset is imported in
	 * Will provide the PreviousWorldGenerator to voxel graphs
	 * Runs asynchronously
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	Priority				Priority of the asset: the higher priority ones will be on top of lower priority ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 * @param	bUpdateRender			If the render should be updated. Not needed if done right after creating the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bUpdateRender, bHideLatentWarnings"))
	static void ImportAssetAsReferenceAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelPlaceableItemReference& Reference,
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
		int32 Priority,
		bool bConvertToVoxelSpace = true,
		bool bUpdateRender = true,
		bool bHideLatentWarnings = false);
	
public:

	/**
	 * Add a voxel modifier asset to the world. Should be a graph asset.
	 * Unlike ImportAsset, this WILL provide the Previous World Generator to the graph, so you can access the existing voxel value
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", AdvancedDisplay = "bLockEntireWorld, bConvertToVoxelSpace"))
	static void ImportModifierAsset(
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
		bool bModifyValues = true,
		bool bModifyMaterials = true,
		bool bLockEntireWorld = true,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous World Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bLockEntireWorld, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ImportModifierAssetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
		bool bModifyValues = true,
		bool bModifyMaterials = true,
		bool bLockEntireWorld = true,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);

public:
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous World Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bSubtractive			If false, the inner values are the full ones. If true, the inner values are the empty ones.
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace"))
	static void ImportAsset(
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
		bool bSubtractive = false,
		EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Add a voxel asset to the world: can be a data asset (eg imported from a mesh), or a graph asset
	 * Will not provide the Previous World Generator, so won't work with graphs that use it!
	 * Unlike ImportAssetAsReference, this one copies the asset data into the world. Can be expensive for large assets. Use this if your asset is relatively small
	 * @param	World					The voxel world to edit
	 * @param	Asset					The asset to import
	 * @param	Seeds					By default, the asset uses the Voxel World seeds. You can override them here.
	 * @param	Transform				The transform of the asset: in world space, unless ConvertToVoxelSpace is false
	 * @param	Bounds					The bounds of the asset in voxel space. If the asset is a VoxelTransformableWorldGeneratorWithBounds, they will be set automatically if not provided
	 * @param	bSubtractive			If false, the inner values are the full ones. If true, the inner values are the empty ones.
	 * @param	MergeMode				How the values and materials of the asset should be merged with existing ones
	 * @param	bConvertToVoxelSpace	If true, the transform is in world space. If false, it's in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (AutoCreateRefTerm = "Seeds", DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bSetAllMaterials, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ImportAssetAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		UVoxelTransformableWorldGenerator* Asset,
		const TMap<FName, int32>& Seeds,
		FTransform Transform,
		FIntBox Bounds,
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

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools")
	static void InvertDataAsset(UVoxelDataAsset* Asset, UVoxelDataAsset*& InvertedAsset);
	
public:

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools")
	static void SetDataAssetMaterial(UVoxelDataAsset* Asset, UVoxelDataAsset*& NewAsset, FVoxelMaterial Material);

public:
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World"))
	static UVoxelDataAsset* CreateDataAssetFromWorldSection(
		AVoxelWorld* World,
		FIntBox Bounds,
		bool bCopyMaterials);

public:
	/**
	 * Add a disable edits box to the world
	 * @param	World			The voxel world to edit
	 * @param	Bounds			The bounds of the box in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World"))
	static void AddDisableEditsBox(
		FVoxelPlaceableItemReference& Reference,
		AVoxelWorld* World,
		FIntBox Bounds);
	
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
		FVoxelPlaceableItemReference& Reference,
		AVoxelWorld* World,
		FIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	/**
	 * Remove a placeable item (voxel asset or disable edit box) from the voxel world
	 * @param	World						The voxel world to edit
	 * @param	Reference					The reference to the placeable item
	 * @param	bResetOverlappingChunksData	By default, manually edited (= dirty) chunks affected by the item are kept as-is, as it's not possible to remove the item while keeping the other edits that happened to that chunk.
	 *										Enabling this option will reset such chunks instead, removing any other edit applied to them
	 * @param	bUpdateRender				If the render should be updated. Not needed if a disable edits box
	 * @param	Error						Error if unsuccessful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Asset Tools", meta = (DefaultToSelf = "World", Keywords = "Disable edits box import voxel asset actor", AdvancedDisplay = "bResetOverlappingChunksData, bUpdateRender"))
	static bool RemovePlaceableItem(AVoxelWorld* World, FVoxelPlaceableItemReference Reference, FString& Error, bool bResetOverlappingChunksData = false, bool bUpdateRender = true);
};