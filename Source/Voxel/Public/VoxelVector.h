// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

// We use std functions here as they support both float and doubles
#include <cmath>

// Vector that optionally has double precision
struct FVoxelVector
{
	v_flt X;
	v_flt Y;
	v_flt Z;

	FVoxelVector() = default;
	FVoxelVector(v_flt X, v_flt Y, v_flt Z)
		: X(X)
		, Y(Y)
		, Z(Z)
	{
	}
	explicit FVoxelVector(EForceInit)
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
	{
	}
	FVoxelVector(const FVector& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
		, Z(Vector.Z)
	{
	}
	FVoxelVector(const FIntVector& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
		, Z(Vector.Z)
	{
	}
	
	/** A zero vector (0,0,0) */
	static VOXEL_API const FVoxelVector ZeroVector;

	/** One vector (1,1,1) */
	static VOXEL_API const FVoxelVector OneVector;

	/** Unreal up vector (0,0,1) */
	static VOXEL_API const FVoxelVector UpVector;

	/** Unreal down vector (0,0,-1) */
	static VOXEL_API const FVoxelVector DownVector;

	/** Unreal forward vector (1,0,0) */
	static VOXEL_API const FVoxelVector ForwardVector;

	/** Unreal backward vector (-1,0,0) */
	static VOXEL_API const FVoxelVector BackwardVector;

	/** Unreal right vector (0,1,0) */
	static VOXEL_API const FVoxelVector RightVector;

	/** Unreal left vector (0,-1,0) */
	static VOXEL_API const FVoxelVector LeftVector;

	FORCEINLINE FIntVector ToInt() const
	{
		return FIntVector(X, Y, Z);
	}
	FORCEINLINE FVector ToFloat() const
	{
		return FVector(X, Y, Z);
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);
	}

	static FORCEINLINE FIntVector ToInt(const FVoxelVector& V)
	{
		return V.ToInt();
	}
	static FORCEINLINE FVector ToFloat(const FVoxelVector& V)
	{
		return V.ToFloat();
	}

	FORCEINLINE FVoxelVector operator+(const FVoxelVector& V) const
	{
		return FVoxelVector(X + V.X, Y + V.Y, Z + V.Z);
	}
	FORCEINLINE FVoxelVector operator+(const FIntVector& V) const
	{
		return FVoxelVector(X + V.X, Y + V.Y, Z + V.Z);
	}

	FORCEINLINE FVoxelVector operator-(const FVoxelVector& V) const
	{
		return FVoxelVector(X - V.X, Y - V.Y, Z - V.Z);
	}
	FORCEINLINE FVoxelVector operator-(const FIntVector& V) const
	{
		return FVoxelVector(X - V.X, Y - V.Y, Z - V.Z);
	}

	FORCEINLINE FVoxelVector operator-(v_flt Bias) const
	{
		return FVoxelVector(X - Bias, Y - Bias, Z - Bias);
	}

	FORCEINLINE FVoxelVector operator+(v_flt Bias) const
	{
		return FVoxelVector(X + Bias, Y + Bias, Z + Bias);
	}

	FORCEINLINE FVoxelVector operator*(v_flt Scale) const
	{
		return FVoxelVector(X * Scale, Y * Scale, Z * Scale);
	}

	FORCEINLINE FVoxelVector operator/(v_flt Scale) const
	{
		const v_flt RScale = 1.f / Scale;
		return FVoxelVector(X * RScale, Y * RScale, Z * RScale);
	}

	FORCEINLINE FVoxelVector operator*(const FVoxelVector& V) const
	{
		return FVoxelVector(X * V.X, Y * V.Y, Z * V.Z);
	}

	FORCEINLINE FVoxelVector operator/(const FVoxelVector& V) const
	{
		return FVoxelVector(X / V.X, Y / V.Y, Z / V.Z);
	}

	FORCEINLINE bool operator==(const FVoxelVector& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z;
	}

	FORCEINLINE bool operator!=(const FVoxelVector& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	FORCEINLINE bool Equals(const FVoxelVector& V, v_flt Tolerance) const
	{
		return FMath::Abs(X - V.X) <= Tolerance && FMath::Abs(Y - V.Y) <= Tolerance && FMath::Abs(Z - V.Z) <= Tolerance;
	}

	FORCEINLINE bool AllComponentsEqual(v_flt Tolerance) const
	{
		return FMath::Abs(X - Y) <= Tolerance && FMath::Abs(X - Z) <= Tolerance && FMath::Abs(Y - Z) <= Tolerance;
	}


	FORCEINLINE FVoxelVector operator-() const
	{
		return FVoxelVector(-X, -Y, -Z);
	}


	FORCEINLINE FVoxelVector operator+=(const FVoxelVector& V)
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}

	FORCEINLINE FVoxelVector operator-=(const FVoxelVector& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}

	FORCEINLINE FVoxelVector operator*=(v_flt Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}

	FORCEINLINE FVoxelVector operator/=(v_flt V)
	{
		const v_flt RV = 1.f / V;
		X *= RV; Y *= RV; Z *= RV;
		return *this;
	}

	FORCEINLINE FVoxelVector operator*=(const FVoxelVector& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z;
		return *this;
	}

	FORCEINLINE FVoxelVector operator/=(const FVoxelVector& V)
	{
		X /= V.X; Y /= V.Y; Z /= V.Z;
		return *this;
	}

	FORCEINLINE v_flt& operator[](int32 Index)
	{
		check(Index >= 0 && Index < 3);
		return (&X)[Index];
	}

	FORCEINLINE v_flt operator[](int32 Index)const
	{
		check(Index >= 0 && Index < 3);
		return (&X)[Index];
	}
	
	FORCEINLINE v_flt GetMax() const
	{
		return FMath::Max(FMath::Max(X, Y), Z);
	}

	FORCEINLINE v_flt GetAbsMax() const
	{
		return FMath::Max(FMath::Max(FMath::Abs(X), FMath::Abs(Y)), FMath::Abs(Z));
	}

	FORCEINLINE v_flt GetMin() const
	{
		return FMath::Min(FMath::Min(X, Y), Z);
	}

	FORCEINLINE v_flt GetAbsMin() const
	{
		return FMath::Min(FMath::Min(FMath::Abs(X), FMath::Abs(Y)), FMath::Abs(Z));
	}

	FORCEINLINE FVoxelVector ComponentMin(const FVoxelVector& Other) const
	{
		return FVoxelVector(FMath::Min(X, Other.X), FMath::Min(Y, Other.Y), FMath::Min(Z, Other.Z));
	}

	FORCEINLINE FVoxelVector ComponentMax(const FVoxelVector& Other) const
	{
		return FVoxelVector(FMath::Max(X, Other.X), FMath::Max(Y, Other.Y), FMath::Max(Z, Other.Z));
	}

	FORCEINLINE FVoxelVector GetAbs() const
	{
		return FVoxelVector(FMath::Abs(X), FMath::Abs(Y), FMath::Abs(Z));
	}

	FORCEINLINE v_flt Size() const
	{
		return std::sqrt(X * X + Y * Y + Z * Z);
	}

	FORCEINLINE v_flt SizeSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}

