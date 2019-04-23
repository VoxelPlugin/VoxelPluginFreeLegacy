// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMathUtilities.h"
#include "VoxelMaterial.generated.h"

FORCEINLINE uint8 CastToUINT8(int32 Value)
{
	ensureAlwaysMsgf(0 <= Value && Value < 256, TEXT("Invalid uint8 value: %d"), Value);
	return Value;
}

template<typename T>
struct VOXEL_API FVoxelMaterialBase
{
	inline uint8 GetIndex() const { return 0; }

	inline uint8 GetR() const { return 0; }
	inline uint8 GetG() const { return 0; }
	inline uint8 GetB() const { return 0; }
	inline uint8 GetA() const { return 0; }

	inline uint8 GetIndexA() const { return 0; }
	inline uint8 GetIndexB() const { return 0; }
	inline uint8 GetBlend() const { return 0; }

	inline FVector2D GetUVs() const { return {}; }

public:
	inline void SetIndex(uint8 NewIndex) {}

	inline void SetR(uint8 NewR) {}
	inline void SetG(uint8 NewG) {}
	inline void SetB(uint8 NewB) {}
	inline void SetA(uint8 NewA) {}

	inline void SetIndexA(uint8 NewIndexA) {}
	inline void SetIndexB(uint8 NewIndexB) {}
	inline void SetBlend(uint8 NewBlend) {}

public:
	// Amount: between 0 and 1
	inline void AddColor(const FLinearColor& Color, float Amount, bool bPaintR = true, bool bPaintG = true, bool bPaintB = true, bool bPaintA = true) { AddColor(Color.ToFColor(false), Amount, bPaintR, bPaintG, bPaintB, bPaintA); }
	inline void AddColor(const FColor& Color, float Amount, bool bPaintR = true, bool bPaintG = true, bool bPaintB = true, bool bPaintA = true)
	{
		if (bPaintR)
		{
			static_cast<T*>(this)->SetR(FVoxelUtilities::LerpUINT8(static_cast<T*>(this)->GetR(), Color.R, Amount));
		}
		if (bPaintG)
		{
			static_cast<T*>(this)->SetG(FVoxelUtilities::LerpUINT8(static_cast<T*>(this)->GetG(), Color.G, Amount));
		}
		if (bPaintB)
		{
			static_cast<T*>(this)->SetB(FVoxelUtilities::LerpUINT8(static_cast<T*>(this)->GetB(), Color.B, Amount));
		}
		if (bPaintA)
		{
			static_cast<T*>(this)->SetA(FVoxelUtilities::LerpUINT8(static_cast<T*>(this)->GetA(), Color.A, Amount));
		}
	}
	inline void SetColor(const FLinearColor& Color) { SetColor(Color.ToFColor(false)); }
	inline void SetColor(const FColor& Color)
	{
		static_cast<T*>(this)->SetR(Color.R);
		static_cast<T*>(this)->SetG(Color.G);
		static_cast<T*>(this)->SetB(Color.B);
		static_cast<T*>(this)->SetA(Color.A);
	}
	inline FColor GetColor() const
	{
		return FColor(
			static_cast<const T*>(this)->GetR(),
			static_cast<const T*>(this)->GetG(),
			static_cast<const T*>(this)->GetB(),
			static_cast<const T*>(this)->GetA());
	}
	inline FLinearColor GetLinearColor() const
	{
		return FLinearColor(
			FVoxelUtilities::UINT8ToFloat(static_cast<const T*>(this)->GetR()),
			FVoxelUtilities::UINT8ToFloat(static_cast<const T*>(this)->GetG()),
			FVoxelUtilities::UINT8ToFloat(static_cast<const T*>(this)->GetB()),
			FVoxelUtilities::UINT8ToFloat(static_cast<const T*>(this)->GetA()));
	}

public:
	inline bool operator==(const FVoxelMaterialBase& Other) const
	{
		checkf(false, TEXT("Should be overriden in child class"));
		return true;
	}

	inline bool operator!=(const FVoxelMaterialBase& Other) const
	{
		checkf(false, TEXT("Should be overriden in child class"));
		return false;
	}

public:
	friend inline FArchive& operator<<(FArchive &Ar, FVoxelMaterialBase& Material)
	{
		checkf(false, TEXT("Should be overriden in child class"));
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		checkf(false, TEXT("Should be overriden in child class"));
		Ar << *this;
		return true;
	}

	static inline FVoxelMaterialBase SerializeCompat(FArchive &Ar, uint32 ConfigFlags)
	{
		checkf(false, TEXT("Should be overriden in child class"));
		FVoxelMaterialBase Material;
		Ar << Material;
		return Material;
	}
};

/**
 * To add new fields to your material, you need to edit:
 * - The constructors
 * - operator==
 * - operator!=
 * - operator<<
 * - SerializeCompat
 * You can send those to your UE material through GetUVs: you'll need to use the CustomFVoxelMaterial UV Config
 */
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterial 
#if CPP // Hide the template from UHT
	: public FVoxelMaterialBase<FVoxelMaterial>
