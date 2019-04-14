// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntVectorUtilities.h"
#include "IntBox.generated.h"

/**
 * A Box with int32 coordinates
 */
USTRUCT(BlueprintType)
struct VOXEL_API FIntBox
{
	GENERATED_BODY()

	// Min of the box. Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Min;

	// Max of the box. Exclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Max;

	const static FIntBox Infinite;

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

	FIntBox(int32 X, int32 Y, int32 Z)
		: Min(X, Y, Z)
		, Max(X + 1, Y + 1, Z + 1)
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
	inline FIntVector Size() const
	{
		return Max - Min;
	}

	inline FVector GetCenter() const
	{
		return (FVector)(Min + Max) / 2.f;
	}

	/**
	 * Get the corners that are inside the box (max - 1)
	 */
	inline TArray<FIntVector> GetCorners() const
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

	inline bool IsValid() const
	{
		return Min.X < Max.X && Min.Y < Max.Y && Min.Z < Max.Z;
	}

	/**
	 * Checks whether the given location is inside this box.
	 *
	 * @param	V	The location to test for inside the bounding volume.
	 * @return	true if location is inside this volume.
	 */
	template<typename TVector>
	inline bool IsInside(const TVector& V) const
	{
		return IsInside(V.X, V.Y, V.Z);
	}
	
	template<typename TNumeric>
	inline bool IsInside(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		return ((X >= Min.X) && (X < Max.X) && (Y >= Min.Y) && (Y < Max.Y) && (Z >= Min.Z) && (Z < Max.Z));
	}


	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	inline bool Intersect(const FIntBox& Other) const
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

	inline bool Contains(const FIntBox& Other) const
	{
		return Min.X <= Other.Min.X && Min.Y <= Other.Min.Y && Min.Z <= Other.Min.Z &&
			   Max.X >= Other.Max.X && Max.Y >= Other.Max.Y && Max.Z >= Other.Max.Z;
	}

