// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities/VoxelVectorUtilities.h"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"
#include "Async/ParallelFor.h"
#include "VoxelIntBox.generated.h"

enum class EInverseTransform : uint8
{
	True,
	False
};

/**
 * A Box with int32 coordinates
 */
USTRUCT(BlueprintType, meta=(HasNativeMake="Voxel.VoxelIntBoxLibrary.MakeIntBox", HasNativeBreak="Voxel.VoxelIntBoxLibrary.BreakIntBox"))
struct VOXEL_API FVoxelIntBox
{
	GENERATED_BODY()

	// Min of the box. Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Min;

	// Max of the box. Exclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Max;

	const static FVoxelIntBox Infinite;

	FVoxelIntBox()
		: Min(FIntVector::ZeroValue)
		, Max(FIntVector::ZeroValue)
	{
	}

	FVoxelIntBox(const FIntVector& InMin, const FIntVector& InMax)
		: Min(InMin)
		, Max(InMax)
	{
		if (!ensureMsgf(Min.X <= Max.X, TEXT("%d <= %d"), Min.X, Max.X)) Max.X = Min.X;
		if (!ensureMsgf(Min.Y <= Max.Y, TEXT("%d <= %d"), Min.Y, Max.Y)) Max.Y = Min.Y;
		if (!ensureMsgf(Min.Z <= Max.Z, TEXT("%d <= %d"), Min.Z, Max.Z)) Max.Z = Min.Z;
	}
	explicit FVoxelIntBox(int32 Min, const FIntVector& Max)
		: FVoxelIntBox(FIntVector(Min), Max)
	{
	}
	explicit FVoxelIntBox(const FIntVector& Min, int32 Max)
		: FVoxelIntBox(Min, FIntVector(Max))
	{
	}
	explicit FVoxelIntBox(int32 Min, int32 Max)
		: FVoxelIntBox(FIntVector(Min), FIntVector(Max))
	{
	}
	explicit FVoxelIntBox(const FVector& Min, const FVector& Max)
		: FVoxelIntBox(FVoxelUtilities::FloorToInt(Min), FVoxelUtilities::CeilToInt(Max) + 1)
	{
	}
	explicit FVoxelIntBox(const FVoxelVector& Min, const FVoxelVector& Max)
		: FVoxelIntBox(FVoxelUtilities::FloorToInt(Min), FVoxelUtilities::CeilToInt(Max) + 1)
	{
	}

	explicit FVoxelIntBox(const FVector& Position)
		: FVoxelIntBox(Position, Position)
	{
	}
	explicit FVoxelIntBox(const FVoxelVector& Position)
		: FVoxelIntBox(Position, Position)
	{
	}

	explicit FVoxelIntBox(const FIntVector& Position)
		: FVoxelIntBox(Position, Position + 1)
	{
	}
	explicit FVoxelIntBox(const FBox& Box)
		: FVoxelIntBox(Box.Min, Box.Max)
	{
	}

	explicit FVoxelIntBox(int32 X, int32 Y, int32 Z)
		: FVoxelIntBox(FIntVector(X, Y, Z), FIntVector(X + 1, Y + 1, Z + 1))
	{
	}

	template<typename T>
	explicit FVoxelIntBox(const TArray<T>& Data)
	{
		if (!ensure(Data.Num() > 0))
		{
			Min = Max = FIntVector::ZeroValue;
			return;
		}

		*this = FVoxelIntBox(Data[0]);
		for (int32 Index = 1; Index < Data.Num(); Index++)
		{
			*this = *this + Data[Index];
		}
	}

