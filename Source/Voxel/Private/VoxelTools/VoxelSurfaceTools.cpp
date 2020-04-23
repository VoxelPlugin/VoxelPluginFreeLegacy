// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelShaders/VoxelDistanceFieldShader.h"
#include "VoxelRichCurveUtilities.h"
#include "VoxelDirection.h"

#include "Curves/CurveFloat.h"
#include "Async/ParallelFor.h"
#include "DrawDebugHelpers.h"

template<bool bComputeNormals, bool bOnlyInside, uint8 DirectionMask>
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

	OutVoxels.Reserve(Bounds.Count());
	for (int32 X = 1; X < Size.X - 1; X++)
	{
		for (int32 Y = 1; Y < Size.Y - 1; Y++)
		{
			for (int32 Z = 1; Z < Size.Z - 1; Z++)
			{
				const FVoxelValue Value = GetValue(X, Y, Z);
				if (bOnlyInside && Value.IsEmpty())
				{
					continue;
				}
				
				bool bAdd = false;
				const auto GetOtherValue = [&](uint8 Direction, int32 DX, int32 DY, int32 DZ)
				{
					const FVoxelValue OtherValue = GetValue(X + DX, Y + DY, Z + DZ);
					if (!DirectionMask || (!Value.IsEmpty() && (Direction & DirectionMask)))
					{
						bAdd |= Value.IsEmpty() != OtherValue.IsEmpty();
					}
					return OtherValue.ToFloat();
				};
				
				// Note: if bComputeNormals = false, could be faster by not computing other values once bAdd = true
				const float GradientX = GetOtherValue(EVoxelDirection::XMax, 1, 0, 0) - GetOtherValue(EVoxelDirection::XMin, -1, 0, 0);
				const float GradientY = GetOtherValue(EVoxelDirection::YMax, 0, 1, 0) - GetOtherValue(EVoxelDirection::YMin, 0, -1, 0);
				const float GradientZ = GetOtherValue(EVoxelDirection::ZMax, 0, 0, 1) - GetOtherValue(EVoxelDirection::ZMin, 0, 0, -1);

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

template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<false, false, 0>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);
template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<true , false, 0>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);
template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<false, true , 0>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);
template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxelsImpl<true , true , 0>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(
	FVoxelData& Data,
	const FIntBox& Bounds,
	const float MaxDistance,
	const bool bMultiThreaded,
	TArray<FSurfaceVoxel>& OutVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

	const auto Get = [&](auto& Array, auto Index) -> auto&
	{
#if VOXEL_DEBUG
		return Array[Index];
#else
		return Array.GetData()[Index];
#endif
	};
	
	const FIntVector Size = Bounds.Size();

	// Else ParallelFor might crash
	if (Size.X < 3 || Size.Y < 3 || Size.Z < 3) return;

	TArray<FFloat16> DistanceField;
	DistanceField.SetNumUninitialized(Bounds.Count());
	const TArray<FVoxelValue> Values = Data.ParallelGet<FVoxelValue>(Bounds, !bMultiThreaded);

	{
		VOXEL_SCOPE_COUNTER("Expand values to floats");
		ParallelFor(DistanceField.Num(), [&](int32 Index)
			{
				Get(DistanceField, Index) = Get(Values, Index).ToFloat();
			}, !bMultiThreaded);
	}

	{
		VOXEL_SCOPE_COUNTER("Compute distance field");
		auto DataPtr = MakeVoxelShared<TArray<FFloat16>>(MoveTemp(DistanceField));
		DataPtr->SetNum(Size.X * Size.Y * Size.Z);
		auto Helper = MakeVoxelShared<FVoxelDistanceFieldShaderHelper>();
		Helper->StartCompute(Size, DataPtr, FMath::CeilToInt(MaxDistance), true);
		Helper->WaitForCompletion();
		DistanceField = MoveTemp(*DataPtr);
	}

	VOXEL_SCOPE_COUNTER("Create OutVoxels");
	OutVoxels.SetNumUninitialized(Bounds.Count());
	FThreadSafeCounter Count;
	// Distance on boundaries is invalid, don't consider these
	ParallelFor(Size.X - 2, [&](int32 XIndex)
	{
		const int32 X = XIndex + 1;
		for (int32 Y = 1; Y < Size.Y - 1; Y++)
		{
			for (int32 Z = 1; Z < Size.Z - 1; Z++)
			{
				const int32 Index = X + Y * Size.X + Z * Size.X * Size.Y;
				float Distance = Get(DistanceField, Index);
				if (Distance > MaxDistance) 
				{
					continue;
				}
				const FVoxelValue Value = Get(Values, Index);
				Distance *= Value.Sign();
				const FIntVector Position(X, Y, Z);
				const int32 OutVoxelsIndex = Count.Increment() - 1;
				Get(OutVoxels, OutVoxelsIndex) = FSurfaceVoxel{ Bounds.Min + Position, FVector(0), Distance };
			}
		}
	}, !bMultiThreaded);
	OutVoxels.SetNum(Count.GetValue(), false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bComputeNormals>
void UVoxelSurfaceTools::FindSurfaceVoxels2DImpl(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	
	FindSurfaceVoxelsImpl<bComputeNormals, false, EVoxelDirection::ZMax>(Data, Bounds, OutVoxels);
	
	TMap<FIntPoint, FSurfaceVoxel> Columns;
	for (auto& Voxel : OutVoxels)
	{
		if (auto* Existing = Columns.Find(FIntPoint(Voxel.Position.X, Voxel.Position.Y)))
		{
			if (Existing->Position.Z < Voxel.Position.Z)
			{
				*Existing = Voxel;
			}
		}
		else
		{
			Columns.Add(FIntPoint(Voxel.Position.X, Voxel.Position.Y), Voxel);
		}
	}

	OutVoxels.Reset();
	Columns.GenerateValueArray(OutVoxels);
}

template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxels2DImpl<false>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);
template VOXEL_API void UVoxelSurfaceTools::FindSurfaceVoxels2DImpl<true>(FVoxelData& Data, const FIntBox& Bounds, TArray<FSurfaceVoxel>& OutVoxels);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxels(
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World,
	FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsImpl<true>(Data, Bounds, Voxels));
}

