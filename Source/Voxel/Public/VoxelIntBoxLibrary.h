// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelIntBox.h"
#include "VoxelMessages.h"
#include "VoxelIntBoxLibrary.generated.h"

UCLASS()
class UVoxelIntBoxLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Makes an Int Box. Min must be <= to Max */
	UFUNCTION(BlueprintPure, Category="Math|VoxelIntBox", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelIntBox MakeIntBox(FIntVector Min, FIntVector Max)
	{
		if (Min.X >= Max.X ||
			Min.Y >= Max.Y ||
			Min.Z >= Max.Z)
		{
			FVoxelMessages::Error(FString::Printf(TEXT("MakeIntBox: Min should be < to Max! Min: %s; Max: %s"), *Min.ToString(), *Max.ToString()));
		}
		return FVoxelIntBox::SafeConstruct(Min, Max);
	}
	
	/** Breaks an Int Box */
	UFUNCTION(BlueprintPure, Category="Math|VoxelIntBox", meta=(NativeBreakFunc))
	static void BreakIntBox(FVoxelIntBox Box, FIntVector& Min, FIntVector& Max)
	{
		Min = Box.Min;
		Max = Box.Max;
	}

public:
	UFUNCTION(BlueprintPure, Category="Math|VoxelIntBox", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelIntBoxWithValidity MakeIntBoxWithValidity(FVoxelIntBox Box, bool bIsValid = true)
	{
		if (bIsValid)
		{
			return Box;
		}
		else
		{
			return {};
		}
	}
	UFUNCTION(BlueprintPure, Category="Math|VoxelIntBox", meta=(NativeBreakFunc))
	static void BreakIntBoxWithValidity(FVoxelIntBoxWithValidity BoxWithValidity, FVoxelIntBox& Box, bool& bIsValid)
	{
		if (BoxWithValidity.IsValid())
		{
			bIsValid = true;
			Box = BoxWithValidity.GetBox();
		}
		else
		{
			bIsValid = false;
			Box = {};
		}
	}

public:
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox InfiniteBox()
	{
		return FVoxelIntBox::Infinite;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox TranslateBox(FVoxelIntBox Box, FIntVector Position)
	{
		return Box.Translate(Position);
	}
	// Will move the box so that GetCenter = 0,0,0. Will extend it if its size is odd
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox Center(FVoxelIntBox Box)
	{
		return Box.Center();
	}
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox RemoveTranslation(FVoxelIntBox Box)
	{
		return Box.RemoveTranslation();
	}

	UFUNCTION(BlueprintPure, meta=(DisplayName = "ToString (VoxelIntBox)", CompactNodeTitle = "->", BlueprintAutocast), Category="Utilities|String")
	static FString Conv_IntBoxToString(FVoxelIntBox IntBox)
	{
		return IntBox.ToString();
	}

	// From -Radius(included) to Radius(excluded)
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox MakeBoxFromLocalPositionAndRadius(FIntVector Position, int32 Radius)
	{
		return FVoxelIntBox(FIntVector(-Radius), FIntVector(Radius)).Translate(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox MakeBoxFromPositionAndRadius(FVector Position, float Radius)
	{
		Radius = FMath::Max(0.f, Radius);
		return FVoxelIntBox(FVoxelUtilities::FloorToInt(Position - Radius), FVoxelUtilities::CeilToInt(Position + Radius));
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static bool IsIntVectorInsideBox(FVoxelIntBox Box, FIntVector Position)
	{
		return Box.Contains(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static bool IsVectorInsideBox(FVoxelIntBox Box, FVector Position)
	{
		return Box.ContainsFloat(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FIntVector GetSize(FVoxelIntBox Box)
	{
		return Box.Size();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVector GetCenter(FVoxelIntBox Box)
	{
		return Box.GetCenter().ToFloat();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static TArray<FIntVector> GetCorners(FVoxelIntBox Box)
	{
		return TArray<FIntVector>(Box.GetCorners(0));
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static bool Intersect(FVoxelIntBox Box, FVoxelIntBox Other)
	{
		return Box.Intersect(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static bool Contains(FVoxelIntBox Box, FVoxelIntBox Other)
	{
		return Box.Contains(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static bool IsValid(FVoxelIntBox Box)
	{
		return Box.IsValid();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox Overlap(FVoxelIntBox A, FVoxelIntBox B)
	{
		return A.Overlap(B);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox Extend(FVoxelIntBox Box, int32 Extent)
	{
		return Box.Extend(Extent);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox Extend_IntVector(FVoxelIntBox Box, FIntVector Extent)
	{
		return Box.Extend(Extent);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox")
	static FVoxelIntBox ApplyTransform(FVoxelIntBox Box, FTransform Transform)
	{
		return Box.ApplyTransform(Transform);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox", meta = (CompactNodeTitle = "+"))
	static FVoxelIntBox AddPoint(FVoxelIntBox Box, FIntVector Point)
	{
		return Box + Point;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox", meta = (CompactNodeTitle = "+"))
	static FVoxelIntBox AddBox(FVoxelIntBox Box, FVoxelIntBox BoxToAdd)
	{
		return Box + BoxToAdd;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|VoxelIntBox", meta = (CompactNodeTitle = "x"))
	static FVoxelIntBox Scale(FVoxelIntBox Box, int32 Scale)
	{
		return Box * Scale;
	}
	
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (VoxelIntBox)", CompactNodeTitle = "==", Keywords = "== equal"), Category="Math|VoxelIntBox")
	static bool EqualEqual_IntBoxIntBox(FVoxelIntBox A, FVoxelIntBox B)
	{
		return A == B;
	}

	UFUNCTION(BlueprintPure, meta=(DisplayName = "NotEqual (VoxelIntBox)", CompactNodeTitle = "!=", Keywords = "!= not equal"), Category="Math|VoxelIntBox")
	static bool NotEqual_IntBoxIntBox(FVoxelIntBox A, FVoxelIntBox B)
	{
		return A != B;
	}
	
	UFUNCTION(BlueprintPure, Category="Math|VoxelIntBox", meta=(Keywords="construct build", NativeMakeFunc))
	static FVoxelIntBox MakeIntBoxFromPoints(TArray<FVector> Points)
	{
		if (Points.Num() == 0)
		{
			FVoxelMessages::Error(FUNCTION_ERROR("No points!"));
			Points.Add(FVector::ZeroVector);
		}

		if (Points.Num() == 2)
		{
			return FVoxelIntBox::SafeConstruct(Points[0], Points[1]);
		}
		else
		{
			return FVoxelIntBox(Points);
		}
	}
};