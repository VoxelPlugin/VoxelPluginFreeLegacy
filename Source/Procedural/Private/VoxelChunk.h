// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TransitionDirection.h"
#include "VoxelChunk.generated.h"

class AVoxelWorld;
class AVoxelTransitionChunk;
class UProceduralMeshComponent;
class VoxelThread;

UCLASS()
class AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	friend class VoxelThread;

	AVoxelChunk();
	~AVoxelChunk();

	void Init(FIntVector position, int depth, AVoxelWorld* world);

	void Update(bool async);

	void BasicUpdate();

	void Unload();

	int GetDepth();

	signed char GetValue(int x, int y, int z);

	FColor GetColor(int x, int y, int z);

	bool ChunkHasHigherRes[6];

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
		UProceduralMeshComponent* PrimaryMesh;

	// Lower corner
	UPROPERTY(VisibleAnywhere)
		FIntVector Position;

	UPROPERTY(VisibleAnywhere)
		int Depth;

	AVoxelWorld* World;

	bool bNeedSectionUpdate;


	FAsyncTask<VoxelThread>* Task;

	FProcMeshSection Section;

	UPROPERTY(VisibleAnywhere)
		bool bNeedDeletion;
	UPROPERTY(VisibleAnywhere)
		float TimeUntilDeletion;

	bool bAdjacentChunksNeedUpdate;


	AVoxelChunk* GetChunk(TransitionDirection direction);

	void Delete();
};