// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelSurfaceToolsImpl.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelUtilities/VoxelRichCurveUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"
#include "VoxelDirection.h"

#include "Curves/CurveFloat.h"
#include "Async/ParallelFor.h"
#include "DrawDebugHelpers.h"

bool FVoxelSurfaceEditsStack::HasErrors(const FVoxelSurfaceEditsVoxels& Voxels, FString& OutErrors) const
{
	OutErrors.Reset();
	
	for (const auto& Element : Stack)
	{
		if ((Element.Flags & EVoxelSurfaceEditsStackElementFlags::NeedValues) && !Voxels.Info.bHasValues)
		{
			OutErrors += FString::Printf(TEXT("%s needs values to be computed!\n"), *Element.Name);
		}
		
		if ((Element.Flags & EVoxelSurfaceEditsStackElementFlags::NeedNormals) && !Voxels.Info.bHasNormals)
		{
			OutErrors += FString::Printf(TEXT("%s needs normals to be computed!\n"), *Element.Name);
		}
		
		if ((Element.Flags & EVoxelSurfaceEditsStackElementFlags::ShouldBeLast) && &Element != &Stack.Last())
		{
			OutErrors += FString::Printf(TEXT("%s needs to be the last element of the stack!\n"), *Element.Name);
		}
	}

	return !OutErrors.IsEmpty();
}

