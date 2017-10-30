#pragma once
#include "CoreMinimal.h"
#include "VoxelBox.generated.h"


USTRUCT(BlueprintType)
struct VOXEL_API FVoxelBox
{
	GENERATED_BODY()

public:
	FIntVector Min; // Inclusive
	FIntVector Max; // Exclusive

	FVoxelBox()
		: Min(FIntVector::ZeroValue)
		, Max(FIntVector::ZeroValue)
	{
	}

	FVoxelBox(const FIntVector Min, const FIntVector Max)
		: Min(Min)
		, Max(Max)
	{
	}

	/**
	 * Checks whether the given location is inside this box.
	 *
	 * @param In The location to test for inside the bounding volume.
	 * @return true if location is inside this volume.
	 */
	FORCEINLINE bool IsInside(const int X, const int Y, const int Z) const
	{
		return ((X > Min.X) && (X < Max.X) && (Y > Min.Y) && (Y < Max.Y) && (Z > Min.Z) && (Z < Max.Z));
	}


	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	FORCEINLINE bool Intersect(const FVoxelBox& Other) const
	{
		if ((Min.X > Other.Max.X) || (Other.Min.X > Max.X))
		{
			return false;
		}

		if ((Min.Y > Other.Max.Y) || (Other.Min.Y > Max.Y))
		{
			return false;
		}

		if ((Min.Z > Other.Max.Z) || (Other.Min.Z > Max.Z))
		{
			return false;
		}

		return true;
	}

	/**
	 * Returns the overlap FVoxelBox of two box
	 *
	 * @param Other The bounding box to test overlap
	 * @return the overlap box. It can be 0 if they don't overlap
	 */
	FVoxelBox Overlap(const FVoxelBox& Other) const
	{
		if (Intersect(Other) == false)
		{
			static FVoxelBox EmptyBox = FVoxelBox();
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

		return FVoxelBox(MinVector, MaxVector);
	}
};