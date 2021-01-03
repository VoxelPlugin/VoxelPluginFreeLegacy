// Copyright 2021 Phyronnaz

#include "ActorFactoryVoxelMagicaVoxScene.h"
#include "VoxelImporters/VoxelMagicaVoxImporter.h"
#include "EngineUtils.h"

UActorFactoryVoxelMagicaVoxScene::UActorFactoryVoxelMagicaVoxScene()
{
	DisplayName = VOXEL_LOCTEXT("Voxel Magica Vox Scene Actor");
	NewActorClass = AVoxelMagicaVoxSceneActor::StaticClass();
}

void UActorFactoryVoxelMagicaVoxScene::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	if (NewActor->HasAnyFlags(RF_Transient))
	{
		// Preview actor - do not spawn any additional actor there!
		return;
	}
	
	auto* Scene = Cast<UVoxelMagicaVoxScene>(Asset);
	auto* SceneActor = Cast<AVoxelMagicaVoxSceneActor>(NewActor);
	if (!ensure(Scene) || !ensure(SceneActor))
	{
		return;
	}
	
	SceneActor->SetScene(Scene);
}

bool UActorFactoryVoxelMagicaVoxScene::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	return AssetData.GetClass() && AssetData.GetClass()->IsChildOf(UVoxelMagicaVoxScene::StaticClass());
}