// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVersionsFixup.h"

namespace FVoxelUtilities
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

	inline FIntVector Abs(const FIntVector& Vector)
	{
		return FIntVector(
			FMath::Abs(Vector.X),
			FMath::Abs(Vector.Y),
			FMath::Abs(Vector.Z));
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

	inline FIntVector Clamp(const FIntVector& V, int32 Min, int32 Max)
	{
		return FIntVector(
			FMath::Clamp(V.X, Min, Max),
			FMath::Clamp(V.Y, Min, Max),
			FMath::Clamp(V.Z, Min, Max));
	}
	inline int32 DivideFloor(int32 Dividend, int32 Divisor)
	{
		int32 Q = Dividend / Divisor;
		int32 R = Dividend % Divisor;
		if ((R != 0) && ((R < 0) != (Divisor < 0)))
		{
			Q--;
		}
		return Q;
	}
	inline FIntVector DivideFloor(const FIntVector& V, int32 Divisor)
	{
		return FIntVector(
			DivideFloor(V.X, Divisor),
			DivideFloor(V.Y, Divisor),
			DivideFloor(V.Z, Divisor));
	}
	inline uint64 SquaredSize(const FIntVector& V)
	{
		return V.X * V.X + V.Y * V.Y + V.Z * V.Z;
	}

	inline TArray<FIntVector, TFixedAllocator<8>> GetNeighbors(const FVector& P)
	{
		int32 MinX = FMath::FloorToInt(P.X);
		int32 MinY = FMath::FloorToInt(P.Y);
		int32 MinZ = FMath::FloorToInt(P.Z);

		int32 MaxX = FMath::CeilToInt(P.X);
		int32 MaxY = FMath::CeilToInt(P.Y);
		int32 MaxZ = FMath::CeilToInt(P.Z);

		return {
		FIntVector(MinX, MinY, MinZ),
		FIntVector(MaxX, MinY, MinZ),
		FIntVector(MinX, MaxY, MinZ),
		FIntVector(MaxX, MaxY, MinZ),
		FIntVector(MinX, MinY, MaxZ),
		FIntVector(MaxX, MinY, MaxZ),
		FIntVector(MinX, MaxY, MaxZ),
		FIntVector(MaxX, MaxY, MaxZ)
		};
	}
	inline TArray<FIntPoint, TFixedAllocator<4>> GetNeighbors(float X, float Y)
	{
		int32 MinX = FMath::FloorToInt(X);
		int32 MinY = FMath::FloorToInt(Y);

		int32 MaxX = FMath::CeilToInt(X);
		int32 MaxY = FMath::CeilToInt(Y);

		return {
		FIntPoint(MinX, MinY),
		FIntPoint(MaxX, MinY),
		FIntPoint(MinX, MaxY),
		FIntPoint(MaxX, MaxY)
		};
	}

	inline void AddNeighborsToArray(const FIntVector& V, TArray<FIntVector>& Array)
	{
		const int32& X = V.X;
		const int32& Y = V.Y;
		const int32& Z = V.Z;

		uint32 Pos = Array.AddUninitialized(6);
		FIntVector* Ptr = Array.GetData() + Pos;

		new (Ptr++) FIntVector(X - 1, Y, Z);
		new (Ptr++) FIntVector(X + 1, Y, Z);

		new (Ptr++) FIntVector(X, Y - 1, Z);
		new (Ptr++) FIntVector(X, Y + 1, Z);

		new (Ptr++) FIntVector(X, Y, Z - 1);
		new (Ptr++) FIntVector(X, Y, Z + 1);

		check(Ptr == Array.GetData() + Array.Num());
	}
};

inline FIntVector operator-(const FIntVector& V)
{
	return FIntVector(-V.X, -V.Y, -V.Z);
}
inline FIntVector operator*(int32 I, const FIntVector& V)
{
	return FIntVector(I * V.X, I * V.Y, I * V.Z);
}

UE_DEPRECATED("0", "Don't use this operator as it casts to int32 the float operand")
inline FIntVector operator*(const FIntVector& V, float A)
{
	check(false);
	return V;
}

