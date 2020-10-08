// Copyright 2020 Phyronnaz

#include "ActorFactoryVoxelPlaceableItems.h"
#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActor.h"
#include "VoxelPlaceableItems/Actors/VoxelDisableEditsBox.h"
#include "VoxelPlaceableItems/Actors/VoxelAssetActor.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelWorld.h"
#include "EngineUtils.h"

UActorFactoryVoxelPlaceableItem::UActorFactoryVoxelPlaceableItem()
{
	DisplayName = VOXEL_LOCTEXT("Voxel Placeable Item Actor");
	NewActorClass = AVoxelPlaceableItemActor::StaticClass();
}

void UActorFactoryVoxelPlaceableItem::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	AVoxelPlaceableItemActor* ItemActor = CastChecked<AVoxelPlaceableItemActor>(NewActor);
	
	for (TActorIterator<AVoxelWorld> ActorItr(ItemActor->GetWorld()); ActorItr; ++ActorItr)
	{
		ItemActor->PreviewWorld = *ActorItr;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

UActorFactoryVoxelDisableEditsBox::UActorFactoryVoxelDisableEditsBox()
{
	DisplayName = VOXEL_LOCTEXT("Voxel Disable Edits Box");
	NewActorClass = AVoxelDisableEditsBox::StaticClass();
}

///////////////////////////////////////////////////////////////////////////////

UActorFactoryVoxelAssetActor::UActorFactoryVoxelAssetActor()
{
	DisplayName = VOXEL_LOCTEXT("Voxel Asset Actor");
	NewActorClass = AVoxelAssetActor::StaticClass();
}

void UActorFactoryVoxelAssetActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);
	InitActor(Asset, NewActor);
}

void UActorFactoryVoxelAssetActor::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	Super::PostCreateBlueprint(Asset, CDO);
	InitActor(Asset, CDO);
}

bool UActorFactoryVoxelAssetActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	// We allow creating AVoxelAssetActor without an existing asset
	if (UActorFactory::CanCreateActorFrom(AssetData, OutErrorMsg))
	{
		return true;
	}
	if (!ensure(AssetData.IsValid()))
	{
		return false;
	}
	auto* Class = AssetData.GetClass();
	if(!Class)
	{
		return false;
	}
	if (Class->IsChildOf(UVoxelTransformableGenerator::StaticClass()))
	{
		return true;
	}
	return false;
}

UObject* UActorFactoryVoxelAssetActor::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	AVoxelAssetActor* AssetActor = CastChecked<AVoxelAssetActor>(ActorInstance);
	return AssetActor->Generator.GetObject();
}

void UActorFactoryVoxelAssetActor::InitActor(UObject* Asset, AActor* NewActor)
{
	if (!Asset || !NewActor)
	{
		return;
	}

	AVoxelAssetActor* AssetActor = CastChecked<AVoxelAssetActor>(NewActor);
	if (auto* Generator = Cast<UVoxelTransformableGenerator>(Asset))
	{
		AssetActor->Generator = Generator;
		AssetActor->bOverrideAssetBounds = !Asset->IsA<UVoxelTransformableGeneratorWithBounds>();
	}
}