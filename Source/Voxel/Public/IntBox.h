// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVectorUtilities.h"
#include "VoxelIntVectorUtilities.h"
#include "IntBox.generated.h"

enum class EInverseTransform : uint8
{
	True,
	False
};

/**
 * A Box with int32 coordinates
 */
USTRUCT(BlueprintType, meta=(HasNativeMake="Voxel.IntBoxLibrary.MakeIntBox", HasNativeBreak="Voxel.IntBoxLibrary.BreakIntBox"))
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

	FIntBox(const FIntVector& InMin, const FIntVector& InMax)
		: Min(InMin)
		, Max(InMax)
	{
		if (!ensureMsgf(Min.X <= Max.X, TEXT("%d <= %d"), Min.X, Max.X)) Max.X = Min.X;
		if (!ensureMsgf(Min.Y <= Max.Y, TEXT("%d <= %d"), Min.Y, Max.Y)) Max.Y = Min.Y;
		if (!ensureMsgf(Min.Z <= Max.Z, TEXT("%d <= %d"), Min.Z, Max.Z)) Max.Z = Min.Z;
	}
	explicit FIntBox(int32 Min, const FIntVector& Max)
		: FIntBox(FIntVector(Min), Max)
	{
	}
	explicit FIntBox(const FIntVector& Min, int32 Max)
		: FIntBox(Min, FIntVector(Max))
	{
	}
	explicit FIntBox(int32 Min, int32 Max)
		: FIntBox(FIntVector(Min), FIntVector(Max))
	{
	}
	explicit FIntBox(const FVector& Min, const FVector& Max)
		: FIntBox(FVoxelUtilities::FloorToInt(Min), FVoxelUtilities::CeilToInt(Max) + 1)
	{
	}
	explicit FIntBox(const FVoxelVector& Min, const FVoxelVector& Max)
		: FIntBox(FVoxelUtilities::FloorToInt(Min), FVoxelUtilities::CeilToInt(Max) + 1)
	{
	}

	explicit FIntBox(const FVector& Position)
		: FIntBox(Position, Position)
	{
	}
	explicit FIntBox(const FVoxelVector& Position)
		: FIntBox(Position, Position)
	{
	}

	explicit FIntBox(const FIntVector& Position)
		: FIntBox(Position, Position + 1)
	{
	}
	explicit FIntBox(const FBox& Box)
		: FIntBox(Box.Min, Box.Max)
	{
	}

	explicit FIntBox(int32 X, int32 Y, int32 Z)
		: FIntBox(FIntVector(X, Y, Z), FIntVector(X + 1, Y + 1, Z + 1))
	{
	}

	template<typename T>
	explicit FIntBox(const TArray<T>& Data)
	{
		if (!ensure(Data.Num() > 0))
		{
			Min = Max = FIntVector::ZeroValue;
			return;
		}

		*this = FIntBox(Data[0]);
		for (int32 Index = 1; Index < Data.Num(); Index++)
		{
			*this = *this + Data[Index];
		}
	}

	FORCEINLINE static FIntBox SafeConstruct(const FIntVector& A, const FIntVector& B)
	{
		FIntBox Box;
		Box.Min = FVoxelUtilities::ComponentMin(A, B);
		Box.Max = FVoxelUtilities::ComponentMax3(A, B, Box.Min + FIntVector(1, 1, 1));
		return Box;
	}

	FORCEINLINE FIntVector Size() const
	{
		ensure(SizeIs32Bit());
		return Max - Min;
	}
	FORCEINLINE FVoxelVector GetCenter() const
	{
		return FVoxelVector(Min + Max) / 2.f;
	}
	FORCEINLINE uint64 Count() const
	{
		return
			uint64(Max.X - Min.X) *
			uint64(Max.Y - Min.Y) *
			uint64(Max.Z - Min.Z);
	}

	FORCEINLINE bool SizeIs32Bit() const
	{
		return
			int64(Max.X) - int64(Min.X) < MAX_int32 &&
			int64(Max.Y) - int64(Min.Y) < MAX_int32 &&
			int64(Max.Z) - int64(Min.Z) < MAX_int32;
	}
	
	/**
	 * Get the corners that are inside the box (max - 1)
	 */
	TArray<FIntVector, TFixedAllocator<8>> GetCorners(int32 MaxBorderSize) const
	{
		return {
			FIntVector(Min.X, Min.Y, Min.Z),
			FIntVector(Max.X - MaxBorderSize, Min.Y, Min.Z),
			FIntVector(Min.X, Max.Y - MaxBorderSize, Min.Z),
			FIntVector(Max.X - MaxBorderSize, Max.Y - MaxBorderSize, Min.Z),
			FIntVector(Min.X, Min.Y, Max.Z - MaxBorderSize),
			FIntVector(Max.X - MaxBorderSize, Min.Y, Max.Z - MaxBorderSize),
			FIntVector(Min.X, Max.Y - MaxBorderSize, Max.Z - MaxBorderSize),
			FIntVector(Max.X - MaxBorderSize, Max.Y - MaxBorderSize, Max.Z - MaxBorderSize)
		};
	}
	FString ToString() const
	{
		return FString::Printf(TEXT("(%d/%d, %d/%d, %d/%d)"), Min.X, Max.X, Min.Y, Max.Y, Min.Z, Max.Z);
	}

	FORCEINLINE bool IsValid() const
	{
		return Min.X < Max.X && Min.Y < Max.Y && Min.Z < Max.Z;
	}
	
	template<typename T>
	FORCEINLINE bool ContainsTemplate(T X, T Y, T Z) const
	{
		return ((X >= Min.X) && (X < Max.X) && (Y >= Min.Y) && (Y < Max.Y) && (Z >= Min.Z) && (Z < Max.Z));
	}
	template<typename T>
	FORCEINLINE typename TEnableIf<TOr<TIsSame<T, FVector>, TIsSame<T, FVoxelVector>, TIsSame<T, FIntVector>>::Value, bool>::Type ContainsTemplate(const T& V) const
	{
		return ContainsTemplate(V.X, V.Y, V.Z);
	}
	template<typename T>
	FORCEINLINE typename TEnableIf<TOr<TIsSame<T, FBox>, TIsSame<T, FIntBox>>::Value, bool>::Type ContainsTemplate(const T& Other) const
	{
		return Min.X <= Other.Min.X && Min.Y <= Other.Min.Y && Min.Z <= Other.Min.Z &&
			   Max.X >= Other.Max.X && Max.Y >= Other.Max.Y && Max.Z >= Other.Max.Z;
	}

	FORCEINLINE bool Contains(int32 X, int32 Y, int32 Z) const
	{
		return ContainsTemplate(X, Y, Z);
	}
	FORCEINLINE bool Contains(const FIntVector& V) const
	{
		return ContainsTemplate(V);
	}
	FORCEINLINE bool Contains(const FIntBox& Other) const
	{
		return ContainsTemplate(Other);
	}

	// Not an overload as the float behavior can be a bit tricky. Use ContainsTemplate if the input type is unkown
	FORCEINLINE bool ContainsFloat(float X, float Y, float Z) const
	{
		return ContainsTemplate(X, Y, Z);
	}
	FORCEINLINE bool ContainsFloat(const FVector& V) const
	{
		return ContainsTemplate(V);
	}
	FORCEINLINE bool ContainsFloat(const FVoxelVector& V) const
	{
		return ContainsTemplate(V);
	}
	FORCEINLINE bool ContainsFloat(const FBox& Other) const
	{
		return ContainsTemplate(Other);
	}
	
	template<typename T>
	bool Contains(T X, T Y, T Z) const = delete;

	/**
	 * Checks whether the given bounding box intersects this bounding box.
	 *
	 * @param Other The bounding box to intersect with.
	 * @return true if the boxes intersect, false otherwise.
	 */
	template<typename TBox>
	FORCEINLINE bool Intersect(const TBox& Other) const
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
	/**
	 * Useful for templates taking a box or coordinates
	 */
	template<typename TNumeric>
	FORCEINLINE bool Intersect(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		return ContainsTemplate(X, Y, Z);
	}
	FIntBox Overlap(const FIntBox& Other) const
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
	FIntBox Union(const FIntBox& Other) const
	{
		FIntVector MinVector, MaxVector;

		MinVector.X = FMath::Min(Min.X, Other.Min.X);
		MaxVector.X = FMath::Max(Max.X, Other.Max.X);

		MinVector.Y = FMath::Min(Min.Y, Other.Min.Y);
		MaxVector.Y = FMath::Max(Max.Y, Other.Max.Y);

		MinVector.Z = FMath::Min(Min.Z, Other.Min.Z);
		MaxVector.Z = FMath::Max(Max.Z, Other.Max.Z);

		return FIntBox(MinVector, MaxVector);
	}

	FORCEINLINE FIntVector Clamp(const FIntVector& P) const
	{
		return FVoxelUtilities::Clamp(P, Min, Max - 1);
	}
	// union(return value, Other) = this
	TArray<FIntBox, TFixedAllocator<6>> Difference(const FIntBox& Other) const
	{
		if (!Intersect(Other))
		{
			return { *this };
		}

		TArray<FIntBox, TFixedAllocator<6>> OutBoxes;
		
		if (Min.Z < Other.Min.Z)
		{
			// Add bottom
			OutBoxes.Emplace(Min, FIntVector(Max.X, Max.Y, Other.Min.Z));
		}
		if (Other.Max.Z < Max.Z)
		{
			// Add top
			OutBoxes.Emplace(FIntVector(Min.X, Min.Y, Other.Max.Z), Max);
		}

		const int32 MinZ = FMath::Max(Min.Z, Other.Min.Z);
		const int32 MaxZ = FMath::Min(Max.Z, Other.Max.Z);

		if (Min.X < Other.Min.X)
		{
			// Add X min
			OutBoxes.Emplace(FIntVector(Min.X, Min.Y, MinZ), FIntVector(Other.Min.X, Max.Y, MaxZ));
		}
		if (Other.Max.X < Max.X)
		{
			// Add X max
			OutBoxes.Emplace(FIntVector(Other.Max.X, Min.Y, MinZ), FIntVector(Max.X, Max.Y, MaxZ));
		}
		
		const int32 MinX = FMath::Max(Min.X, Other.Min.X);
		const int32 MaxX = FMath::Min(Max.X, Other.Max.X);

		if (Min.Y < Other.Min.Y)
		{
			// Add Y min
			OutBoxes.Emplace(FIntVector(MinX, Min.Y, MinZ), FIntVector(MaxX, Other.Min.Y, MaxZ));
		}
		if (Other.Max.Y < Max.Y)
		{
			// Add Y max
			OutBoxes.Emplace(FIntVector(MinX, Other.Max.Y, MinZ), FIntVector(MaxX, Max.Y, MaxZ));
		}

		return OutBoxes;
	}

	FORCEINLINE uint64 ComputeSquaredDistanceFromBoxToPoint(const FIntVector& Point) const
	{
		// Accumulates the distance as we iterate axis
		uint64 DistSquared = 0;

		// Check each axis for min/max and add the distance accordingly
		if (Point.X < Min.X)
		{
			DistSquared += FMath::Square<uint64>(Min.X - Point.X);
		}
		else if (Point.X > Max.X)
		{
			DistSquared += FMath::Square<uint64>(Point.X - Max.X);
		}

		if (Point.Y < Min.Y)
		{
			DistSquared += FMath::Square<uint64>(Min.Y - Point.Y);
		}
		else if (Point.Y > Max.Y)
		{
			DistSquared += FMath::Square<uint64>(Point.Y - Max.Y);
		}

		if (Point.Z < Min.Z)
		{
			DistSquared += FMath::Square<uint64>(Min.Z - Point.Z);
		}
		else if (Point.Z > Max.Z)
		{
			DistSquared += FMath::Square<uint64>(Point.Z - Max.Z);
		}

		return DistSquared;
	}

	FORCEINLINE bool IsMultipleOf(int32 Step) const
	{
		return Min.X % Step == 0 && Min.Y % Step == 0 && Min.Z % Step == 0 &&
			   Max.X % Step == 0 && Max.Y % Step == 0 && Max.Z % Step == 0;
	}
	
	// OldBox included in NewBox, but NewBox not included in OldBox
	FORCEINLINE FIntBox MakeMultipleOfBigger(int32 Step) const
	{
		FIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideFloor(Min, Step) * Step;
		NewBox.Max = FVoxelUtilities::DivideCeil(Max, Step) * Step;
		return NewBox;
	}
	// NewBox included in OldBox, but OldBox not included in NewBox
	FORCEINLINE FIntBox MakeMultipleOfSmaller(int32 Step) const
	{
		FIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideCeil(Min, Step) * Step;		
		NewBox.Max = FVoxelUtilities::DivideFloor(Max, Step) * Step;
		return NewBox;
	}
	FORCEINLINE FIntBox MakeMultipleOfRoundUp(int32 Step) const
	{
		FIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideCeil(Min, Step) * Step;		
		NewBox.Max = FVoxelUtilities::DivideCeil(Max, Step) * Step;
		return NewBox;
	}

	// Guarantee: union(OutChilds).Contains(this)
	template<typename T>
	inline void Subdivide(int32 ChildrenSize, TArray<FIntBox, T>& OutChildren) const
	{
		const FIntVector LowerBound = FVoxelUtilities::DivideFloor(Min, ChildrenSize) * ChildrenSize;
		const FIntVector UpperBound = FVoxelUtilities::DivideCeil(Max, ChildrenSize) * ChildrenSize;
		for (int32 X = LowerBound.X; X < UpperBound.X; X += ChildrenSize)
		{
			for (int32 Y = LowerBound.Y; Y < UpperBound.Y; Y += ChildrenSize)
			{
				for (int32 Z = LowerBound.Z; Z < UpperBound.Z; Z += ChildrenSize)
				{
					OutChildren.Emplace(FIntVector(X, Y, Z), FIntVector(X + ChildrenSize, Y + ChildrenSize, Z + ChildrenSize));
				}
			}
		}
	}

	FORCEINLINE FIntBox Scale(v_flt S) const
	{
		return { FVoxelUtilities::FloorToInt(FVoxelVector(Min) * S), FVoxelUtilities::CeilToInt(FVoxelVector(Max) * S) };
	}
	
	FORCEINLINE FIntBox Extend(const FIntVector& Amount) const
	{
		return { Min - Amount, Max + Amount };
	}
	FORCEINLINE FIntBox Extend(int32 Amount) const
	{
		return Extend(FIntVector(Amount));
	}
	FORCEINLINE FIntBox Translate(const FIntVector& Position) const
	{
		return FIntBox(Min + Position, Max + Position);
	}
	
	FORCEINLINE FIntBox RemoveTranslation() const
	{
		return FIntBox(0, Max - Min);
	}
	// Will move the box so that GetCenter = 0,0,0. Will extend it if its size is odd
	FIntBox Center() const
	{
		FIntVector NewMin = Min;
		FIntVector NewMax = Max;
		if (FVoxelVector(GetCenter().ToInt()) != GetCenter())
		{
			NewMax = NewMax + 1;
		}
		ensure(FVoxelVector(GetCenter().ToInt()) == GetCenter());
		const FIntVector Offset = GetCenter().ToInt();
		NewMin -= Offset;
		NewMax -= Offset;
		ensure(NewMin + NewMax == FIntVector(0));
		return FIntBox(NewMin, NewMax);
	}

	FORCEINLINE FIntBox& operator*=(int32 Scale)
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}

	FORCEINLINE bool operator==(const FIntBox& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}
	FORCEINLINE bool operator!=(const FIntBox& Other) const
	{
		return Min != Other.Min || Max != Other.Max;
	}

	// More expensive, but should be more random
	FORCEINLINE uint32 GetMurmurHash() const
	{
		return FVoxelUtilities::MurmurHash(Min) ^ FVoxelUtilities::MurmurHash(Max);
	}

	template<typename T>
	void Iterate(T Lambda) const
	{
		for (int32 X = Min.X; X < Max.X; X++)
		{
			for (int32 Y = Min.Y; Y < Max.Y; Y++)
			{
				for (int32 Z = Min.Z; Z < Max.Z; Z++)
				{
					Lambda(X, Y, Z);
				}
			}
		}
	}
	template<typename T>
	void Iterate(int32 Step, T Lambda) const
	{
		for (int32 X = Min.X; X < Max.X; X += Step)
		{
			for (int32 Y = Min.Y; Y < Max.Y; Y += Step)
			{
				for (int32 Z = Min.Z; Z < Max.Z; Z += Step)
				{
					Lambda(X, Y, Z);
				}
			}
		}
	}

	template<EInverseTransform Inverse = EInverseTransform::False>
	FIntBox ApplyTransform(const FTransform& Transform, int32 MaxBorderSize = 0) const
	{
		const auto Corners = GetCorners(MaxBorderSize);

		FIntVector NewMin(MAX_int32);
		FIntVector NewMax(MIN_int32);
		for (int32 Index = 0; Index < 8; Index++)
		{
			const FVector P = 
				Inverse == EInverseTransform::True 
				? Transform.InverseTransformPosition(FVector(Corners[Index])) 
				: Transform.TransformPosition(FVector(Corners[Index]));
			NewMin = FVoxelUtilities::ComponentMin(NewMin, FVoxelUtilities::FloorToInt(P));
			NewMax = FVoxelUtilities::ComponentMax(NewMax, FVoxelUtilities::CeilToInt(P));
		}
		return FIntBox(NewMin, NewMax + MaxBorderSize);
	}
};

