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
	VoxelWorld->MaterialConfig = EVoxelMaterialConfig::RGB;
	VoxelWorld->MaterialCollection = LoadObject<UVoxelMaterialCollectionBase>(nullptr, TEXT("/Voxel/Examples/Materials/Quixel/MC_Quixel"));
	VoxelWorld->VoxelMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Examples/Materials/Quixel/MI_VoxelQuixel_FiveWayBlend_Inst"));
	VoxelWorld->Toggle();
}