	FORCEINLINE static FVoxelIntBox SafeConstruct(const FIntVector& A, const FIntVector& B)
	{
		FVoxelIntBox Box;
		Box.Min = FVoxelUtilities::ComponentMin(A, B);
		Box.Max = FVoxelUtilities::ComponentMax3(A, B, Box.Min + FIntVector(1, 1, 1));
		return Box;
	}
	FORCEINLINE static FVoxelIntBox SafeConstruct(const FVoxelVector& A, const FVoxelVector& B)
	{
		FVoxelIntBox Box;
		Box.Min = FVoxelUtilities::FloorToInt(FVoxelUtilities::ComponentMin(A, B));
		Box.Max = FVoxelUtilities::CeilToInt(FVoxelUtilities::ComponentMax3(A, B, Box.Min + FIntVector(1, 1, 1)));
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
	TVoxelStaticArray<FIntVector, 8> GetCorners(int32 MaxBorderSize) const
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
	FORCEINLINE typename TEnableIf<TOr<TIsSame<T, FBox>, TIsSame<T, FVoxelIntBox>>::Value, bool>::Type ContainsTemplate(const T& Other) const
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
	FORCEINLINE bool Contains(const FVoxelIntBox& Other) const
	{
		return ContainsTemplate(Other);
	}

	// Not an overload as the float behavior can be a bit tricky. Use ContainsTemplate if the input type is unknown
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
	
	template<typename T>
	FORCEINLINE FIntVector Clamp(T P) const
	{
		Clamp(P.X, P.Y, P.Z);
		return P;
	}
	FORCEINLINE void Clamp(int32& X, int32& Y, int32& Z) const
	{
		X = FMath::Clamp(X, Min.X, Max.X - 1);
		Y = FMath::Clamp(Y, Min.Y, Max.Y - 1);
		Z = FMath::Clamp(Z, Min.Z, Max.Z - 1);
		ensureVoxelSlowNoSideEffects(Contains(X, Y, Z));
	}
	template<typename T>
	FORCEINLINE void Clamp(T& X, T& Y, T& Z) const
	{
		// Note: use - 1 even if that's not the closest value for which Contains would return true
		// because it's really hard to figure out that value (largest float f such that f < i)
		X = FMath::Clamp<T>(X, Min.X, Max.X - 1);
		Y = FMath::Clamp<T>(Y, Min.Y, Max.Y - 1);
		Z = FMath::Clamp<T>(Z, Min.Z, Max.Z - 1);
		ensureVoxelSlowNoSideEffects(ContainsTemplate(X, Y, Z));
	}
	FORCEINLINE FVoxelIntBox Clamp(const FVoxelIntBox& Other) const
	{
		// It's not valid to call Clamp if we're not intersecting Other
		ensureVoxelSlowNoSideEffects(Intersect(Other));

		FVoxelIntBox Result;
		
		Result.Min.X = FMath::Clamp(Other.Min.X, Min.X, Max.X - 1);
		Result.Min.Y = FMath::Clamp(Other.Min.Y, Min.Y, Max.Y - 1);
		Result.Min.Z = FMath::Clamp(Other.Min.Z, Min.Z, Max.Z - 1);

		Result.Max.X = FMath::Clamp(Other.Max.X, Min.X + 1, Max.X);
		Result.Max.Y = FMath::Clamp(Other.Max.Y, Min.Y + 1, Max.Y);
		Result.Max.Z = FMath::Clamp(Other.Max.Z, Min.Z + 1, Max.Z);
		
		ensureVoxelSlowNoSideEffects(Other.Contains(Result));
		return Result;
	}

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
	FVoxelIntBox Overlap(const FVoxelIntBox& Other) const
	{
		if (!Intersect(Other))
		{
			return FVoxelIntBox();
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

		return FVoxelIntBox(MinVector, MaxVector);
	}
	FVoxelIntBox Union(const FVoxelIntBox& Other) const
	{
		FIntVector MinVector, MaxVector;

		MinVector.X = FMath::Min(Min.X, Other.Min.X);
		MaxVector.X = FMath::Max(Max.X, Other.Max.X);

		MinVector.Y = FMath::Min(Min.Y, Other.Min.Y);
		MaxVector.Y = FMath::Max(Max.Y, Other.Max.Y);

		MinVector.Z = FMath::Min(Min.Z, Other.Min.Z);
		MaxVector.Z = FMath::Max(Max.Z, Other.Max.Z);

		return FVoxelIntBox(MinVector, MaxVector);
	}

	// union(return value, Other) = this
	TArray<FVoxelIntBox, TFixedAllocator<6>> Difference(const FVoxelIntBox& Other) const
	{
		if (!Intersect(Other))
		{
			return { *this };
		}

		TArray<FVoxelIntBox, TFixedAllocator<6>> OutBoxes;
		
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
	FORCEINLINE FVoxelIntBox MakeMultipleOfBigger(int32 Step) const
	{
		FVoxelIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideFloor(Min, Step) * Step;
		NewBox.Max = FVoxelUtilities::DivideCeil(Max, Step) * Step;
		return NewBox;
	}
	// NewBox included in OldBox, but OldBox not included in NewBox
	FORCEINLINE FVoxelIntBox MakeMultipleOfSmaller(int32 Step) const
	{
		FVoxelIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideCeil(Min, Step) * Step;		
		NewBox.Max = FVoxelUtilities::DivideFloor(Max, Step) * Step;
		return NewBox;
	}
	FORCEINLINE FVoxelIntBox MakeMultipleOfRoundUp(int32 Step) const
	{
		FVoxelIntBox NewBox;
		NewBox.Min = FVoxelUtilities::DivideCeil(Min, Step) * Step;		
		NewBox.Max = FVoxelUtilities::DivideCeil(Max, Step) * Step;
		return NewBox;
	}

	// Guarantee: union(OutChilds).Contains(this)
	template<typename T>
	bool Subdivide(int32 ChildrenSize, TArray<FVoxelIntBox, T>& OutChildren, int32 MaxChildren = -1) const
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
					if (MaxChildren != -1 && OutChildren.Num() > MaxChildren)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	FORCEINLINE FVoxelIntBox Scale(v_flt S) const
	{
		return { FVoxelUtilities::FloorToInt(FVoxelVector(Min) * S), FVoxelUtilities::CeilToInt(FVoxelVector(Max) * S) };
	}
	FORCEINLINE FVoxelIntBox Scale(const FVoxelVector& S) const
	{
		return SafeConstruct(FVoxelVector(Min) * S, FVoxelVector(Max) * S);
	}
	
	FORCEINLINE FVoxelIntBox Extend(const FIntVector& Amount) const
	{
		return { Min - Amount, Max + Amount };
	}
	FORCEINLINE FVoxelIntBox Extend(int32 Amount) const
	{
		return Extend(FIntVector(Amount));
	}
	FORCEINLINE FVoxelIntBox Translate(const FIntVector& Position) const
	{
		return FVoxelIntBox(Min + Position, Max + Position);
	}
	
	FORCEINLINE FVoxelIntBox RemoveTranslation() const
	{
		return FVoxelIntBox(0, Max - Min);
	}
	// Will move the box so that GetCenter = 0,0,0. Will extend it if its size is odd
	FVoxelIntBox Center() const
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
		return FVoxelIntBox(NewMin, NewMax);
	}

	FORCEINLINE FVoxelIntBox& operator*=(int32 Scale)
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}

	FORCEINLINE bool operator==(const FVoxelIntBox& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}
	FORCEINLINE bool operator!=(const FVoxelIntBox& Other) const
	{
		return Min != Other.Min || Max != Other.Max;
	}

	// More expensive, but should be more random
	FORCEINLINE uint32 GetMurmurHash() const
	{
		return FVoxelUtilities::MurmurHash(Min) ^ FVoxelUtilities::MurmurHash(Max);
	}

	template<typename T>
	FORCEINLINE void Iterate(T Lambda) const
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
	FORCEINLINE void Iterate(int32 Step, T Lambda) const
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
	
	template<typename T>
	FORCEINLINE void ParallelSplit(T Lambda, bool bForceSingleThread = false) const
	{
		const FIntVector Half = (Min + Max) / 2;
		ParallelFor(8, [&](int32 Index)
		{
			const FVoxelIntBox LocalBounds(
				FIntVector(
					(Index & 0x1) ? Half.X : Min.X,
					(Index & 0x2) ? Half.Y : Min.Y,
					(Index & 0x4) ? Half.Z : Min.Z),
				FIntVector(
					(Index & 0x1) ? Max.X : Half.X,
					(Index & 0x2) ? Max.Y : Half.Y,
					(Index & 0x4) ? Max.Z : Half.Z));
			Lambda(LocalBounds);
		}, bForceSingleThread);
	}
	template<typename T>
	FORCEINLINE void ParallelIterate(T Lambda, bool bForceSingleThread = false) const
	{
		ParallelFor(Size().X, [&](int32 Index)
		{
			const int32 X = Min.X + Index;
			checkVoxelSlow(X < Max.X);
			for (int32 Y = Min.Y; Y < Max.Y; Y++)
			{
				for (int32 Z = Min.Z; Z < Max.Z; Z++)
				{
					Lambda(X, Y, Z);
				}
			}
		}, bForceSingleThread);
	}

	// MaxBorderSize: if we do a 180 rotation for example, min and max are inverted
	// If we don't work on values that are actually inside the box, the resulting box will be wrong
	template<EInverseTransform Inverse = EInverseTransform::False>
	FVoxelIntBox ApplyTransform(const FTransform& Transform, int32 MaxBorderSize = 1) const
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
		return FVoxelIntBox(NewMin, NewMax + MaxBorderSize);
	}
};

