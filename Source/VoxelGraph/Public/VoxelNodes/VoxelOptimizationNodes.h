// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "Curves/CurveFloat.h"
#include "VoxelOptimizationNodes.generated.h"

// Nodes before this won't be computed for range analysis
UCLASS(DisplayName = "Static Clamp", Category = "Optimization", meta = (Keywords = "range analysis"))
class VOXELGRAPH_API UVoxelNode_StaticClampFloat : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Static Clamp")
	float Min = 0;

	UPROPERTY(EditAnywhere, Category = "Static Clamp")
	float Max = 0;

	UVoxelNode_StaticClampFloat();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};

// Use this to debug the range of a value. Will plot the runtime values in a graph
UCLASS(DisplayName = "Range Analysis Debugger", Category = "Optimization")
class VOXELGRAPH_API UVoxelNode_RangeAnalysisDebuggerFloat : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Bounds")
	float Min = 0;

	UPROPERTY(VisibleAnywhere, Category = "Bounds")
	float Max = 0;

public:
	UPROPERTY(EditAnywhere, Category = "Graph")
	float GraphMin = -100;
	
	UPROPERTY(EditAnywhere, Category = "Graph")
	float GraphMax = 100;

	UPROPERTY(EditAnywhere, Category = "Graph", meta = (ClampMin = 0.001))
	float GraphStep = 1;

	UPROPERTY(EditAnywhere, Category = "Graph")
	FRuntimeFloatCurve Curve;
	
	struct FVoxelBins
	{
		const float Min;
		const float Max;
		const float Step;

		float MinValue = 0;
		float MaxValue = 0;
		bool bMinMaxInit = false;
		TArray<uint64> Counts;

		FVoxelBins(float Min, float Max, float Step)
			: Min(Min)
			, Max(Max)
			, Step(Step)
		{
			const int32 Num = FMath::CeilToInt((Max - Min) / Step);
			Counts.SetNum(Num);
		}
		FVoxelBins(const FVoxelBins& Other)
			: FVoxelBins(Other.Min, Other.Max, Other.Step)
		{
			MinValue = Other.MinValue;
			MaxValue = Other.MaxValue;
			bMinMaxInit = Other.bMinMaxInit;
			Counts = Other.Counts;
		}

		inline void AddStat(float Value)
		{
			FScopeLock Lock(&Section);
			AddToMinMax(Value);
			const int32 Bin = FMath::FloorToInt((Value - Min) / Step);
			if (Counts.IsValidIndex(Bin))
			{
				Counts[Bin]++;
			}
		}

		inline void AddOtherBins(const FVoxelBins& Other)
		{
			FScopeLock Lock(&Section);
			if (!ensure(Min == Other.Min && Max == Other.Max && Step == Other.Step && Counts.Num() == Other.Counts.Num()))
			{
				return;
			}
			for (int32 Index = 0; Index < Counts.Num(); Index++)
			{
				Counts[Index] += Other.Counts[Index];
			}
			AddToMinMax(Other.MinValue);
			AddToMinMax(Other.MaxValue);
		}

		inline void AddToCurve(FRichCurve& InCurve) const
		{
			for (int32 Index = 0; Index < Counts.Num(); Index++)
			{
				const float Value = Min + Index * Step;
				InCurve.AddKey(Value, Counts[Index]);
			}
		}

	public:
		inline void AddToMinMax(float Value)
		{
			if (!bMinMaxInit)
			{
				bMinMaxInit = true;
				MinValue = Value;
				MaxValue = Value;
			}
			else
			{
				MinValue = FMath::Min(MinValue, Value);
				MaxValue = FMath::Max(MaxValue, Value);
			}
		}

		FCriticalSection Section;
	};

	TUniquePtr<FVoxelBins> Bins = MakeUnique<FVoxelBins>(GraphMin, GraphMax, GraphStep);

	UVoxelNode_RangeAnalysisDebuggerFloat();

	void UpdateFromBin();
	void UpdateGraph();
	void Reset();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
#endif
};

// Runs a for loop
UCLASS(DisplayName = "Stress", Category = "Optimization")
class VOXELGRAPH_API UVoxelNode_Sleep : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 NumberOfLoops = 1000;

	UVoxelNode_Sleep();
};

// In range analysis, does the union of the inputs ranges. In other modes, returns 0
UCLASS(DisplayName = "Range Union", Category = "Optimization")
class VOXELGRAPH_API UVoxelNode_RangeUnion : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_RangeUnion();
};