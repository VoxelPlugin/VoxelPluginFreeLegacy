// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactoryVoxelMagicaVoxScene.generated.h"

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelMagicaVoxScene : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelMagicaVoxScene();

	//~ Begin UActorFactory Interface
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory Interface
};