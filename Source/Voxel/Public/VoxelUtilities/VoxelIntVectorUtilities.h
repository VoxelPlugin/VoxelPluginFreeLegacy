// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

namespace FVoxelUtilities
{
	FORCEINLINE FIntVector RoundToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::RoundToInt(Vector.X),
			FMath::RoundToInt(Vector.Y),
			FMath::RoundToInt(Vector.Z));
	}
	FORCEINLINE FIntVector FloorToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::FloorToInt(Vector.X),
			FMath::FloorToInt(Vector.Y),
			FMath::FloorToInt(Vector.Z));
	}
	FORCEINLINE FIntVector CeilToInt(const FVector& Vector)
	{
		return FIntVector(
			FMath::CeilToInt(Vector.X),
			FMath::CeilToInt(Vector.Y),
			FMath::CeilToInt(Vector.Z));
	}

	FORCEINLINE FIntVector Abs(const FIntVector& Vector)
	{
		return FIntVector(
			FMath::Abs(Vector.X),
			FMath::Abs(Vector.Y),
			FMath::Abs(Vector.Z));
	}

	FORCEINLINE FIntVector ComponentMax(const FIntVector& A, const FIntVector& B)
	{
		return FIntVector(
			FMath::Max(A.X, B.X),
			FMath::Max(A.Y, B.Y),
			FMath::Max(A.Z, B.Z));
	}
	FORCEINLINE FIntVector ComponentMin(const FIntVector& A, const FIntVector& B)
	{
		return FIntVector(
			FMath::Min(A.X, B.X),
			FMath::Min(A.Y, B.Y),
			FMath::Min(A.Z, B.Z));
	}

	FORCEINLINE FIntVector ComponentMin3(const FIntVector& A, const FIntVector& B, const FIntVector& C)
	{
		return ComponentMin(A, ComponentMin(B, C));
	}
	FORCEINLINE FIntVector ComponentMax3(const FIntVector& A, const FIntVector& B, const FIntVector& C)
	{
		return ComponentMax(A, ComponentMax(B, C));
	}

	FORCEINLINE FVector ComponentMin3(const FVector& A, const FVector& B, const FVector& C)
	{
		return A.ComponentMin(B.ComponentMin(C));
	}
	FORCEINLINE FVector ComponentMax3(const FVector& A, const FVector& B, const FVector& C)
	{
		return A.ComponentMax(B.ComponentMax(C));
	}

	FORCEINLINE bool CountIs32Bits(const FIntVector& Size)
	{
		return FMath::Abs(int64(Size.X) * int64(Size.Y) * int64(Size.Z)) < MAX_int32;
	}

	// Defaults to the "lowest" axis if equal (will return X if X and Y are equal)
	template<typename TVector>
	FORCEINLINE int32 GetArgMin(const TVector& V)
	{
		if (V.X <= V.Y && V.X <= V.Z)
		{
			return 0;
		}
		else if (V.Y <= V.Z)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}
	// Defaults to the "lowest" axis if equal (will return X if X and Y are equal)
	template<typename TVector>
	FORCEINLINE int32 GetArgMax(const TVector& V)
	{
		if (V.X >= V.Y && V.X >= V.Z)
		{
			return 0;
		}
		else if (V.Y >= V.Z)
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	FORCEINLINE FIntVector Clamp(const FIntVector& V, const FIntVector& Min, const FIntVector& Max)
	{
		return FIntVector(
			FMath::Clamp(V.X, Min.X, Max.X),
			FMath::Clamp(V.Y, Min.Y, Max.Y),
			FMath::Clamp(V.Z, Min.Z, Max.Z));
	}
	FORCEINLINE FIntVector DivideFloor(const FIntVector& V, int32 Divisor)
	{
		return FIntVector(
			DivideFloor(V.X, Divisor),
			DivideFloor(V.Y, Divisor),
			DivideFloor(V.Z, Divisor));
	}
	FORCEINLINE FIntVector DivideCeil(const FIntVector& V, int32 Divisor)
	{
		return FIntVector(
			DivideCeil(V.X, Divisor),
			DivideCeil(V.Y, Divisor),
			DivideCeil(V.Z, Divisor));
	}
	FORCEINLINE FIntVector DivideRound(const FIntVector& V, int32 Divisor)
	{
		return FIntVector(
			DivideRound(V.X, Divisor),
			DivideRound(V.Y, Divisor),
			DivideRound(V.Z, Divisor));
	}
	FORCEINLINE uint64 SquaredSize(const FIntVector& V)
	{
		return FMath::Square<int64>(V.X) + FMath::Square<int64>(V.Y) + FMath::Square<int64>(V.Z);
	}

	FORCEINLINE TVoxelStaticArray<FIntVector, 8> GetNeighbors(const FVector& P)
	{
		const int32 MinX = FMath::FloorToInt(P.X);
		const int32 MinY = FMath::FloorToInt(P.Y);
		const int32 MinZ = FMath::FloorToInt(P.Z);

		const int32 MaxX = FMath::CeilToInt(P.X);
		const int32 MaxY = FMath::CeilToInt(P.Y);
		const int32 MaxZ = FMath::CeilToInt(P.Z);

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
	FORCEINLINE TVoxelStaticArray<FIntPoint, 4> GetNeighbors(float X, float Y)
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);

		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);

		return {
		FIntPoint(MinX, MinY),
		FIntPoint(MaxX, MinY),
		FIntPoint(MinX, MaxY),
		FIntPoint(MaxX, MaxY)
		};
	}
	
	inline TArray<FIntVector, TFixedAllocator<6>> GetImmediateNeighbors(const FIntVector& V)
	{
		return {
			FIntVector(V.X - 1, V.Y, V.Z),
			FIntVector(V.X + 1, V.Y, V.Z),
			FIntVector(V.X, V.Y - 1, V.Z),
			FIntVector(V.X, V.Y + 1, V.Z),
			FIntVector(V.X, V.Y, V.Z - 1),
			FIntVector(V.X, V.Y, V.Z + 1)
		};
	}
	inline void AddImmediateNeighborsToArray(const FIntVector& V, TArray<FIntVector>& Array)
	{
		const int32& X = V.X;
		const int32& Y = V.Y;
		const int32& Z = V.Z;

		const uint32 Pos = Array.AddUninitialized(6);
		FIntVector* Ptr = Array.GetData() + Pos;

		new (Ptr++) FIntVector(X - 1, Y, Z);
		new (Ptr++) FIntVector(X + 1, Y, Z);

		new (Ptr++) FIntVector(X, Y - 1, Z);
		new (Ptr++) FIntVector(X, Y + 1, Z);

		new (Ptr++) FIntVector(X, Y, Z - 1);
		new (Ptr++) FIntVector(X, Y, Z + 1);

		checkVoxelSlow(Ptr == Array.GetData() + Array.Num());
	}

	FORCEINLINE uint32 MurmurHash(const FIntVector& V)
	{
		return
			FVoxelUtilities::MurmurHash32(V.X) ^
			FVoxelUtilities::MurmurHash32(V.Y) ^
			FVoxelUtilities::MurmurHash32(V.Z);
	}
};

