// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelConfigEnums.h"
#include "VoxelFoliage.generated.h"

USTRUCT(BlueprintType)
struct FVoxelFoliage
{
	GENERATED_BODY()

	FVoxelFoliage()
	{		
	}
	FVoxelFoliage(EForceInit)
	{
		R = G = B = A = 0;
	}
	FVoxelFoliage(uint8 InR, uint8 InG, uint8 InB, uint8 InA)
	{
		R = InR;
		G = InG;
		B = InB;
		A = InA;
	}

public:
	static inline FVoxelFoliage NotSet()
	{
		return { 0,0,0,0 };
	}

public:
	inline uint8& GetChannel(EVoxelRGBA Layer)
	{
		switch (Layer)
		{
		default: checkVoxelSlow(false);
		case EVoxelRGBA::R: return R;
		case EVoxelRGBA::G: return G;
		case EVoxelRGBA::B: return B;
		case EVoxelRGBA::A: return A;
		}
	}
	inline uint8 GetChannel(EVoxelRGBA Layer) const
	{
		return const_cast<FVoxelFoliage&>(*this).GetChannel(Layer);
	}

	inline float GetChannelValue(EVoxelRGBA Layer) const
	{
		const uint8 Value = GetChannel(Layer);
		ensure(Value != 0);
		return (Value - 1) / 254.f;
	}
	inline void SetChannelValue(EVoxelRGBA Layer, float Value)
	{
		GetChannel(Layer) = 1 + FMath::Clamp(FMath::FloorToInt(Value * 254.999f), 0, 254);
	}

	inline bool IsChannelSet(EVoxelRGBA Layer) const
	{
		return GetChannel(Layer) != 0;
	}
	inline void ClearChannel(EVoxelRGBA Layer)
	{
		GetChannel(Layer) = 0;
	}

public:
	inline friend FArchive& operator<<(FArchive& Ar, FVoxelFoliage& Foliage)
	{
		Ar << Foliage.R;
		Ar << Foliage.G;
		Ar << Foliage.B;
		Ar << Foliage.A;
		return Ar;
	}
	friend bool operator==(const FVoxelFoliage& Lhs, const FVoxelFoliage& Rhs)
	{
		return Lhs.R == Rhs.R
			&& Lhs.G == Rhs.G
			&& Lhs.B == Rhs.B
			&& Lhs.A == Rhs.A;
	}
	friend bool operator!=(const FVoxelFoliage& Lhs, const FVoxelFoliage& Rhs)
	{
		return !(Lhs == Rhs);
	}
	inline bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

private:
	uint8 R;
	uint8 G;
	uint8 B;
	uint8 A;
};

template <>
struct TTypeTraits<FVoxelFoliage> : TTypeTraitsBase<FVoxelFoliage>
{
	enum
	{
		IsBytewiseComparable = true,
	};
};

template<>
struct TStructOpsTypeTraits<FVoxelFoliage> : TStructOpsTypeTraitsBase2<FVoxelFoliage>
{
	enum 
	{
		WithZeroConstructor      = true,
		WithNoInitConstructor    = true,
		WithNoDestructor         = true,
		WithSerializer           = true,
		WithIdenticalViaEquality = true
	};
};