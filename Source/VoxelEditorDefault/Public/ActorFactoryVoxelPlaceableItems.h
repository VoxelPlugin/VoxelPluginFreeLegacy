// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactoryVoxelPlaceableItems.generated.h"

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelPlaceableItem : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelPlaceableItem();

	//~ Begin UActorFactory Interface
	void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	//~ End UActorFactory Interface
};

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelDisableEditsBox : public UActorFactoryVoxelPlaceableItem
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelDisableEditsBox();
};

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelAssetActor : public UActorFactoryVoxelPlaceableItem
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelAssetActor();

	//~ Begin UActorFactory Interface
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//~ End UActorFactory Interface

private:
	virtual void InitActor(UObject* Asset, AActor* NewActor);
};