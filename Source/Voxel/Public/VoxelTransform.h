// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelQuat.h"

// Double precision transform
struct FVoxelTransform
{
public:
	FVoxelTransform() = default;

	FORCEINLINE FVoxelTransform(const FVoxelQuat& InRotation, const FVoxelVector& InTranslation, const FVoxelVector& InScale3D = FVoxelVector::OneVector)
		: Rotation(InRotation)
		, Translation(InTranslation)
		, Scale3D(InScale3D)
	{
		DiagnosticCheckNaN_All();
	}
	FORCEINLINE FVoxelTransform(const FTransform& Transform)
		: FVoxelTransform(Transform.GetRotation(), Transform.GetTranslation(), Transform.GetScale3D())
	{
	}

	FTransform ToFloat() const
	{
		return FTransform(GetRotation().ToFloat(), GetTranslation().ToFloat(), GetScale3D().ToFloat());
	}

public:
	FORCEINLINE FVoxelTransform Inverse() const
	{
		const FVoxelQuat InvRotation = Rotation.Inverse();
		const FVoxelVector InvScale3D = GetSafeScaleReciprocal(Scale3D);
		const FVoxelVector InvTranslation = InvRotation * (InvScale3D * -Translation);

		return FVoxelTransform(InvRotation, InvTranslation, InvScale3D);
	}

public:
	static FORCEINLINE bool AnyHasNegativeScale(const FVoxelVector& InScale3D, const FVoxelVector& InOtherScale3D)
	{
		return
			InScale3D.X < 0.f || 
			InScale3D.Y < 0.f || 
			InScale3D.Z < 0.f ||
			InOtherScale3D.X < 0.f || 
			InOtherScale3D.Y < 0.f || 
			InOtherScale3D.Z < 0.f;
	}
	static FORCEINLINE void Multiply(FVoxelTransform* OutTransform, const FVoxelTransform* A, const FVoxelTransform* B)
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
			*OutTransform = A->ToFloat() * B->ToFloat();
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
	FORCEINLINE FVoxelQuat GetRotation() const
	{
		DiagnosticCheckNaN_Rotate();
		return Rotation;
	}

	/**
	* Returns the translation component
	*
	* @return The translation component
	*/
	FORCEINLINE FVoxelVector GetTranslation() const
	{
		DiagnosticCheckNaN_Translate();
		return Translation;
	}

	/**
	* Returns the Scale3D component
	*
	* @return The Scale3D component
	*/
	FORCEINLINE FVoxelVector GetScale3D() const
	{
		DiagnosticCheckNaN_Scale3D();
		return Scale3D;
	}
	
	FORCEINLINE void SetTranslation(const FVoxelVector& NewTranslation)
	{
		Translation = NewTranslation;
		DiagnosticCheckNaN_Translate();
	}

	FORCEINLINE void ScaleTranslation(float Scale)
	{
		Translation *= Scale;
		DiagnosticCheckNaN_Translate();
	}

	FORCEINLINE FVoxelTransform operator*(const FVoxelTransform& Other) const
	{
		FVoxelTransform Output;
		Multiply(&Output, this, &Other);
		return Output;
	}
	FORCEINLINE void operator*=(const FVoxelTransform& Other)
	{
		Multiply(this, this, &Other);
	}

	FORCEINLINE bool operator==(const FVoxelTransform& Other) const
	{
		return Rotation == Other.Rotation
			&& Translation == Other.Translation
			&& Scale3D == Other.Scale3D;
	}
	FORCEINLINE bool operator!=(const FVoxelTransform& Other) const
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
		const FRotator R(GetRotation().ToFloat());
		const FVector T(GetTranslation().ToFloat());
		const FVector S(GetScale3D().ToFloat());

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
			ensureMsgf(false, TEXT("FVoxelTransform Scale3D contains NaN: %s"), *Scale3D.ToString());
			const_cast<FVoxelTransform*>(this)->Scale3D = FVoxelVector::OneVector;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN_Translate() const
	{
		if (Translation.ContainsNaN())
		{
			ensureMsgf(false, TEXT("FVoxelTransform Translation contains NaN: %s"), *Translation.ToString());
			const_cast<FVoxelTransform*>(this)->Translation = FVoxelVector::ZeroVector;
		}
	}

	FORCEINLINE void DiagnosticCheckNaN_Rotate() const
	{
		if (Rotation.ContainsNaN())
		{
			ensureMsgf(false, TEXT("FVoxelTransform Rotation contains NaN: %s"), *Rotation.ToString());
			const_cast<FVoxelTransform*>(this)->Rotation = FVoxelQuat::Identity;
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
			ensureMsgf(false, TEXT("FVoxelTransform transform is not valid: %s"), *ToHumanReadableString());
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
	FORCEINLINE FVoxelVector TransformPosition(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(Scale3D * V) + Translation;
	}


	FORCEINLINE FVoxelVector TransformPositionNoScale(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(V) + Translation;
	}


	FORCEINLINE FVoxelVector TransformVector(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(Scale3D * V);
	}


	FORCEINLINE FVoxelVector TransformVectorNoScale(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return Rotation.RotateVector(V);
	}


	// do backward operation when inverse, translation -> rotation -> scale
	FORCEINLINE FVoxelVector InverseTransformPosition(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V - Translation)) * GetSafeScaleReciprocal(Scale3D);
	}


	// do backward operation when inverse, translation -> rotation
	FORCEINLINE FVoxelVector InverseTransformPositionNoScale(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V - Translation));
	}


	// do backward operation when inverse, translation -> rotation -> scale
	FORCEINLINE FVoxelVector InverseTransformVector(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V)) * GetSafeScaleReciprocal(Scale3D);
	}


	// do backward operation when inverse, translation -> rotation
	FORCEINLINE FVoxelVector InverseTransformVectorNoScale(const FVoxelVector& V) const
	{
		DiagnosticCheckNaN_All();
		return (Rotation.UnrotateVector(V));
	}

	FORCEINLINE FVoxelQuat TransformRotation(const FVoxelQuat& Q) const
	{
		return GetRotation() * Q;
	}

	FORCEINLINE FVoxelQuat InverseTransformRotation(const FVoxelQuat& Q) const
	{
		return GetRotation().Inverse() * Q;
	}

private:
	/** Rotation of this transformation, as a quaternion. */
	FVoxelQuat Rotation = FVoxelQuat::Identity;
	/** Translation of this transformation, as a vector. */
	FVoxelVector Translation = FVoxelVector::ZeroVector;
	/** 3D scale (always applied in local space) as a vector. */
	FVoxelVector Scale3D = FVoxelVector::OneVector;

	/**
	 * mathematically if you have 0 scale, it should be infinite,
	 * however, in practice if you have 0 scale, and relative transform doesn't make much sense
	 * anymore because you should be instead of showing gigantic infinite mesh
	 * also returning BIG_NUMBER causes sequential NaN issues by multiplying
	 * so we hardcode as 0
	 */
	static FORCEINLINE FVoxelVector GetSafeScaleReciprocal(const FVoxelVector& InScale, v_flt Tolerance = SMALL_NUMBER)
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