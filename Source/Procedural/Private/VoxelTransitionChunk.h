// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelTransitionChunk.generated.h"

class AVoxelWorld;
class URuntimeMeshComponent;

UCLASS()
class AVoxelTransitionChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelTransitionChunk();

	void Update();

	void Init(FIntVector position, bool isAlongX, bool isAlongY, bool isAlongZ);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		URuntimeMeshComponent* PrimaryMesh;

	FIntVector Position;

	bool IsAlongX;
	bool IsAlongY;
	bool IsAlongZ;
};
