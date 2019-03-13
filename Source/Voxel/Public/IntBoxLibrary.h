// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IntBox.h"
#include "IntBoxLibrary.generated.h"

UCLASS()
class UIntBoxLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox TranslateBox(const FIntBox& Box, const FIntVector& Position)
	{
		return Box.TranslateBy(Position);
	}

	UFUNCTION(BlueprintPure, meta=(DisplayName = "ToString (IntBox)", CompactNodeTitle = "->", BlueprintAutocast), Category="Utilities|String")
	static FString Conv_IntBoxToString(const FIntBox& IntBox)
	{
		return IntBox.ToString();
	}

	// From -Radius(included) to Radius(excluded)
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox MakeBoxFromLocalPositionAndRadius(const FIntVector& Position, int Radius)
	{
		return FIntBox(FIntVector(-Radius), FIntVector(Radius)).TranslateBy(Position);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool IsIntVectorInsideBox(const FIntBox& Box, const FIntVector& Position)
	{
		return Box.IsInside(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool IsVectorInsideBox(const FIntBox& Box, const FVector& Position)
	{
		return Box.IsInside(Position);
	}

	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntVector GetSize(const FIntBox& Box)
	{
		return Box.Size();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FVector GetCenter(const FIntBox& Box)
	{
		return Box.GetCenter();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static TArray<FIntVector> GetCorners(const FIntBox& Box)
	{
		return Box.GetCorners();
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool Intersect(const FIntBox& Box, const FIntBox& Other)
	{
		return Box.Intersect(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static bool Contains(const FIntBox& Box, const FIntBox& Other)
	{
		return Box.Contains(Other);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox")
	static FIntBox Overlap(const FIntBox& A, const FIntBox& B)
	{
		return A.Overlap(B);
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "+"))
	static FIntBox AddPoint(const FIntBox& Box, const FIntVector& Point)
	{
		return Box + Point;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "+"))
	static FIntBox AddBox(const FIntBox& Box, const FIntBox& BoxToAdd)
	{
		return Box + BoxToAdd;
	}
	
	UFUNCTION(BlueprintPure, Category = "Math|IntBox", meta = (CompactNodeTitle = "x"))
	static FIntBox Scale(const FIntBox& Box, int Scale)
	{
		return Box * Scale;
	}
};