	FORCEINLINE FVoxelVector GetSafeNormal(v_flt Tolerance = SMALL_NUMBER) const
	{
		const v_flt SquareSum = X * X + Y * Y + Z * Z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			return *this;
		}
		else if (SquareSum < Tolerance)
		{
			return FVoxelVector(0, 0, 0);
		}

		return *this / std::sqrt(SquareSum);
	}

	FORCEINLINE bool Normalize(v_flt Tolerance = SMALL_NUMBER)
	{
		const v_flt SquareSum = X * X + Y * Y + Z * Z;
		if (SquareSum > Tolerance)
		{
			*this /= std::sqrt(SquareSum);
			return true;
		}
		return false;
	}

	FORCEINLINE FVoxelVector operator^(const FVoxelVector& V) const
	{
		return FVoxelVector
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}

	FORCEINLINE float operator|(const FVoxelVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	static FORCEINLINE float DotProduct(const FVoxelVector& A, const FVoxelVector& B)
	{
		return A | B;
	}
	
	static FORCEINLINE float Dist(const FVoxelVector& V1, const FVoxelVector& V2)
	{
		return FMath::Sqrt(DistSquared(V1, V2));
	}
	static FORCEINLINE float Distance(const FVoxelVector& V1, const FVoxelVector& V2)
	{
		return Dist(V1, V2);
	}

	static FORCEINLINE float DistXY(const FVoxelVector& V1, const FVoxelVector& V2)
	{
		return FMath::Sqrt(DistSquaredXY(V1, V2));
	}

	static FORCEINLINE float DistSquared(const FVoxelVector& V1, const FVoxelVector& V2)
	{
		return FMath::Square(V2.X - V1.X) + FMath::Square(V2.Y - V1.Y) + FMath::Square(V2.Z - V1.Z);
	}

	static FORCEINLINE float DistSquaredXY(const FVoxelVector& V1, const FVoxelVector& V2)
	{
		return FMath::Square(V2.X - V1.X) + FMath::Square(V2.Y - V1.Y);
	}
};

FORCEINLINE FVoxelVector operator*(v_flt Scale, const FVoxelVector& V)
{
	return V.operator*(Scale);
}

FORCEINLINE FVoxelVector operator-(const FVector& A, const FVoxelVector& B)
{
	return FVoxelVector(A) - B;
}
FORCEINLINE FVoxelVector operator-(const FIntVector& A, const FVoxelVector& B)
{
	return FVoxelVector(A) - B;
}

FORCEINLINE FVoxelVector operator+(const FVector& A, const FVoxelVector& B)
{
	return FVoxelVector(A) + B;
}
FORCEINLINE FVoxelVector operator+(const FIntVector& A, const FVoxelVector& B)
{
	return FVoxelVector(A) + B;
}

struct FVoxelVector2D
{
	v_flt X;
	v_flt Y;

	FVoxelVector2D() = default;
	FVoxelVector2D(v_flt X, v_flt Y)
		: X(X)
		, Y(Y)
	{
	}
	explicit FVoxelVector2D(EForceInit)
		: X(0.0f)
		, Y(0.0f)
	{
	}
	FVoxelVector2D(const FVector2D& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
	{
	}
	FVoxelVector2D(const FIntPoint& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
	{
	}

	FORCEINLINE v_flt Size() const
	{
		return std::sqrt(X * X + Y * Y);
	}

	FORCEINLINE v_flt operator^(const FVoxelVector2D& V) const
	{
		return X * V.Y - Y * V.X;
	}

	FORCEINLINE float operator|(const FVoxelVector2D& V) const
	{
		return X * V.X + Y * V.Y;
	}

	static FORCEINLINE float DotProduct(const FVoxelVector2D& A, const FVoxelVector2D& B)
	{
		return A | B;
	}
};