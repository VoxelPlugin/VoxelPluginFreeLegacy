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

	using FVoxelEditBase::FVoxelEditBase;
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

enum class ESDFMergeMode
{
	Union,
	Intersection,
	Override
};

UCLASS()
class VOXEL_API UVoxelSurfaceTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	template<bool bComputeNormals = true>
	static void FindSurfaceVoxelsImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		TArray<FSurfaceVoxel>& OutVoxels);
	
	/**
	 * Find voxels that are on the surface
	 * @param	World					The voxel world
	 * @param	Bounds					Bounds to look in
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void FindSurfaceVoxels(
		TArray<FSurfaceVoxel>& Voxels,
		AVoxelWorld* World,
		FIntBox Bounds);
	
	/**
	 * Find voxels that are on the surface
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
	template<typename T>
	static void ApplyStrengthFunctionImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const FVector& Center,
		T GetStrength)
	{
		VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
		int32 Num = 0;
		for (const auto& Voxel : Voxels)
		{
			const float Distance = FVector::Distance(Center, FVector(Voxel.Position));
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
	template<ESDFMergeMode MergeMode, typename T>
	static void ApplySDFImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		T GetDistance)
	{
		VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
		for (auto& Voxel : Voxels)
		{
			const float Distance = GetDistance(FVector(Voxel.Position));
			const float Target =
				MergeMode == ESDFMergeMode::Union
				? FMath::Min(Distance, Voxel.Value)
				: MergeMode == ESDFMergeMode::Intersection
				? FMath::Max(Distance, Voxel.Value)
				: Distance; // Override
			const float Difference = Target - Voxel.Value;
			Voxel.Strength *= FMath::Clamp(Difference, -1.f, 1.f);
		}
	}
	static void ApplyFlattenImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const FPlane& Plane)
	{
		ApplySDFImpl<ESDFMergeMode::Override>(Voxels, [&](const FVector& Position) { return Plane.PlaneDot(Position); });
	}
	static void ApplySphereImpl(
		TArray<FSurfaceVoxelWithStrength>& Voxels,
		const FVector& Center,
		const float Radius,
		const bool bAdd)
	{
		if (bAdd)
		{
			ApplySDFImpl<ESDFMergeMode::Union>(Voxels, [&](const FVector& Position) { return FVector::Distance(Position, Center) - Radius; });
		}
		else
		{
			ApplySDFImpl<ESDFMergeMode::Intersection>(Voxels, [&](const FVector& Position) { return Radius - FVector::Distance(Position, Center); });
		}
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
	 * @param	Voxels					The surface voxels
	 * @param	World					The voxel world, required if bConvertToVoxelSpace = true
	 * @param	PlanePoint				A point in the flatten plane, in world space if bConvertToVoxelSpace = true
	 * @param	PlaneNormal				The normal of the plane
	 * @param	bConvertToVoxelSpace	If true, converts PlanePoint from world space to voxel space. Requires World to be non null
	 * @return	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplyFlatten(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		AVoxelWorld* World,
		FVector PlanePoint,
		FVector PlaneNormal = FVector(0, 0, 1),
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Make surface voxels go towards a sphere shape
	 * @param	Voxels					The surface voxels
	 * @param	World					The voxel world, required if bConvertToVoxelSpace = true
	 * @param	Center					The sphere center, in world space if bConvertToVoxelSpace = true
	 * @param	Radius					The sphere radius, in cm if bConvertToVoxelSpace = true
	 * @param	bAdd					Whether to add or remove the sphere
	 * @param	bConvertToVoxelSpace	If true, converts Center and Radius from world space to voxel space. Requires World to be non null
	 * @return	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (AdvancedDisplay = "bConvertToVoxelSpace", DefaultToSelf = "World"))
	static TArray<FSurfaceVoxelWithStrength> ApplySphere(
		const TArray<FSurfaceVoxelWithStrength>& Voxels,
		AVoxelWorld* World,
		FVector Center,
		float Radius,
		bool bAdd = true,
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
		const FVoxelHardnessHandler& HardnessHandler);
	static void EditVoxelValuesImpl(
		FVoxelData& Data,
		const FIntBox& Bounds,
		const TArray<FVoxelValueEdit>& Voxels,
		const FVoxelHardnessHandler& HardnessHandler)
	{
		TArray<FModifiedVoxelValue> ModifiedVoxels;
		EditVoxelValuesImpl<false>(Data, ModifiedVoxels, Bounds, Voxels, HardnessHandler);
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World"))
	static void EditVoxelValues(
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelValueEdit>& Voxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Surface Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void EditVoxelValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FModifiedVoxelValue>& ModifiedVoxels,
		AVoxelWorld* World,
		const TArray<FVoxelValueEdit>& Voxels,
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
};