FVoxelSurfaceEditsProcessedVoxels FVoxelSurfaceEditsStack::Execute(const FVoxelSurfaceEditsVoxels& Voxels, bool bComputeBounds) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	auto ProcessedVoxels = TArray<FVoxelSurfaceEditsVoxel>(*Voxels.Voxels);
	for (const auto& Element : Stack)
	{
		Element.Apply(Voxels.Info, ProcessedVoxels);
	}
	
	FVoxelSurfaceEditsProcessedVoxels Result;

	if (ProcessedVoxels.Num() > 0 && bComputeBounds)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("ComputeBounds");

		Result.Bounds = FVoxelIntBox(ProcessedVoxels[0].Position);
		for (auto& Voxel : ProcessedVoxels)
		{
			Result.Bounds = Result.Bounds + Voxel.Position;
		}
	}
	
	Result.Info = Voxels.Info;
	Result.Voxels = MakeVoxelSharedCopy(MoveTemp(ProcessedVoxels));
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<uint8 DirectionMask>
FVoxelSurfaceEditsVoxels UVoxelSurfaceTools::FindSurfaceVoxelsImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		bool bComputeNormals_Dynamic, 
		bool bOnlyOutputNonEmptyVoxels_Dynamic)
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

	TArray<FVoxelSurfaceEditsVoxelBase> OutVoxels;
	OutVoxels.Reserve(Bounds.Count());

	FVoxelUtilities::StaticBranch(bComputeNormals_Dynamic, bOnlyOutputNonEmptyVoxels_Dynamic, [&](auto bComputeNormals_Static, auto bOnlyOutputNonEmptyVoxels_Static)
	{
		for (int32 X = 1; X < Size.X - 1; X++)
		{
			for (int32 Y = 1; Y < Size.Y - 1; Y++)
			{
				for (int32 Z = 1; Z < Size.Z - 1; Z++)
				{
					const FVoxelValue Value = GetValue(X, Y, Z);
					if (bOnlyOutputNonEmptyVoxels_Static && Value.IsEmpty())
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
						if (bComputeNormals_Static)
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
	});

	FVoxelSurfaceEditsVoxels EditsVoxels;
	EditsVoxels.Info.bHasValues = true;
	EditsVoxels.Info.bHasExactDistanceField = false;
	EditsVoxels.Info.bHasNormals = bComputeNormals_Dynamic;
	EditsVoxels.Info.bIs2D = false;

	EditsVoxels.Voxels = MakeVoxelSharedCopy(MoveTemp(OutVoxels));
	
	return EditsVoxels;
}

template VOXEL_API FVoxelSurfaceEditsVoxels UVoxelSurfaceTools::FindSurfaceVoxelsImpl<0>(FVoxelData&, const FVoxelIntBox&, bool, bool);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceEditsVoxels UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const float MaxDistance,
	const bool bMultiThreaded)
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
	if (Size.X < 3 || Size.Y < 3 || Size.Z < 3) return {};

	TArray<FFloat16> DistanceField;
	DistanceField.SetNumUninitialized(Bounds.Count());
	const TArray<FVoxelValue> Values = Data.ParallelGet<FVoxelValue>(Bounds, !bMultiThreaded);

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Expand values to floats");
		ParallelFor(DistanceField.Num(), [&](int32 Index)
			{
				Get(DistanceField, Index) = Get(Values, Index).ToFloat();
			}, !bMultiThreaded);
	}

	FVoxelDistanceFieldUtilities::ComputeDistanceField_GPU(Size, DistanceField, EVoxelDistanceFieldInputType::Densities, FMath::CeilToInt(MaxDistance));

	VOXEL_ASYNC_SCOPE_COUNTER("Create OutVoxels");
	TArray<FVoxelSurfaceEditsVoxelBase> OutVoxels;
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
				Get(OutVoxels, OutVoxelsIndex) = FVoxelSurfaceEditsVoxelBase{ Bounds.Min + Position, FVector(0), Distance };
			}
		}
	}, !bMultiThreaded);
	OutVoxels.SetNum(Count.GetValue(), false);
	
	FVoxelSurfaceEditsVoxels EditsVoxels;
	EditsVoxels.Info.bHasValues = true;
	EditsVoxels.Info.bHasExactDistanceField = true;
	EditsVoxels.Info.bHasNormals = false;
	EditsVoxels.Info.bIs2D = false;

	EditsVoxels.Voxels = MakeVoxelSharedCopy(MoveTemp(OutVoxels));
	
	return EditsVoxels;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceEditsVoxels UVoxelSurfaceTools::FindSurfaceVoxels2DImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, bool bComputeNormals)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

	FVoxelSurfaceEditsVoxels EditsVoxels = FindSurfaceVoxelsImpl<EVoxelDirection::ZMax>(Data, Bounds, bComputeNormals);
	
	TMap<FIntPoint, FVoxelSurfaceEditsVoxelBase> Columns;
	Columns.Reserve(Bounds.Size().X * Bounds.Size().Y);
	
	for (const auto& Voxel : *EditsVoxels.Voxels)
	{
		const FIntPoint Point(Voxel.Position.X, Voxel.Position.Y);
		if (auto* Existing = Columns.Find(Point))
		{
			if (Existing->Position.Z < Voxel.Position.Z)
			{
				*Existing = Voxel;
			}
		}
		else
		{
			Columns.Add(Point, Voxel);
		}
	}

	TArray<FVoxelSurfaceEditsVoxelBase> OutVoxels;
	Columns.GenerateValueArray(OutVoxels);
	
	EditsVoxels.Voxels = MakeVoxelSharedCopy(MoveTemp(OutVoxels));
	EditsVoxels.Info.bIs2D = true;
	
	return EditsVoxels;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxels(
	FVoxelSurfaceEditsVoxels& Voxels,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bComputeNormals)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, Voxels = FindSurfaceVoxelsImpl(Data, Bounds, bComputeNormals));
}

void UVoxelSurfaceTools::FindSurfaceVoxelsAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	FVoxelSurfaceEditsVoxels& Voxels,
	AVoxelWorld* World, 
	FVoxelIntBox Bounds,
	bool bComputeNormals,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, InVoxels = FindSurfaceVoxelsImpl(Data, Bounds, bComputeNormals));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceField(
	FVoxelSurfaceEditsVoxels& Voxels,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	float MaxDistance,
	bool bMultiThreaded)
{
	// TODO compute normals
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, Voxels = FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, MaxDistance, bMultiThreaded));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::FindSurfaceVoxels2D(
	FVoxelSurfaceEditsVoxels& Voxels,
	AVoxelWorld* World, 
	FVoxelIntBox Bounds,
	bool bComputeNormals)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, Voxels = FindSurfaceVoxels2DImpl(Data, Bounds, bComputeNormals));
}