FORCEINLINE uint32 GetTypeHash(const FIntBox& Box)
{
	static_assert(sizeof(FIntBox) == 6 * sizeof(int32), "Alignement error");
	return FCrc::MemCrc32(&Box, sizeof(FIntBox));
}

FORCEINLINE FIntBox operator*(const FIntBox& Box, int32 Scale)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}

FORCEINLINE FIntBox operator*(int32 Scale, const FIntBox& Box)
{
	FIntBox Copy = Box;
	return Copy *= Scale;
}

FORCEINLINE FIntBox operator+(const FIntBox& Box, const FIntBox& Other)
{
	FIntBox Copy = Box;
	Copy.Min = FVoxelUtilities::ComponentMin(Copy.Min, Other.Min);
	Copy.Max = FVoxelUtilities::ComponentMax(Copy.Max, Other.Max);
	return Copy;
}

FORCEINLINE FIntBox operator+(const FIntBox& Box, const FIntVector& Point)
{
	return Box + FIntBox(Point);
}

FORCEINLINE FArchive& operator<<(FArchive& Ar, FIntBox& Box)
{
	Ar << Box.Min;
	Ar << Box.Max;
	return Ar;
}

struct FIntBoxWithValidity
{
	FIntBoxWithValidity() = default;
	explicit FIntBoxWithValidity(const FIntBox& Box)
		: Box(Box)
		, bValid(true)
	{
	}

