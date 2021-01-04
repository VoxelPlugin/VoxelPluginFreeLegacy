// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelQuat.h"

// Double precision transform
struct FVoxelDoubleTransform
{
public:
	FVoxelDoubleTransform() = default;

	FORCEINLINE FVoxelDoubleTransform(const FVoxelDoubleQuat& InRotation, const FVoxelDoubleVector& InTranslation, const FVoxelDoubleVector& InScale3D = FVoxelDoubleVector::OneVector)
		: Rotation(InRotation)
		, Translation(InTranslation)
		, Scale3D(InScale3D)
	{
		DiagnosticCheckNaN_All();
	}
	FORCEINLINE FVoxelDoubleTransform(const FTransform& Transform)
		: FVoxelDoubleTransform(Transform.GetRotation(), Transform.GetTranslation(), Transform.GetScale3D())
	{
	}

	operator FTransform() const
	{
		return FTransform(GetRotation().Rotator(), GetTranslation(), GetScale3D());
	}

public:
	FORCEINLINE FVoxelDoubleTransform Inverse() const
	{
		const FVoxelDoubleQuat InvRotation = Rotation.Inverse();
		const FVoxelDoubleVector InvScale3D = GetSafeScaleReciprocal(Scale3D);
		const FVoxelDoubleVector InvTranslation = InvRotation * (InvScale3D * -Translation);

		return FVoxelDoubleTransform(InvRotation, InvTranslation, InvScale3D);
	}

public:
	static FORCEINLINE bool AnyHasNegativeScale(const FVoxelDoubleVector& InScale3D, const FVoxelDoubleVector& InOtherScale3D)
	{
		return
			InScale3D.X < 0.f || 
			InScale3D.Y < 0.f || 
			InScale3D.Z < 0.f ||
			InOtherScale3D.X < 0.f || 
			InOtherScale3D.Y < 0.f || 
			InOtherScale3D.Z < 0.f;
	}
	static FORCEINLINE void Multiply(FVoxelDoubleTransform* OutTransform, const FVoxelDoubleTransform* A, const FVoxelDoubleTransform* B)
	{
		A->DiagnosticCheckNaN_All();
		B->DiagnosticCheckNaN_All();

		checkVoxelSlow(A->IsRotationNormalized());
		checkVoxelSlow(B->IsRotationNormalized());

		//	When Q = quaternion, S = single scalar scale, and T = translation
		//	QST(A) = Q(A), S(A), T(A), and QST(B) = Q(B), S(B), T(B)

		//	QST (AxB) 

		// QST(A) = Q(A)*S(A)*P*-Q(A) + T(A)
		// QST(AxB) = Q(B)*S(B)*QST(A)*-Q(B) + T(B)
		// QST(AxB) = Q(B)*S(B)*[Q(A)*S(A)*P*-Q(A) + T(A)]*-Q(B) + T(B)
		// QST(AxB) = Q(B)*S(B)*Q(A)*S(A)*P*-Q(A)*-Q(B) + Q(B)*S(B)*T(A)*-Q(B) + T(B)
		// QST(AxB) = [Q(B)*Q(A)]*[S(B)*S(A)]*P*-[Q(B)*Q(A)] + Q(B)*S(B)*T(A)*-Q(B) + T(B)

		//	Q(AxB) = Q(B)*Q(A)
		//	S(AxB) = S(A)*S(B)
		//	T(AxB) = Q(B)*S(B)*T(A)*-Q(B) + T(B)

		if (AnyHasNegativeScale(A->Scale3D, B->Scale3D))
		{
			ensureMsgf(false, TEXT("Negative scales are not supported"));
			*OutTransform = FTransform(*A) * FTransform(*B);
		}
		else
		{
			OutTransform->Rotation = B->Rotation * A->Rotation;
			OutTransform->Scale3D = A->Scale3D * B->Scale3D;
			OutTransform->Translation = B->Rotation * (B->Scale3D * A->Translation) + B->Translation;
		}

		// we do not support matrix transform when non-uniform
		// that was removed at rev 21 with UE4
	}

