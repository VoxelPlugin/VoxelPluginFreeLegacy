// Copyright 2020 Phyronnaz

#include "ActorFactoryVoxelWorld.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"

#include "Materials/MaterialInterface.h"

UActorFactoryVoxelWorld::UActorFactoryVoxelWorld()
{
	DisplayName = VOXEL_LOCTEXT("Voxel World");
	NewActorClass = AVoxelWorld::StaticClass();
}

void UActorFactoryVoxelWorld::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelWorld* VoxelWorld = CastChecked<AVoxelWorld>(NewActor);
	VoxelWorld->bCreateWorldAutomatically = true;
	VoxelWorld->bUseCameraIfNoInvokersFound = true;
	VoxelWorld->SetWorldGeneratorClass(UVoxelFlatWorldGenerator::StaticClass());
	VoxelWorld->bEnableTessellation = false;
	VoxelWorld->MaterialConfig = EVoxelMaterialConfig::RGB;
	VoxelWorld->MaterialCollection = LoadObject<UVoxelMaterialCollectionBase>(nullptr, TEXT("/Voxel/Examples/Materials/TriplanarExampleCollection/TriplanarExampleCollection"));
	VoxelWorld->VoxelMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Examples/Materials/RGB/M_VoxelMaterial_5WayBlend"));
	VoxelWorld->Toggle();
}