	FORCEINLINE const FIntBox& GetBox() const
	{
		check(IsValid());
		return Box;
	}

	FORCEINLINE bool IsValid() const
	{
		return bValid;
	}
	FORCEINLINE void Reset()
	{
		bValid = false;
	}

	FORCEINLINE FIntBoxWithValidity& operator=(const FIntBox& Other)
	{
		Box = Other;
		bValid = true;
		return *this;
	}
	
	FORCEINLINE bool operator==(const FIntBoxWithValidity& Other) const
	{
		if (bValid != Other.bValid)
		{
			return false;
		}
		if (!bValid && !Other.bValid)
		{
			return true;
		}
		return Box == Other.Box;
	}
	FORCEINLINE bool operator!=(const FIntBoxWithValidity& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE FIntBoxWithValidity& operator+=(const FIntBox& Other)
	{
		if (bValid)
		{
			Box = Box + Other;
		}
		else
		{
			Box = Other;
			bValid = true;
		}
		return *this;
	}
	FORCEINLINE FIntBoxWithValidity& operator+=(const FIntBoxWithValidity& Other)
	{
		if (Other.bValid)
		{
			*this += Other.GetBox();
		}
		return *this;
	}

	FORCEINLINE FIntBoxWithValidity& operator+=(const FIntVector& Point)
	{
		return *this += FIntBox(Point);
	}
	
private:
	FIntBox Box;
	bool bValid = false;
};

template<typename T>
FORCEINLINE FIntBoxWithValidity operator+(const FIntBoxWithValidity& Box, const T& Other)
{
	FIntBoxWithValidity Copy = Box;
	return Copy += Other;
}