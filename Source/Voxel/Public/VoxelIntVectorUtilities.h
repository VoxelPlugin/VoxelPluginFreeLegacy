// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMath.h"

namespace FVoxelIntVector
{
	inline FIntVector RoundToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::RoundToInt(Vector.X),
			FMath::RoundToInt(Vector.Y),
			FMath::RoundToInt(Vector.Z));
	}
	inline FIntVector FloorToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::FloorToInt(Vector.X),
			FMath::FloorToInt(Vector.Y),
			FMath::FloorToInt(Vector.Z));
	}
	inline FIntVector CeilToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::CeilToInt(Vector.X),
			FMath::CeilToInt(Vector.Y),
			FMath::CeilToInt(Vector.Z));
	}
	inline FIntVector Max(const FIntVector& A, const FIntVector& B)
	{
		return FIntVector(
			FMath::Max(A.X, B.X),
			FMath::Max(A.Y, B.Y),
			FMath::Max(A.Z, B.Z));
	}
	inline FIntVector Min(const FIntVector& A, const FIntVector& B)
	{
		return FIntVector(
			FMath::Min(A.X, B.X),
			FMath::Min(A.Y, B.Y),
			FMath::Min(A.Z, B.Z));
	}

	inline FIntVector Clamp(const FIntVector& V, int Min, int Max)
	{
		return FIntVector(
			FMath::Clamp(V.X, Min, Max),
			FMath::Clamp(V.Y, Min, Max),
			FMath::Clamp(V.Z, Min, Max));
	}
	inline FIntVector DivideFloor(const FIntVector& V, int Divisor)
	{
		return FIntVector(
			FVoxelMath::DivideFloor(V.X, Divisor),
			FVoxelMath::DivideFloor(V.Y, Divisor),
			FVoxelMath::DivideFloor(V.Z, Divisor));
	}
	inline uint64 SquaredSize(const FIntVector& V)
	{
		return V.X * V.X + V.Y * V.Y + V.Z * V.Z;
	}
};

inline FIntVector operator-(const FIntVector& V)
{
	return FIntVector(-V.X, -V.Y, -V.Z);
}
inline FIntVector operator*(int I, const FIntVector& V)
{
	return FIntVector(I * V.X, I * V.Y, I * V.Z);
}

DEPRECATED("0", "Don't use this operator as it casts to int32 the float operand")
inline FIntVector operator*(const FIntVector& V, float A)
{
	check(false);
	return V;
}