	/**
	* Returns the rotation component
	*
	* @return The rotation component
	*/
	FORCEINLINE FVoxelDoubleQuat GetRotation() const
	{
		DiagnosticCheckNaN_Rotate();
		return Rotation;
	}

	/**
	* Returns the translation component
	*
	* @return The translation component
	*/
	FORCEINLINE FVoxelDoubleVector GetTranslation() const
	{
		DiagnosticCheckNaN_Translate();
		return Translation;
	}

	/**
	* Returns the Scale3D component
	*
	* @return The Scale3D component
	*/
	FORCEINLINE FVoxelDoubleVector GetScale3D() const
	{
		DiagnosticCheckNaN_Scale3D();
		return Scale3D;
	}
	
	FORCEINLINE void SetTranslation(const FVoxelDoubleVector& NewTranslation)
	{
		Translation = NewTranslation;
		DiagnosticCheckNaN_Translate();
	}

	FORCEINLINE void ScaleTranslation(float Scale)
	{
		Translation *= Scale;
		DiagnosticCheckNaN_Translate();
	}

	FORCEINLINE FVoxelDoubleTransform operator*(const FVoxelDoubleTransform& Other) const
	{
		FVoxelDoubleTransform Output;
		Multiply(&Output, this, &Other);
		return Output;
	}
	FORCEINLINE void operator*=(const FVoxelDoubleTransform& Other)
	{
		Multiply(this, this, &Other);
	}

	FORCEINLINE bool operator==(const FVoxelDoubleTransform& Other) const
	{
		return Rotation == Other.Rotation
			&& Translation == Other.Translation
			&& Scale3D == Other.Scale3D;
	}
	FORCEINLINE bool operator!=(const FVoxelDoubleTransform& Other) const
	{
		return !(*this == Other);
	}

public:
	FORCEINLINE bool IsRotationNormalized() const
	{
		return Rotation.IsNormalized();
	}
	bool ContainsNaN() const
	{
		return (Translation.ContainsNaN() || Rotation.ContainsNaN() || Scale3D.ContainsNaN());
	}
	bool IsValid() const
	{
		if (ContainsNaN())
		{
			return false;
		}

		if (!Rotation.IsNormalized())
		{
			return false;
		}

		return true;
	}
	FString ToHumanReadableString() const
	{
		const FRotator R = GetRotation();
		const FVector T = GetTranslation();
		const FVector S = GetScale3D();

		FString Output = FString::Printf(TEXT("Rotation: Pitch %f Yaw %f Roll %f\r\n"), R.Pitch, R.Yaw, R.Roll);
		Output += FString::Printf(TEXT("Translation: %f %f %f\r\n"), T.X, T.Y, T.Z);
		Output += FString::Printf(TEXT("Scale3D: %f %f %f\r\n"), S.X, S.Y, S.Z);

		return Output;
	}

#if VOXEL_ENABLE_NAN_DIAGNOSTIC
	FORCEINLINE void DiagnosticCheckNaN_Scale3D() const
	{
		if (Scale3D.ContainsNaN())
		{
			ensureMsgf(false, TEXT("FVoxelDoubleTransform Scale3D contains NaN: %s"), *Scale3D.ToString());
			const_cast<FVoxelDoubleTransform*>(this)->Scale3D = FVoxelDoubleVector::OneVector;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN_Translate() const
	{
		if (Translation.ContainsNaN())
		{
			ensureMsgf(false, TEXT("FVoxelDoubleTransform Translation contains NaN: %s"), *Translation.ToString());
			const_cast<FVoxelDoubleTransform*>(this)->Translation = FVoxelDoubleVector::ZeroVector;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN_Rotate() const
	{
		if (Rotation.ContainsNaN())
		{
			ensureMsgf(false, TEXT("FVoxelDoubleTransform Rotation contains NaN: %s"), *Rotation.ToString());
			const_cast<FVoxelDoubleTransform*>(this)->Rotation = FVoxelDoubleQuat::Identity;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN_All() const
	{
		DiagnosticCheckNaN_Scale3D();
		DiagnosticCheckNaN_Rotate();
		DiagnosticCheckNaN_Translate();
	}

	FORCEINLINE void DiagnosticCheck_IsValid() const
	{
		DiagnosticCheckNaN_All();
		if (!IsValid())
		{
			ensureMsgf(false, TEXT("FVoxelDoubleTransform transform is not valid: %s"), *ToHumanReadableString());
		}
	}
#else
	FORCEINLINE void DiagnosticCheckNaN_Translate() const {}
	FORCEINLINE void DiagnosticCheckNaN_Rotate() const {}
	FORCEINLINE void DiagnosticCheckNaN_Scale3D() const {}
	FORCEINLINE void DiagnosticCheckNaN_All() const {}
	FORCEINLINE void DiagnosticCheck_IsValid() const {}
#endif

public:
	FORCEINLINE FVoxelDoubleVector TransformPosition(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(Scale3D * V) + Translation;
	}


	FORCEINLINE FVoxelDoubleVector TransformPositionNoScale(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(V) + Translation;
	}


	FORCEINLINE FVoxelDoubleVector TransformVector(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(Scale3D * V);
	}


	FORCEINLINE FVoxelDoubleVector TransformVectorNoScale(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(V);
	}


	// do backward operation when inverse, translation -> rotation -> scale
	FORCEINLINE FVoxelDoubleVector InverseTransformPosition(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V - Translation)) * GetSafeScaleReciprocal(Scale3D);
	}


	// do backward operation when inverse, translation -> rotation
	FORCEINLINE FVoxelDoubleVector InverseTransformPositionNoScale(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V - Translation));
	}


	// do backward operation when inverse, translation -> rotation -> scale
	FORCEINLINE FVoxelDoubleVector InverseTransformVector(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V)) * GetSafeScaleReciprocal(Scale3D);
	}