FORCEINLINE uint32 GetTypeHash(const FVoxelIntBox& Box)
{
	static_assert(sizeof(FVoxelIntBox) == 6 * sizeof(int32), "Alignement error");
	return FCrc::MemCrc32(&Box, sizeof(FVoxelIntBox));
}

FORCEINLINE FVoxelIntBox operator*(const FVoxelIntBox& Box, int32 Scale)
{
	FVoxelIntBox Copy = Box;
	return Copy *= Scale;
}

FORCEINLINE FVoxelIntBox operator*(int32 Scale, const FVoxelIntBox& Box)
{
	FVoxelIntBox Copy = Box;
	return Copy *= Scale;
}

FORCEINLINE FVoxelIntBox operator+(const FVoxelIntBox& Box, const FVoxelIntBox& Other)
{
	FVoxelIntBox Copy = Box;
	Copy.Min = FVoxelUtilities::ComponentMin(Copy.Min, Other.Min);
	Copy.Max = FVoxelUtilities::ComponentMax(Copy.Max, Other.Max);
	return Copy;
}

FORCEINLINE FVoxelIntBox operator+(const FVoxelIntBox& Box, const FIntVector& Point)
{
	return Box + FVoxelIntBox(Point);
}

FORCEINLINE FVoxelIntBox operator+(const FVoxelIntBox& Box, const FVector& Point)
{
	return Box + FVoxelIntBox(Point);
}

