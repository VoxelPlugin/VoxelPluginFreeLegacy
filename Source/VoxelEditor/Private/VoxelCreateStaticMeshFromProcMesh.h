// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UVoxelProceduralMeshComponent;

class FVoxelCreateStaticMeshFromProcMesh
{
public:
	static void Init();

	static UStaticMesh* Create(UVoxelProceduralMeshComponent* Component, TFunction<UStaticMesh*()> CreateObject, bool bRecomputeNormals, bool bAllowTransientMaterials);

private:
	static void OnLightingBuildStarted();
};