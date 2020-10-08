// Copyright 2020 Phyronnaz

#include "NodeFunctions/VoxelNodeFunctions.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "VoxelGenerators/VoxelGeneratorInstance.inl"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelMessages.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "Async/Async.h"

/** Util to find float value on bezier defined by 4 control points */
static TVoxelRange<v_flt> BezierInterp(v_flt P0, v_flt P1, v_flt P2, v_flt P3, const TVoxelRange<v_flt>& Alpha)
{
	const TVoxelRange<v_flt> P01 = FVoxelNodeFunctions::SafeLerp(P0, P1, Alpha);
	const TVoxelRange<v_flt> P12 = FVoxelNodeFunctions::SafeLerp(P1, P2, Alpha);
	const TVoxelRange<v_flt> P23 = FVoxelNodeFunctions::SafeLerp(P2, P3, Alpha);
	const TVoxelRange<v_flt> P012 = FVoxelNodeFunctions::SafeLerp(P01, P12, Alpha);
	const TVoxelRange<v_flt> P123 = FVoxelNodeFunctions::SafeLerp(P12, P23, Alpha);
	const TVoxelRange<v_flt> P0123 = FVoxelNodeFunctions::SafeLerp(P012, P123, Alpha);

	return P0123;
}

TVoxelRange<v_flt> EvalForTwoKeys(const FRichCurveKey& Key1, const FRichCurveKey& Key2, const TVoxelRange<v_flt>& InTime)
{
	const v_flt Diff = Key2.Time - Key1.Time;

	if (Diff > 0.f && Key1.InterpMode != RCIM_Constant)
	{
		const TVoxelRange<v_flt> Alpha = (InTime - Key1.Time) / Diff;
		const v_flt P0 = Key1.Value;
		const v_flt P3 = Key2.Value;

		if (Key1.InterpMode == RCIM_Linear)
		{
			return FVoxelNodeFunctions::SafeLerp(P0, P3, Alpha);
		}
		else
		{
			const v_flt OneThird = 1.0f / 3.0f;
			const v_flt P1 = P0 + (Key1.LeaveTangent * Diff * OneThird);
			const v_flt P2 = P3 - (Key2.ArriveTangent * Diff * OneThird);

			return BezierInterp(P0, P1, P2, P3, Alpha);
		}
	}
	else
	{
		return Key1.Value;
	}
}

inline void AddKeysBounds(const TArray<FRichCurveKey>& Keys, const TVoxelRange<v_flt>& Time, TArray<v_flt>& Bounds)
{
	for (int32 Index = 0; Index < Keys.Num() - 1; Index++)
	{
		auto& KeyA = Keys[Index];
		auto& KeyB = Keys[Index + 1];
		if (TVoxelRange<v_flt>(KeyA.Time, KeyB.Time).Intersects(Time))
		{
			auto Range = EvalForTwoKeys(KeyA, KeyB, FVoxelNodeFunctions::Clamp<v_flt>(Time, KeyA.Time, KeyB.Time));
			Bounds.Add(Range.Min);
			Bounds.Add(Range.Max);
		}
	}
}

inline v_flt GetInferiorInfinityValue(const FRichCurve& Curve, v_flt InTime)
{
	const auto& Keys = Curve.Keys;
	if (Curve.PreInfinityExtrap == RCCE_Linear)
	{
		const v_flt DT = Keys[1].Time - Keys[0].Time;

		if (FMath::IsNearlyZero(DT))
		{
			return Keys[0].Value;
		}
		else
		{
			const v_flt DV = Keys[1].Value - Keys[0].Value;
			const v_flt Slope = DV / DT;

			return Slope * (InTime - Keys[0].Time) + Keys[0].Value;
		}
	}
	else
	{
		// Otherwise if constant or in a cycle or oscillate, always use the first key value
		return Keys[0].Value;
	}
}

