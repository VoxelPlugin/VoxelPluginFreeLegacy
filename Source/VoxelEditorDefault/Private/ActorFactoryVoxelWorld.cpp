// Copyright 2019 Phyronnaz

#include "ActorFactoryVoxelWorld.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"
#include "VoxelMaterialCollection.h"

#include "Materials/MaterialInterface.h"

#define LOCTEXT_NAMESPACE "Voxel"

UActorFactoryVoxelWorld::UActorFactoryVoxelWorld()
{
	DisplayName = LOCTEXT("VoxelWorldDisplayName", "Voxel World");
	NewActorClass = AVoxelWorld::StaticClass();
}

void UActorFactoryVoxelWorld::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelWorld* VoxelWorld = CastChecked<AVoxelWorld>(NewActor);
	VoxelWorld->bCreateWorldAutomatically = true;
	VoxelWorld->bUseCameraIfNoInvokersFound = true;
	VoxelWorld->SetWorldGeneratorClass(UVoxelFlatWorldGenerator::StaticClass());
	VoxelWorld->bEnableTessellation = true;
	VoxelWorld->MaterialConfig = EVoxelMaterialConfig::SingleIndex;
	VoxelWorld->MaterialCollection = LoadObject<UVoxelMaterialCollection>(this, TEXT("/Voxel/Example/TriplanarExampleCollection/TriplanarExampleCollection"));
	VoxelWorld->VoxelMaterial = LoadObject<UMaterialInterface>(this, TEXT("/Voxel/M_VoxelMaterial_Colors"));
	VoxelWorld->TessellatedVoxelMaterial = LoadObject<UMaterialInterface>(this, TEXT("/Voxel/M_VoxelMaterial_Colors_Tess"));
}

#undef LOCTEXT_NAMESPACE
