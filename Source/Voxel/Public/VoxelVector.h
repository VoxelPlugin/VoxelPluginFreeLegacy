// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

// We use std functions here as they support both float and doubles
#include <cmath>

// Double precision vector
struct FVoxelDoubleVector
{
	double X;
	double Y;
	double Z;

	FVoxelDoubleVector() = default;
	FVoxelDoubleVector(double X, double Y, double Z)
		: X(X)
		, Y(Y)
		, Z(Z)
	{
	}
	explicit FVoxelDoubleVector(EForceInit)
		: X(0.0f)
		, Y(0.0f)
		, Z(0.0f)
	{
	}
	FVoxelDoubleVector(const FVector& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
		, Z(Vector.Z)
	{
	}
	FVoxelDoubleVector(const FIntVector& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
		, Z(Vector.Z)
	{
	}
	
	/** A zero vector (0,0,0) */
	static VOXEL_API const FVoxelDoubleVector ZeroVector;

	/** One vector (1,1,1) */
	static VOXEL_API const FVoxelDoubleVector OneVector;

	/** Unreal up vector (0,0,1) */
	static VOXEL_API const FVoxelDoubleVector UpVector;

	/** Unreal down vector (0,0,-1) */
	static VOXEL_API const FVoxelDoubleVector DownVector;

	/** Unreal forward vector (1,0,0) */
	static VOXEL_API const FVoxelDoubleVector ForwardVector;

	/** Unreal backward vector (-1,0,0) */
	static VOXEL_API const FVoxelDoubleVector BackwardVector;

	/** Unreal right vector (0,1,0) */
	static VOXEL_API const FVoxelDoubleVector RightVector;

	/** Unreal left vector (0,-1,0) */
	static VOXEL_API const FVoxelDoubleVector LeftVector;

	operator FVector() const
	{
		return FVector(X, Y, Z);
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);
	}

	FORCEINLINE FVoxelDoubleVector operator+(const FVoxelDoubleVector& V) const
	{
		return FVoxelDoubleVector(X + V.X, Y + V.Y, Z + V.Z);
	}
	FORCEINLINE FVoxelDoubleVector operator+(const FIntVector& V) const
	{
		return FVoxelDoubleVector(X + V.X, Y + V.Y, Z + V.Z);
	}

	FORCEINLINE FVoxelDoubleVector operator-(const FVoxelDoubleVector& V) const
	{
		return FVoxelDoubleVector(X - V.X, Y - V.Y, Z - V.Z);
	}
	FORCEINLINE FVoxelDoubleVector operator-(const FIntVector& V) const
	{
		return FVoxelDoubleVector(X - V.X, Y - V.Y, Z - V.Z);
	}

	FORCEINLINE FVoxelDoubleVector operator-(double Bias) const
	{
		return FVoxelDoubleVector(X - Bias, Y - Bias, Z - Bias);
	}

	FORCEINLINE FVoxelDoubleVector operator+(double Bias) const
	{
		return FVoxelDoubleVector(X + Bias, Y + Bias, Z + Bias);
	}

	FORCEINLINE FVoxelDoubleVector operator*(double Scale) const
	{
		return FVoxelDoubleVector(X * Scale, Y * Scale, Z * Scale);
	}

	FORCEINLINE FVoxelDoubleVector operator/(double Scale) const
	{
		const double RScale = 1.f / Scale;
		return FVoxelDoubleVector(X * RScale, Y * RScale, Z * RScale);
	}

	FORCEINLINE FVoxelDoubleVector operator*(const FVoxelDoubleVector& V) const
	{
		return FVoxelDoubleVector(X * V.X, Y * V.Y, Z * V.Z);
	}

	FORCEINLINE FVoxelDoubleVector operator/(const FVoxelDoubleVector& V) const
	{
		return FVoxelDoubleVector(X / V.X, Y / V.Y, Z / V.Z);
	}

	FORCEINLINE bool operator==(const FVoxelDoubleVector& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z;
	}

	FORCEINLINE bool operator!=(const FVoxelDoubleVector& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	FORCEINLINE bool Equals(const FVoxelDoubleVector& V, double Tolerance) const
	{
		return FMath::Abs(X - V.X) <= Tolerance && FMath::Abs(Y - V.Y) <= Tolerance && FMath::Abs(Z - V.Z) <= Tolerance;
	}

	FORCEINLINE bool AllComponentsEqual(double Tolerance) const
	{
		return FMath::Abs(X - Y) <= Tolerance && FMath::Abs(X - Z) <= Tolerance && FMath::Abs(Y - Z) <= Tolerance;
	}


	FORCEINLINE FVoxelDoubleVector operator-() const
	{
		return FVoxelDoubleVector(-X, -Y, -Z);
	}


	FORCEINLINE FVoxelDoubleVector operator+=(const FVoxelDoubleVector& V)
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}

	FORCEINLINE FVoxelDoubleVector operator-=(const FVoxelDoubleVector& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}

	FORCEINLINE FVoxelDoubleVector operator*=(double Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}

	FORCEINLINE FVoxelDoubleVector operator/=(double V)
	{
		const double RV = 1.f / V;
		X *= RV; Y *= RV; Z *= RV;
		return *this;
	}

	FORCEINLINE FVoxelDoubleVector operator*=(const FVoxelDoubleVector& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z;
		return *this;
	}

	FORCEINLINE FVoxelDoubleVector operator/=(const FVoxelDoubleVector& V)
	{
		X /= V.X; Y /= V.Y; Z /= V.Z;
		return *this;
	}

	FORCEINLINE double& operator[](int32 Index)
	{
		check(Index >= 0 && Index < 3);
		return (&X)[Index];
	}

	FORCEINLINE double operator[](int32 Index)const
	{
		check(Index >= 0 && Index < 3);
		return (&X)[Index];
	}
	
	FORCEINLINE double GetMax() const
	{
		return FMath::Max(FMath::Max(X, Y), Z);
	}

	FORCEINLINE double GetAbsMax() const
	{
		return FMath::Max(FMath::Max(FMath::Abs(X), FMath::Abs(Y)), FMath::Abs(Z));
	}

	FORCEINLINE double GetMin() const
	{
		return FMath::Min(FMath::Min(X, Y), Z);
	}

	FORCEINLINE double GetAbsMin() const
	{
		return FMath::Min(FMath::Min(FMath::Abs(X), FMath::Abs(Y)), FMath::Abs(Z));
	}

	FORCEINLINE FVoxelDoubleVector ComponentMin(const FVoxelDoubleVector& Other) const
	{
		return FVoxelDoubleVector(FMath::Min(X, Other.X), FMath::Min(Y, Other.Y), FMath::Min(Z, Other.Z));
	}

	FORCEINLINE FVoxelDoubleVector ComponentMax(const FVoxelDoubleVector& Other) const
	{
		return FVoxelDoubleVector(FMath::Max(X, Other.X), FMath::Max(Y, Other.Y), FMath::Max(Z, Other.Z));
	}

	FORCEINLINE FVoxelDoubleVector GetAbs() const
	{
		return FVoxelDoubleVector(FMath::Abs(X), FMath::Abs(Y), FMath::Abs(Z));
	}

	FORCEINLINE bool ContainsNaN() const
	{
		return !FMath::IsFinite(X)
			|| !FMath::IsFinite(Y)
			|| !FMath::IsFinite(Z);
	}

	FORCEINLINE double Size() const
	{
		return std::sqrt(X * X + Y * Y + Z * Z);
	}

	FORCEINLINE double SizeSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}

