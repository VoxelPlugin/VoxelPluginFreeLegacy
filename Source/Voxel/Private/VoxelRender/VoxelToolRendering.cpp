// Copyright 2021 Phyronnaz

#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelToolRenderingSubsystemProxy);

FVoxelToolRenderingId FVoxelToolRenderingManager::CreateTool(bool bEnabled)
{
	VOXEL_FUNCTION_COUNTER();
	
	FScopeLock Lock(&Section);
	return Tools.Add({ bEnabled });
}

void FVoxelToolRenderingManager::RemoveTool(FVoxelToolRenderingId Id)
{
	VOXEL_FUNCTION_COUNTER();
	
	FScopeLock Lock(&Section);
	if (ensure(Tools.IsValidIndex(Id)))
	{
		Tools.RemoveAt(Id);
		RecomputeToolsMaterials_AssumeLocked();
	}
}

bool FVoxelToolRenderingManager::IsValidTool(FVoxelToolRenderingId Id) const
{
	VOXEL_FUNCTION_COUNTER();
	
	FScopeLock Lock(&Section);
	return Tools.IsValidIndex(Id);
}

void FVoxelToolRenderingManager::EditTool(FVoxelToolRenderingId Id, TFunctionRef<void(FVoxelToolRendering&)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	FScopeLock Lock(&Section);
	if (ensure(Tools.IsValidIndex(Id)))
	{
		Lambda(Tools[Id]);
		RecomputeToolsMaterials_AssumeLocked();
	}
}

void FVoxelToolRenderingManager::IterateTools(TFunctionRef<void(const FVoxelToolRendering&)> Lambda) const
{
	FScopeLock Lock(&Section);
	for (auto& Tool : Tools)
	{
		Lambda(Tool);
	}
}

void FVoxelToolRenderingManager::RecomputeToolsMaterials_AssumeLocked()
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