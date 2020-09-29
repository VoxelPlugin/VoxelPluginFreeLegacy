// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMaterial.generated.h"

namespace EVoxelMaterialConfigFlag
{
	enum Type : uint32
	{
		EnableA     = 0x01,
		EnableR     = 0x02,
		EnableG     = 0x04,
		//          = 0x08, Forgot to use that one :(
		EnableB     = 0x10,
		EnableUV0   = 0x20,
		EnableUV1   = 0x40,
		EnableUV2   = 0x80,
		EnableUV3   = 0x100
	};
}
constexpr uint32 GVoxelMaterialConfigFlag =
	EVoxelMaterialConfigFlag::EnableA   * VOXEL_MATERIAL_ENABLE_A   +
	EVoxelMaterialConfigFlag::EnableR   * VOXEL_MATERIAL_ENABLE_R   +
	EVoxelMaterialConfigFlag::EnableG   * VOXEL_MATERIAL_ENABLE_G   +
	EVoxelMaterialConfigFlag::EnableB   * VOXEL_MATERIAL_ENABLE_B   +
	EVoxelMaterialConfigFlag::EnableUV0 * VOXEL_MATERIAL_ENABLE_UV0 +
	EVoxelMaterialConfigFlag::EnableUV1 * VOXEL_MATERIAL_ENABLE_UV1 +
	EVoxelMaterialConfigFlag::EnableUV2 * VOXEL_MATERIAL_ENABLE_UV2 +
	EVoxelMaterialConfigFlag::EnableUV3 * VOXEL_MATERIAL_ENABLE_UV3;

UENUM(BlueprintType, DisplayName = "Voxel Material Mask", meta = (Bitflags))
enum class EVoxelMaterialMask_BP : uint8
{
	R,
	G,
	B,
	A,
	U0,
	V0,
	U1,
	V1,
	U2,
	V2,
	U3,
	V3,
};

namespace EVoxelMaterialMask
{
	enum Type : uint32
	{
		R  = 1 << 0,
		G  = 1 << 1,
		B  = 1 << 2,
		A  = 1 << 3,
		U0 = 1 << 4,
		V0 = 1 << 5,
		U1 = 1 << 6,
		V1 = 1 << 7,
		U2 = 1 << 8,
		V2 = 1 << 9,
		U3 = 1 << 10,
		V3 = 1 << 11,

		None = 0,
		All = R | G | B| A | U0 | U1 | U2 | U3 | V0 | V1 | V2 | V3,

		RGB  = R | G | B,
		RGBA = R | G | B | A,
		
		SingleIndex = A,
		
		MultiIndex_Blend0    = R,
		MultiIndex_Blend1    = G,
		MultiIndex_Blend2    = B,
		MultiIndex_Wetness   = A,
		MultiIndex_Index0    = U0,
		MultiIndex_Index1    = V0,
		MultiIndex_Index2    = U1,
		MultiIndex_Index3    = V1,
		MultiIndex_NoWetness = MultiIndex_Blend0 |
		                       MultiIndex_Blend1 |
		                       MultiIndex_Blend2 |
							   MultiIndex_Index0 |
							   MultiIndex_Index1 |
							   MultiIndex_Index2 |
							   MultiIndex_Index3,
		MultiIndex = MultiIndex_NoWetness | MultiIndex_Wetness,

		UV0 = U0 | V0,
		UV1 = U1 | V1,
		UV2 = U2 | V2,
		UV3 = U3 | V3,
		
		UV = UV0 | UV1 | UV2 | UV3
	};
}

// Base class without all the #if ENABLE_SOMETHING
template<typename T>
struct TVoxelMaterialImpl
{
	FORCEINLINE TVoxelMaterialImpl()
	{
	}
	
