// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelRichCurveUtilities.h"

#include "Curves/CurveFloat.h"

template<bool bComputeNormals>
void UVoxelSurfaceTools::FindSurfaceVoxelsImpl(
	FVoxelData& Data,
	const FIntBox& Bounds,
	TArray<FSurfaceVoxel>& OutVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	const FIntVector Size = Bounds.Size();

	const TArray<FVoxelValue> Values = Data.GetValues(Bounds);
	
	const auto GetValue = [&](int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(0 <= X && X < Size.X);
		checkVoxelSlow(0 <= Y && Y < Size.Y);
		checkVoxelSlow(0 <= Z && Z < Size.Z);
		const int32 Index = X + Y * Size.X + Z * Size.X * Size.Y;
		return Values.GetData()[Index];
	};

	for (int32 X = 1; X < Size.X - 1; X++)
	{
		for (int32 Y = 1; Y < Size.Y - 1; Y++)
		{
			for (int32 Z = 1; Z < Size.Z - 1; Z++)
			{
				const FVoxelValue Value = GetValue(X, Y, Z);
				bool bAdd = false;
				const auto GetOtherValue = [&](int32 DX, int32 DY, int32 DZ)
				{
					const FVoxelValue OtherValue = GetValue(X + DX, Y + DY, Z + DZ);
					bAdd |= Value.IsEmpty() != OtherValue.IsEmpty();
					return OtherValue.ToFloat();
				};

				const float GradientX = GetOtherValue(1, 0, 0) - GetOtherValue(-1, 0, 0);
				const float GradientY = GetOtherValue(0, 1, 0) - GetOtherValue(0, -1, 0);
				const float GradientZ = GetOtherValue(0, 0, 1) - GetOtherValue(0, 0, -1);

				if (bAdd)
				{
					FVector Normal;
					if (bComputeNormals)
					{
						Normal = FVector(GradientX, GradientY, GradientZ).GetSafeNormal();
					}
					else
					{
						Normal = FVector(ForceInit);
					}
					OutVoxels.Add({ Bounds.Min + FIntVector(X, Y, Z), Normal, Value.ToFloat() });
				}
			}
		}
	}
}

template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<false>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);
template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<true>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);

void UVoxelSurfaceTools::FindSurfaceVoxels(
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World,
	FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsImpl(Data, Bounds, Voxels));
}

