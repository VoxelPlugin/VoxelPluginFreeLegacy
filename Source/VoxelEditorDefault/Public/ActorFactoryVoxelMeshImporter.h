// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "ActorFactoryVoxelMeshImporter.generated.h"

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelMeshImporter : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelMeshImporter()
	{
		DisplayName = VOXEL_LOCTEXT("Voxel Mesh Importer");
		NewActorClass = AVoxelMeshImporter::StaticClass();
	}
};