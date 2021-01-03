// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UMaterial;
class UVoxelGenerator;
class UVoxelProceduralMeshComponent;

struct VOXEL_API FVoxelEditorDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FFixVoxelLandscapeMaterial, UMaterial*);
	static FFixVoxelLandscapeMaterial FixVoxelLandscapeMaterial;
	
	DECLARE_DELEGATE_RetVal_FourParams(
		UStaticMesh*,
		FCreateStaticMeshFromProcMesh,
		UVoxelProceduralMeshComponent* /* Component */,
		TFunction<UStaticMesh*()> /* CreateObject */,
		bool /* bRecomputeNormals */,
		bool /* bAllowTransientMaterials */);
	
	static FCreateStaticMeshFromProcMesh CreateStaticMeshFromProcMesh;
	
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnVoxelGraphUpdated, UVoxelGenerator*);
	static FOnVoxelGraphUpdated OnVoxelGraphUpdated;
};