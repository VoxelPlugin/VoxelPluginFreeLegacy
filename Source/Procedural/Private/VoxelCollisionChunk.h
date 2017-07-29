// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RuntimeMeshComponent.h"
#include "VoxelCollisionChunk.generated.h"

class AVoxelChunk;

UCLASS()
class AVoxelCollisionChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelCollisionChunk();

	UPROPERTY()
		AVoxelChunk* VoxelChunk;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
		URuntimeMeshComponent* CollisionMesh;

	float TimeBeforeCollisionCooking;
};
