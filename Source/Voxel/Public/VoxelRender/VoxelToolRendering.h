// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "Misc/ScopeLock.h"

class FVoxelMaterialInterface;

struct FVoxelToolRendering
{
	bool bEnabled = false;
	FBox WorldBounds;
	TVoxelSharedPtr<FVoxelMaterialInterface> Material;
};

DEFINE_TYPED_VOXEL_SPARSE_ARRAY_ID(FVoxelToolRenderingId);

class FVoxelToolRenderingManager : public TVoxelSharedFromThis<FVoxelToolRenderingManager>
{
public:
	FVoxelToolRenderingManager() = default;
	~FVoxelToolRenderingManager() = default;

	inline FVoxelToolRenderingId CreateTool(bool bEnabled = false)
	{
		FScopeLock Lock(&Section);
		return Tools.Add({ bEnabled });
	}
	inline void RemoveTool(FVoxelToolRenderingId Id)
	{
		FScopeLock Lock(&Section);
		if (!ensure(Tools.IsValidIndex(Id))) return;
		Tools.RemoveAt(Id);
		RecomputeToolsMaterials();
	}
	template<typename T>
	inline void EditTool(FVoxelToolRenderingId Id, T Lambda)
	{
		FScopeLock Lock(&Section);
		if (!ensure(Tools.IsValidIndex(Id))) return;
		Lambda(Tools[Id]);
		RecomputeToolsMaterials();
	}
	inline bool IsValidTool(FVoxelToolRenderingId Id) const
	{
		FScopeLock Lock(&Section);
		return Tools.IsValidIndex(Id);
	}
	template<typename T>
	inline void IterateTools(T Lambda) const
	{
		FScopeLock Lock(&Section);
		for (auto& Tool : Tools)
		{
			Lambda(Tool);
		}
	}
	inline const TArray<TVoxelSharedPtr<const FVoxelMaterialInterface>>& GetToolsMaterials() const
	{
		check(IsInGameThread());
		return ToolsMaterials;
	}

private:
	mutable FCriticalSection Section;
	TVoxelTypedSparseArray<FVoxelToolRenderingId, FVoxelToolRendering> Tools;
	TArray<TVoxelSharedPtr<const FVoxelMaterialInterface>> ToolsMaterials;

	inline void RecomputeToolsMaterials()
	{
		check(IsInGameThread());
		ToolsMaterials.Reset();
		for (auto& Tool : Tools)
		{
			if (Tool.Material.IsValid())
			{
				ToolsMaterials.Add(Tool.Material);
			}
		}
	}
};