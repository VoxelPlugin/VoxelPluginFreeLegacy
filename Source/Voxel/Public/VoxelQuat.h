// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVector.h"

struct FVoxelQuat 
{
public:
	v_flt X;
	v_flt Y;
	v_flt Z;
	v_flt W;

public:
	static VOXEL_API const FVoxelQuat Identity;
	
	FORCEINLINE FVoxelQuat()
	{
	}
	explicit FORCEINLINE FVoxelQuat(EForceInit ZeroOrNot)
		: X(0)
		, Y(0)
		, Z(0)
		, W(ZeroOrNot == ForceInitToZero ? 0.0f : 1.0f)
	{
	}
	FORCEINLINE FVoxelQuat(v_flt InX, v_flt InY, v_flt InZ, v_flt InW)
		: X(InX)
		, Y(InY)
		, Z(InZ)
		, W(InW)
	{
	}
	FORCEINLINE FVoxelQuat(const FQuat& Quat)
		: X(Quat.X)
		, Y(Quat.Y)
		, Z(Quat.Z)
		, W(Quat.W)
	{
	}
	
	FORCEINLINE FQuat ToFloat() const
	{
		return FQuat(X, Y, Z, W);
	}

	FORCEINLINE FVoxelQuat Inverse() const
	{
		checkVoxelSlow(IsNormalized());
		return FVoxelQuat(-X, -Y, -Z, W);
	}
	FORCEINLINE FVoxelVector RotateVector(FVoxelVector V) const
	{
		// http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
		// V' = V + 2w(Q x V) + (2Q x (Q x V))
		// refactor:
		// V' = V + w(2(Q x V)) + (Q x (2(Q x V)))
		// T = 2(Q x V);
		// V' = V + w*(T) + (Q x T)

		const FVoxelVector Q(X, Y, Z);
		const FVoxelVector T = 2.f * FVoxelVector::CrossProduct(Q, V);
		const FVoxelVector Result = V + (W * T) + FVoxelVector::CrossProduct(Q, T);
		return Result;
	}
	FORCEINLINE FVoxelVector UnrotateVector(FVoxelVector V) const
	{
		const FVoxelVector Q(-X, -Y, -Z); // Inverse
		const FVoxelVector T = 2.f * FVoxelVector::CrossProduct(Q, V);
		const FVoxelVector Result = V + (W * T) + FVoxelVector::CrossProduct(Q, T);
		return Result;
	}

	FORCEINLINE v_flt SizeSquared() const
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

	FORCEINLINE FVoxelVector operator*(const FVoxelVector& V) const
	{
		return RotateVector(V);
	}
	FORCEINLINE FVoxelQuat operator*(const FVoxelQuat& Q) const
	{
		FVoxelQuat Result;
		VectorQuaternionMultiply(Result, *this, Q);

		Result.DiagnosticCheckNaN();

		return Result;
	}

	static FORCEINLINE void VectorQuaternionMultiply(FVoxelQuat& R, const FVoxelQuat& A, const FVoxelQuat& B)
	{
		// store intermediate results in temporaries
		const v_flt TX = A.W * B.X + A.X * B.W + A.Y * B.Z - A.Z * B.Y;
		const v_flt TY = A.W * B.Y - A.X * B.Z + A.Y * B.W + A.Z * B.X;
		const v_flt TZ = A.W * B.Z + A.X * B.Y - A.Y * B.X + A.Z * B.W;
		const v_flt TW = A.W * B.W - A.X * B.X - A.Y * B.Y - A.Z * B.Z;

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
			*const_cast<FVoxelQuat*>(this) = FQuat::Identity;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN(const TCHAR* Message) const
	{
		if (ContainsNaN())
		{
			ensureMsgf(false, TEXT("%s: FQuat contains NaN: %s"), Message, *ToString());
			*const_cast<FVoxelQuat*>(this) = FQuat::Identity;
		}
	}
#else
	FORCEINLINE void DiagnosticCheckNaN() const {}
	FORCEINLINE void DiagnosticCheckNaN(const TCHAR* Message) const {}
#endif
};