// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"

namespace ConstExprUtils
{
	template<typename  T>
	static FORCEINLINE constexpr T Clamp(const T X, const T Min, const T Max)
	{
		return X < Min ? Min : X < Max ? X : Max;
	}
	static FORCEINLINE constexpr int32 Floor(float F)
	{
		if (int32(F) == F)
		{
			return int32(F);
		}
		if (F < 0)
		{
			return int32(F) - 1;
		}
		else
		{
			return int32(F);
		}
	}
	static FORCEINLINE constexpr int32 RoundToInt(float F)
	{
		return Floor(F + 0.5f);
	}
}

namespace EVoxelValueConfigFlag
{
	enum Type : uint32
	{
		EightBitsValue   = 0x01,
		SixteenBitsValue = 0x02
	};
}
constexpr uint32 GVoxelValueConfigFlag = EIGHT_BITS_VOXEL_VALUE ? EVoxelValueConfigFlag::EightBitsValue : EVoxelValueConfigFlag::SixteenBitsValue;

template<typename T, T MAX_VOXELVALUE>
struct TVoxelValueImpl
{
public:
	// Full voxel
	FORCEINLINE constexpr static TVoxelValueImpl Full()
	{
		return InternalConstructor(-MAX_VOXELVALUE);
	}
	// Completely empty voxel
	FORCEINLINE constexpr static TVoxelValueImpl Empty()
	{
		return InternalConstructor(MAX_VOXELVALUE);
	}
	// Precision of FVoxelValue used for comparisons
	FORCEINLINE constexpr static TVoxelValueImpl Precision()
	{
		return InternalConstructor(1);
	}

public:
	TVoxelValueImpl()
	{
	}
	
	FORCEINLINE explicit constexpr TVoxelValueImpl(EForceInit)
		: F(0)
	{
	}
	FORCEINLINE explicit constexpr TVoxelValueImpl(float InValue)
		: F(ClampToStorage(ConstExprUtils::RoundToInt(ConstExprUtils::Clamp(InValue, -10.f, 10.f) * MAX_VOXELVALUE)))
	{
		// Float clamp: to avoid integer overflow. Can use 10.f as ClampToStorage is done on int32
	}
	FORCEINLINE explicit constexpr TVoxelValueImpl(double InValue)
		: TVoxelValueImpl(float(InValue))
	{
	}

	template<typename X>
	TVoxelValueImpl(X) = delete;

public:
	FORCEINLINE constexpr bool IsNull() const { return GetStorage() == 0; }
	FORCEINLINE constexpr bool IsEmpty() const { return GetStorage() > 0; }
	FORCEINLINE constexpr bool IsTotallyEmpty() const { return GetStorage() == MAX_VOXELVALUE; }
	FORCEINLINE constexpr bool IsTotallyFull() const { return GetStorage() == -MAX_VOXELVALUE; }

	FORCEINLINE constexpr float ToFloat() const { return float(GetStorage()) / float(MAX_VOXELVALUE); }
	FORCEINLINE constexpr TVoxelValueImpl GetInverse() const { return InternalConstructor(-GetStorage()); }

