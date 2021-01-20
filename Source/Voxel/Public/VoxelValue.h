// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelContainers/VoxelBitArray.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"
#include "VoxelContainers/VoxelStaticBitArray.h"

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
		EightBitsValue   = 1 << 0,
		SixteenBitsValue = 1 << 1,
		OneBitValue      = 1 << 2
	};
}
constexpr uint32 GVoxelValueConfigFlag = 
	ONE_BIT_VOXEL_VALUE
	? EVoxelValueConfigFlag::OneBitValue
	: EIGHT_BITS_VOXEL_VALUE
	? EVoxelValueConfigFlag::EightBitsValue
	: EVoxelValueConfigFlag::SixteenBitsValue;

template<typename T>
struct TVoxelValueImpl
{
private:
	static constexpr T MAX_VOXELVALUE = TNumericLimits<T>::Max();
	static constexpr T MIN_VOXELVALUE = -TNumericLimits<T>::Max();
	static constexpr T INVALID_VOXELVALUE = TNumericLimits<T>::Min();

	static_assert(int64(INVALID_VOXELVALUE) == int64(MIN_VOXELVALUE) - 1, "");
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
	// Special voxel value, never reached using the normal constructor
	FORCEINLINE constexpr static TVoxelValueImpl Special()
	{
		return InternalConstructor(INVALID_VOXELVALUE);
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

public:
	FORCEINLINE constexpr bool IsNull() const { return GetStorage() == 0; }
	FORCEINLINE constexpr bool IsEmpty() const { return GetStorage() > 0; }
	FORCEINLINE constexpr bool IsTotallyEmpty() const { return GetStorage() == MAX_VOXELVALUE; }
	FORCEINLINE constexpr bool IsTotallyFull() const { return GetStorage() == -MAX_VOXELVALUE; }

	FORCEINLINE constexpr int32 Sign() const { return GetStorage() > 0 ? 1 : GetStorage() == 0 ? 0 : -1; }

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

public:
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
};

struct FVoxelBitValue
{
public:
	FORCEINLINE constexpr static FVoxelBitValue Full()
	{
		return InternalConstructor(false);
	}
	FORCEINLINE constexpr static FVoxelBitValue Empty()
	{
		return InternalConstructor(true);
	}

public:
	FVoxelBitValue()
	{
	}
	
	FORCEINLINE explicit constexpr FVoxelBitValue(EForceInit)
		: bIsEmpty(false)
	{
	}
	FORCEINLINE explicit constexpr FVoxelBitValue(float InValue)
		: bIsEmpty(InValue > 0)
	{
	}
	FORCEINLINE explicit constexpr FVoxelBitValue(double InValue)
		: bIsEmpty(InValue > 0)
	{
	}

public:
	// Implicit casts for convenience when using the bit array
	FORCEINLINE FVoxelBitValue(bool bInIsEmpty)
		: bIsEmpty(bInIsEmpty)
	{
	}
	FORCEINLINE FVoxelBitValue(FVoxelBitReference bInIsEmpty)
		: bIsEmpty(bInIsEmpty)
	{
	}
	FORCEINLINE FVoxelBitValue(FVoxelConstBitReference bInIsEmpty)
		: bIsEmpty(bInIsEmpty)
	{
	}
	FORCEINLINE operator bool() const
	{
		return bIsEmpty;
	}

	template<typename T>
	FVoxelBitValue(T) = delete;

public:
	FORCEINLINE constexpr bool IsNull() const { return false; }
	FORCEINLINE constexpr bool IsEmpty() const { return bIsEmpty; }
	FORCEINLINE constexpr bool IsTotallyEmpty() const { return bIsEmpty; }
	FORCEINLINE constexpr bool IsTotallyFull() const { return !bIsEmpty; }

	FORCEINLINE constexpr int32 Sign() const { return bIsEmpty ? 1 : -1; }

	FORCEINLINE constexpr float ToFloat() const { return Sign(); }
	FORCEINLINE constexpr FVoxelBitValue GetInverse() const { return InternalConstructor(!bIsEmpty); }

