// Copyright 2018 Phyronnaz

#include "VoxelWorldFactory.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"

#define LOCTEXT_NAMESPACE "VoxelWorldFactory"

UVoxelWorldFactory::UVoxelWorldFactory()
{
	DisplayName = LOCTEXT("VoxelWorldDisplayName", "Voxel World");
	NewActorClass = AVoxelWorld::StaticClass();
}

void UVoxelWorldFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelWorld* VoxelWorld = CastChecked<AVoxelWorld>(NewActor);
	VoxelWorld->SetCreateWorldAutomatically(true);
	VoxelWorld->SetWorldGeneratorClass(UVoxelFlatWorldGenerator::StaticClass());
}

#undef LOCTEXT_NAMESPACE
