// Copyright 2020 Phyronnaz

#include "ActorFactoryVoxelWorld.h"
#include "VoxelWorld.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelGenerators/VoxelFlatGenerator.h"
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
	VoxelWorld->SetGeneratorClass(UVoxelFlatGenerator::StaticClass());
	VoxelWorld->MaterialConfig = EVoxelMaterialConfig::RGB;
	VoxelWorld->MaterialCollection = FVoxelExampleUtilities::LoadExampleObject<UVoxelMaterialCollectionBase>(TEXT("/Voxel/Examples/Materials/Quixel/MC_Quixel"));
	VoxelWorld->VoxelMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Materials/Quixel/MI_VoxelQuixel_FiveWayBlend_Inst"));
	VoxelWorld->Toggle();
}