	FORCEINLINE explicit TVoxelMaterialImpl(EForceInit)
	{
#if VOXEL_MATERIAL_DEFAULT_IS_WHITE
		SetR(255);
		SetG(255);
		SetB(255);
		SetA(255);
#else
		SetR(0);
		SetG(0);
		SetB(0);
		SetA(0);
#endif
		
		SetU0(0);
		SetU1(0);
		SetU2(0);
		SetU3(0);
		
		SetV0(0);
		SetV1(0);
		SetV2(0);
		SetV3(0);
	}

	FORCEINLINE static T Default()
	{
		return T(ForceInit);
	}
	
public:
#define DEFINE_GETTER_SETTER(Name) \
	FORCEINLINE void Set##Name(uint8 New##Name) { static_cast<T&>(*this).Impl_Set##Name(New##Name); } \
	FORCEINLINE void Set##Name(int32 New##Name) { static_cast<T&>(*this).Impl_Set##Name(FVoxelUtilities::CastToUINT8(New##Name)); } \
	FORCEINLINE void Set##Name##_AsFloat(float New##Name) { static_cast<T&>(*this).Impl_Set##Name(FVoxelUtilities::FloatToUINT8(New##Name)); } \
	template<typename X> \
	FORCEINLINE void Set##Name(X New##Name) \
	{ \
		static_assert(!TIsSame<X, float>::Value, PREPROCESSOR_TO_STRING(Set##Name) ": need to use " PREPROCESSOR_TO_STRING(Set##Name##_AsFloat)); \
		static_assert(TIsSame<X, float>::Value, PREPROCESSOR_TO_STRING(Set##Name) ": need to cast to float, uint8 or int32"); \
	} \
	FORCEINLINE uint8 Get##Name() const { return static_cast<const T&>(*this).Impl_Get##Name(); } \
	FORCEINLINE float Get##Name##_AsFloat() const { return FVoxelUtilities::UINT8ToFloat(static_cast<const T&>(*this).Impl_Get##Name()); }
	
	DEFINE_GETTER_SETTER(R)
	DEFINE_GETTER_SETTER(G)
	DEFINE_GETTER_SETTER(B)
	DEFINE_GETTER_SETTER(A)
	DEFINE_GETTER_SETTER(U0)
	DEFINE_GETTER_SETTER(U1)
	DEFINE_GETTER_SETTER(U2)
	DEFINE_GETTER_SETTER(U3)
	DEFINE_GETTER_SETTER(V0)
	DEFINE_GETTER_SETTER(V1)
	DEFINE_GETTER_SETTER(V2)
	DEFINE_GETTER_SETTER(V3)
#undef DEFINE_GETTER_SETTER

public:
#define DEFINE_UV_GETTER_SETTER(Name, Type, Suffix) \
	FORCEINLINE Type Get##Name##Suffix(int32 Tex) const \
	{ \
		switch (Tex) \
		{ \
		case 0: return Get##Name##0##Suffix(); \
		case 1: return Get##Name##1##Suffix(); \
		case 2: return Get##Name##2##Suffix(); \
		case 3: return Get##Name##3##Suffix(); \
		default: return 0; \
		} \
	} \
	template<typename X> \
	FORCEINLINE void Set##Name##Suffix(int32 Tex, X Value) \
	{ \
		switch (Tex) \
		{ \
		case 0: Set##Name##0##Suffix(Value); break; \
		case 1: Set##Name##1##Suffix(Value); break; \
		case 2: Set##Name##2##Suffix(Value); break; \
		case 3: Set##Name##3##Suffix(Value); break; \
		default: break; \
		} \
	}

	DEFINE_UV_GETTER_SETTER(U, uint8,)
	DEFINE_UV_GETTER_SETTER(V, uint8,)
	DEFINE_UV_GETTER_SETTER(U, float, _AsFloat)
	DEFINE_UV_GETTER_SETTER(V, float, _AsFloat)

#undef DEFINE_UV_GETTER_SETTER

public:
#define DEFINE_FORWARD(Name, Forward) \
	FORCEINLINE void Set##Name(uint8 New##Name) { Set##Forward(New##Name); } \
	FORCEINLINE void Set##Name(int32 New##Name) { Set##Forward(New##Name); } \
	FORCEINLINE void Set##Name##_AsFloat(float New##Name) { Set##Forward##_AsFloat(New##Name); } \
	template<typename X> \
	FORCEINLINE void Set##Name(X New##Name) \
	{ \
		static_assert(!TIsSame<X, float>::Value, PREPROCESSOR_TO_STRING(Set##Name) ": need to use " PREPROCESSOR_TO_STRING(Set##Name##_AsFloat)); \
		static_assert(TIsSame<X, float>::Value, PREPROCESSOR_TO_STRING(Set##Name) ": need to cast to float, uint8 or int32"); \
	} \
	FORCEINLINE uint8 Get##Name() const { return Get##Forward(); } \
	FORCEINLINE float Get##Name##_AsFloat() const { return Get##Forward##_AsFloat(); }

	DEFINE_FORWARD(SingleIndex, A)

	DEFINE_FORWARD(MultiIndex_Blend0, R)
	DEFINE_FORWARD(MultiIndex_Blend1, G)
	DEFINE_FORWARD(MultiIndex_Blend2, B)
	DEFINE_FORWARD(MultiIndex_Wetness, A)
	DEFINE_FORWARD(MultiIndex_Index0, U0)
	DEFINE_FORWARD(MultiIndex_Index1, V0)
	DEFINE_FORWARD(MultiIndex_Index2, U1)
	DEFINE_FORWARD(MultiIndex_Index3, V1)
#undef DEFINE_FORWARD

public:
	FORCEINLINE uint32 GetPackedColor() const
	{
		return
			(uint32(GetR()) <<  0) |
			(uint32(GetG()) <<  8) |
			(uint32(GetB()) << 16) |
			(uint32(GetA()) << 24);
	}

public:
	FORCEINLINE void SetColor(const FColor& Color)
	{
		SetR(Color.R);
		SetG(Color.G);
		SetB(Color.B);
		SetA(Color.A);
	}
	FORCEINLINE FColor GetColor() const
	{
		return FColor(
			GetR(),
			GetG(),
			GetB(),
			GetA());
	}
	
	FORCEINLINE FVector4 GetColorVector() const
	{
		return FVector4(
			GetR_AsFloat(),
			GetG_AsFloat(),
			GetB_AsFloat(),
			GetA_AsFloat());
	}
	
	FORCEINLINE void SetColor(const FLinearColor& Color)
	{
		SetR_AsFloat(Color.R);
		SetG_AsFloat(Color.G);
		SetB_AsFloat(Color.B);
		SetA_AsFloat(Color.A);
	}
	FORCEINLINE FLinearColor GetLinearColor() const
	{
		return FLinearColor(
			GetR_AsFloat(),
			GetG_AsFloat(),
			GetB_AsFloat(),
			GetA_AsFloat());
	}

public:
	FORCEINLINE void SetUV_AsFloat(int32 Tex, const FVector2D& UV)
	{
		SetU_AsFloat(Tex, UV.X);
		SetV_AsFloat(Tex, UV.Y);
	}
	FORCEINLINE FVector2D GetUV_AsFloat(int32 Tex) const
	{
		return FVector2D(GetU_AsFloat(Tex), GetV_AsFloat(Tex));
	}

public:
	FORCEINLINE static T CreateFromColor(const FLinearColor& Color)
	{
		T Material(ForceInit);
		Material.SetColor(Color);
		return Material;
	}
	FORCEINLINE static T CreateFromColor(const FColor& Color)
	{
		T Material(ForceInit);
		Material.SetColor(Color);
		return Material;
	}

public:
	FORCEINLINE void CopyFrom(const T& Other, uint32 Mask)
	{
		if (Mask & EVoxelMaterialMask::R) SetR(Other.GetR());
		if (Mask & EVoxelMaterialMask::G) SetG(Other.GetG());
		if (Mask & EVoxelMaterialMask::B) SetB(Other.GetB());
		if (Mask & EVoxelMaterialMask::A) SetA(Other.GetA());

		if (Mask & EVoxelMaterialMask::U0) SetU0(Other.GetU0());
		if (Mask & EVoxelMaterialMask::U1) SetU1(Other.GetU1());
		if (Mask & EVoxelMaterialMask::U2) SetU2(Other.GetU2());
		if (Mask & EVoxelMaterialMask::U3) SetU3(Other.GetU3());

		if (Mask & EVoxelMaterialMask::V0) SetV0(Other.GetV0());
		if (Mask & EVoxelMaterialMask::V1) SetV1(Other.GetV1());
		if (Mask & EVoxelMaterialMask::V2) SetV2(Other.GetV2());
		if (Mask & EVoxelMaterialMask::V3) SetV3(Other.GetV3());
	}

public:
	FORCEINLINE bool operator==(const T& Other) const
	{
		return
			GetR() == Other.GetR() &&
			GetG() == Other.GetG() &&
			GetB() == Other.GetB() &&
			GetA() == Other.GetA() &&
			GetU0() == Other.GetU0() &&
			GetU1() == Other.GetU1() &&
			GetU2() == Other.GetU2() &&
			GetV3() == Other.GetU3() &&
			GetV0() == Other.GetV0() &&
			GetV1() == Other.GetV1() &&
			GetV2() == Other.GetV2() &&
			GetV3() == Other.GetV3();
	}
	FORCEINLINE bool operator!=(const T& Other) const
	{
		return
			GetR() != Other.GetR() ||
			GetG() != Other.GetG() ||
			GetB() != Other.GetB() ||
			GetA() != Other.GetA() ||
			GetU0() != Other.GetU0() ||
			GetU1() != Other.GetU1() ||
			GetU2() != Other.GetU2() ||
			GetV3() != Other.GetU3() ||
			GetV0() != Other.GetV0() ||
			GetV1() != Other.GetV1() ||
			GetV2() != Other.GetV2() ||
			GetV3() != Other.GetV3();
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct TVoxelMaterialStorage
{
	FORCEINLINE static TVoxelMaterialStorage<T> SerializeWithCustomConfig(FArchive& Ar, uint32 ConfigFlags)
	{
		check(Ar.IsLoading());

		T R = 0;
		T G = 0;
		T B = 0;
		T A = 0;
		
		T U0 = 0;
		T V0 = 0;
		T U1 = 0;
		T V1 = 0;
		T U2 = 0;
		T V2 = 0;
		T U3 = 0;
		T V3 = 0;

		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableR)
		{
			Ar << R;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableG)
		{
			Ar << G;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableB)
		{
			Ar << B;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableA)
		{
			Ar << A;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableUV0)
		{
			Ar << U0;
			Ar << V0;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableUV1)
		{
			Ar << U1;
			Ar << V1;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableUV2)
		{
			Ar << U2;
			Ar << V2;
		}
		if (ConfigFlags & EVoxelMaterialConfigFlag::EnableUV3)
		{
			Ar << U3;
			Ar << V3;
		}
		
		TVoxelMaterialStorage<T> Storage;
		Storage.Impl_SetR(R);
		Storage.Impl_SetG(G);
		Storage.Impl_SetB(B);
		Storage.Impl_SetA(A);
		Storage.Impl_SetU0(U0);
		Storage.Impl_SetV0(V0);
		Storage.Impl_SetU1(U1);
		Storage.Impl_SetV1(V1);
		Storage.Impl_SetU2(U2);
		Storage.Impl_SetV2(V2);
		Storage.Impl_SetU3(U3);
		Storage.Impl_SetV3(V3);
		return Storage;
	}
	friend FORCEINLINE FArchive& operator<<(FArchive& Ar, TVoxelMaterialStorage<T>& Storage)
	{
#if VOXEL_MATERIAL_ENABLE_R
		Ar << Storage.R;
#endif
#if VOXEL_MATERIAL_ENABLE_G
		Ar << Storage.G;
#endif
#if VOXEL_MATERIAL_ENABLE_B
		Ar << Storage.B;
#endif
#if VOXEL_MATERIAL_ENABLE_A
		Ar << Storage.A;
#endif
#if VOXEL_MATERIAL_ENABLE_UV0
		Ar << Storage.U0;
		Ar << Storage.V0;
#endif
#if VOXEL_MATERIAL_ENABLE_UV1
		Ar << Storage.U1;
		Ar << Storage.V1;
#endif
#if VOXEL_MATERIAL_ENABLE_UV2
		Ar << Storage.U2;
		Ar << Storage.V2;
#endif
#if VOXEL_MATERIAL_ENABLE_UV3
		Ar << Storage.U3;
		Ar << Storage.V3;
#endif
		return Ar;
	}

public:
	static constexpr int32 NumChannels = 
		VOXEL_MATERIAL_ENABLE_R + 
		VOXEL_MATERIAL_ENABLE_G + 
		VOXEL_MATERIAL_ENABLE_B + 
		VOXEL_MATERIAL_ENABLE_A + 
		2 * VOXEL_MATERIAL_ENABLE_UV0 +
		2 * VOXEL_MATERIAL_ENABLE_UV1 +
		2 * VOXEL_MATERIAL_ENABLE_UV2 +
		2 * VOXEL_MATERIAL_ENABLE_UV3;
	
	static constexpr uint32 ChannelsMask = 
		EVoxelMaterialMask::R * VOXEL_MATERIAL_ENABLE_R + 
		EVoxelMaterialMask::G * VOXEL_MATERIAL_ENABLE_G + 
		EVoxelMaterialMask::B * VOXEL_MATERIAL_ENABLE_B + 
		EVoxelMaterialMask::A * VOXEL_MATERIAL_ENABLE_A + 
		EVoxelMaterialMask::UV0 * VOXEL_MATERIAL_ENABLE_UV0 +
		EVoxelMaterialMask::UV1 * VOXEL_MATERIAL_ENABLE_UV1 +
		EVoxelMaterialMask::UV2 * VOXEL_MATERIAL_ENABLE_UV2 +
		EVoxelMaterialMask::UV3 * VOXEL_MATERIAL_ENABLE_UV3;

	FORCEINLINE T& GetRaw(int32 Channel)
	{
		checkVoxelSlow(0 <= Channel && Channel < NumChannels);
		return *(reinterpret_cast<T*>(this) + Channel);
	}
	FORCEINLINE T GetRaw(int32 Channel) const
	{
		checkVoxelSlow(0 <= Channel && Channel < NumChannels);
		return *(reinterpret_cast<const T*>(this) + Channel);
	}

	FORCEINLINE T& operator[](int32 Channel)
	{
		return GetRaw(Channel);
	}
	FORCEINLINE T operator[](int32 Channel) const
	{
		return GetRaw(Channel);
	}

public:
	FORCEINLINE T Impl_GetR() const
	{
#if VOXEL_MATERIAL_ENABLE_R
		return R;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetG() const
	{
#if VOXEL_MATERIAL_ENABLE_G
		return G;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetB() const
	{
#if VOXEL_MATERIAL_ENABLE_B
		return B;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetA() const
	{
#if VOXEL_MATERIAL_ENABLE_A
		return A;
#else
		return 0;
#endif
	}
	
	FORCEINLINE T Impl_GetU0() const
	{
#if VOXEL_MATERIAL_ENABLE_UV0
		return U0;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetU1() const
	{
#if VOXEL_MATERIAL_ENABLE_UV1
		return U1;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetU2() const
	{
#if VOXEL_MATERIAL_ENABLE_UV2
		return U2;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetU3() const
	{
#if VOXEL_MATERIAL_ENABLE_UV3
		return U3;
#else
		return 0;
#endif
	}
	
	FORCEINLINE T Impl_GetV0() const
	{
#if VOXEL_MATERIAL_ENABLE_UV0
		return V0;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetV1() const
	{
#if VOXEL_MATERIAL_ENABLE_UV1
		return V1;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetV2() const
	{
#if VOXEL_MATERIAL_ENABLE_UV2
		return V2;
#else
		return 0;
#endif
	}
	FORCEINLINE T Impl_GetV3() const
	{
#if VOXEL_MATERIAL_ENABLE_UV3
		return V3;
#else
		return 0;
#endif
	}

public:
	FORCEINLINE void Impl_SetR(T Value)
	{
#if VOXEL_MATERIAL_ENABLE_R
		R = Value;
#endif
	}
	FORCEINLINE void Impl_SetG(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_G
		G = Value;
#endif
	}
	FORCEINLINE void Impl_SetB(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_B
		B = Value;
#endif
	}
	FORCEINLINE void Impl_SetA(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_A
		A = Value;
#endif
	}
	
	FORCEINLINE void Impl_SetU0(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV0
		U0 = Value;
#endif
	}
	FORCEINLINE void Impl_SetU1(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV1
		U1 = Value;
#endif
	}
	FORCEINLINE void Impl_SetU2(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV2
		U2 = Value;
#endif
	}
	FORCEINLINE void Impl_SetU3(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV3
		U3 = Value;
#endif
	}
	
	FORCEINLINE void Impl_SetV0(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV0
		V0 = Value;
#endif
	}
	FORCEINLINE void Impl_SetV1(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV1
		V1 = Value;
#endif
	}
	FORCEINLINE void Impl_SetV2(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV2
		V2 = Value;
#endif
	}
	FORCEINLINE void Impl_SetV3(T Value) 
	{
#if VOXEL_MATERIAL_ENABLE_UV3
		V3 = Value;
#endif
	}

private:
#if VOXEL_MATERIAL_ENABLE_R
	T R;
#endif
#if VOXEL_MATERIAL_ENABLE_G
	T G;
#endif
#if VOXEL_MATERIAL_ENABLE_B
	T B;
#endif
#if VOXEL_MATERIAL_ENABLE_A
	T A;
#endif
#if VOXEL_MATERIAL_ENABLE_UV0
	T U0;
	T V0;
#endif
#if VOXEL_MATERIAL_ENABLE_UV1
	T U1;
	T V1;
#endif
#if VOXEL_MATERIAL_ENABLE_UV2
	T U2;
	T V2;
#endif
#if VOXEL_MATERIAL_ENABLE_UV3
	T U3;
	T V3;
#endif
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// TODO Make not compatible with BP and have a serialization-safe BP version of it
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterial 
#if CPP // Hide the template from UHT
	: public TVoxelMaterialImpl<FVoxelMaterial>
	, public TVoxelMaterialStorage<uint8>
#endif
{
	GENERATED_BODY()
	
public:
	FORCEINLINE FVoxelMaterial()
	{
	}
	FORCEINLINE FVoxelMaterial(EForceInit)
		: TVoxelMaterialImpl<FVoxelMaterial>(ForceInit)
	{
	}
	FORCEINLINE FVoxelMaterial(const TVoxelMaterialStorage<uint8>& Storage)
		: TVoxelMaterialStorage<uint8>(Storage)
	{
	}

	FORCEINLINE bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}
};

static_assert(FVoxelMaterial::NumChannels <= sizeof(FVoxelMaterial), "");

template <>
struct TTypeTraits<FVoxelMaterial> : TTypeTraitsBase<FVoxelMaterial>
{
	enum
	{
		IsBytewiseComparable = true
	};
};

template<>
struct TStructOpsTypeTraits<FVoxelMaterial> : TStructOpsTypeTraitsBase2<FVoxelMaterial>
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