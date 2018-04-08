// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.generated.h"

/**
 * A Box with int coordinates
 */
USTRUCT(BlueprintType)
struct VOXEL_API FIntBox
{
	GENERATED_BODY()

	// Min of the box. Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Min;

	// Max of the box. Exclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector Max;

	// +/- 1000: prevents integers overflow
	FORCEINLINE static FIntBox Infinite() { return FIntBox(FIntVector(MIN_int32 + 1000, MIN_int32 + 1000, MIN_int32 + 1000), FIntVector(MAX_int32 - 1000, MAX_int32 - 1000, MAX_int32 - 1000)); }

	FIntBox()
		: Min(FIntVector::ZeroValue)
		, Max(FIntVector::ZeroValue)
	{
	}

	FIntBox(const FIntVector& Min, const FIntVector& Max)
		: Min(Min)
		, Max(Max)
	{
	}

	FIntBox(const FIntVector& Position)
		: Min(Position)
		, Max(Position + FIntVector(1, 1, 1))
	{
	}

	/**
	 * Translate the box
	 */
	FIntBox TranslateBy(const FIntVector& Position) const
	{
		return FIntBox(Min + Position, Max + Position);
	}

	/**
	 * Get the size of this box
	 */
	FORCEINLINE FIntVector Size() const
	{
		return Max - Min;
	}

	/**
	 * Checks whether the given location is inside this box.
	 *
	 * @param	V	The location to test for inside the bounding volume.
	 * @return	true if location is inside this volume.
	 */
	FORCEINLINE bool IsInside(const FIntVector& V) const
	{
		return IsInside(V.X, V.Y, V.Z);
	}

	FORCEINLINE bool IsInside(int X, int Y, int Z) const
	{
		return ((X >= Min.X) && (X < Max.X) && (Y >= Min.Y) && (Y < Max.Y) && (Z >= Min.Z) && (Z < Max.Z));
	}


	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	FORCEINLINE bool Intersect(const FIntBox& Other) const
	{
		if ((Min.X >= Other.Max.X) || (Other.Min.X >= Max.X))
		{
			return false;
		}

		if ((Min.Y >= Other.Max.Y) || (Other.Min.Y >= Max.Y))
		{
			return false;
		}

		if ((Min.Z >= Other.Max.Z) || (Other.Min.Z >= Max.Z))
		{
			return false;
		}

		return true;
	}

	FORCEINLINE bool Contains(const FIntBox& Other) const
	{
		return Min.X <= Other.Min.X && Min.Y <= Other.Min.Y && Min.Z <= Other.Min.Z &&
			   Max.X > Other.Max.X && Max.Y > Other.Max.Y && Max.Z > Other.Max.Z;
	}

	/**
	 * Returns the overlap FVoxelBox of two box
	 *
	 * @param Other The bounding box to test overlap
	 * @return the overlap box. It can be 0 if they don't overlap
	 */
	FIntBox Overlap(const FIntBox& Other) const
	{
		if (!Intersect(Other))
		{
			static FIntBox EmptyBox = FIntBox();
			return EmptyBox;
		}

		// otherwise they overlap
		// so find overlapping box
		FIntVector MinVector, MaxVector;

		MinVector.X = FMath::Max(Min.X, Other.Min.X);
		MaxVector.X = FMath::Min(Max.X, Other.Max.X);

		MinVector.Y = FMath::Max(Min.Y, Other.Min.Y);
		MaxVector.Y = FMath::Min(Max.Y, Other.Max.Y);

		MinVector.Z = FMath::Max(Min.Z, Other.Min.Z);
		MaxVector.Z = FMath::Min(Max.Z, Other.Max.Z);

		return FIntBox(MinVector, MaxVector);
	}

	FORCEINLINE int ComputeSquaredDistanceFromBoxToPoint(const FIntVector& Point) const
	{
		// Accumulates the distance as we iterate axis
		int DistSquared = 0.f;

		// Check each axis for min/max and add the distance accordingly
		// NOTE: Loop manually unrolled for > 2x speed up
		if (Point.X < Min.X)
		{
			DistSquared += FMath::Square(Point.X - Min.X);
		}
		else if (Point.X > Max.X)
		{
			DistSquared += FMath::Square(Point.X - Max.X);
		}

		if (Point.Y < Min.Y)
		{
			DistSquared += FMath::Square(Point.Y - Min.Y);
		}
		else if (Point.Y > Max.Y)
		{
			DistSquared += FMath::Square(Point.Y - Max.Y);
		}

		if (Point.Z < Min.Z)
		{
			DistSquared += FMath::Square(Point.Z - Min.Z);
		}
		else if (Point.Z > Max.Z)
		{
			DistSquared += FMath::Square(Point.Z - Max.Z);
		}

		return DistSquared;
	}

	/**
	 * Get the corners that are inside the box (max - 1)
	 */
	TArray<FIntVector> GetCorners() const
	{
		return {
			FIntVector(Min.X    , Min.Y    , Min.Z),
			FIntVector(Max.X - 1, Min.Y    , Min.Z),
			FIntVector(Min.X    , Max.Y - 1, Min.Z),
			FIntVector(Max.X - 1, Max.Y - 1, Min.Z),
			FIntVector(Min.X    , Min.Y    , Max.Z - 1),
			FIntVector(Max.X - 1, Min.Y    , Max.Z - 1),
			FIntVector(Min.X    , Max.Y - 1, Max.Z - 1),
			FIntVector(Max.X - 1, Max.Y - 1, Max.Z - 1)
		};
	}

	/**
	 * Scales this box
	 */
	FORCEINLINE FIntBox& operator*=(int Scale)
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}

	FORCEINLINE bool operator==(const FIntBox& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}
};

FORCEINLINE uint32 GetTypeHash(const FIntBox& Box)
{
	return FCrc::MemCrc32(&Box, sizeof(FIntBox));
}

FORCEINLINE FIntBox operator*(const FIntBox& Box, int Scale)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}

FORCEINLINE FIntBox operator*(int Scale, const FIntBox& Box)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}