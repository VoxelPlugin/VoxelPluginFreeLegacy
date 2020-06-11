// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelTexture.h"
#include "VoxelConfigEnums.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelTools/VoxelToolsBase.h"
#include "VoxelTools/VoxelSurfaceEdits.h"
#include "VoxelSurfaceTools.generated.h"

struct FVoxelHardnessHandler;
struct FLatentActionInfo;
class FVoxelData;
class UCurveFloat;
class AVoxelWorld;

UENUM(BlueprintType)
enum class EVoxelSDFMergeMode : uint8
{
	// Additive mode: will only grow the surface
	Union,
	// Destructive mode: will only shrink the surface
	Intersection,
	// Will add and remove at the same time
	Override
};

UCLASS()
class VOXEL_API UVoxelSurfaceTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// DirectionMask: if not 0, will only add full voxels with an empty voxel in a direction that's in DirectionMask
	template<uint8 DirectionMask = 0>
	static FVoxelSurfaceEditsVoxels FindSurfaceVoxelsImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		bool bComputeNormals,
		bool bOnlyOutputNonEmptyVoxels = false);

	// TODO bComputeNormals
	static FVoxelSurfaceEditsVoxels FindSurfaceVoxelsFromDistanceFieldImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		float MaxDistance,
		bool bMultiThreaded);

	static FVoxelSurfaceEditsVoxels FindSurfaceVoxels2DImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		bool bComputeNormals);
	
	/**
	 * Find voxels that are on the surface. Faster than FindSurfaceVoxelsFromDistanceField, but the values won't be exact distances
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 * @param	bComputeNormals			If true, compute the voxel normals. More expensive, but required for some functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void FindSurfaceVoxels(
		FVoxelSurfaceEditsVoxels& Voxels,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bComputeNormals = false);
	
	/**
	 * Find voxels that are on the surface. Faster than FindSurfaceVoxelsFromDistanceField, but the values won't be exact distances
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 * @param	bComputeNormals			If true, compute the voxel normals. More expensive, but required for some functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void FindSurfaceVoxelsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelSurfaceEditsVoxels& Voxels,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bComputeNormals = false,
		bool bHideLatentWarnings = false);
	
	/**
	 * Find voxels that are on the surface using an exact computation of the distance field using the GPU
	 * @param	World			The voxel world
	 * @param	Bounds			Bounds to look in
	 * @param	MaxDistance		How far from the surface the distance field needs to be exact. Keep low for better performance. Approximation: MaxDistance = Edit Strength + 3
	 * @param	bMultiThreaded	If true will multithread the CPU loops
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bMultiThreaded"))
	static void FindSurfaceVoxelsFromDistanceField(
		FVoxelSurfaceEditsVoxels& Voxels,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		float MaxDistance = 2,
		bool bMultiThreaded = false);
	
	/**
	 * Find voxels that are on the surface. Only keep the one with the surface right above them that are facing up. If 2 surface voxels have the same X Y, will only keep the one with the higher Z
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 * @param	bComputeNormals			If true, compute the voxel normals. More expensive, but required for some functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void FindSurfaceVoxels2D(
		FVoxelSurfaceEditsVoxels& Voxels,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bComputeNormals = false);
	
	/**
	 * Find voxels that are on the surface. Only keep the one with the surface right above them that are facing up. If 2 surface voxels have the same X Y, will only keep the one with the higher Z
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 * @param	bComputeNormals			If true, compute the voxel normals. More expensive, but required for some functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void FindSurfaceVoxels2DAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelSurfaceEditsVoxels& Voxels,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bComputeNormals = false,
		bool bHideLatentWarnings = false);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static FVoxelSurfaceEditsStack AddToStack(FVoxelSurfaceEditsStack Stack, FVoxelSurfaceEditsStackElement Element);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static FVoxelSurfaceEditsProcessedVoxels ApplyStack(FVoxelSurfaceEditsVoxels Voxels, FVoxelSurfaceEditsStack Stack);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void ApplyStackAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		FVoxelSurfaceEditsVoxels Voxels, 
		FVoxelSurfaceEditsStack Stack,
		bool bHideLatentWarnings = false);
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools")
	static FVoxelIntBox GetBounds(FVoxelSurfaceEditsProcessedVoxels Voxels) { return Voxels.Bounds; }
	
public:
	// Apply a constant strength to the surface voxels
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools")
	static FVoxelSurfaceEditsStackElement ApplyConstantStrength(float Strength = 1);
	
	/**
	 * Apply a strength curve to surface voxels, based on their distance from a point:
	 * Strength = Curve.SampleAt(Distance(Voxel.Position, Center) / Radius)
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius to divide the distance by, in cm if bConvertToVoxelSpace = true
	 * @param	StrengthCurve			The strength curve
	 * @param	bConvertToVoxelSpace	Converts Center and Radius from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplyStrengthCurve(
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		UCurveFloat* StrengthCurve,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a linear falloff to surface voxels, based on their distance from a point.
	 * If Distance is between 0 and Radius: Strength = 1
	 * If Distance is between Radius and Radius + Falloff: 0 <= Strength <= 1
	 * If Distance is beyond Radius + Falloff: Strength = 0
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplyLinearFalloff(
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		float Falloff,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a smooth falloff to surface voxels, based on their distance from a point.
	 * If Distance is between 0 and Radius: Strength = 1
	 * If Distance is between Radius and Radius + Falloff: 0 <= Strength <= 1
	 * If Distance is beyond Radius + Falloff: Strength = 0
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplySmoothFalloff(
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		float Falloff,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a spherical falloff to surface voxels, based on their distance from a point.
	 * If Distance is between 0 and Radius: Strength = 1
	 * If Distance is between Radius and Radius + Falloff: 0 <= Strength <= 1
	 * If Distance is beyond Radius + Falloff: Strength = 0
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplySphericalFalloff(
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		float Falloff,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a tip falloff to surface voxels, based on their distance from a point.
	 * If Distance is between 0 and Radius: Strength = 1
	 * If Distance is between Radius and Radius + Falloff: 0 <= Strength <= 1
	 * If Distance is beyond Radius + Falloff: Strength = 0
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplyTipFalloff(
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		float Falloff,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a strength mask to surface voxels, based on their position projected onto a plane
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Mask					The mask to apply
	 * @param	EditPosition			The voxel positions are computed relative to this. In world space if bConvertToVoxelSpace = true
	 * @param	ScaleX					The sampling scale on the X axis. The bigger, the bigger the projected image will be.
	 *									Recommended: Wanted size in voxels of the image / image size in pixels.
	 *									Can use GetStrengthMaskScale.
	 * @param	ScaleY					The sampling scale on the Y axis. The bigger, the bigger the projected image will be.
	 *									Recommended: Wanted size in voxels of the image / image size in pixels.
	 *									Can use GetStrengthMaskScale.
	 * @param	PlaneNormal
	 * @param	PlaneTangent
	 * @param	SamplerMode
	 * @param	bConvertToVoxelSpace	Converts Center and Radius from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplyStrengthMask(
		AVoxelWorld* World,
		FVoxelFloatTexture Mask,
		FVector EditPosition,
		float ScaleX = 1,
		float ScaleY = 1,
		FVector PlaneNormal = FVector(0, 0, 1),
		FVector PlaneTangent = FVector(1, 0, 0),
		EVoxelSamplerMode SamplerMode = EVoxelSamplerMode::Tile,
		bool bConvertToVoxelSpace = true);

	/**
	 * Compute the scale for ApplyStrengthMask from a wanted size
	 * @param	World					The voxel world, required if bConvertToVoxelSpace = true
	 * @param	Mask					The mask
	 * @param	SizeX					The wanted size on the X axis, in cm if bConvertToVoxelSpace = true
	 * @param	SizeY					The wanted size on the Y axis, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts SizeX and SizeY from world space to voxel space. Requires World to be non null
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static void GetStrengthMaskScale(
		float& ScaleX,
		float& ScaleY,
		AVoxelWorld* World,
		FVoxelFloatTexture Mask,
		float SizeX = 1000.f,
		float SizeY = 1000.f,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply terracing
	 * @param	TerraceHeightInVoxels	The height of the terraces in voxels
	 * @param	Angle					The angle in degrees of the terraces borders. Not entirely precise. Between 0 and 180.
	 * @param	ImmutableVoxels			The number of voxels to not change per terrace/height of the "top layer" of each terrace
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "ImmutableVoxels"))
	static FVoxelSurfaceEditsStackElement ApplyTerrace(
		int32 TerraceHeightInVoxels = 5,
		float Angle = 75,
		int32 ImmutableVoxels = 1);
	
	/**
	 * Make surface voxels go towards a plane
	 * Important: if bExactDistanceField = true, this node should be called last! Modifying strengths after it will result
	 * in glitchy behavior
	 * @param	World					The voxel world, required if bConvertToVoxelSpace = true
	 * @param	PlanePoint				A point in the flatten plane, in world space if bConvertToVoxelSpace = true
	 * @param	PlaneNormal				The normal of the plane
	 * @param	MergeMode				How to merge the plane SDF
	 * @param	bConvertToVoxelSpace	If true, converts PlanePoint from world space to voxel space. Requires World to be non null
	 * @return	
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static FVoxelSurfaceEditsStackElement ApplyFlatten(
		AVoxelWorld* World,
		FVector PlanePoint,
		FVector PlaneNormal = FVector(0, 0, 1),
		EVoxelSDFMergeMode MergeMode = EVoxelSDFMergeMode::Override,
		bool bConvertToVoxelSpace = true);

public:
	static void EditVoxelValuesImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor,
		bool bHasValues,
		TArray<FModifiedVoxelValue>* OutModifiedVoxels = nullptr);

	// Bounds need to encompass Bounds(Voxels).Extend(0, 0, MaxStrength + DistanceDivisor + 2)!
	static void EditVoxelValues2DImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor,
		TArray<FModifiedVoxelValue>* OutModifiedVoxels = nullptr);

	// Will call EditVoxelValuesImpl or EditVoxelValues2DImpl based on ProcessedVoxels
	static void EditVoxelValuesImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor,
		TArray<FModifiedVoxelValue>* OutModifiedVoxels = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "DistanceDivisor"))
	static void EditVoxelValues(
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
		float DistanceDivisor = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "DistanceDivisor, bHideLatentWarnings"))
	static void EditVoxelValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
		float DistanceDivisor = 1.f,
		bool bHideLatentWarnings = false);

public:
	static void EditVoxelMaterialsImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		TArray<FModifiedVoxelMaterial>* OutModifiedVoxels = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void EditVoxelMaterials(
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World,
		const FVoxelPaintMaterial& PaintMaterial,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void EditVoxelMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World,
		const FVoxelPaintMaterial& PaintMaterial,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
		bool bHideLatentWarnings = false);

public:
	static void EditVoxelFoliageImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		EVoxelRGBA Layer,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		TArray<FModifiedVoxelFoliage>* OutModifiedVoxels = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void EditVoxelFoliage(
		TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
		AVoxelWorld* World,
		EVoxelRGBA Layer,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void EditVoxelFoliageAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
		AVoxelWorld* World,
		EVoxelRGBA Layer,
		FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
		bool bHideLatentWarnings = false);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void DebugSurfaceVoxels(
		AVoxelWorld* World,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float Lifetime = 1);
};