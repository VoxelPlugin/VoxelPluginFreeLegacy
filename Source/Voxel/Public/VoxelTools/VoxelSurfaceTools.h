// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelTexture.h"
#include "VoxelConfigEnums.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelTools/VoxelToolsBase.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelSurfaceTools.generated.h"

struct FVoxelHardnessHandler;
struct FLatentActionInfo;
class FVoxelData;
class UCurveFloat;
class AVoxelWorld;

USTRUCT(BlueprintType)
struct FSurfaceVoxel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVector Normal = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value = 0;

	FSurfaceVoxel() = default;
	FSurfaceVoxel(const FIntVector& Position, const FVector& Normal, float Value)
		: Position(Position)
		, Normal(Normal)
		, Value(Value)
	{
	}
};

USTRUCT(BlueprintType)
struct FSurfaceVoxelWithStrength : public FSurfaceVoxel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Strength = 0;

	FSurfaceVoxelWithStrength() = default;
	FSurfaceVoxelWithStrength(const FSurfaceVoxel& Voxel, float Strength)
		: FSurfaceVoxel(Voxel)
		, Strength(Strength)
	{
	}
};

USTRUCT(BlueprintType)
struct FVoxelEditBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Strength = 0;

	FVoxelEditBase() = default;
	FVoxelEditBase(const FIntVector& Position, float Strength)
		: Position(Position)
		, Strength(Strength)
	{
	}
};

USTRUCT(BlueprintType)
struct FVoxelValueEdit : public FVoxelEditBase
{
	GENERATED_BODY()

	// Current value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value = 0.f;
	
	FVoxelValueEdit() = default;
	FVoxelValueEdit(const FVoxelEditBase& EditStrength, float Value)
		: FVoxelEditBase(EditStrength)
		, Value(Value)
	{
	}
};
	
USTRUCT(BlueprintType)
struct FVoxelMaterialEdit : public FVoxelEditBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelPaintMaterial PaintMaterial;

	FVoxelMaterialEdit() = default;
	FVoxelMaterialEdit(const FVoxelEditBase& EditStrength, const FVoxelPaintMaterial& PaintMaterial)
		: FVoxelEditBase(EditStrength)
		, PaintMaterial(PaintMaterial)
	{
	}
};

