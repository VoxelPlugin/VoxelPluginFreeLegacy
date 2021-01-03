// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/ScopeLock.h"
#include "VoxelSubsystem.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "VoxelToolRendering.generated.h"

class FVoxelMaterialInterface;

struct FVoxelToolRendering
{
	bool bEnabled = false;
	FBox WorldBounds;
	TVoxelSharedPtr<FVoxelMaterialInterface> Material;
};

DECLARE_TYPED_VOXEL_SPARSE_ARRAY_ID(FVoxelToolRenderingId);

UCLASS()
class VOXEL_API UVoxelToolRenderingSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelToolRenderingManager);
};

class FVoxelToolRenderingManager : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelToolRenderingSubsystemProxy);

	FVoxelToolRenderingId CreateTool(bool bEnabled = false);
	void RemoveTool(FVoxelToolRenderingId Id);
	bool IsValidTool(FVoxelToolRenderingId Id) const;
	void EditTool(FVoxelToolRenderingId Id, TFunctionRef<void(FVoxelToolRendering&)> Lambda);

	void IterateTools(TFunctionRef<void(const FVoxelToolRendering&)> Lambda) const;

	const TArray<TVoxelSharedPtr<const FVoxelMaterialInterface>>& GetToolsMaterials() const
	{
		check(IsInGameThread());
		return ToolsMaterials;
	}

private:
	mutable FCriticalSection Section;
	TVoxelTypedSparseArray<FVoxelToolRenderingId, FVoxelToolRendering> Tools;
	TArray<TVoxelSharedPtr<const FVoxelMaterialInterface>> ToolsMaterials;

	void RecomputeToolsMaterials_AssumeLocked();
};