// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactoryVoxelWorld.generated.h"

UCLASS()
class VOXELEDITORDEFAULT_API UActorFactoryVoxelWorld : public UActorFactory
{
	GENERATED_BODY()

public:
	UActorFactoryVoxelWorld();

	//~ Begin UActorFactory Interface
	void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	//~ End UActorFactory Interface
};