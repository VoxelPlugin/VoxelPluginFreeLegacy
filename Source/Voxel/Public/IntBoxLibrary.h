// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IntBox.h"
#include "VoxelMessages.h"
#include "IntBoxLibrary.generated.h"

UCLASS()
class UIntBoxLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Makes an Int Box. Min must be <= to Max */
	UFUNCTION(BlueprintPure, Category="Math|IntBox", meta=(Keywords="construct build", NativeMakeFunc))
	static FIntBox MakeIntBox(FIntVector Min, FIntVector Max)
	{
		if (Min.X >= Max.X ||
			Min.Y >= Max.Y ||
			Min.Z >= Max.Z)
		{
			FVoxelMessages::Error(FString::Printf(TEXT("MakeIntBox: Min should be < to Max! Min: %s; Max: %s"), *Min.ToString(), *Max.ToString()));
		}
		return FIntBox::SafeConstruct(Min, Max);
	}
	
	/** Breaks an Int Box */
	UFUNCTION(BlueprintPure, Category="Math|IntBox", meta=(NativeBreakFunc))
	static void BreakIntBox(FIntBox Box, FIntVector& Min, FIntVector& Max)
	{
		Min = Box.Min;
		Max = Box.Max;
	}

	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox InfiniteBox()
	{
		return FIntBox::Infinite;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox TranslateBox(FIntBox Box, FIntVector Position)
	{
		return Box.Translate(Position);
	}
	// Will move the box so that GetCenter = 0,0,0. Will extend it if its size is odd
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox Center(FIntBox Box)
	{
		return Box.Center();
	}
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox RemoveTranslation(FIntBox Box)
	{
		return Box.RemoveTranslation();
	}

	UFUNCTION(BlueprintPure, meta=(DisplayName = "ToString (IntBox)", CompactNodeTitle = "->", BlueprintAutocast), Category="Utilities|String")
	static FString Conv_IntBoxToString(FIntBox IntBox)
	{
		return IntBox.ToString();
	}

	// From -Radius(included) to Radius(excluded)
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox MakeBoxFromLocalPositionAndRadius(FIntVector Position, int32 Radius)
	{
		return FIntBox(FIntVector(-Radius), FIntVector(Radius)).Translate(Position);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool IsIntVectorInsideBox(FIntBox Box, FIntVector Position)
	{
		return Box.Contains(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool IsVectorInsideBox(FIntBox Box, FVector Position)
	{
		return Box.ContainsFloat(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntVector GetSize(FIntBox Box)
	{
		return Box.Size();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FVector GetCenter(FIntBox Box)
	{
		return Box.GetCenter().ToFloat();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static TArray<FIntVector> GetCorners(FIntBox Box)
	{
		return TArray<FIntVector>(Box.GetCorners(0));
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool Intersect(FIntBox Box, FIntBox Other)
	{
		return Box.Intersect(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool Contains(FIntBox Box, FIntBox Other)
	{
		return Box.Contains(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool IsValid(FIntBox Box)
	{
		return Box.IsValid();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox Overlap(FIntBox A, FIntBox B)
	{
		return A.Overlap(B);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox Extend(FIntBox Box, int32 Extent)
	{
		return Box.Extend(Extent);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox Extend_IntVector(FIntBox Box, FIntVector Extent)
	{
		return Box.Extend(Extent);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox ApplyTransform(FIntBox Box, FTransform Transform)
	{
		return Box.ApplyTransform(Transform);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "+"))
	static FIntBox AddPoint(FIntBox Box, FIntVector Point)
	{
		return Box + Point;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "+"))
	static FIntBox AddBox(FIntBox Box, FIntBox BoxToAdd)
	{
		return Box + BoxToAdd;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "x"))
	static FIntBox Scale(FIntBox Box, int32 Scale)
	{
		return Box * Scale;
	}
	
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (IntBox)", CompactNodeTitle = "==", Keywords = "== equal"), Category="Math|IntBox")
	static bool EqualEqual_IntBoxIntBox(FIntBox A, FIntBox B)
	{
		return A == B;
	}

	UFUNCTION(BlueprintPure, meta=(DisplayName = "NotEqual (IntBox)", CompactNodeTitle = "!=", Keywords = "!= not equal"), Category="Math|IntBox")
	static bool NotEqual_IntBoxIntBox(FIntBox A, FIntBox B)
	{
		return A != B;
	}
};

