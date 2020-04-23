// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "Math/RandomStream.h"

namespace FVoxelUtilities
{
	FORCEINLINE constexpr int32 PositiveMod(int32 X, int32 Y)
	{
		return ((X % Y) + Y) % Y;
	}
	
	FORCEINLINE constexpr int32 DivideFloor(int32 Dividend, int32 Divisor)
	{
		int32 Q = Dividend / Divisor;
		int32 R = Dividend % Divisor;
		if ((R != 0) && ((R < 0) != (Divisor < 0)))
		{
			Q--;
		}
		return Q;
	}
	FORCEINLINE constexpr int32 DivideCeil(int32 Dividend, int32 Divisor)
	{
		return (Dividend > 0) ? 1 + (Dividend - 1) / Divisor : (Dividend / Divisor);
	}
	FORCEINLINE constexpr int32 DivideRound(int32 Dividend, int32 Divisor)
	{
		const int32 R = PositiveMod(Dividend, Divisor);
		if (R < Divisor / 2)
		{
			return DivideFloor(Dividend, Divisor);
		}
		else
		{
			return DivideCeil(Dividend, Divisor);
		}
	}

	FORCEINLINE constexpr int32 IntLog2(int32 X)
	{
		int32 Exp = -1;
		while (X)
		{
			X >>= 1;
			++Exp;
		}
		return Exp;
	}

	FORCEINLINE uint8 CastToUINT8(int32 Value)
	{
		ensureMsgfVoxelSlowNoSideEffects(0 <= Value && Value < 256, TEXT("Invalid uint8 value: %d"), Value);
		return Value;
	}

	template<typename T>
	FORCEINLINE constexpr T Clamp( const T X, const T Min, const T Max )
	{
		return X < Min ? Min : X < Max ? X : Max;
	}

	FORCEINLINE constexpr int8 ClampToINT8(int32 Value)
	{
		return Clamp<int32>(Value, MIN_int8, MAX_int8);
	}
	FORCEINLINE constexpr uint8 ClampToUINT8(int32 Value)
	{
		return Clamp<int32>(Value, MIN_uint8, MAX_uint8);
	}
	FORCEINLINE constexpr int8 ClampToINT16(int32 Value)
	{
		return Clamp<int32>(Value, MIN_int16, MAX_int16);
	}
	FORCEINLINE constexpr uint16 ClampToUINT16(int32 Value)
	{
		return Clamp<int32>(Value, MIN_uint16, MAX_uint16);
	}

	FORCEINLINE uint8 FloatToUINT8(float Float)
	{
		return ClampToUINT8(FMath::FloorToInt(Float * 255.999f));
	}
	FORCEINLINE constexpr float UINT8ToFloat(uint8 Int)
	{
		return Int / 255.f;
	}

	FORCEINLINE FColor FloatToUINT8(FVector4 Float)
	{
		return
		FColor
		{
			FloatToUINT8(Float.X),
			FloatToUINT8(Float.Y),
			FloatToUINT8(Float.Z),
			FloatToUINT8(Float.W)
		};
	}
	FORCEINLINE FVector4 UINT8ToFloat(FColor Int)
	{
		return
		FVector4
		{
			UINT8ToFloat(Int.R),
			UINT8ToFloat(Int.G),
			UINT8ToFloat(Int.B),
			UINT8ToFloat(Int.A)
		};
	}
	
	FORCEINLINE uint16 FloatToUINT16(float Float)
	{
		return ClampToUINT16(FMath::FloorToInt(Float * 65535.999f));
	}
	FORCEINLINE constexpr float UINT16ToFloat(uint16 Int)
	{
		return Int / 65535.f;
	}
	
	FORCEINLINE constexpr uint32 MurmurHash32(uint32 Hash)
	{
		Hash ^= Hash >> 16;
		Hash *= 0x85ebca6b;
		Hash ^= Hash >> 13;
		Hash *= 0xc2b2ae35;
		Hash ^= Hash >> 16;
		return Hash;
	}
	FORCEINLINE uint32 MurmurHash32(int32 Hash)
	{
		return MurmurHash32(*reinterpret_cast<uint32*>(&Hash));
	}
	// Slow!
	template<typename T, typename... TArgs>
	FORCEINLINE uint32 MurmurHash32(T A, TArgs... Args)
	{
		return MurmurHash32(MurmurHash32(A) ^ MurmurHash32(Args...));
	}

	FORCEINLINE constexpr uint64 MurmurHash64(uint64 Hash)
	{
		Hash ^= Hash >> 33;
		Hash *= 0xff51afd7ed558ccd;
		Hash ^= Hash >> 33;
		Hash *= 0xc4ceb9fe1a85ec53;
		Hash ^= Hash >> 33;
		return Hash;
	}
	FORCEINLINE uint64 MurmurHash64(int64 Hash)
	{
		return MurmurHash64(*reinterpret_cast<uint64*>(&Hash));
	}
	// Slow!
	template<typename T, typename... TArgs>
	FORCEINLINE uint32 MurmurHash64(T A, TArgs... Args)
	{
		return MurmurHash32(MurmurHash64(A) ^ MurmurHash64(Args...));
	}