void UVoxelSurfaceTools::FindSurfaceVoxels2DAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	FVoxelSurfaceEditsVoxels& Voxels, 
	AVoxelWorld* World, 
	FVoxelIntBox Bounds,
	bool bComputeNormals, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, InVoxels = FindSurfaceVoxels2DImpl(Data, Bounds, bComputeNormals));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceEditsStack UVoxelSurfaceTools::AddToStack(FVoxelSurfaceEditsStack Stack, FVoxelSurfaceEditsStackElement Element)
{
	Stack.Add(Element);
	return Stack;
}

FVoxelSurfaceEditsProcessedVoxels UVoxelSurfaceTools::ApplyStack(FVoxelSurfaceEditsVoxels Voxels, FVoxelSurfaceEditsStack Stack)
{
	FString Error;
	if (Stack.HasErrors(Voxels, Error))
	{
		FVoxelMessages::Warning(FString::Printf(TEXT("ApplyStack: %s"), *Error));
	}
	return Stack.Execute(Voxels);
}

void UVoxelSurfaceTools::ApplyStackAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	FVoxelSurfaceEditsVoxels Voxels, 
	FVoxelSurfaceEditsStack Stack, 
	bool bHideLatentWarnings)
{
	FString Error;
	if (Stack.HasErrors(Voxels, Error))
	{
		FVoxelMessages::Warning(FString::Printf(TEXT("ApplyStackAsync: %s"), *Error));
	}

	FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		ProcessedVoxels,
		[=](FVoxelSurfaceEditsProcessedVoxels& InProcessedVoxels)
		{
			InProcessedVoxels = Stack.Execute(Voxels);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyConstantStrength(float Strength)
{
	return
	{
		"ApplyConstantStrength",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyConstantStrengthImpl(Voxels, Strength);
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyStrengthCurve(
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	UCurveFloat* StrengthCurve,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	if (!StrengthCurve)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Strength Curve!"));
		return {};
	}
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	
	return
	{
		"ApplyStrengthCurve",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyDistanceStrengthFunctionImpl(Voxels, Center, Info.bIs2D, [=](float Distance)
			{
				return FVoxelRichCurveUtilities::Eval(StrengthCurve->FloatCurve, Distance / Radius);
			});
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyLinearFalloff(
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);

	return
	{
		"ApplyLinearFalloff",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyDistanceStrengthFunctionImpl(Voxels, Center, Info.bIs2D, [=](float Distance)
			{
				return FVoxelUtilities::LinearFalloff(Distance, Radius, Falloff);
			});
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplySmoothFalloff(
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	return
	{
		"ApplySmoothFalloff",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyDistanceStrengthFunctionImpl(Voxels, Center, Info.bIs2D, [=](float Distance)
			{
				return FVoxelUtilities::SmoothFalloff(Distance, Radius, Falloff);
			});
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplySphericalFalloff(
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	return
	{
		"ApplySphericalFalloff",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyDistanceStrengthFunctionImpl(Voxels, Center, Info.bIs2D, [=](float Distance)
			{
				return FVoxelUtilities::SphericalFalloff(Distance, Radius, Falloff);
			});
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyTipFalloff(
	AVoxelWorld* World,
	FVector InCenter,
	float InRadius,
	float InFalloff,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	const FVoxelVector Center = GET_VOXEL_TOOL_REAL(InCenter);
	const float Radius = GET_VOXEL_TOOL_REAL(InRadius);
	const float Falloff = GET_VOXEL_TOOL_REAL(InFalloff);
	
	return
	{
		"ApplyTipFalloff",
		EVoxelSurfaceEditsStackElementFlags::None,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyDistanceStrengthFunctionImpl(Voxels, Center, Info.bIs2D, [=](float Distance)
			{
				return FVoxelUtilities::TipFalloff(Distance, Radius, Falloff);
			});
		}
	};
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyStrengthMask(
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

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyTerrace(
	int32 TerraceHeightInVoxels,
	float Angle,
	int32 ImmutableVoxels)
{
	VOXEL_PRO_ONLY();
}

FVoxelSurfaceEditsStackElement UVoxelSurfaceTools::ApplyFlatten(
	AVoxelWorld* World,
	FVector PlanePoint,
	FVector PlaneNormal,
	EVoxelSDFMergeMode MergeMode,
	bool bConvertToVoxelSpace)
{
	CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE();
	
	PlanePoint = GET_VOXEL_TOOL_REAL(PlanePoint).ToFloat();

	if (bConvertToVoxelSpace)
	{
		PlaneNormal = World->GetActorTransform().InverseTransformVector(PlaneNormal).GetSafeNormal();
	}
	
	return
	{
		"ApplyFlatten",
		EVoxelSurfaceEditsStackElementFlags::NeedValues | EVoxelSurfaceEditsStackElementFlags::ShouldBeLast,
		[=](auto& Info, auto& Voxels)
		{
			FVoxelSurfaceToolsImpl::ApplyFlattenImpl(Info, Voxels, FPlane(PlanePoint, PlaneNormal.GetSafeNormal()), MergeMode);
		}
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::EditVoxelValuesImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
	const FVoxelHardnessHandler& HardnessHandler,
	float DistanceDivisor,
	bool bHasValues,
	TArray<FModifiedVoxelValue>* OutModifiedVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		OctreeAccelerator.EditValue(Voxel.Position, [&](FVoxelValue& Value)
		{
			const float OldValue = bHasValues ? Voxel.Value : Value.ToFloat();
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
			
			if (OutModifiedVoxels)
			{
				FModifiedVoxelValue ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldValue = OldValue;
				ModifiedVoxel.NewValue = NewValue;
				OutModifiedVoxels->Add(ModifiedVoxel);
			}
		});
	}
}

void UVoxelSurfaceTools::EditVoxelValues2DImpl(
	FVoxelData& Data, 
	const FVoxelIntBox& Bounds, 
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels, 
	const FVoxelHardnessHandler& HardnessHandler, 
	float DistanceDivisor,
	TArray<FModifiedVoxelValue>* OutModifiedVoxels)
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
				if ((bIsAdding && WantedValue < FloatValue) ||
					(!bIsAdding && WantedValue > FloatValue))
				{
					if (OutModifiedVoxels)
					{
						FModifiedVoxelValue ModifiedVoxel;
						ModifiedVoxel.Position = Voxel.Position;
						ModifiedVoxel.OldValue = CurrentValue.ToFloat();
						ModifiedVoxel.NewValue = WantedValue;
						OutModifiedVoxels->Add(ModifiedVoxel);
					}
					CurrentValue = FVoxelValue(WantedValue);
				}
			});
		}
	}
}

void UVoxelSurfaceTools::EditVoxelValuesImpl(
	FVoxelData& Data, 
	const FVoxelIntBox& Bounds,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels, 
	const FVoxelHardnessHandler& HardnessHandler, 
	float DistanceDivisor, 
	TArray<FModifiedVoxelValue>* OutModifiedVoxels)
{
	if (ProcessedVoxels.Info.bIs2D)
	{
		EditVoxelValues2DImpl(Data, Bounds, *ProcessedVoxels.Voxels, HardnessHandler, DistanceDivisor, OutModifiedVoxels);
	}
	else
	{
		EditVoxelValuesImpl(Data, Bounds, *ProcessedVoxels.Voxels, HardnessHandler, DistanceDivisor, ProcessedVoxels.Info.bHasValues, OutModifiedVoxels);
	}
}

void UVoxelSurfaceTools::EditVoxelValues(
	TArray<FModifiedVoxelValue>& ModifiedVoxels, 
	AVoxelWorld* World,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
	float DistanceDivisor)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;
	const FVoxelHardnessHandler HardnessHandler(*World);

	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelValuesImpl(Data, Bounds, ProcessedVoxels, HardnessHandler, DistanceDivisor, &ModifiedVoxels));
}

void UVoxelSurfaceTools::EditVoxelValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedVoxels,
	AVoxelWorld* World,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
	float DistanceDivisor,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;
	const FVoxelHardnessHandler HardnessHandler(*World);
	
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelValuesImpl(Data, Bounds, ProcessedVoxels, HardnessHandler, DistanceDivisor, &InModifiedVoxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::EditVoxelMaterialsImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const FVoxelPaintMaterial& PaintMaterial,
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
	TArray<FModifiedVoxelMaterial>* OutModifiedVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		if (Data.IsInWorld(Voxel.Position))
		{
			const FVoxelMaterial OldMaterial = OctreeAccelerator.GetMaterial(Voxel.Position, 0);
			FVoxelMaterial NewMaterial = OldMaterial;
			PaintMaterial.ApplyToMaterial(NewMaterial, Voxel.Strength);
			OctreeAccelerator.SetMaterial(Voxel.Position, NewMaterial);

			if (OutModifiedVoxels)
			{
				FModifiedVoxelMaterial ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldMaterial = OldMaterial;
				ModifiedVoxel.NewMaterial = NewMaterial;
				OutModifiedVoxels->Add(ModifiedVoxel);
			}
		}
	}
}

void UVoxelSurfaceTools::EditVoxelMaterials(
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
	AVoxelWorld* World,
	const FVoxelPaintMaterial& PaintMaterial,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;
	
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, UpdateRender, EditVoxelMaterialsImpl(Data, Bounds, PaintMaterial, *ProcessedVoxels.Voxels, &ModifiedVoxels));
}

void UVoxelSurfaceTools::EditVoxelMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
	AVoxelWorld* World,
	const FVoxelPaintMaterial& PaintMaterial,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		ModifiedVoxels.Reset();
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;

	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, UpdateRender, NO_PREFIX, EditVoxelMaterialsImpl(Data, Bounds, PaintMaterial, *ProcessedVoxels.Voxels, &InModifiedVoxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::EditVoxelFoliageImpl(
	FVoxelData& Data,
	const FVoxelIntBox& Bounds,
	const EVoxelRGBA Layer,
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
	TArray<FModifiedVoxelFoliage>* OutModifiedVoxels)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Voxel : Voxels)
	{
		if (Data.IsInWorld(Voxel.Position))
		{
			const FVoxelFoliage OldFoliage = OctreeAccelerator.Get<FVoxelFoliage>(Voxel.Position, 0);
			const float OldValue = OldFoliage.IsChannelSet(Layer) ? OldFoliage.GetChannelValue(Layer) : 0;
			
			FVoxelFoliage NewFoliage = OldFoliage;
			NewFoliage.SetChannelValue(Layer, FMath::Lerp(OldValue, 1.f, Voxel.Strength));

			OctreeAccelerator.Set<FVoxelFoliage>(Voxel.Position, NewFoliage);

			if (OutModifiedVoxels)
			{
				FModifiedVoxelFoliage ModifiedVoxel;
				ModifiedVoxel.Position = Voxel.Position;
				ModifiedVoxel.OldFoliage = OldFoliage;
				ModifiedVoxel.NewFoliage = NewFoliage;
				OutModifiedVoxels->Add(ModifiedVoxel);
			}
		}
	}
}

void UVoxelSurfaceTools::EditVoxelFoliage(
	TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
	AVoxelWorld* World,
	EVoxelRGBA Layer,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;

	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Write, DoNotUpdateRender, EditVoxelFoliageImpl(Data, Bounds, Layer, *ProcessedVoxels.Voxels, &ModifiedVoxels));
}

void UVoxelSurfaceTools::EditVoxelFoliageAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelFoliage>& ModifiedVoxels,
	AVoxelWorld* World,
	EVoxelRGBA Layer,
	FVoxelSurfaceEditsProcessedVoxels ProcessedVoxels,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (ProcessedVoxels.Voxels->Num() == 0)
	{
		return;
	}

	const FVoxelIntBox Bounds = ProcessedVoxels.Bounds;
	
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(ModifiedVoxels, Write, DoNotUpdateRender, NO_PREFIX, EditVoxelFoliageImpl(Data, Bounds, Layer, *ProcessedVoxels.Voxels, &InModifiedVoxels));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceTools::DebugSurfaceVoxels(
	AVoxelWorld* World, 
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	float Lifetime)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	for (auto& Voxel : *ProcessedVoxels.Voxels)
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