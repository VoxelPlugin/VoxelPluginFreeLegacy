// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVector.h"

struct FVoxelDoubleQuat 
{
public:
	double X;
	double Y;
	double Z;
	double W;

public:
	static VOXEL_API const FVoxelDoubleQuat Identity;
	
	FORCEINLINE FVoxelDoubleQuat()
	{
	}
	explicit FORCEINLINE FVoxelDoubleQuat(EForceInit ZeroOrNot)
		: X(0)
		, Y(0)
		, Z(0)
		, W(ZeroOrNot == ForceInitToZero ? 0.0f : 1.0f)
	{
	}
	FORCEINLINE FVoxelDoubleQuat(double InX, double InY, double InZ, double InW)
		: X(InX)
		, Y(InY)
		, Z(InZ)
		, W(InW)
	{
	}
	FORCEINLINE FVoxelDoubleQuat(const FQuat& Quat)
		: X(Quat.X)
		, Y(Quat.Y)
		, Z(Quat.Z)
		, W(Quat.W)
	{
	}
	
	FORCEINLINE operator FQuat() const
	{
		return FQuat(X, Y, Z, W);
	}
	FORCEINLINE operator FRotator() const
	{
		return Rotator();
	}
	FORCEINLINE FRotator Rotator() const
	{
		const FQuat Quat = *this;
		return FRotator(Quat);
	}

	FORCEINLINE FVoxelDoubleQuat Inverse() const
	{
		checkVoxelSlow(IsNormalized());
		return FVoxelDoubleQuat(-X, -Y, -Z, W);
	}
	FORCEINLINE FVoxelDoubleVector RotateVector(FVoxelDoubleVector V) const
	{
		// http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
		// V' = V + 2w(Q x V) + (2Q x (Q x V))
		// refactor:
		// V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
		// T = 2(Q x V);
		// V' = V + w*(T) + (Q x T)

		const FVoxelDoubleVector Q(X, Y, Z);
		const FVoxelDoubleVector T = 2.f * FVoxelDoubleVector::CrossProduct(Q, V);
		const FVoxelDoubleVector Result = V + (W * T) + FVoxelDoubleVector::CrossProduct(Q, T);
		return Result;
	}
	FORCEINLINE FVoxelDoubleVector UnrotateVector(FVoxelDoubleVector V) const
	{
		const FVoxelDoubleVector Q(-X, -Y, -Z); // Inverse
		const FVoxelDoubleVector T = 2.f * FVoxelDoubleVector::CrossProduct(Q, V);
		const FVoxelDoubleVector Result = V + (W * T) + FVoxelDoubleVector::CrossProduct(Q, T);
		return Result;
	}

	FORCEINLINE double SizeSquared() const
	{
		return (X * X + Y * Y + Z * Z + W * W);
	}
	FORCEINLINE bool IsNormalized() const
	{
		return (FMath::Abs(1.f - SizeSquared()) < THRESH_QUAT_NORMALIZED);
	}
	FORCEINLINE bool ContainsNaN() const
	{
		return !FMath::IsFinite(X)
			|| !FMath::IsFinite(Y)
			|| !FMath::IsFinite(Z)
			|| !FMath::IsFinite(W);
	}
	FORCEINLINE FString ToString() const
	{
		return FString::Printf(TEXT("X=%.9f Y=%.9f Z=%.9f W=%.9f"), X, Y, Z, W);
	}

	FORCEINLINE FVoxelDoubleVector operator*(const FVoxelDoubleVector& V) const
	{
		return RotateVector(V);
	}
	FORCEINLINE FVoxelDoubleQuat operator*(const FVoxelDoubleQuat& Q) const
	{
		FVoxelDoubleQuat Result;
		VectorQuaternionMultiply(Result, *this, Q);

		Result.DiagnosticCheckNaN();

		return Result;
	}

	FORCEINLINE bool operator==(const FVoxelDoubleQuat& Other) const
	{
		return X == Other.X
			&& Y == Other.Y
			&& Z == Other.Z
			&& W == Other.W;
	}

	static FORCEINLINE void VectorQuaternionMultiply(FVoxelDoubleQuat& R, const FVoxelDoubleQuat& A, const FVoxelDoubleQuat& B)
	{
		// store intermediate results in temporaries
		const double TX = A.W * B.X + A.X * B.W + A.Y * B.Z - A.Z * B.Y;
		const double TY = A.W * B.Y - A.X * B.Z + A.Y * B.W + A.Z * B.X;
		const double TZ = A.W * B.Z + A.X * B.Y - A.Y * B.X + A.Z * B.W;
		const double TW = A.W * B.W - A.X * B.X - A.Y * B.Y - A.Z * B.Z;

		// copy intermediate result to *this
		R.X = TX;
		R.Y = TY;
		R.Z = TZ;
		R.W = TW;
	}

public:
#if VOXEL_ENABLE_NAN_DIAGNOSTIC
	FORCEINLINE void DiagnosticCheckNaN() const
	{
		if (ContainsNaN())
		{
			ensureMsgf(false, TEXT("FQuat contains NaN: %s"), *ToString());
			*const_cast<FVoxelDoubleQuat*>(this) = FQuat::Identity;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN(const TCHAR* Message) const
	{
		if (ContainsNaN())
		{
			ensureMsgf(false, TEXT("%s: FQuat contains NaN: %s"), Message, *ToString());
			*const_cast<FVoxelDoubleQuat*>(this) = FQuat::Identity;
		}
	}
#else
	FORCEINLINE void DiagnosticCheckNaN() const {}
	FORCEINLINE void DiagnosticCheckNaN(const TCHAR* Message) const {}
#endif
};

#if VOXEL_DOUBLE_PRECISION
using FVoxelQuat = FVoxelDoubleQuat;
#else
using FVoxelQuat = FQuat;
#endif