	FORCEINLINE FVoxelDoubleVector GetSafeNormal(double Tolerance = SMALL_NUMBER) const
	{
		const double SquareSum = X * X + Y * Y + Z * Z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			return *this;
		}
		else if (SquareSum < Tolerance)
		{
			return FVoxelDoubleVector(0, 0, 0);
		}

		return *this / std::sqrt(SquareSum);
	}

	FORCEINLINE bool Normalize(double Tolerance = SMALL_NUMBER)
	{
		const double SquareSum = X * X + Y * Y + Z * Z;
		if (SquareSum > Tolerance)
		{
			*this /= std::sqrt(SquareSum);
			return true;
		}
		return false;
	}

	FORCEINLINE FVoxelDoubleVector operator^(const FVoxelDoubleVector& V) const
	{
		return FVoxelDoubleVector
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}
	static FORCEINLINE FVoxelDoubleVector CrossProduct(const FVoxelDoubleVector& A, const FVoxelDoubleVector& B)
	{
		return A ^ B;
	}

	FORCEINLINE float operator|(const FVoxelDoubleVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	static FORCEINLINE float DotProduct(const FVoxelDoubleVector& A, const FVoxelDoubleVector& B)
	{
		return A | B;
	}
	
	static FORCEINLINE float Dist(const FVoxelDoubleVector& V1, const FVoxelDoubleVector& V2)
	{
		return FMath::Sqrt(DistSquared(V1, V2));
	}
	static FORCEINLINE float Distance(const FVoxelDoubleVector& V1, const FVoxelDoubleVector& V2)
	{
		return Dist(V1, V2);
	}

	static FORCEINLINE float DistXY(const FVoxelDoubleVector& V1, const FVoxelDoubleVector& V2)
	{
		return FMath::Sqrt(DistSquaredXY(V1, V2));
	}

	static FORCEINLINE float DistSquared(const FVoxelDoubleVector& V1, const FVoxelDoubleVector& V2)
	{
		return FMath::Square(V2.X - V1.X) + FMath::Square(V2.Y - V1.Y) + FMath::Square(V2.Z - V1.Z);
	}

	static FORCEINLINE float DistSquaredXY(const FVoxelDoubleVector& V1, const FVoxelDoubleVector& V2)
	{
		return FMath::Square(V2.X - V1.X) + FMath::Square(V2.Y - V1.Y);
	}
};

