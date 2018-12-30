// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "VoxelWorldFactory.generated.h"

UCLASS()
class UVoxelWorldFactory : public UActorFactory
{
	GENERATED_BODY()

public:
	UVoxelWorldFactory();

	//~ Begin UActorFactory Interface
	void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	//~ End UActorFactory Interface
};