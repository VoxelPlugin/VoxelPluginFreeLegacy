// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "VoxelRender/VoxelBlendedMaterial.h"

class IVoxelMaterialInterface;

struct FVoxelChunkMaterials
{
public:
	FVoxelChunkMaterials() = default;

	template<typename T>
	inline TVoxelSharedPtr<IVoxelMaterialInterface> GetSingleMaterial(bool bEnableTessellation, T Create)
	{
		if (!SingleMaterial[bEnableTessellation].IsValid())
		{
			SingleMaterial[bEnableTessellation] = Create();
		}
		return SingleMaterial[bEnableTessellation];
	}
	template<typename T>
	inline TVoxelSharedPtr<IVoxelMaterialInterface> GetMultipleMaterial(const FVoxelBlendedMaterialUnsorted& Key, bool bEnableTessellation, T Create)
	{
		auto* Result = Materials[bEnableTessellation].Find(Key);
		if(!Result)
		{
			auto New = Create();
			Result = &Materials[bEnableTessellation].Add(Key, New);
		}
		return *Result;
	}

	inline void Reset()
	{
		SingleMaterial[0].Reset();
		SingleMaterial[1].Reset();
		Materials[0].Empty();
		Materials[1].Empty();
	}

private:
	TStaticArray<TVoxelSharedPtr<IVoxelMaterialInterface>, 2> SingleMaterial;
	TStaticArray<TMap<FVoxelBlendedMaterialUnsorted, TVoxelSharedPtr<IVoxelMaterialInterface>>, 2> Materials;
};