FORCEINLINE FVoxelDoubleVector operator*(float Scale, const FVoxelDoubleVector& V)
{
	return V.operator*(Scale);
}
FORCEINLINE FVoxelDoubleVector operator*(double Scale, const FVoxelDoubleVector& V)
{
	return V.operator*(Scale);
}
FORCEINLINE FVoxelDoubleVector operator*(int32 Scale, const FVoxelDoubleVector& V)
{
	return V.operator*(Scale);
}

FORCEINLINE FVoxelDoubleVector operator-(const FVector& A, const FVoxelDoubleVector& B)
{
	return FVoxelDoubleVector(A) - B;
}
FORCEINLINE FVoxelDoubleVector operator-(const FIntVector& A, const FVoxelDoubleVector& B)
{
	return FVoxelDoubleVector(A) - B;
}

FORCEINLINE FVoxelDoubleVector operator+(const FVector& A, const FVoxelDoubleVector& B)
{
	return FVoxelDoubleVector(A) + B;
}
FORCEINLINE FVoxelDoubleVector operator+(const FIntVector& A, const FVoxelDoubleVector& B)
{
	return FVoxelDoubleVector(A) + B;
}

FORCEINLINE FVector operator-(const FIntVector& A, const FVector& B)
{
	return FVector(A) - B;
}
FORCEINLINE FVector operator-(const FVector& A, const FIntVector& B)
{
	return A - FVector(B);
}

FORCEINLINE FVector operator+(const FIntVector& A, const FVector& B)
{
	return FVector(A) + B;
}
FORCEINLINE FVector operator+(const FVector& A, const FIntVector& B)
{
	return A + FVector(B);
}
FORCEINLINE FVector operator+=(FVector& A, const FIntVector& B)
{
	return A += FVector(B);
}

struct FVoxelDoubleVector2D
{
	double X;
	double Y;

	FVoxelDoubleVector2D() = default;
	FVoxelDoubleVector2D(double X, double Y)
		: X(X)
		, Y(Y)
	{
	}
	explicit FVoxelDoubleVector2D(EForceInit)
		: X(0.0f)
		, Y(0.0f)
	{
	}
	FVoxelDoubleVector2D(const FVector2D& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
	{
	}
	FVoxelDoubleVector2D(const FIntPoint& Vector)
		: X(Vector.X)
		, Y(Vector.Y)
	{
	}

	FORCEINLINE double Size() const
	{
		return std::sqrt(X * X + Y * Y);
	}

	FORCEINLINE double operator^(const FVoxelDoubleVector2D& V) const
	{
		return X * V.Y - Y * V.X;
	}

	FORCEINLINE float operator|(const FVoxelDoubleVector2D& V) const
	{
		return X * V.X + Y * V.Y;
	}

	static FORCEINLINE float DotProduct(const FVoxelDoubleVector2D& A, const FVoxelDoubleVector2D& B)
	{
		return A | B;
	}
};

#if VOXEL_DOUBLE_PRECISION
using FVoxelVector = FVoxelDoubleVector;
using FVoxelVector2D = FVoxelDoubleVector2D;
#else
using FVoxelVector = FVector;
using FVoxelVector2D = FVector2D;
#endif