void UVoxelSurfaceTools::FindSurfaceVoxelsAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World, 
	FIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsImpl<true>(Data, Bounds, InVoxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceField(
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World,
	FIntBox Bounds,
	float MaxDistance,
	bool bMultiThreaded)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, MaxDistance, bMultiThreaded, Voxels));
}

#if 0
void UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World,
	FIntBox Bounds,
	float MaxDistance,
	bool bMultiThreaded,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, MaxDistance, bMultiThreaded, InVoxels));
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxels2D(
	TArray<FSurfaceVoxel>& Voxels,
	AVoxelWorld* World, 
	FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxels2DImpl<true>(Data, Bounds, Voxels));
}

void UVoxelSurfaceTools::FindSurfaceVoxels2DAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	TArray<FSurfaceVoxel>& Voxels, 
	AVoxelWorld* World, 
	FIntBox Bounds, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, FindSurfaceVoxels2DImpl<true>(Data, Bounds, InVoxels));
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
	FVector InCenter,
	float InRadius,
	UCurveFloat* StrengthCurve,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	if (!StrengthCurve)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Strength Curve!"));
		return {};
	}
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);

	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelRichCurveUtilities::Eval(StrengthCurve->FloatCurve, Distance / Radius); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyLinearFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::LinearFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplySmoothFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::SmoothFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplySphericalFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::SphericalFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyTipFalloff(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyStrengthFunctionImpl(NewVoxels, Center, [=](float Distance) { return FVoxelUtilities::TipFalloff(Distance, Radius, Falloff); });
	return NewVoxels;
}

TArray<FSurfaceVoxelWithStrength> UVoxelSurfaceTools::ApplyStrengthMask(
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	AVoxelWorld* World,
	FVoxelFloatTexture Mask,
	FVector InEditPosition,
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
	EVoxelSDFMergeMode MergeMode,
	bool bExactDistanceField,
	bool bConvertToVoxelSpace)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	PlanePoint = GET_VOXEL_TOOL_REAL(PlanePoint).ToFloat();

	TArray<FSurfaceVoxelWithStrength> NewVoxels = Voxels;
	ApplyFlattenImpl(NewVoxels, FPlane(PlanePoint, PlaneNormal.GetSafeNormal()), MergeMode, bExactDistanceField);
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
			Edits.Emplace(FVoxelEditBase(Voxel.Position, Voxel.Strength), Voxel.Value);
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
	const FVoxelHardnessHandler& HardnessHandler,
	float DistanceDivisor)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		OctreeAccelerator.EditValue(Voxel.Position, [&](FVoxelValue& Value)
		{
			// ProjectionHit will set the value to NAN
			// Bit of a hack
			const float OldValue = FMath::IsNaN(Voxel.Value) ? Value.ToFloat() : Voxel.Value;
			float Strength = Voxel.Strength;
			if (HardnessHandler.NeedsToCompute())
			{
				Strength /= HardnessHandler.GetHardness(OctreeAccelerator.GetMaterial(Voxel.Position, 0));
			}
			const float NewValue = OldValue + Strength;
			Value = FVoxelValue(NewValue / DistanceDivisor);

			if (Value.IsNull())
			{
				// 0 does weird things, so don't set the value to it
				// (creates blocks)
				Value -= FVoxelValue::Precision();
			}
			
			if (bComputeModifiedVoxels)
			{
				FModifiedVoxelValue ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldValue = OldValue;
				ModifiedVoxel.NewValue = NewValue;
				OutModifiedVoxels.Add(ModifiedVoxel);
			}
		});
	}
}