	// do backward operation when inverse, translation -> rotation
	FORCEINLINE FVoxelDoubleVector InverseTransformVectorNoScale(const FVoxelDoubleVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V));
	}

	FORCEINLINE FVoxelDoubleQuat TransformRotation(const FVoxelDoubleQuat& Q) const
	{
		return GetRotation() * Q;
	}

	FORCEINLINE FVoxelDoubleQuat InverseTransformRotation(const FVoxelDoubleQuat& Q) const
	{
		return GetRotation().Inverse() * Q;
	}

private:
	/** Rotation of this transformation, as a quaternion. */
	FVoxelDoubleQuat Rotation = FVoxelDoubleQuat::Identity;
	/** Translation of this transformation, as a vector. */
	FVoxelDoubleVector Translation = FVoxelDoubleVector::ZeroVector;
	/** 3D scale (always applied in local space) as a vector. */
	FVoxelDoubleVector Scale3D = FVoxelDoubleVector::OneVector;

	/**
	 * mathematically if you have 0 scale, it should be infinite,
	 * however, in practice if you have 0 scale, and relative transform doesn't make much sense
	 * anymore because you should be instead of showing gigantic infinite mesh
	 * also returning BIG_NUMBER causes sequential NaN issues by multiplying
	 * so we hardcode as 0
	 */
	static FORCEINLINE FVoxelDoubleVector GetSafeScaleReciprocal(const FVoxelDoubleVector& InScale, double Tolerance = SMALL_NUMBER)
	{
		FVector SafeReciprocalScale;
		if (FMath::Abs(InScale.X) <= Tolerance)
		{
			SafeReciprocalScale.X = 0.f;
		}
		else
		{
			SafeReciprocalScale.X = 1 / InScale.X;
		}

		if (FMath::Abs(InScale.Y) <= Tolerance)
		{
			SafeReciprocalScale.Y = 0.f;
		}
		else
		{
			SafeReciprocalScale.Y = 1 / InScale.Y;
		}

		if (FMath::Abs(InScale.Z) <= Tolerance)
		{
			SafeReciprocalScale.Z = 0.f;
		}
		else
		{
			SafeReciprocalScale.Z = 1 / InScale.Z;
		}

		return SafeReciprocalScale;
	}
};

#if VOXEL_DOUBLE_PRECISION
using FVoxelTransform = FVoxelDoubleTransform;
#else
using FVoxelTransform = FTransform;
#endif