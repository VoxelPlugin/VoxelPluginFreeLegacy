// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.generated.h"

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterial
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
#else 
	inline uint8 GetIndex() const { return 0; }
#endif
#if ENABLE_VOXELCOLORS
	inline uint8 GetR() const { return R; }
	inline uint8 GetG() const { return G; }
	inline uint8 GetB() const { return B; }
	inline uint8 GetA() const { return GetIndex(); }

	inline uint8 GetIndexA() const { return R; }
	inline uint8 GetIndexB() const { return G; }
	inline uint8 GetBlend() const { return B; }
#else
	inline uint8 GetR() const { return 0; }
	inline uint8 GetG() const { return 0; }
	inline uint8 GetB() const { return 0; }
	inline uint8 GetA() const { return 0; }

	inline uint8 GetIndexA() const { return 0; }
	inline uint8 GetIndexB() const { return 0; }
	inline uint8 GetBlend() const { return 0; }
#endif
#if ENABLE_VOXELACTORS
	inline uint8 GetVoxelActorId() const { return VoxelActorId; }
#else
	inline uint8 GetVoxelActorId() const { return 0; }
#endif
#if ENABLE_VOXELGRASS
	inline uint8 GetVoxelGrassId() const { return VoxelGrassId; }
#else
	inline uint8 GetVoxelGrassId() const { return 0; }
#endif

public:
#if !DISABLE_VOXELINDEX
	inline void SetIndex(uint8 NewIndex) { Index = NewIndex; }
#else
	inline void SetIndex(uint8 NewIndex) {}
#endif
#if ENABLE_VOXELCOLORS
	inline void SetR(uint8 NewR) { R = NewR; }
	inline void SetG(uint8 NewG) { G = NewG; }
	inline void SetB(uint8 NewB) { B = NewB; }
	inline void SetA(uint8 NewA) { SetIndex(NewA); }

	inline void SetIndexA(uint8 NewIndexA) { R = NewIndexA; }
	inline void SetIndexB(uint8 NewIndexB) { G = NewIndexB; }
	inline void SetBlend(uint8 NewBlend) { B = NewBlend; }
#else
	inline void SetR(uint8 NewR) {}
	inline void SetG(uint8 NewG) {}
	inline void SetB(uint8 NewB) {}
	inline void SetA(uint8 NewA) {}

	inline void SetIndexA(uint8 NewIndexA) {}
	inline void SetIndexB(uint8 NewIndexB) {}
	inline void SetBlend(uint8 NewBlend) {}
#endif
#if ENABLE_VOXELACTORS
	inline void SetVoxelActorId(uint8 NewVoxelSpawnedActorId) { VoxelActorId = NewVoxelSpawnedActorId; }
#else
	inline void SetVoxelActorId(uint8 NewVoxelSpawnedActorId) {}
#endif
#if ENABLE_VOXELGRASS
	inline void SetVoxelGrassId(uint8 NewVoxelGrassId) { VoxelGrassId = NewVoxelGrassId; }
#else
	inline void SetVoxelGrassId(uint8 NewVoxelGrassId) {}
#endif

public:
#if ENABLE_VOXELCOLORS
	// Amount: between 0 and 1
	inline void AddColor(const FLinearColor& Color, float Amount) { AddColor(Color.ToFColor(false), Amount); }
	inline void AddColor(const FColor& Color, float Amount)
	{
		R     = FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(R    , Color.R, Amount)), 0, 255);
		G     = FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(G    , Color.G, Amount)), 0, 255);
		B     = FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(B    , Color.B, Amount)), 0, 255);
#if !DISABLE_VOXELINDEX
		Index = FMath::Clamp<int>(FMath::RoundToInt(FMath::Lerp<float>(Index, Color.A, Amount)), 0, 255);
#endif
	}
	inline void SetColor(const FLinearColor& Color) { SetColor(Color.ToFColor(false)); }
	inline void SetColor(const FColor& Color)
	{
		R     = Color.R;
		G     = Color.G;
		B     = Color.B;
#if !DISABLE_VOXELINDEX
		Index = Color.A;
#endif
	}
	inline FColor GetColor() const { return FColor(R, G, B, Index); }
	inline FLinearColor GetLinearColor() const { return FLinearColor(R / 255.999f, G / 255.999f, B / 255.999f, Index / 255.999f); }
#else
	inline void AddColor(const FLinearColor& Color, float Amount) {}
	inline void AddColor(const FColor& Color, float Amount) {}
	inline void SetColor(const FLinearColor& Color) {}
	inline void SetColor(const FColor& Color) {}
	inline FColor GetColor() const { return FColor(); }
	inline FLinearColor GetLinearColor() const { return FLinearColor(); }
#endif

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
			if (ConfigFlags & EnableRGBA)
			{
				Ar << R;
			}
			else
			{
				R = Index;
			}
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
	enum { IsBytewiseComparable = true };
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