// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelRevertTool.h"

#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"
#include "VoxelWorld.h"

UVoxelRevertTool::UVoxelRevertTool()
{
	ToolName = TEXT("Revert");
}

void UVoxelRevertTool::Tick()
{
	Super::Tick();
	
	CurrentHistoryPosition = GetVoxelWorld()->GetData().GetHistoryPosition();
	HistoryPosition = FMath::Clamp(HistoryPosition, 0, CurrentHistoryPosition);
}

FVoxelIntBoxWithValidity UVoxelRevertTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	auto& World = *GetVoxelWorld();

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = SharedConfig->BrushSize / 2.f / World.VoxelSize;

	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		return {};
	}
	const auto BoundsToCache = GetBoundsToCache(Bounds);

	auto& Data = World.GetData();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);	
	CacheData<FVoxelValue>(Data, BoundsToCache);

	if (GetTickData().IsAlternativeMode())
	{
		FVoxelSphereToolsImpl::RevertSphereToGenerator(
			DataImpl,
			VoxelPosition,
			VoxelRadius,
			bRevertValues,
			bRevertMaterials);
	}
	else
	{
		FVoxelSphereToolsImpl::RevertSphere(
			DataImpl,
			VoxelPosition,
			VoxelRadius,
			HistoryPosition,
			bRevertValues,
			bRevertMaterials);
	}

	return Bounds;
}