	/**
	 * Returns the overlap FVoxelBox of two box
	 *
	 * @param Other The bounding box to test overlap
	 * @return the overlap box. It can be 0 if they don't overlap
	 */
	inline FIntBox Overlap(const FIntBox& Other) const
	{
		if (!Intersect(Other))
		{
			return FIntBox();
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

	static inline void GetRemainingBoxes(const FIntBox& Initial, const FIntBox& BoxToSubstract, TArray<FIntBox>& OutBoxes)
	{
		if (!Initial.Intersect(BoxToSubstract))
		{
			OutBoxes.Add(Initial);
			return;
		}
		
		if (Initial.Min.Z < BoxToSubstract.Min.Z)
		{
			// Add bottom
			OutBoxes.Emplace(Initial.Min, FIntVector(Initial.Max.X, Initial.Max.Y, BoxToSubstract.Min.Z));
		}
		if (BoxToSubstract.Max.Z < Initial.Max.Z)
		{
			// Add top
			OutBoxes.Emplace(FIntVector(Initial.Min.X, Initial.Min.Y, BoxToSubstract.Max.Z), Initial.Max);
		}

		int32 ZMin = FMath::Max(Initial.Min.Z, BoxToSubstract.Min.Z);
		int32 ZMax = FMath::Min(Initial.Max.Z, BoxToSubstract.Max.Z);

		if (Initial.Min.X < BoxToSubstract.Min.X)
		{
			// Add X min
			OutBoxes.Emplace(FIntVector(Initial.Min.X, Initial.Min.Y, ZMin), FIntVector(BoxToSubstract.Min.X, Initial.Max.Y, ZMax));
		}
		if (BoxToSubstract.Max.X < Initial.Max.X)
		{
			// Add X max
			OutBoxes.Emplace(FIntVector(BoxToSubstract.Max.X, Initial.Min.Y, ZMin), FIntVector(Initial.Max.X, Initial.Max.Y, ZMax));
		}
		
		int32 XMin = FMath::Max(Initial.Min.X, BoxToSubstract.Min.X);
		int32 XMax = FMath::Min(Initial.Max.X, BoxToSubstract.Max.X);

		if (Initial.Min.Y < BoxToSubstract.Min.Y)
		{
			// Add Y min
			OutBoxes.Emplace(FIntVector(XMin, Initial.Min.Y, ZMin), FIntVector(XMax, BoxToSubstract.Min.Y, ZMax));
		}
		if (BoxToSubstract.Max.Y < Initial.Max.Y)
		{
			// Add Y max
			OutBoxes.Emplace(FIntVector(XMin, BoxToSubstract.Max.Y, ZMin), FIntVector(XMax, Initial.Max.Y, ZMax));
		}
	}

	template<typename T = uint32>
	inline T ComputeSquaredDistanceFromBoxToPoint(const FIntVector& Point) const
	{
		// Accumulates the distance as we iterate axis
		T DistSquared = 0;

		// Check each axis for min/max and add the distance accordingly
		// NOTE: Loop manually unrolled for > 2x speed up
		if (Point.X < Min.X)
		{
			DistSquared += FMath::Square<T>(Point.X - Min.X);
		}
		else if (Point.X > Max.X)
		{
			DistSquared += FMath::Square<T>(Point.X - Max.X);
		}

		if (Point.Y < Min.Y)
		{
			DistSquared += FMath::Square<T>(Point.Y - Min.Y);
		}
		else if (Point.Y > Max.Y)
		{
			DistSquared += FMath::Square<T>(Point.Y - Max.Y);
		}

		if (Point.Z < Min.Z)
		{
			DistSquared += FMath::Square<T>(Point.Z - Min.Z);
		}
		else if (Point.Z > Max.Z)
		{
			DistSquared += FMath::Square<T>(Point.Z - Max.Z);
		}

		return DistSquared;
	}

	template<typename T>
	inline T ComputeSquaredDistanceFromBoxToBox(const FIntBox& Box) const
	{
		return FMath::Min<T>(ComputeSquaredDistanceFromBoxToPoint<T>(Box.Min), ComputeSquaredDistanceFromBoxToPoint<T>(Box.Max));
	}

	inline bool IsMultipleOf(int32 Step) const
	{
		return Min.X % Step == 0 && Min.Y % Step == 0 && Min.Z % Step == 0 &&
			   Max.X % Step == 0 && Max.Y % Step == 0 && Max.Z % Step == 0;
	}

	// NewBox included in OldBox, but OldBox not included in NewBox
	inline void MakeMultipleOfExclusive(int32 Step)
	{
		Min.X = FMath::CeilToInt((float)Min.X / Step) * Step;
		Min.Y = FMath::CeilToInt((float)Min.Y / Step) * Step;
		Min.Z = FMath::CeilToInt((float)Min.Z / Step) * Step;
		
		Max.X = FMath::CeilToInt((float)Max.X / Step) * Step;
		Max.Y = FMath::CeilToInt((float)Max.Y / Step) * Step;
		Max.Z = FMath::CeilToInt((float)Max.Z / Step) * Step;
	}
	// OldBox included in NewBox, but NewBox not included in OldBox
	inline void MakeMultipleOfInclusive(int32 Step)
	{
		Min.X = FMath::FloorToInt((float)Min.X / Step) * Step;
		Min.Y = FMath::FloorToInt((float)Min.Y / Step) * Step;
		Min.Z = FMath::FloorToInt((float)Min.Z / Step) * Step;
		
		Max.X = FMath::CeilToInt((float)Max.X / Step) * Step;
		Max.Y = FMath::CeilToInt((float)Max.Y / Step) * Step;
		Max.Z = FMath::CeilToInt((float)Max.Z / Step) * Step;
	}

	// union(OutChilds).Contains(this)
	inline void Subdivide(int32 ChildsSize, TArray<FIntBox>& OutChilds) const
	{
		FIntVector LowerBound = FVoxelUtilities::FloorToInt(FVector(Min) / ChildsSize) * ChildsSize;
		FIntVector UpperBound = FVoxelUtilities::CeilToInt(FVector(Max) / ChildsSize) * ChildsSize;
		for (int32 X = LowerBound.X; X < UpperBound.X; X += ChildsSize)
		{
			for (int32 Y = LowerBound.Y; Y < UpperBound.Y; Y += ChildsSize)
			{
				for (int32 Z = LowerBound.Z; Z < UpperBound.Z; Z += ChildsSize)
				{
					OutChilds.Emplace(FIntVector(X, Y, Z), FIntVector(X + ChildsSize, Y + ChildsSize, Z + ChildsSize));
				}
			}
		}
	}
	
	inline FIntBox Extend(const FIntVector& Amount) const
	{
		return { Min - Amount, Max + Amount };
	}
	inline FIntBox Extend(int32 Amount) const
	{
		return Extend(FIntVector(Amount));
	}

	inline FString ToString() const
	{
		return FString::Printf(TEXT("(%d/%d, %d/%d, %d/%d)"), Min.X, Max.X, Min.Y, Max.Y, Min.Z, Max.Z);
	}

	/**
	 * Scales this box
	 */
	inline FIntBox& operator*=(int32 Scale)
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}

	inline bool operator==(const FIntBox& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}

	inline FIntBox& operator+=(const FIntBox& Other)
	{
		Min.X = FMath::Min(Min.X, Other.Min.X);
		Min.Y = FMath::Min(Min.Y, Other.Min.Y);
		Min.Z = FMath::Min(Min.Z, Other.Min.Z);
		
		Max.X = FMath::Max(Max.X, Other.Max.X);
		Max.Y = FMath::Max(Max.Y, Other.Max.Y);
		Max.Z = FMath::Max(Max.Z, Other.Max.Z);
		
		return *this;
	}

	inline FIntBox& operator+=(const FIntVector& Point)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
		Min.Z = FMath::Min(Min.Z, Point.Z);
		
		Max.X = FMath::Max(Max.X, Point.X + 1);
		Max.Y = FMath::Max(Max.Y, Point.Y + 1);
		Max.Z = FMath::Max(Max.Z, Point.Z + 1);
		
		return *this;
	}
};

inline uint32 GetTypeHash(const FIntBox& Box)
{
	return FCrc::MemCrc32(&Box, sizeof(FIntBox));
}

inline FIntBox operator*(const FIntBox& Box, int32 Scale)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}

inline FIntBox operator*(int32 Scale, const FIntBox& Box)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}

inline FIntBox operator+(const FIntBox& Box, const FIntBox& Other)
{
	FIntBox Copy = Box;
	return Copy += Other;
}

inline FIntBox operator+(const FIntBox& Box, const FIntVector& Point)
{
	FIntBox Copy = Box;
	return Copy += Point;
}

inline FArchive& operator<<(FArchive& Ar, FIntBox& Box)
{
	Ar << Box.Min;
	Ar << Box.Max;
	return Ar;
}