#endif
{
	GENERATED_BODY()
	
public:
	FVoxelMaterial()
	{
#if !DISABLE_VOXELINDEX
		Index = 0;
#endif
#if ENABLE_VOXELCOLORS
		R = 0;
		G = 0;
		B = 0;
#endif
	}

	FVoxelMaterial(ENoInit NoInit)
	{
	}
public:
#if !DISABLE_VOXELINDEX
	inline uint8 GetIndex() const { return Index; }
#endif
#if ENABLE_VOXELCOLORS
	inline uint8 GetR() const { return R; }
	inline uint8 GetG() const { return G; }
	inline uint8 GetB() const { return B; }
	inline uint8 GetA() const { return GetIndex(); }

	inline uint8 GetIndexA() const { return R; }
	inline uint8 GetIndexB() const { return G; }
	inline uint8 GetBlend() const { return B; }
#endif

	inline FVector2D GetUVs() const
	{
		// Use your custom fields here
		return {};
	}

public:
#if !DISABLE_VOXELINDEX
	inline void SetIndex(uint8 NewIndex) { Index = NewIndex; }
#endif
#if ENABLE_VOXELCOLORS
	inline void SetR(uint8 NewR) { R = NewR; }
	inline void SetG(uint8 NewG) { G = NewG; }
	inline void SetB(uint8 NewB) { B = NewB; }
	inline void SetA(uint8 NewA) { SetIndex(NewA); }

	inline void SetIndexA(uint8 NewIndexA) { R = NewIndexA; }
	inline void SetIndexB(uint8 NewIndexB) { G = NewIndexB; }
	inline void SetBlend(uint8 NewBlend) { B = NewBlend; }
#endif

public:
	inline void SetIndex(int32 NewIndex) { SetIndex(CastToUINT8(NewIndex)); }

	inline void SetR(int32 NewR) { SetR(CastToUINT8(NewR)); }
	inline void SetG(int32 NewG) { SetG(CastToUINT8(NewG)); }
	inline void SetB(int32 NewB) { SetB(CastToUINT8(NewB)); }
	inline void SetA(int32 NewA) { SetA(CastToUINT8(NewA)); }

	inline void SetIndexA(int32 NewIndexA) { SetIndexA(CastToUINT8(NewIndexA)); }
	inline void SetIndexB(int32 NewIndexB) { SetIndexB(CastToUINT8(NewIndexB)); }
	inline void SetBlend(int32 NewBlend) { SetBlend(CastToUINT8(NewBlend)); }

public:
	template<typename T>
	inline void SetIndex(T) = delete;

	template<typename T>
	inline void SetR(T) = delete;
	template<typename T>
	inline void SetG(T) = delete;
	template<typename T>
	inline void SetB(T) = delete;
	template<typename T>
	inline void SetA(T) = delete;
	
	template<typename T>
	inline void SetIndexA(T) = delete;
	template<typename T>
	inline void SetIndexB(T) = delete;
	template<typename T>
	inline void SetBlend(T) = delete;

public:
	inline bool operator==(const FVoxelMaterial& Other) const
	{
		return true
#if !DISABLE_VOXELINDEX
			&& Index      == Other.Index
#endif
#if ENABLE_VOXELCOLORS
			&& R          == Other.R
			&& G          == Other.G
			&& B          == Other.B
#endif
			;
	}

	inline bool operator!=(const FVoxelMaterial& Other) const
	{
		return false
#if !DISABLE_VOXELINDEX
			|| Index      != Other.Index
#endif
#if ENABLE_VOXELCOLORS
			|| R          != Other.R
			|| G          != Other.G
			|| B          != Other.B
#endif
			;
	}

public:
	friend inline FArchive& operator<<(FArchive &Ar, FVoxelMaterial& Material)
	{
#if !DISABLE_VOXELINDEX
		Ar << Material.Index;
#endif
#if ENABLE_VOXELCOLORS
		Ar << Material.R;
		Ar << Material.G;
		Ar << Material.B;
#endif
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

	/**
	 * Serialize an old version material
	 */
	static inline FVoxelMaterial SerializeCompat(FArchive &Ar, uint32 ConfigFlags)
	{
		check(Ar.IsLoading());

		uint8 Index = 0;
		uint8 R = 0;
		uint8 G = 0;
		uint8 B = 0;
		uint8 VoxelActor = 0;
		uint8 VoxelGrass = 0;

		if (!(ConfigFlags & DisableIndex))
		{
			Ar << Index;
		}
		if (ConfigFlags & EnableVoxelColors)
		{
			Ar << R;
			Ar << G;
			Ar << B;
		}
		if (ConfigFlags & EnableVoxelSpawnedActors)
		{
			Ar << VoxelActor;
		}
		if (ConfigFlags & EnableVoxelGrass)
		{
			Ar << VoxelGrass;
		}

		FVoxelMaterial Material;
		Material.SetIndex(Index);
		Material.SetR(R);
		Material.SetG(G);
		Material.SetB(B);

		return Material;
	}
	
private:
#if !DISABLE_VOXELINDEX
	uint8 Index;
#endif
#if ENABLE_VOXELCOLORS
	uint8 R;
	uint8 G;
	uint8 B;
#endif
};

template <>
struct TTypeTraits<FVoxelMaterial> : public TTypeTraitsBase<FVoxelMaterial>
{
	enum
	{
		IsBytewiseComparable = true
	};
};

template<>
struct TStructOpsTypeTraits<FVoxelMaterial> : public TStructOpsTypeTraitsBase2<FVoxelMaterial>
{
	enum 
	{
		WithSerializer = true,
		WithIdenticalViaEquality = true
	};
};