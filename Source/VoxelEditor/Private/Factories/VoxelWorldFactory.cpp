// Copyright 2019 Phyronnaz

#include "VoxelWorldFactory.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"
#include "VoxelMaterialCollection.h"

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
	VoxelWorld->SetEnableTessellation(true);
	VoxelWorld->SetMaterialConfig(EVoxelMaterialConfig::SingleIndex);
	VoxelWorld->SetMaterialCollection(LoadObject<UVoxelMaterialCollection>(this, TEXT("/Voxel/Example/TriplanarExampleCollection/TriplanarExampleCollection")));
	VoxelWorld->bShowPopupIfNoInvokers = true;
}

#undef LOCTEXT_NAMESPACE