	FORCEINLINE uint32 MurmurHash32xN(uint32 const* RESTRICT Hash, int32 Size, uint32 Seed = 0)
	{
		uint32 H = Seed;
		for (int32 Index = 0; Index < Size; ++Index)
		{
			uint32 K = Hash[Index];
			K *= 0xcc9e2d51;
			K = (K << 15) | (K >> 17);
			K *= 0x1b873593;
			H ^= K;
			H = (H << 13) | (H >> 19);
			H = H * 5 + 0xe6546b64;
		}

		H ^= uint32(Size);
		H ^= H >> 16;
		H *= 0x85ebca6b;
		H ^= H >> 13;
		H *= 0xc2b2ae35;
		H ^= H >> 16;
		return H;
	}

	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 *  -----> X
	 */
	template<typename T, typename U = float>
	FORCEINLINE T BilinearInterpolation(T A, T B, T C, T D, U X, U Y)
	{
		T AB = FMath::Lerp<T, U>(A, B, X);
		T CD = FMath::Lerp<T, U>(C, D, X);
		return FMath::Lerp<T, U>(AB, CD, Y);
	}
	
	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 * 0-----> X
	 * Y
	 * ^ G - H
	 * | |   |
	 * | E - F
	 * 1-----> X
	 */
	template<typename T, typename U = float>
	FORCEINLINE T TrilinearInterpolation(
		T A, T B, T C, T D,
		T E, T F, T G, T H,
		U X, U Y, U Z)
	{
		const T ABCD = BilinearInterpolation<T, U>(A, B, C, D, X, Y);
		const T EFGH = BilinearInterpolation<T, U>(E, F, G, H, X, Y);
		return FMath::Lerp<T, U>(ABCD, EFGH, Z);
	}

	template<typename T>
	FORCEINLINE T&& VariadicMin(T&& Val)
	{
		return Forward<T>(Val);
	}
	template<typename T0, typename T1, typename... Ts>
	FORCEINLINE auto VariadicMin(T0&& Val1, T1&& Val2, Ts&&... Vals)
	{
		return (Val1 < Val2) ?
			VariadicMin(Val1, Forward<Ts>(Vals)...) :
			VariadicMin(Val2, Forward<Ts>(Vals)...);
	}

	template<typename T>
	FORCEINLINE T&& VariadicMax(T&& Val)
	{
		return Forward<T>(Val);
	}
	template<typename T0, typename T1, typename... Ts>
	FORCEINLINE auto VariadicMax(T0&& Val1, T1&& Val2, Ts&&... Vals)
	{
		return (Val1 > Val2) ?
			VariadicMax(Val1, Forward<Ts>(Vals)...) :
			VariadicMax(Val2, Forward<Ts>(Vals)...);
	}

	FORCEINLINE float LinearFalloff(float Distance, float Radius, float Falloff)
	{
		return Distance <= Radius
			? 1.0f
			: Radius + Falloff <= Distance
			? 0.f
			: 1.0f - (Distance - Radius) / Falloff;
	}
	FORCEINLINE float SmoothFalloff(float Distance, float Radius, float Falloff)
	{
		const float X = LinearFalloff(Distance, Radius, Falloff);
		return FMath::SmoothStep(0, 1, X);
	}
	FORCEINLINE float SphericalFalloff(float Distance, float Radius, float Falloff)
	{
		return Distance <= Radius
			? 1.0f
			: Radius + Falloff <= Distance
			? 0.f
			: FMath::Sqrt(1.0f - FMath::Square((Distance - Radius) / Falloff));
	}
	FORCEINLINE float TipFalloff(float Distance, float Radius, float Falloff)
	{
		return Distance <= Radius
			? 1.0f
			: Radius + Falloff <= Distance
			? 0.f
			: 1.0f - FMath::Sqrt(1.0f - FMath::Square((Falloff + Radius - Distance) / Falloff));
	}

	FORCEINLINE uint32 Popc(uint32 Value)
	{
		// Java: use >>> instead of >>
		// C or C++: use uint32_t
		Value = Value - ((Value >> 1) & 0x55555555);
		Value = (Value & 0x33333333) + ((Value >> 2) & 0x33333333);
		return (((Value + (Value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	// Returns distance to voxel with Density
	FORCEINLINE float GetAbsDistanceFromDensities(float Density, float OtherDensity)
	{
		ensureVoxelSlowNoSideEffects(Density > 0 != OtherDensity > 0);
		return Density / (Density - OtherDensity);
	}
}