	FString ToString() const { return LexToString(bIsEmpty); }

public:
	FORCEINLINE constexpr bool operator==(const FVoxelBitValue& Rhs) const { return GetStorage() == Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator!=(const FVoxelBitValue& Rhs) const { return GetStorage() != Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator<(const FVoxelBitValue& Rhs) const { return GetStorage() < Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator>(const FVoxelBitValue& Rhs) const { return GetStorage() > Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator<=(const FVoxelBitValue& Rhs) const { return GetStorage() <= Rhs.GetStorage(); }
	FORCEINLINE constexpr bool operator>=(const FVoxelBitValue& Rhs) const { return GetStorage() >= Rhs.GetStorage(); }

	FORCEINLINE constexpr FVoxelBitValue& operator-=(const FVoxelBitValue& Rhs) { if (Rhs.bIsEmpty) bIsEmpty = false; return *this; }
	FORCEINLINE constexpr FVoxelBitValue& operator+=(const FVoxelBitValue& Rhs) { if (Rhs.bIsEmpty) bIsEmpty = true; return *this; }
	FORCEINLINE constexpr FVoxelBitValue operator-(const FVoxelBitValue& Rhs) const { auto Copy = *this; return Copy -= Rhs; }
	FORCEINLINE constexpr FVoxelBitValue operator+(const FVoxelBitValue& Rhs) const { auto Copy = *this; return Copy += Rhs; }
	
public:
	FORCEINLINE constexpr bool& GetStorage()
	{
		return bIsEmpty;
	}
	FORCEINLINE constexpr bool GetStorage() const
	{
		return bIsEmpty;
	}

public:
	FORCEINLINE static constexpr FVoxelBitValue InternalConstructor(bool bInIsEmpty)
	{
		FVoxelBitValue Value(ForceInit);
		Value.bIsEmpty = bInIsEmpty;
		return Value;
	}

private:
	bool bIsEmpty;
};

using FVoxelValue8 = TVoxelValueImpl<int8>;
using FVoxelValue16 = TVoxelValueImpl<int16>;

#if ONE_BIT_VOXEL_VALUE
using FVoxelValue = FVoxelBitValue;
#elif EIGHT_BITS_VOXEL_VALUE
using FVoxelValue = FVoxelValue8;
#else
using FVoxelValue = FVoxelValue16;
#endif

#if ONE_BIT_VOXEL_VALUE
using FVoxelValueArray = FVoxelBitArray32;
using FVoxelValueArray64 = FVoxelBitArray64;
template<uint32 Size>
using TVoxelValueStaticArray = TVoxelStaticBitArray<Size>;
#else
using FVoxelValueArray = TArray<FVoxelValue>;
using FVoxelValueArray64 = TArray64<FVoxelValue>;
template<uint32 Size>
using TVoxelValueStaticArray = TVoxelStaticArray<FVoxelValue, Size>;
#endif

static_assert(ONE_BIT_VOXEL_VALUE || FVoxelValue(0.f).IsNull(), "FVoxelValue error");
static_assert(!FVoxelValue(0.f).IsEmpty(), "FVoxelValue error"); // 0 is inside
static_assert(FVoxelValue::Empty().IsTotallyEmpty(), "FVoxelValue error");
static_assert(FVoxelValue::Full().IsTotallyFull(), "FVoxelValue error");
static_assert(FVoxelValue::Empty().ToFloat() == 1.f, "FVoxelValue error");
static_assert(FVoxelValue::Full().ToFloat() == -1.f, "FVoxelValue error");
static_assert(FVoxelValue(1.f) == FVoxelValue::Empty(), "FVoxelValue error");
static_assert(FVoxelValue(-1.f) == FVoxelValue::Full(), "FVoxelValue error");

template <>
struct TTypeTraits<FVoxelValue> : TTypeTraitsBase<FVoxelValue>
{
	enum { IsBytewiseComparable = true };
};

struct FVoxelValueConverter
{
	FORCEINLINE static constexpr FVoxelValue16 Convert8To16(FVoxelValue8 Value)
	{
		FVoxelValue16 Result(ForceInit);
		
		// Make sure special cases are handled correctly
		// eg FVoxelValue16::Empty() >> 8 == FVoxelValue16::Special() >> 8
		if (Value == FVoxelValue8::Full())
		{
			Result = FVoxelValue16::Full();
		}
		else if (Value == FVoxelValue8::Empty())
		{
			Result = FVoxelValue16::Empty();
		}
		else if (Value == FVoxelValue8::Special())
		{
			Result = FVoxelValue16::Special();
		}
		else
		{
			Result = FVoxelValue16::InternalConstructor(int16(Value.GetStorage()) << 8);
		}
		ensureVoxelSlowNoSideEffects(Result.IsEmpty() == Value.IsEmpty());
		return Result;
	}
	FORCEINLINE static constexpr FVoxelValue8 Convert16To8(FVoxelValue16 Value)
	{
		FVoxelValue8 Result(ForceInit);
		
		// Make sure special cases are handled correctly
		// eg FVoxelValue16::Empty() >> 8 == FVoxelValue16::Special() >> 8
		if (Value == FVoxelValue16::Full())
		{
			Result = FVoxelValue8::Full();
		}
		else if (Value == FVoxelValue16::Empty())
		{
			Result = FVoxelValue8::Empty();
		}
		else if (Value == FVoxelValue16::Special())
		{
			Result = FVoxelValue8::Special();
		}
		else
		{
			// DivideCeil: so that data between 0 and 255 is mapped to 1 and not 0 which would have a different sign
			Result = FVoxelValue8::InternalConstructor(FVoxelUtilities::ClampToINT8(FVoxelUtilities::DivideCeil(Value.GetStorage(), 256)));
		}
		ensureVoxelSlowNoSideEffects(Result.IsEmpty() == Value.IsEmpty());
		return Result;
	}

	FORCEINLINE static constexpr FVoxelValue ConvertValue(FVoxelBitValue Value)
	{
#if ONE_BIT_VOXEL_VALUE
		return Value;
#else
		return Value.IsEmpty() ? FVoxelValue::Empty() : FVoxelValue::Full();
#endif
	}
	FORCEINLINE static constexpr FVoxelValue ConvertValue(FVoxelValue8 Value)
	{
#if ONE_BIT_VOXEL_VALUE
		return FVoxelValue::InternalConstructor(Value.IsEmpty());
#elif EIGHT_BITS_VOXEL_VALUE
		return Value;
#else
		return Convert8To16(Value);
#endif
	}
	FORCEINLINE static constexpr FVoxelValue ConvertValue(FVoxelValue16 Value)
	{
#if ONE_BIT_VOXEL_VALUE
		return FVoxelValue::InternalConstructor(Value.IsEmpty());
#elif EIGHT_BITS_VOXEL_VALUE
		return Convert16To8(Value);
#else
		return Value;
#endif
	}

	template<typename Array>
	static FVoxelValueArray64 ConvertValuesImpl(const Array& Values)
	{
		FVoxelValueArray64 Result;
		Result.Reserve(Values.Num());
		for (auto& Value : Values)
		{
			Result.Add(ConvertValue(Value));
		}
		return Result;
	}

#if ONE_BIT_VOXEL_VALUE
	static FVoxelValueArray64 ConvertValues(FVoxelBitArray64&& Values)
	{
		return MoveTemp(Values);
	}
	template<typename T>
	static FVoxelValueArray64 ConvertValues(TArray64<T>&& Values)
	{
		return ConvertValuesImpl(Values);
	}
#else
	static FVoxelValueArray64 ConvertValues(FVoxelBitArray64&& Values)
	{
		return ConvertValuesImpl(TArray<FVoxelBitValue>(Values));
	}
	static FVoxelValueArray64 ConvertValues(TArray64<FVoxelValue8>&& Values)
	{
#if EIGHT_BITS_VOXEL_VALUE
		return MoveTemp(Values);
#else
		return ConvertValuesImpl(Values);
#endif
	}
	static FVoxelValueArray64 ConvertValues(TArray64<FVoxelValue16>&& Values)
	{
#if EIGHT_BITS_VOXEL_VALUE
		return ConvertValuesImpl(Values);
#else
		return MoveTemp(Values);
#endif
	}
#endif
};

static_assert(FVoxelValueConverter::ConvertValue(FVoxelValue8::Full()) == FVoxelValueConverter::ConvertValue(FVoxelValue16::Full()), "FVoxelValue error");
static_assert(FVoxelValueConverter::ConvertValue(FVoxelValue8::Empty()) == FVoxelValueConverter::ConvertValue(FVoxelValue16::Empty()), "FVoxelValue error");
static_assert(FVoxelValueConverter::ConvertValue(FVoxelValue8::Special()) == FVoxelValueConverter::ConvertValue(FVoxelValue16::Special()), "FVoxelValue error");
static_assert(FVoxelValueConverter::ConvertValue(FVoxelValue8::InternalConstructor(0)) == FVoxelValueConverter::ConvertValue(FVoxelValue16::InternalConstructor(0)), "FVoxelValue error");
static_assert(FVoxelValueConverter::ConvertValue(FVoxelValue16::InternalConstructor(+1)).IsEmpty(), "FVoxelValue error");
static_assert(!FVoxelValueConverter::ConvertValue(FVoxelValue16::InternalConstructor(-1)).IsEmpty(), "FVoxelValue error");