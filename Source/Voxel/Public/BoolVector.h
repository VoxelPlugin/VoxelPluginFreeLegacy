// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "BoolVector.generated.h"

USTRUCT(BlueprintType)
struct VOXEL_API FBoolVector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bZ = false;

	inline FVector ToFVector() const
	{
		return FVector(bX, bY, bZ);
	}
	inline FBoolVector operator!() const
	{
		return { !bX, !bY, !bZ };
	}

	friend inline bool operator==(const FBoolVector& Lhs, const FBoolVector& Rhs)
	{
		return Lhs.bX == Rhs.bX
			&& Lhs.bY == Rhs.bY
			&& Lhs.bZ == Rhs.bZ;
	}
	friend inline bool operator!=(const FBoolVector& Lhs, const FBoolVector& Rhs)
	{
		return Lhs.bX != Rhs.bX
			|| Lhs.bY != Rhs.bY
			|| Lhs.bZ != Rhs.bZ;
	}
};