// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.generated.h"

#define MAX_VOXELVALUE (MAX_int16 / 2)

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelValue
{
	GENERATED_BODY()

	const static FVoxelValue Full;
	const static FVoxelValue Empty;

public:
	FVoxelValue() : F(0) {}
    FVoxelValue(float InValue) : F(FMath::Clamp<int>(FMath::RoundToInt(FMath::Clamp<float>(InValue, -1, 1) * MAX_VOXELVALUE), MIN_int16, MAX_int16)) {}

public:
	inline bool IsNull() const { return F == 0; }
	inline bool IsValid() const { return !FMath::IsNaN(F) && FMath::IsFinite(F); }
	inline bool IsEmpty() const { return F > 0; }
	inline bool IsTotallyEmpty() const { return F >= MAX_VOXELVALUE; }
	inline bool IsTotallyFull() const { return F <= -MAX_VOXELVALUE; }
	// this / (this - A)
	inline float ThisDividedByThisMinusA(const FVoxelValue& A) const { return ToFloat() / (ToFloat() - A.ToFloat()); }
	inline float ThisDividedByThisMinusA(const FVoxelValue& A, bool& bSuccess) const { float Alpha = ThisDividedByThisMinusA(A); bSuccess = !FMath::IsNaN(Alpha) && FMath::IsFinite(Alpha); return Alpha; }
	inline FVoxelValue& Inverse() { F = -F; return *this; }
	inline FVoxelValue GetInverse() const { FVoxelValue V = *this; return V.Inverse(); }
	inline float ToFloat() const { return float(F) / float(MAX_VOXELVALUE); }
	inline FString ToString() const { return FString::SanitizeFloat(ToFloat()); }
	
public:	
	bool operator==(const FVoxelValue& rhs) const { return F == rhs.F; }
	bool operator!=(const FVoxelValue& rhs) const { return F != rhs.F; }
	bool operator<(const FVoxelValue& rhs) const { return F < rhs.F; }
	bool operator>(const FVoxelValue& rhs) const { return F > rhs.F; }
	bool operator<=(const FVoxelValue& rhs) const { return F <= rhs.F; }
	bool operator>=(const FVoxelValue& rhs) const { return F >= rhs.F; }
	
public:
	friend inline FArchive& operator<<(FArchive &Ar, FVoxelValue& Save)
	{
		Ar << Save.F;
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

private:
	int16 F;
};

template <>
struct TTypeTraits<FVoxelValue> : public TTypeTraitsBase<FVoxelValue>
{
	enum { IsBytewiseComparable = true };
};

template<>
struct TStructOpsTypeTraits<FVoxelValue> : public TStructOpsTypeTraitsBase2<FVoxelValue>
{
	enum 
	{
		WithSerializer = true,
		WithIdenticalViaEquality = true
	};
};