void UVoxelSurfaceTools::FindSurfaceVoxelsAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World, 
	FIntBox Bounds, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsImpl(Data, Bounds, InVoxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::AddStrengthToSurfaceVoxels(const TArray<FSurfaceVoxel>& Voxels, float Strength)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	TArray<FSurfaceVoxelWithStrength> NewVoxels;
	NewVoxels.Reserve(Voxels.Num());
	for (auto& Voxel : Voxels)
	{
		NewVoxels.Emplace(Voxel, Strength);
	}
	return NewVoxels;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyConstantStrength(
	const TArray<FSurfaceVoxelWithStrength>& Voxels, 
	float Strength)
{
	VOXEL_FUNCTION_COUNTER();
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyConstantStrengthImpl(NewVoxels, Strength);
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyStrengthCurve(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector Center,
	float Radius,
	UCurveFloat* StrengthCurve,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	if (!StrengthCurve)
	{
		FVoxelMessages::Error(NSLOCTEXT("Voxel", "", "CreateSphericalStrengthHitArrayFromPositions: Invalid Strength Curve!"));
		return {};
	}

	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);

	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelRichCurveUtilities::Eval(StrengthCurve->FloatCurve, Distance / Radius); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyLinearFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector Center,
	float Radius,
	float Falloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);
	Falloff = GET_VOXEL_TOOL_REAL(Falloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::LinearFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplySmoothFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector Center,
	float Radius,
	float Falloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);
	Falloff = GET_VOXEL_TOOL_REAL(Falloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::SmoothFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplySphericalFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector Center,
	float Radius,
	float Falloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);
	Falloff = GET_VOXEL_TOOL_REAL(Falloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::SphericalFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyTipFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector Center,
	float Radius,
	float Falloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);
	Falloff = GET_VOXEL_TOOL_REAL(Falloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::TipFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyStrengthMask(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVoxelFloatTexture Mask,
	FVector EditPosition,
	float ScaleX,
	float ScaleY,
	FVector PlaneNormal,
	FVector PlaneTangent,
	EVoxelSamplerMode SamplerMode,
	bool bConvertToVoxelSpace)
{
	VOXEL_PRO_ONLY();
}

void UVoxelSurfaceTools::GetStrengthMaskScale(
	float& ScaleX,
	float& ScaleY,
	AVoxelWorld* World,
	FVoxelFloatTexture Mask,
	float SizeX,
	float SizeY, 
	bool bConvertToVoxelSpace)
{
	VOXEL_PRO_ONLY_VOID();
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyTerrace(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	int32 TerraceHeightInVoxels,
	float Angle,
	int32 ImmutableVoxels)
{
	VOXEL_PRO_ONLY();
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyFlatten(
	const TArray<FSurfaceVoxelWithStrength>& Voxels, 
	AVoxelWorld* World,
	FVector PlanePoint,
	FVector PlaneNormal,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	PlanePoint = GET_VOXEL_TOOL_REAL(PlanePoint);

	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyFlattenImpl(NewVoxels, FPlane(PlanePoint, PlaneNormal.GetSafeNormal()));
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplySphere(
	const TArray<FSurfaceVoxelWithStrength>& Voxels, 
	AVoxelWorld* World,
	FVector Center, 
	float Radius,
	bool bAdd,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	Center = GET_VOXEL_TOOL_REAL(Center);
	Radius = GET_VOXEL_TOOL_REAL(Radius);

	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplySphereImpl(NewVoxels, Center, Radius, bAdd);
	return NewVoxels;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVoxelValueEdit> UVoxelSurfaceTools::CreateValueEditsFromSurfaceVoxels(
	const TArray<FSurfaceVoxelWithStrength>& Voxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	TArray<FVoxelValueEdit> Edits;
	Edits.Reserve(Voxels.Num());
	for (auto& Voxel : Voxels)
	{
		if (Voxel.Strength != 0)
		{
			Edits.Emplace(Voxel.Position, Voxel.Strength);
		}
	}
	return Edits;
}

TArray<FVoxelMaterialEdit> UVoxelSurfaceTools::CreateMaterialEditsFromSurfaceVoxels(const TArray<FSurfaceVoxelWithStrength>& Voxels, FVoxelPaintMaterial PaintMaterial)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	TArray<FVoxelMaterialEdit> Edits;
	Edits.Reserve(Voxels.Num());
	for (auto& Voxel : Voxels)
	{
		if (Voxel.Strength != 0)
		{
			Edits.Emplace(FVoxelEditBase(Voxel.Position, Voxel.Strength), PaintMaterial);
		}
	}
	return Edits;
}

TArray<FVoxelFoliageEdit> UVoxelSurfaceTools::CreateFoliageArrayFromStrengths(const TArray<FSurfaceVoxelWithStrength>& Voxels, EVoxelRGBA Layer)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	TArray<FVoxelFoliageEdit> Edits;
	Edits.Reserve(Voxels.Num());
	for (auto& Voxel : Voxels)
	{
		if (Voxel.Strength != 0)
		{
			Edits.Emplace(FVoxelEditBase(Voxel.Position, Voxel.Strength), Layer);
		}
	}
	return Edits;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline FIntBox GetBoundsFromHits(const TArray<T>& Hits)
{
	FIntBox Bounds(Hits[0].Position);
	for (auto& Hit : Hits)
	{
		Bounds = Bounds + Hit.Position;
	}
	return Bounds;
}

template<bool bComputeModifiedVoxels>
void UVoxelSurfaceTools::EditVoxelValuesImpl(
	FVoxelData& Data,
	TArray<FModifiedVoxelValue>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelValueEdit>& Voxels, 
	const FVoxelHardnessHandler& HardnessHandler)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		if (Data.IsInWorld(Voxel.Position))
		{
			const float OldValue = OctreeAccelerator.GetValue(Voxel.Position, 0).ToFloat();
			float Strength = Voxel.Strength;
			if (HardnessHandler.NeedsToCompute())
			{
				Strength /= HardnessHandler.GetHardness(OctreeAccelerator.GetMaterial(Voxel.Position, 0));
			}
			const float NewValue = OldValue + Strength;
			OctreeAccelerator.SetValue(Voxel.Position, FVoxelValue(NewValue));

			if (bComputeModifiedVoxels)
			{
				FModifiedVoxelValue ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldValue = OldValue;
				ModifiedVoxel.NewValue = NewValue;
				OutModifiedVoxels.Add(ModifiedVoxel);
			}
		}
	}
}

template VOXEL_API void UVoxelSurfaceTools::EditVoxelValuesImpl<false>(
	FVoxelData& Data,
	TArray<FModifiedVoxelValue>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelValueEdit>& Voxels,
	const FVoxelHardnessHandler& HardnessHandler);
template VOXEL_API void UVoxelSurfaceTools::EditVoxelValuesImpl<true>(
	FVoxelData& Data,
	TArray<FModifiedVoxelValue>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelValueEdit>& Voxels,
	const FVoxelHardnessHandler& HardnessHandler);

void UVoxelSurfaceTools::EditVoxelValues(
	TArray<FModifiedVoxelValue>& ModifiedVoxels, 
	AVoxelWorld* World, 
	const TArray<FVoxelValueEdit>& Voxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);
	const FVoxelHardnessHandler HardnessHandler(*World);

	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelValuesImpl(Data, ModifiedVoxels, Bounds, Voxels, HardnessHandler));
}

void UVoxelSurfaceTools::EditVoxelValuesAsync(
	UObject* WorldContextObject,	
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelValueEdit>& Voxels,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);
	const FVoxelHardnessHandler HardnessHandler(*World);

	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelValuesImpl(Data, InModifiedVoxels, Bounds, Voxels, HardnessHandler));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bComputeModifiedVoxels>