USTRUCT(BlueprintType)
struct FVoxelFoliageEdit : public FVoxelEditBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelRGBA Layer = EVoxelRGBA::R;

	FVoxelFoliageEdit() = default;
	FVoxelFoliageEdit(const FVoxelEditBase& EditStrength, EVoxelRGBA Layer)
		: FVoxelEditBase(EditStrength)
		, Layer(Layer)
	{
	}
};

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
	template<bool bComputeNormals, bool bOnlyInside = false, uint8 DirectionMask = 0>
	static void FindSurfaceVoxelsImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		TArray<FSurfaceVoxel>& OutVoxels);

	static void FindSurfaceVoxelsFromDistanceFieldImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		float MaxDistance,
		bool bMultiThreaded,
		TArray<FSurfaceVoxel>& OutVoxels);

	template<bool bComputeNormals>
	static void FindSurfaceVoxels2DImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		TArray<FSurfaceVoxel>& OutVoxels);
	
	/**
	 * Find voxels that are on the surface. Faster than FindSurfaceVoxelsFromDistanceField, but the values won't be exact distances
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void FindSurfaceVoxels(
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds);
	
	/**
	 * Find voxels that are on the surface. Faster than FindSurfaceVoxelsFromDistanceField, but the values won't be exact distances
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void FindSurfaceVoxelsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds,
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
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds,
		float MaxDistance = 2,
		bool bMultiThreaded = false);

#if 0 // Not safe since we are firing up a shader
	/**
	 * Find voxels that are on the surface using an exact computation of the distance field using the GPU
	 * @param	World			The voxel world
	 * @param	Bounds			Bounds to look in
	 * @param	MaxDistance		How far from the surface the distance field needs to be exact. Keep low for better performance. Approximation: MaxDistance = Edit Strength + 3
	 * @param	bMultiThreaded	If true will multithread the CPU loops
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bMultiThreaded, bHideLatentWarnings"))
	static void FindSurfaceVoxelsFromDistanceFieldAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds,
		float MaxDistance = 2,
		bool bMultiThreaded = false,
		bool bHideLatentWarnings = false);
#endif
	
	/**
	 * Find voxels that are on the surface. Only keep the one with the surface right above them that are facing up. If 2 surface voxels have the same X Y, will only keep the one with the higher Z
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void FindSurfaceVoxels2D(
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds);
	
	/**
	 * Find voxels that are on the surface. Only keep the one with the surface right above them that are facing up. If 2 surface voxels have the same X Y, will only keep the one with the higher Z
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void FindSurfaceVoxels2DAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	// Create an array of SurfaceVoxelWithStrength from SurfaceVoxels
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static TArray<FSurfaceVoxelWithStrength> AddStrengthToSurfaceVoxels(
		const TArray<FSurfaceVoxel>& Voxels, 
		float Strength = 1);

public:
	static void ApplyConstantStrengthImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const float Strength)
	{
		VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
		for (auto& Voxel : Voxels)
		{
			Voxel.Strength *= Strength;
		}
	}
	template<bool b2D = false, typename T>
	static void ApplyStrengthFunctionImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const FVoxelVector& Center,
		T GetStrength)
	{
		VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
		int32 Num = 0;
		for (const auto& Voxel : Voxels)
		{
			const float Distance = b2D
				? FVector2D::Distance(FVector2D(Center.X, Center.Y), FVector2D(Voxel.Position.X, Voxel.Position.Y))
				: FVoxelVector::Distance(Center, FVoxelVector(Voxel.Position));
			const float Strength = GetStrength(Distance);
			if (Strength != 0)
			{
				auto& NewVoxel = Voxels[Num++];
				NewVoxel = Voxel;
				NewVoxel.Strength *= Strength;
			}
		}
		check(Num <= Voxels.Num());
		Voxels.SetNum(Num);
	}

	// Should always be called last if bExactDistanceField = true
	template<typename T>
	static void ApplySDFImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		EVoxelSDFMergeMode MergeMode,
		bool bExactDistanceField,
		T GetDistance)
	{
		VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
		for (auto& Voxel : Voxels)
		{
			const float CurrentDistance = Voxel.Value;
			const float OtherDistance = GetDistance(FVector(Voxel.Position));
			const float WantedDistance =
				MergeMode == EVoxelSDFMergeMode::Union
				? FMath::Min(CurrentDistance, OtherDistance)
				: MergeMode == EVoxelSDFMergeMode::Intersection
				? FMath::Max(OtherDistance, CurrentDistance)
				: OtherDistance; // Override

			if (bExactDistanceField)
			{
				// No strength should be applied after ApplySDFImpl if we want a good result
				const float IntermediateDistance = FMath::Lerp(CurrentDistance, WantedDistance, Voxel.Strength);
				Voxel.Strength = IntermediateDistance - CurrentDistance;
			}
			else
			{
				const float Difference = WantedDistance - Voxel.Value;
				// We cannot go too fast if we didn't compute the exact distance field
				Voxel.Strength *= FMath::Clamp(Difference, -1.f, 1.f);
			}
		}
	}
	static void ApplyFlattenImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const FPlane& Plane,
		EVoxelSDFMergeMode MergeMode,
		bool bExactDistanceField)
	{
		ApplySDFImpl(Voxels, MergeMode, bExactDistanceField, [&](const FVector& Position) { return Plane.PlaneDot(Position); });
	}

public:
	// Apply a constant strength to the surface voxels
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static TArray<FSurfaceVoxelWithStrength> ApplyConstantStrength(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		float Strength = 1);
	
	/**
	 * Apply a strength curve to surface voxels, based on their distance from a point:
	 * Strength = Curve.SampleAt(Distance(Voxel.Position, Center) / Radius)
	 * @param	Voxels					The voxels
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius to divide the distance by, in cm if bConvertToVoxelSpace = true
	 * @param	StrengthCurve			The strength curve
	 * @param	bConvertToVoxelSpace	Converts Center and Radius from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyStrengthCurve(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
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
	 * @param	Voxels					The voxels
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyLinearFalloff(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
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
	 * @param	Voxels					The voxels
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplySmoothFalloff(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
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
	 * @param	Voxels					The voxels
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplySphericalFalloff(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
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
	 * @param	Voxels					The voxels
	 * @param	World					The voxel world, can be null if bConvertToVoxelSpace = false
	 * @param	Center					The center to compute the distance from, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The radius, in cm if bConvertToVoxelSpace = true
	 * @param	Falloff					The falloff, in cm if bConvertToVoxelSpace = true
	 * @param	bConvertToVoxelSpace	Converts Center, Radius and Falloff from world space to voxel space. Requires World to be non null
	 * @return	New voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyTipFalloff(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		float Falloff,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a strength mask to surface voxels, based on their position projected onto a plane
	 * @param	Voxels					The voxels
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyStrengthMask(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "ImmutableVoxels"))
	static TArray<FSurfaceVoxelWithStrength> ApplyTerrace(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		int32 TerraceHeightInVoxels = 5,
		float Angle = 75,
		int32 ImmutableVoxels = 1);
	
	/**
	 * Make surface voxels go towards a plane
	 * Important: if bExactDistanceField = true, this node should be called last! Modifying strengths after it will result
	 * in glitchy behavior
	 * @param	Voxels					The surface voxels
	 * @param	World					The voxel world, required if bConvertToVoxelSpace = true
	 * @param	PlanePoint				A point in the flatten plane, in world space if bConvertToVoxelSpace = true
	 * @param	PlaneNormal				The normal of the plane
	 * @param	MergeMode				How to merge the plane SDF
	 * @param	bExactDistanceField		Set to true if you used FindSurfaceVoxelsFromDistanceField
	 * @param	bConvertToVoxelSpace	If true, converts PlanePoint from world space to voxel space. Requires World to be non null
	 * @return	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyFlatten(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		AVoxelWorld* World,
		FVector PlanePoint,
		FVector PlaneNormal = FVector(0, 0, 1),
		EVoxelSDFMergeMode MergeMode = EVoxelSDFMergeMode::Override,
		bool bExactDistanceField = false,
		bool bConvertToVoxelSpace = true);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static TArray<FVoxelValueEdit> CreateValueEditsFromSurfaceVoxels(
		const TArray<FSurfaceVoxelWithStrength>& Voxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static TArray<FVoxelMaterialEdit> CreateMaterialEditsFromSurfaceVoxels(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		FVoxelPaintMaterial PaintMaterial);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools")
	static TArray<FVoxelFoliageEdit> CreateFoliageArrayFromStrengths(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		EVoxelRGBA Layer);

public:
	template<bool bComputeModifiedVoxels = true>
	static void EditVoxelValuesImpl(
		FVoxelData& Data,
		TArray<FModifiedVoxelValue>& OutModifiedVoxels,
		const FIntBox& Bounds,
		const TArray<FVoxelValueEdit>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor);
	static void EditVoxelValuesImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		const TArray<FVoxelValueEdit>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor)
	{
		TArray<FModifiedVoxelValue> ModifiedVoxels;
		EditVoxelValuesImpl<false>(Data, ModifiedVoxels, Bounds, Voxels, HardnessHandler, DistanceDivisor);
	}

	// Bounds need to encompass Bounds(Voxels).Extend(0, 0, MaxStrength + DistanceDivisor + 2)!
	static void EditVoxelValues2DImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		const TArray<FVoxelValueEdit>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler,
		float DistanceDivisor);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "DistanceDivisor"))
	static void EditVoxelValues(
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelValueEdit>& Voxels,
		float DistanceDivisor = 1.f);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "DistanceDivisor, bHideLatentWarnings"))
	static void EditVoxelValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelValueEdit>& Voxels,
		float DistanceDivisor = 1.f,
		bool bHideLatentWarnings = false);

public:
	template<bool bComputeModifiedVoxels = true>
	static void EditVoxelMaterialsImpl(
		FVoxelData& Data,
		TArray<FModifiedVoxelMaterial>& OutModifiedVoxels,
		const FIntBox& Bounds,
		const TArray<FVoxelMaterialEdit>& Voxels);
	static void EditVoxelMaterialsImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		const TArray<FVoxelMaterialEdit>& Voxels)
	{
		TArray<FModifiedVoxelMaterial> ModifiedVoxels;
		EditVoxelMaterialsImpl<false>(Data, ModifiedVoxels, Bounds, Voxels);
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void EditVoxelMaterials(
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelMaterialEdit>& Voxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void EditVoxelMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelMaterialEdit>& Voxels,
		bool bHideLatentWarnings = false);

public:
	template<bool bComputeModifiedVoxels = true>
	static void EditVoxelFoliageImpl(
		FVoxelData& Data,
		TArray<FModifiedVoxelFoliage>& OutModifiedVoxels,
		const FIntBox& Bounds,
		const TArray<FVoxelFoliageEdit>& Voxels);
	static void EditVoxelFoliageImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		const TArray<FVoxelFoliageEdit>& Voxels)
	{
		TArray<FModifiedVoxelFoliage> ModifiedVoxels;
		EditVoxelFoliageImpl<false>(Data, ModifiedVoxels, Bounds, Voxels);
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void EditVoxelFoliage(
		TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelFoliageEdit>& Voxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void EditVoxelFoliageAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelFoliageEdit>& Voxels,
		bool bHideLatentWarnings = false);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void DebugSurfaceVoxels(
		AVoxelWorld* World,
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		float Lifetime = 1);
};