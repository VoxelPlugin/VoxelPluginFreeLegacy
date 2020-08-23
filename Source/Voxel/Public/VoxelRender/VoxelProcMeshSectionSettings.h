// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class FVoxelMaterialInterface;

struct FVoxelProcMeshSectionSettings
{
	TVoxelSharedPtr<FVoxelMaterialInterface> Material;
	bool bEnableCollisions = false;
	bool bEnableNavmesh = false;
	bool bEnableTessellation = false;
	bool bSectionVisible = true;

	FVoxelProcMeshSectionSettings() = default;
	FVoxelProcMeshSectionSettings(
		const TVoxelSharedPtr<FVoxelMaterialInterface>& MaterialInterface, 
		bool bEnableCollisions, 
		bool bEnableNavmesh, 
		bool bEnableTessellation, 
		bool bSectionVisible)
		: Material(MaterialInterface)
		, bEnableCollisions(bEnableCollisions)
		, bEnableNavmesh(bEnableNavmesh)
		, bEnableTessellation(bEnableTessellation)
		, bSectionVisible(bSectionVisible)
	{
	}
	
	bool operator==(const FVoxelProcMeshSectionSettings& Other) const
	{
		return
			Material == Other.Material &&
			bEnableCollisions == Other.bEnableCollisions &&
			bEnableNavmesh == Other.bEnableNavmesh &&
			bEnableTessellation == Other.bEnableTessellation &&
			bSectionVisible == Other.bSectionVisible;
	}
	friend uint32 GetTypeHash(const FVoxelProcMeshSectionSettings& Settings)
	{
		return
			GetTypeHash(Settings.Material) +
			131 * Settings.bEnableCollisions +
			9109 * Settings.bEnableNavmesh +
			50551 * Settings.bEnableTessellation +
			100019 * Settings.bSectionVisible;
	}
};