void UVoxelSurfaceTools::EditVoxelMaterialsImpl(
	FVoxelData& Data,
	TArray<FModifiedVoxelMaterial>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelMaterialEdit>& Voxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		if (Data.IsInWorld(Voxel.Position))
		{
			const FVoxelMaterial OldMaterial = OctreeAccelerator.GetMaterial(Voxel.Position, 0);
			FVoxelMaterial NewMaterial = OldMaterial;
			Voxel.PaintMaterial.ApplyToMaterial(NewMaterial, Voxel.Strength);
			OctreeAccelerator.SetMaterial(Voxel.Position, NewMaterial);

			if (bComputeModifiedVoxels)
			{
				FModifiedVoxelMaterial ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldMaterial = OldMaterial;
				ModifiedVoxel.NewMaterial = NewMaterial;
				OutModifiedVoxels.Add(ModifiedVoxel);
			}
		}
	}
}

template VOXEL_API void UVoxelSurfaceTools::EditVoxelMaterialsImpl<false>(
	FVoxelData& Data,
	TArray<FModifiedVoxelMaterial>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelMaterialEdit>& Voxels);
template VOXEL_API void UVoxelSurfaceTools::EditVoxelMaterialsImpl<true>(
	FVoxelData& Data,
	TArray<FModifiedVoxelMaterial>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelMaterialEdit>& Voxels);

void UVoxelSurfaceTools::EditVoxelMaterials(
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelMaterialEdit>& Voxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelMaterialsImpl(Data, ModifiedVoxels, Bounds, Voxels));
}

void UVoxelSurfaceTools::EditVoxelMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelMaterialEdit>& Voxels,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);

	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelMaterialsImpl(Data, InModifiedVoxels, Bounds, Voxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bComputeModifiedVoxels >
void UVoxelSurfaceTools::EditVoxelFoliageImpl(
	FVoxelData& Data,
	TArray<FModifiedVoxelFoliage>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelFoliageEdit>& Voxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		if (Data.IsInWorld(Voxel.Position))
		{
			const FVoxelFoliage OldFoliage = OctreeAccelerator.Get<FVoxelFoliage>(Voxel.Position, 0);
			const float OldValue = OldFoliage.IsChannelSet(Voxel.Layer) ? OldFoliage.GetChannelValue(Voxel.Layer) : 0;
			FVoxelFoliage NewFoliage = OldFoliage;
			NewFoliage.SetChannelValue(Voxel.Layer, FMath::Lerp(OldValue, 1.f, Voxel.Strength));
			OctreeAccelerator.Set<FVoxelFoliage>(Voxel.Position, NewFoliage);

			if (bComputeModifiedVoxels)
			{
				FModifiedVoxelFoliage ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldFoliage = OldFoliage;
				ModifiedVoxel.NewFoliage = NewFoliage;
				OutModifiedVoxels.Add(ModifiedVoxel);
			}
		}
	}
}

template VOXEL_API void UVoxelSurfaceTools::EditVoxelFoliageImpl<false>(
	FVoxelData& Data,
	TArray<FModifiedVoxelFoliage>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelFoliageEdit>& Voxels);
template VOXEL_API void UVoxelSurfaceTools::EditVoxelFoliageImpl<true>(
	FVoxelData& Data,
	TArray<FModifiedVoxelFoliage>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelFoliageEdit>& Voxels);

void UVoxelSurfaceTools::EditVoxelFoliage(
	TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelFoliageEdit>& Voxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);
	// TODO: should it be UpdateRender here?
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelFoliageImpl(Data, ModifiedVoxels, Bounds, Voxels));
}

void UVoxelSurfaceTools::EditVoxelFoliageAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelFoliageEdit>& Voxels,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Voxels.Num() == 0)
	{
		return;
	}

	const FIntBox Bounds = GetBoundsFromHits(Voxels);
	
	// TODO: should it be UpdateRender here?
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelFoliageImpl(Data, InModifiedVoxels, Bounds, Voxels));
}