	FString ToString() const { return FString::SanitizeFloat(ToFloat()); }

public:	
	FORCEINLINE constexpr bool operator==(const TVoxelValueImpl& Rhs) const { return GetStorage() == Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator!=(const TVoxelValueImpl& Rhs) const { return GetStorage() != Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator<(const TVoxelValueImpl& Rhs) const { return GetStorage() < Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator>(const TVoxelValueImpl& Rhs) const { return GetStorage() > Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator<=(const TVoxelValueImpl& Rhs) const { return GetStorage() <= Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator>=(const TVoxelValueImpl& Rhs) const { return GetStorage() >= Rhs.GetStorage(); }

	FORCEINLINE constexpr TVoxelValueImpl& operator-=(const TVoxelValueImpl& Rhs) { GetStorage() = ClampToStorage(int32(GetStorage()) - int32(Rhs.GetStorage())); return *this; }
	FORCEINLINE constexpr TVoxelValueImpl& operator+=(const TVoxelValueImpl& Rhs) { GetStorage() = ClampToStorage(int32(GetStorage()) + int32(Rhs.GetStorage())); return *this; }
	FORCEINLINE constexpr TVoxelValueImpl operator-(const TVoxelValueImpl& Rhs) const { auto Copy = *this; return Copy -= Rhs; }
	FORCEINLINE constexpr TVoxelValueImpl operator+(const TVoxelValueImpl& Rhs) const { auto Copy = *this; return Copy += Rhs; }
	
public:
	FORCEINLINE constexpr T& GetStorage()
	{
		return F;
	}
	FORCEINLINE constexpr T GetStorage() const
	{
		return F;
	}

private:
	FORCEINLINE static constexpr TVoxelValueImpl InternalConstructor(T F)
	{
		TVoxelValueImpl Value(ForceInit);
		Value.F = F;
		return Value;
	}
	FORCEINLINE static constexpr int16 ClampToStorage(int32 F)
	{
		return ConstExprUtils::Clamp<int32>(F, -MAX_VOXELVALUE, MAX_VOXELVALUE);
	}

private:
	T F;

	friend struct FVoxelValueConverter;
};

using FVoxelValue8 = TVoxelValueImpl<int8, 127>;
using FVoxelValue16 = TVoxelValueImpl<int16, 0x7fff>;

#if EIGHT_BITS_VOXEL_VALUE
using FVoxelValue = FVoxelValue8;
#else
using FVoxelValue = FVoxelValue16;
#endif

static_assert(FVoxelValue(0.f).IsNull(), "FVoxelValue error");
static_assert(!FVoxelValue(0.f).IsEmpty(), "FVoxelValue error"); // 0 is inside
static_assert(FVoxelValue::Empty().IsTotallyEmpty(), "FVoxelValue error");
static_assert(FVoxelValue::Full().IsTotallyFull(), "FVoxelValue error");
static_assert(FVoxelValue::Empty().ToFloat() == 1.f, "FVoxelValue error");
static_assert(FVoxelValue::Full().ToFloat() == -1.f, "FVoxelValue error");
static_assert(FVoxelValue(1.f) == FVoxelValue::Empty(), "FVoxelValue error");
static_assert(FVoxelValue(-1.f) == FVoxelValue::Full(), "FVoxelValue error");
static_assert(FVoxelValue(FVoxelValue::Precision().ToFloat()) == FVoxelValue::Precision(), "FVoxelValue error");

template <>
struct TTypeTraits<FVoxelValue> : TTypeTraitsBase<FVoxelValue>
{
	enum { IsBytewiseComparable = true };
};

struct FVoxelValueConverter
{
	template<typename T, typename TEnableIf<!TIsSame<FVoxelValue, T>::Value && TIsSame<FVoxelValue8, T>::Value, int>::Type = 0>
	FORCEINLINE static FVoxelValue ConvertValue(T Value)
	{
		// Need to make FVoxelValue16 a dependent type
		return TEnableIf<sizeof(T) != 0, FVoxelValue16>::Type::InternalConstructor(int16(Value.GetStorage()) << 8);
	}
	template<typename T, typename TEnableIf<!TIsSame<FVoxelValue, T>::Value && TIsSame<FVoxelValue16, T>::Value, int>::Type = 0>
	FORCEINLINE static FVoxelValue ConvertValue(T Value)
	{
		// Need to make FVoxelValue8 a dependent type
		return TEnableIf<sizeof(T) != 0, FVoxelValue8>::Type::InternalConstructor(int8(Value.GetStorage() >> 8));
	}
	template<typename T, typename TEnableIf<TIsSame<FVoxelValue, T>::Value, int>::Type = 0>
	FORCEINLINE static FVoxelValue ConvertValue(T Value)
	{
		return Value;
	}
	
	template<typename T, typename TEnableIf<TIsSame<FVoxelValue, T>::Value, int>::Type = 0>
	FORCEINLINE static TArray<FVoxelValue> ConvertValues(TArray<T>&& Values)
	{
		return MoveTemp(Values);
	}
	template<typename T, typename TEnableIf<!TIsSame<FVoxelValue, T>::Value, int>::Type = 0>
	static TArray<FVoxelValue> ConvertValues(TArray<T>&& Values)
	{
		static_assert(!TIsSame<FVoxelValue, T>::Value, "");
		TArray<FVoxelValue> Result;
		Result.Reserve(Values.Num());
		for (auto& Value : Values)
		{
			Result.Add(ConvertValue(Value));
		}
		return Result;
	}
};