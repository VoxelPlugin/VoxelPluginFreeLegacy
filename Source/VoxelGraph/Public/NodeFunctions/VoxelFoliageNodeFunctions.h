// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContext.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelFoliage/VoxelSpawnerManagerBase.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

namespace FVoxelFoliageNodeFunctions
{
	FORCEINLINE v_flt SampleFoliageMaterialIndex(int32 Index, const FVoxelContext& Context)
	{
		auto& GetMaterial = Context.Items.QueryData.GetMaterial;
		if (!GetMaterial)
		{
			return 0;
		}
		const FVoxelMaterial Material = GetMaterial();
		const int32 IndexInMaterial = FVoxelUtilities::GetMultiIndexIndex(Material, Index);
		if (IndexInMaterial == -1)
		{
			return 0;
		}
		
		const auto Strengths = FVoxelUtilities::GetMultiIndexStrengths(Material);
		return Strengths[IndexInMaterial];
	}
	FORCEINLINE TVoxelRange<v_flt> SampleFoliageMaterialIndex(TVoxelRange<int32> Index, const FVoxelContextRange& Context)
	{
		return { 0, 1 };
	}

	inline int32 InitBiome(const FVoxelGeneratorInit& InitStruct, TWeakObjectPtr<UVoxelFoliageBiome> Biome)
	{
		const TVoxelSharedPtr<IVoxelSpawnerManagerBase> SpawnerManager = InitStruct.GetSpawnerManager();
		return SpawnerManager ? SpawnerManager->RegisterBiome(Biome.Get()) : -1;
	}
}