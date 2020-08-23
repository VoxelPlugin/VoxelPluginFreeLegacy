// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelMaterialIndices.h"

class FVoxelMaterialInterface;

struct FVoxelChunkMaterials
{
public:
	FVoxelChunkMaterials() = default;

	template<typename T>
	TVoxelSharedRef<FVoxelMaterialInterface> FindOrAddSingle(T Create)
	{
		if (!SingleMaterial.IsValid())
		{
			const TVoxelSharedRef<FVoxelMaterialInterface> NewMaterial = Create();
			SingleMaterial = NewMaterial;
		}
		return SingleMaterial.ToSharedRef();
	}
	template<typename T>
	TVoxelSharedRef<FVoxelMaterialInterface> FindOrAddMultiple(const FVoxelMaterialIndices& Key, T Create)
	{
		auto* Result = Materials.Find(Key);
		if (!Result)
		{
			const TVoxelSharedRef<FVoxelMaterialInterface> NewMaterial = Create();
			Result = &Materials.Add(Key, NewMaterial);
		}
		return Result->ToSharedRef();
	}

	void Reset()
	{
		SingleMaterial.Reset();
		Materials.Empty();
	}

private:
	TVoxelSharedPtr<FVoxelMaterialInterface> SingleMaterial;
	TMap<FVoxelMaterialIndices, TVoxelSharedPtr<FVoxelMaterialInterface>> Materials;
};