FORCEINLINE FIntVector operator-(const FIntVector& V)
{
	return FIntVector(-V.X, -V.Y, -V.Z);
}

FORCEINLINE FIntVector operator-(const FIntVector& V, int32 I)
{
	return FIntVector(V.X - I, V.Y - I, V.Z - I);
}
FORCEINLINE FIntVector operator-(const FIntVector& V, uint32 I)
{
	return FIntVector(V.X - I, V.Y - I, V.Z - I);
}
FORCEINLINE FIntVector operator-(int32 I, const FIntVector& V)
{
	return FIntVector(I - V.X, I - V.Y, I - V.Z);
}
FORCEINLINE FIntVector operator-(uint32 I, const FIntVector& V)
{
	return FIntVector(I - V.X, I - V.Y, I - V.Z);
}

FORCEINLINE FIntVector operator+(const FIntVector& V, int32 I)
{
	return FIntVector(V.X + I, V.Y + I, V.Z + I);
}
FORCEINLINE FIntVector operator+(const FIntVector& V, uint32 I)
{
	return FIntVector(V.X + I, V.Y + I, V.Z + I);
}
FORCEINLINE FIntVector operator+(int32 I, const FIntVector& V)
{
	return FIntVector(I + V.X, I + V.Y, I + V.Z);
}
FORCEINLINE FIntVector operator+(uint32 I, const FIntVector& V)
{
	return FIntVector(I + V.X, I + V.Y, I + V.Z);
}

FORCEINLINE FIntVector operator*(int32 I, const FIntVector& V)
{
	return FIntVector(I * V.X, I * V.Y, I * V.Z);
}
FORCEINLINE FIntVector operator*(uint32 I, const FIntVector& V)
{
	return FIntVector(I * V.X, I * V.Y, I * V.Z);
}
FORCEINLINE FIntVector operator*(const FIntVector& V, uint32 I)
{
	return FIntVector(I * V.X, I * V.Y, I * V.Z);
}
FORCEINLINE FIntVector operator*(const FIntVector& A, const FIntVector& B)
{
	return FIntVector(A.X * B.X, A.Y * B.Y, A.Z * B.Z);
}

FORCEINLINE FIntVector operator%(const FIntVector& A, const FIntVector& B)
{
	return FIntVector(A.X % B.X, A.Y % B.Y, A.Z % B.Z);
}
FORCEINLINE FIntVector operator%(const FIntVector& V, int32 I)
{
	return V % FIntVector(I);
}
FORCEINLINE FIntVector operator%(const FIntVector& V, uint32 I)
{
	return V % FIntVector(I);
}

template<typename T>
FIntVector operator-(const FIntVector& V, T A) = delete;
template<typename T>
FIntVector operator-(T A, const FIntVector& V) = delete;

template<typename T>
FIntVector operator+(const FIntVector& V, T A) = delete;
template<typename T>
FIntVector operator+(T A, const FIntVector& V) = delete;

template<typename T>
FIntVector operator*(const FIntVector& V, T A) = delete;
template<typename T>
FIntVector operator*(T A, const FIntVector& V) = delete;

template<typename T>
FIntVector operator%(const FIntVector& V, T A) = delete;