template VOXEL_API void UVoxelSurfaceTools::EditVoxelValuesImpl<false>(
	FVoxelData& Data,
	TArray<FModifiedVoxelValue>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelValueEdit>& Voxels,
	const FVoxelHardnessHandler& HardnessHandler,
	float DistanceDivisor);
template VOXEL_API void UVoxelSurfaceTools::EditVoxelValuesImpl<true>(
	FVoxelData& Data,
	TArray<FModifiedVoxelValue>& OutModifiedVoxels,
	const FIntBox& Bounds,
	const TArray<FVoxelValueEdit>& Voxels,
	const FVoxelHardnessHandler& HardnessHandler,
	float DistanceDivisor);

void UVoxelSurfaceTools::EditVoxelValues2DImpl(
	FVoxelData& Data, 
	const FIntBox& Bounds, 
	const TArray<FVoxelValueEdit>& Voxels, 
	const FVoxelHardnessHandler& HardnessHandler, 
	float DistanceDivisor)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator Accelerator(Data, Bounds);

	for (auto& Voxel : Voxels)
	{
		const FVoxelValue Value = Accelerator.GetValue(Voxel.Position, 0);
		if (!ensureVoxelSlow(!Value.IsEmpty())) continue; // Shouldn't be empty if it's a 2D edit position
		const FVoxelValue ValueAbove = Accelerator.GetValue(Voxel.Position + FIntVector(0, 0, 1), 0);
		if (!ensureVoxelSlow(ValueAbove.IsEmpty())) continue;

		const float VoxelHeight = FVoxelUtilities::GetAbsDistanceFromDensities(Value.ToFloat(), ValueAbove.ToFloat());
		
		float Strength = Voxel.Strength;
		if (HardnessHandler.NeedsToCompute())
		{
			Strength /= HardnessHandler.GetHardness(Accelerator.GetMaterial(Voxel.Position, 0));
		}

		// -: we want a negative strength to add
		const float Height = Voxel.Position.Z + VoxelHeight - Strength;

		const bool bIsAdding = Strength < 0;

		const int32 A = FMath::FloorToInt(Voxel.Position.Z - DistanceDivisor);
		const int32 B = FMath::CeilToInt(Voxel.Position.Z + DistanceDivisor);
		const int32 C = FMath::FloorToInt(Height - DistanceDivisor);
		const int32 D = FMath::CeilToInt(Height + DistanceDivisor);
		const int32 Start = FMath::Min(FMath::Min(A, B), FMath::Min(C, D));
		const int32 End = FMath::Max(FMath::Max(A, B), FMath::Max(C, D));

		for (int32 Z = Start; Z <= End; Z++)
		{
			Accelerator.EditValue(Voxel.Position.X, Voxel.Position.Y, Z, [&](FVoxelValue& CurrentValue)
			{
				const float FloatValue = CurrentValue.ToFloat();
				const float WantedValue = (Z - Height) / DistanceDivisor;
				if (bIsAdding && WantedValue < FloatValue)
				{
					CurrentValue = FVoxelValue(WantedValue);
				}
				if (!bIsAdding && WantedValue > FloatValue)
				{
					CurrentValue = FVoxelValue(WantedValue);
				}
			});
		}
	}
}

void UVoxelSurfaceTools::EditVoxelValues(
	TArray<FModifiedVoxelValue>& ModifiedVoxels, 
	AVoxelWorld* World,
	const TArray<FVoxelValueEdit>& Voxels,
	float DistanceDivisor)
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
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelValuesImpl(Data, ModifiedVoxels, Bounds, Voxels, HardnessHandler, DistanceDivisor));
}

void UVoxelSurfaceTools::EditVoxelValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedVoxels,
	AVoxelWorld* World,
	const TArray<FVoxelValueEdit>& Voxels,
	float DistanceDivisor,
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

	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelValuesImpl(Data, InModifiedVoxels, Bounds, Voxels, HardnessHandler, DistanceDivisor));
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::DebugSurfaceVoxels(
	AVoxelWorld* World, 
	const TArray<FSurfaceVoxelWithStrength>& Voxels,
	float Lifetime)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	for (auto& Voxel : Voxels)
	{
		const FVector Position = World->LocalToGlobal(Voxel.Position);
		const FLinearColor Color = FMath::Lerp(
			FLinearColor::Black,
			Voxel.Strength > 0 ? FLinearColor::Red : FLinearColor::Green,
			FMath::Clamp(FMath::Abs(Voxel.Strength), 0.f, 1.f));
		DrawDebugPoint(
			World->GetWorld(),
			Position,
			World->VoxelSize / 20,
			Color.ToFColor(false),
			false,
			Lifetime);
	}
}