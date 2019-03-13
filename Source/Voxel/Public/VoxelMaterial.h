// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.generated.h"

FORCEINLINE uint8 CastToUINT8(int Value)
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

	inline uint8 GetVoxelActorId() const { return 0; }
	inline uint8 GetVoxelGrassId() const { return 0; }

public:
	inline void SetIndex(uint8 NewIndex) {}

	inline void SetR(uint8 NewR) {}
	inline void SetG(uint8 NewG) {}
	inline void SetB(uint8 NewB) {}
	inline void SetA(uint8 NewA) {}

	inline void SetIndexA(uint8 NewIndexA) {}
	inline void SetIndexB(uint8 NewIndexB) {}
	inline void SetBlend(uint8 NewBlend) {}

	inline void SetVoxelActorId(uint8 NewVoxelActorId) {}
	inline void SetVoxelGrassId(uint8 NewVoxelGrassId) {}

public:
	// Amount: between 0 and 1
	inline void AddColor(const FLinearColor& Color, float Amount, bool bPaintR = true, bool bPaintG = true, bool bPaintB = true, bool bPaintA = true) { AddColor(Color.ToFColor(false), Amount, bPaintR, bPaintG, bPaintB, bPaintA); }
	inline void AddColor(const FColor& Color, float Amount, bool bPaintR = true, bool bPaintG = true, bool bPaintB = true, bool bPaintA = true)
	{
		if (bPaintR)
		{
			static_cast<T*>(this)->SetR(FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(static_cast<T*>(this)->GetR(), Color.R, Amount)), 0, 255));
		}
		if (bPaintG)
		{
			static_cast<T*>(this)->SetG(FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(static_cast<T*>(this)->GetG(), Color.G, Amount)), 0, 255));
		}
		if (bPaintB)
		{
			static_cast<T*>(this)->SetB(FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(static_cast<T*>(this)->GetB(), Color.B, Amount)), 0, 255));
		}
		if (bPaintA)
		{
			static_cast<T*>(this)->SetA(FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(static_cast<T*>(this)->GetA(), Color.A, Amount)), 0, 255));
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
			static_cast<const T*>(this)->GetR() / 255.999f,
			static_cast<const T*>(this)->GetG() / 255.999f,
			static_cast<const T*>(this)->GetB() / 255.999f,
			static_cast<const T*>(this)->GetA() / 255.999f);
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
#if ENABLE_VOXELACTORS
		VoxelActorId = 0;
#endif
#if ENABLE_VOXELGRASS
		VoxelGrassId = 0;
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
#if ENABLE_VOXELACTORS
	inline uint8 GetVoxelActorId() const { return VoxelActorId; }
#endif
#if ENABLE_VOXELGRASS
	inline uint8 GetVoxelGrassId() const { return VoxelGrassId; }
#endif

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
#if ENABLE_VOXELACTORS
	inline void SetVoxelActorId(uint8 NewVoxelActorId) { VoxelActorId = NewVoxelActorId; }
#endif
#if ENABLE_VOXELGRASS
	inline void SetVoxelGrassId(uint8 NewVoxelGrassId) { VoxelGrassId = NewVoxelGrassId; }
#endif

public:
	inline void SetIndex(int NewIndex) { SetIndex(CastToUINT8(NewIndex)); }

	inline void SetR(int NewR) { SetR(CastToUINT8(NewR)); }
	inline void SetG(int NewG) { SetG(CastToUINT8(NewG)); }
	inline void SetB(int NewB) { SetB(CastToUINT8(NewB)); }
	inline void SetA(int NewA) { SetA(CastToUINT8(NewA)); }

	inline void SetIndexA(int NewIndexA) { SetIndexA(CastToUINT8(NewIndexA)); }
	inline void SetIndexB(int NewIndexB) { SetIndexB(CastToUINT8(NewIndexB)); }
	inline void SetBlend(int NewBlend) { SetBlend(CastToUINT8(NewBlend)); }

	inline void SetVoxelActorId(int NewVoxelActorId) { SetVoxelActorId(CastToUINT8(NewVoxelActorId)); }
	inline void SetVoxelGrassId(int NewVoxelGrassId) { SetVoxelGrassId(CastToUINT8(NewVoxelGrassId)); }

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
	template<typename T>
	inline void SetVoxelActorId(T) = delete;
	template<typename T>
	inline void SetVoxelGrassId(T) = delete;

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
#if ENABLE_VOXELACTORS
			&& VoxelActorId == Other.VoxelActorId
#endif
#if ENABLE_VOXELGRASS
			&& VoxelGrassId == Other.VoxelGrassId
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
#if ENABLE_VOXELACTORS
			|| VoxelActorId != Other.VoxelActorId
#endif
#if ENABLE_VOXELGRASS
			|| VoxelGrassId != Other.VoxelGrassId
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
#if ENABLE_VOXELACTORS
		Ar << Material.VoxelActorId;
#endif
#if ENABLE_VOXELGRASS
		Ar << Material.VoxelGrassId;
#endif
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

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
		Material.SetVoxelActorId(VoxelActor);
		Material.SetVoxelGrassId(VoxelGrass);

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
#if ENABLE_VOXELACTORS
	uint8 VoxelActorId;
#endif
#if ENABLE_VOXELGRASS
	uint8 VoxelGrassId;
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