FORCEINLINE FArchive& operator<<(FArchive& Ar, FVoxelIntBox& Box)
{
	Ar << Box.Min;
	Ar << Box.Max;
	return Ar;
}

// Voxel Int Box with a IsValid flag
USTRUCT(BlueprintType, meta=(HasNativeMake="Voxel.VoxelIntBoxLibrary.MakeIntBoxWithValidity", HasNativeBreak="Voxel.VoxelIntBoxLibrary.BreakIntBoxWithValidity"))
struct FVoxelIntBoxWithValidity
{
	GENERATED_BODY()
	
	FVoxelIntBoxWithValidity() = default;
	FVoxelIntBoxWithValidity(const FVoxelIntBox& Box)
		: Box(Box)
		, bValid(true)
	{
	}

	FORCEINLINE const FVoxelIntBox& GetBox() const
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

	FORCEINLINE FVoxelIntBoxWithValidity& operator=(const FVoxelIntBox& Other)
	{
		Box = Other;
		bValid = true;
		return *this;
	}
	
	FORCEINLINE bool operator==(const FVoxelIntBoxWithValidity& Other) const
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
	FORCEINLINE bool operator!=(const FVoxelIntBoxWithValidity& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE FVoxelIntBoxWithValidity& operator+=(const FVoxelIntBox& Other)
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
	FORCEINLINE FVoxelIntBoxWithValidity& operator+=(const FVoxelIntBoxWithValidity& Other)
	{
		if (Other.bValid)
		{
			*this += Other.GetBox();
		}
		return *this;
	}

	FORCEINLINE FVoxelIntBoxWithValidity& operator+=(const FIntVector& Point)
	{
		return *this += FVoxelIntBox(Point);
	}

	template<typename T>
	FORCEINLINE FVoxelIntBoxWithValidity& operator+=(const TArray<T>& Other)
	{
		for (auto& It : Other)
		{
			*this += It;
		}
		return *this;
	}
	
private:
	UPROPERTY()
	FVoxelIntBox Box;
	
	UPROPERTY()
	bool bValid = false;
};

template<typename T>
FORCEINLINE FVoxelIntBoxWithValidity operator+(const FVoxelIntBoxWithValidity& Box, const T& Other)
{
	FVoxelIntBoxWithValidity Copy = Box;
	return Copy += Other;
}

UE_DEPRECATED(4.24, "Please use FVoxelIntBox instead of FIntBox.")
typedef FVoxelIntBox FIntBox;

UE_DEPRECATED(4.24, "Please use FVoxelIntBoxWithValidity instead of FIntBoxWithValidity.")
typedef FVoxelIntBoxWithValidity FIntBoxWithValidity;