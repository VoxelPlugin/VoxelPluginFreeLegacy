// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chunk.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ChunkActor.generated.h"

UCLASS()
class PROCEDURAL_API AChunkActor : public AActor
{
	GENERATED_BODY()

public:
	AChunkActor();
	void Initialize(Chunk* chunk, float scale);
	void Update();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	Chunk* chunk;
	float scale;
	UPROPERTY(VisibleAnywhere, Category = Materials)
		UProceduralMeshComponent* mesh;

};