inline v_flt GetSuperiorInfinityValue(const FRichCurve& Curve, v_flt InTime)
{
	const auto& Keys = Curve.Keys;
	const int32 NumKeys = Keys.Num();
	if (Curve.PostInfinityExtrap == RCCE_Linear)
	{
		const v_flt DT = Keys[NumKeys - 2].Time - Keys[NumKeys - 1].Time;

		if (FMath::IsNearlyZero(DT))
		{
			return Keys[NumKeys - 1].Value;
		}
		else
		{
			const v_flt DV = Keys[NumKeys - 2].Value - Keys[NumKeys - 1].Value;
			const v_flt Slope = DV / DT;

			return Slope * (InTime - Keys[NumKeys - 1].Time) + Keys[NumKeys - 1].Value;
		}
	}
	else
	{
		// Otherwise if constant or in a cycle or oscillate, always use the last key value
		return Keys[NumKeys - 1].Value;
	}
}

TVoxelRange<v_flt> FVoxelNodeFunctions::GetCurveValue(const FVoxelRichCurve& VoxelCurve, const TVoxelRange<v_flt>& Time)
{
	auto& Curve = VoxelCurve.Curve;
	if (Time.IsSingleValue())
	{
		return FVoxelRichCurveUtilities::Eval(Curve, Time.GetSingleValue());
	}
	else
	{
		auto& Keys = Curve.GetConstRefOfKeys();
		const int32 NumKeys = Keys.Num();

		if (NumKeys == 0)
		{
			// If no keys in curve, return the Default value.
			return 0;
		}
		else if (NumKeys == 1)
		{
			return Keys[0].Value;
		}
		else
		{
			TArray<v_flt> Bounds;
			const bool bMinBelow = Time.Min <= Keys[0].Time;
			const bool bMaxBelow = Time.Max <= Keys[0].Time;
			const bool bMinAbove = Keys[NumKeys - 1].Time <= Time.Min;
			const bool bMaxAbove = Keys[NumKeys - 1].Time <= Time.Max;
			if (bMaxBelow)
			{
				ensure(bMinBelow);
				Bounds.Add(GetInferiorInfinityValue(Curve, Time.Min));
				Bounds.Add(GetInferiorInfinityValue(Curve, Time.Max));
			}
			else if (bMinBelow)
			{
				Bounds.Add(GetInferiorInfinityValue(Curve, Time.Min));
				AddKeysBounds(Keys, Time, Bounds);
			}
			else if (bMinAbove)
			{
				ensure(bMaxAbove);
				Bounds.Add(GetSuperiorInfinityValue(Curve, Time.Min));
				Bounds.Add(GetSuperiorInfinityValue(Curve, Time.Max));
			}
			else if (bMaxAbove)
			{
				Bounds.Add(GetSuperiorInfinityValue(Curve, Time.Max));
				AddKeysBounds(Keys, Time, Bounds);
			}
			else
			{
				ensure(!bMinBelow && !bMinAbove && !bMaxBelow && !bMaxAbove);
				AddKeysBounds(Keys, Time, Bounds);
			}

			v_flt Min = Bounds[0];
			v_flt Max = Bounds[0];
			for (int32 Index = 1; Index < Bounds.Num(); Index++)
			{
				Min = FMath::Min(Bounds[Index], Min);
				Max = FMath::Max(Bounds[Index], Max);
			}
			return
			{
				FMath::Clamp<v_flt>(Min, VoxelCurve.GetMin(), VoxelCurve.GetMax()),
				FMath::Clamp<v_flt>(Max, VoxelCurve.GetMin(), VoxelCurve.GetMax())
			};
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

v_flt FVoxelNodeFunctions::GetPreviousGeneratorValue(
	v_flt X, v_flt Y, v_flt Z, 
	const FVoxelContext& Context,
	const FVoxelGeneratorInstance* DefaultGenerator)
{
	if (Context.Items.IsEmpty())
	{
		if (DefaultGenerator)
		{
			return DefaultGenerator->GetValue(X, Y, Z, Context.LOD, Context.Items);
		}
		else
		{
			return 1;
		}
	}
	else
	{
		const auto NextStack = Context.Items.GetNextStack(Context.GetWorldX(), Context.GetWorldY(), Context.GetWorldZ());
		return NextStack.Get<v_flt>(X, Y, Z, Context.LOD);
	}
}

TVoxelRange<v_flt> FVoxelNodeFunctions::GetPreviousGeneratorValue(
	TVoxelRange<v_flt> X,
	TVoxelRange<v_flt> Y,
	TVoxelRange<v_flt> Z, 
	const FVoxelContextRange& Context,
	const FVoxelGeneratorInstance* DefaultGenerator)
{
	const FVoxelIntBox Bounds = FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	if (Context.Items.IsEmpty())
	{
		if (DefaultGenerator)
		{
			return DefaultGenerator->GetValueRange(Bounds, Context.LOD, Context.Items);
		}
		else
		{
			return 1;
		}
	}
	else
	{
		const auto NextStack = Context.Items.GetNextStack(Context.WorldBounds);
		if (NextStack.IsValid())
		{
			return NextStack.GetValueRange(Bounds, Context.LOD);
		}
		else
		{
			return {-1, 1};
		}
	}
}

FVoxelMaterial FVoxelNodeFunctions::GetPreviousGeneratorMaterial(
	v_flt X, v_flt Y, v_flt Z,
	const FVoxelContext& Context,
	const FVoxelGeneratorInstance* DefaultGenerator)
{
	if (Context.Items.IsEmpty())
	{
		if (DefaultGenerator)
		{
			return DefaultGenerator->GetMaterial(X, Y, Z, Context.LOD, Context.Items);
		}
		else
		{
			return FVoxelMaterial::Default();
		}
	}
	else
	{
		const auto NextStack = Context.Items.GetNextStack(Context.GetWorldX(), Context.GetWorldY(), Context.GetWorldZ());
		return NextStack.Get<FVoxelMaterial>(X, Y, Z, Context.LOD);
	}
}

v_flt FVoxelNodeFunctions::GetPreviousGeneratorCustomOutput(
	const FName& Name, 
	v_flt X, v_flt Y, v_flt Z,
	const FVoxelContext& Context,
	const FVoxelGeneratorInstance* DefaultGenerator)
{
	if (Context.Items.IsEmpty())
	{
		if (DefaultGenerator)
		{
			if (const auto Ptr = DefaultGenerator->CustomPtrs.Float.FindRef(Name))
			{
				return (DefaultGenerator->*Ptr)(X, Y, Z, Context.LOD, Context.Items);
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		const auto NextStack = Context.Items.GetNextStack(Context.GetWorldX(), Context.GetWorldY(), Context.GetWorldZ());
		return NextStack.GetCustomOutput<v_flt>(0, Name, X, Y, Z, Context.LOD);
	}
}

TVoxelRange<v_flt> FVoxelNodeFunctions::GetPreviousGeneratorCustomOutput(
	const FName& Name, 
	TVoxelRange<v_flt> X,
	TVoxelRange<v_flt> Y,
	TVoxelRange<v_flt> Z,
	const FVoxelContextRange& Context,
	const FVoxelGeneratorInstance* DefaultGenerator)
{
	const FVoxelIntBox Bounds = FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	if (Context.Items.IsEmpty())
	{
		if (DefaultGenerator)
		{
			if (const auto Ptr = DefaultGenerator->CustomPtrs.FloatRange.FindRef(Name))
			{
				return TVoxelRange<v_flt>((DefaultGenerator->*Ptr)(Bounds, Context.LOD, Context.Items));
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		const auto NextStack = Context.Items.GetNextStack(Context.WorldBounds);
		if (NextStack.IsValid())
		{
			return NextStack.GetCustomOutputRange<v_flt>(0, Name, Bounds, Context.LOD);
		}
		else
		{
			return TVoxelRange<v_flt>::Infinite();
		}
	}
}

v_flt FVoxelNodeFunctions::GetGeneratorCustomOutput(
	const FVoxelGeneratorInstance& Generator,
	const FName& Name,
	v_flt X, v_flt Y, v_flt Z,
	const FVoxelContext& Context)
{
	if (const auto Ptr = Generator.CustomPtrs.Float.FindRef(Name))
	{
		return (Generator.*Ptr)(X, Y, Z, Context.LOD, FVoxelItemStack(Context.Items.ItemHolder));
	}
	else
	{
		return 0;
	}
}

TVoxelRange<v_flt> FVoxelNodeFunctions::GetGeneratorCustomOutput(
	const FVoxelGeneratorInstance& Generator,
	const FName& Name,
	TVoxelRange<v_flt> X,
	TVoxelRange<v_flt> Y,
	TVoxelRange<v_flt> Z,
	const FVoxelContextRange& Context)
{
	const FVoxelIntBox Bounds = FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	if (const auto Ptr = Generator.CustomPtrs.FloatRange.FindRef(Name))
	{
		return TVoxelRange<v_flt>((Generator.*Ptr)(Bounds, Context.LOD, FVoxelItemStack(Context.Items.ItemHolder)));
	}
	else
	{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void ShowGeneratorMergeError()
{
	const auto Show = []()
	{
		FVoxelMessages::Error("More than 4 recursive calls to Generator Merge, exiting. Make sure you don't have recursive Generator Merge nodes.");
	};

	if (IsInGameThread())
	{
		Show();
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			Show();
		});
	}
}

TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>> FVoxelNodeFunctions::CreateGeneratorArray(const TArray<FVoxelGeneratorPicker>& Generators)
{
	thread_local int32 RecursionDepth = 0;
	struct FDepthGuard { FDepthGuard() { RecursionDepth++; } ~FDepthGuard() { RecursionDepth--; } } DepthGuard;

	check(RecursionDepth > 0);
	if (RecursionDepth > 4)
	{
		ShowGeneratorMergeError();
		return { MakeVoxelShared<FVoxelEmptyGeneratorInstance>() };
	}

	TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>> Result;
	for (auto& Picker : Generators)
	{
		Result.Add(Picker.GetInstance(true));
	}
	if (Result.Num() == 0)
	{
		Result.Add(MakeVoxelShared<FVoxelEmptyGeneratorInstance>());
	}
	return Result;
}

void FVoxelNodeFunctions::ComputeGeneratorsMerge(
	const EVoxelMaterialConfig MaterialConfig, 
	const float Tolerance,
	const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
	const TArray<FName>& FloatOutputsNames,
	const FVoxelContext& Context, 
	v_flt X, v_flt Y, v_flt Z,
	int32 Index0, float Alpha0,
	int32 Index1, float Alpha1,
	int32 Index2, float Alpha2,
	int32 Index3, float Alpha3,
	bool bComputeValue, bool bComputeMaterial, const TArray<bool>& ComputeFloatOutputs,
	v_flt& OutValue,
	FVoxelMaterial& OutMaterial, 
	TArray<v_flt, TInlineAllocator<128>>& OutFloatOutputs,
	int32& NumGeneratorsQueried)
{
	thread_local int32 RecursionDepth = 0;
	struct FDepthGuard { FDepthGuard() { RecursionDepth++; } ~FDepthGuard() { RecursionDepth--; } } DepthGuard;

	NumGeneratorsQueried = 0;
	
	check(RecursionDepth > 0);
	if (RecursionDepth > 4)
	{
		static TSet<TVoxelWeakPtr<FVoxelGeneratorInstance>> StaticInstances;
		if (!StaticInstances.Contains(InInstances[0]))
		{
			StaticInstances.Add(InInstances[0]);
			ShowGeneratorMergeError();
		}
		OutValue = 0;
		OutMaterial = FVoxelMaterial::Default();
		OutFloatOutputs.SetNum(FloatOutputsNames.Num());
		return;
	}
	
	check(InInstances.Num() > 0);

	const auto Items = Context.Items;
	
	Index0 = FMath::Clamp(Index0, 0, InInstances.Num() - 1);
	Index1 = FMath::Clamp(Index1, 0, InInstances.Num() - 1);
	Index2 = FMath::Clamp(Index2, 0, InInstances.Num() - 1);
	Index3 = FMath::Clamp(Index3, 0, InInstances.Num() - 1);

	if (Index0 == Index1) Alpha1 = 0;
	if (Index0 == Index2 || Index1 == Index2) Alpha2 = 0;
	if (Index0 == Index3 || Index1 == Index3 || Index2 == Index3) Alpha3 = 0;

	const float AlphaSum = 
		FMath::Max(0.f, Alpha0) + 
		FMath::Max(0.f, Alpha1) + 
		FMath::Max(0.f, Alpha2) + 
		FMath::Max(0.f, Alpha3);
	if (AlphaSum > 0) 
	{
		Alpha0 /= AlphaSum;
		Alpha1 /= AlphaSum;
		Alpha2 /= AlphaSum;
		Alpha3 /= AlphaSum;
	}

	TArray<const FVoxelGeneratorInstance*, TFixedAllocator<4>> Instances;
	TArray<float, TFixedAllocator<4>> Alphas;

	int32 BestIndex = 0;
	float BestAlpha = 0;

	const auto AddInput = [&](float Alpha, int32 Index)
	{
		if (Alpha >= Tolerance)
		{
			NumGeneratorsQueried++;
			Instances.Add(InInstances[Index].Get());
			Alphas.Add(Alpha);

			if (Alpha > BestAlpha)
			{
				BestIndex = Instances.Num() - 1;
				BestAlpha = Alpha;
			}
		}
	};
	AddInput(Alpha0, Index0);
	AddInput(Alpha1, Index1);
	AddInput(Alpha2, Index2);
	AddInput(Alpha3, Index3);

	if (Instances.Num() == 0)
	{
		ensure(Alpha0 < Tolerance && Alpha1 < Tolerance && Alpha2 < Tolerance && Alpha3 < Tolerance);
		Alpha0 = 1;
		AddInput(Alpha0, Index0);
	}

	const auto GetFloatOutput = [&](auto Lambda)
	{
		float Result = 0;
		for (int32 Index = 0; Index < Instances.Num(); Index++)
		{
			Result += Lambda(*Instances[Index]) * Alphas[Index];
		}
		return Result;
	};
	const auto GetMaterialOutput = [&]()
	{
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
		{
			FLinearColor Result(0, 0, 0, 0);
			for (int32 Index = 0; Index < Instances.Num(); Index++)
			{
				Result += Instances[Index]->GetMaterial(X, Y, Z, Context.LOD, Items).GetLinearColor() * Alphas[Index];
			}
			return FVoxelMaterial::CreateFromColor(Result);
		}
		case EVoxelMaterialConfig::SingleIndex:
		{
			return Instances[BestIndex]->GetMaterial(X, Y, Z, Context.LOD, Items);
		}
// TODO PLACEABLE ITEMS
#if 0
		case EVoxelMaterialConfig::DoubleIndex:
		{
			TArray<int32, TFixedAllocator<8>> NewIndices;
			TArray<float, TFixedAllocator<8>> NewAlphas;
			float BestData = 0;
			for (int32 Index = 0; Index < Instances.Num(); Index++)
			{
				const FVoxelMaterial InstanceMaterial = Instances[Index]->GetMaterial(X, Y, Z, Context.LOD, Items);
				const uint8 IndexA = InstanceMaterial.GetDoubleIndex_IndexA();
				const uint8 IndexB = InstanceMaterial.GetDoubleIndex_IndexB();
				const float Blend = InstanceMaterial.GetDoubleIndex_Blend_AsFloat();
				const float Data = InstanceMaterial.GetDoubleIndex_Data_AsFloat();

				if (Index == BestIndex)
				{
					BestData = Data;
				}
				NewIndices.Add(IndexA);
				NewIndices.Add(IndexB);
				NewAlphas.Add(Alphas[Index] * (1 - Blend));
				NewAlphas.Add(Alphas[Index] * Blend);
			}
			return CreateDoubleIndexMaterial(NewIndices, NewAlphas, BestData);
		}
#endif
		default:
			ensure(false);
			return FVoxelMaterial::Default();
		}
	};

	if (bComputeValue) 
	{
		OutValue = GetFloatOutput([&](const FVoxelGeneratorInstance& Instance) { return Instance.GetValue(X, Y, Z, Context.LOD, Items); });
	}
	if (bComputeMaterial)
	{
		OutMaterial = GetMaterialOutput();
	}
	
	OutFloatOutputs.SetNumUninitialized(FloatOutputsNames.Num());
	for (int32 Index = 0; Index < FloatOutputsNames.Num(); Index++)
	{
		if (ComputeFloatOutputs[Index]) 
		{
			OutFloatOutputs[Index] = GetFloatOutput([&](const FVoxelGeneratorInstance& Instance)
			{
				const auto Ptr = Instance.CustomPtrs.Float.FindRef(FloatOutputsNames[Index]);
				if (Ptr)
				{
					return (Instance.*Ptr)(X, Y, Z, Context.LOD, Items);
				}
				else
				{
					return v_flt(0);
				}
			});
		}
	}
}

void FVoxelNodeFunctions::ComputeGeneratorsMergeRange(
	const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
	const TArray<FName>& FloatOutputsNames,
	const FVoxelContextRange& Context, 
	const TVoxelRange<v_flt> X,
	const TVoxelRange<v_flt> Y,
	const TVoxelRange<v_flt> Z, 
	bool bComputeValue, const TArray<bool>& ComputeFloatOutputs,
	TVoxelRange<v_flt>& OutValue, 
	TArray<TVoxelRange<v_flt>, TInlineAllocator<128>>& OutFloatOutputs,
	TVoxelRange<int32>& NumGeneratorsQueried)
{
	thread_local int32 RecursionDepth = 0;
	struct FDepthGuard { FDepthGuard() { RecursionDepth++; } ~FDepthGuard() { RecursionDepth--; } } DepthGuard;

	NumGeneratorsQueried = { 0, 4 };
	
	check(RecursionDepth > 0);
	if (RecursionDepth > 4)
	{
		static TSet<TVoxelWeakPtr<FVoxelGeneratorInstance>> StaticInstances;
		if (!StaticInstances.Contains(InInstances[0]))
		{
			StaticInstances.Add(InInstances[0]);
			ShowGeneratorMergeError();
		}
		OutValue = 0;
		OutFloatOutputs.SetNum(FloatOutputsNames.Num());
		return;
	}

	const auto Items = Context.Items;
	
	const FVoxelIntBox Bounds = FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	OutFloatOutputs.SetNumUninitialized(FloatOutputsNames.Num());

	const auto ComputeFloatOutputsLambda = [&](auto& Instance, bool bUnion)
	{
		for (int32 OutputIndex = 0; OutputIndex < FloatOutputsNames.Num(); OutputIndex++)
		{
			if (ComputeFloatOutputs[OutputIndex]) 
			{
				TVoxelRange<v_flt> Result;
				const auto Ptr = Instance.CustomPtrs.FloatRange.FindRef(FloatOutputsNames[OutputIndex]);
				if (Ptr)
				{
					Result = (Instance.*Ptr)(Bounds, Context.LOD, Items);
				}
				else
				{
					Result = 0;
				}
				OutFloatOutputs[OutputIndex] = bUnion ? TVoxelRange<v_flt>::Union(OutFloatOutputs[OutputIndex], Result) : Result;
			}
		}
	};

	if (bComputeValue)
	{
		OutValue = InInstances[0]->GetValueRange(Bounds, Context.LOD, Items);
	}
	ComputeFloatOutputsLambda(*InInstances[0], false);

	for (int32 Index = 1; Index < InInstances.Num(); Index++)
	{
		if (bComputeValue)
		{
			OutValue = TVoxelRange<v_flt>::Union(OutValue, InInstances[Index]->GetValueRange(Bounds, Context.LOD, Items));
		}
		ComputeFloatOutputsLambda(*InInstances[Index], true);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRichCurve::FVoxelRichCurve(const FRichCurve& Curve)
	: Curve(Curve)
{
	Curve.GetValueRange(Min, Max);
}

FVoxelRichCurve::FVoxelRichCurve(const UCurveFloat* Curve)
	: FVoxelRichCurve(Curve ? Curve->FloatCurve : FRichCurve())
{
}

FVoxelColorRichCurve::FVoxelColorRichCurve(const UCurveLinearColor* Curve)
{
	if (Curve)
	{
		Curves[0] = FVoxelRichCurve(Curve->FloatCurves[0]);
		Curves[1] = FVoxelRichCurve(Curve->FloatCurves[1]);
		Curves[2] = FVoxelRichCurve(Curve->FloatCurves[2]);
		Curves[3] = FVoxelRichCurve(Curve->FloatCurves[3]);
	}
}