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

	void Init(FIntVector Position, int Depth, AVoxelWorld* World);

	void Update(bool bAsync);

	void BasicUpdate();

	void Unload();

	int GetDepth();

	int Width();

	signed char GetValue(int X, int Y, int Z);

	FColor GetColor(int X, int Y, int Z);

	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
		UProceduralMeshComponent* PrimaryMesh;

	UPROPERTY(EditAnywhere)
		bool bUpdate;

	// Lower corner
	UPROPERTY(VisibleAnywhere)
		FIntVector Position;

	UPROPERTY(VisibleAnywhere)
		int Depth;

	AVoxelWorld* World;

	bool bNeedSectionUpdate;


	FAsyncTask<VoxelThread>* Task;

	UPROPERTY(VisibleAnywhere)
		bool bNeedDeletion;
	UPROPERTY(VisibleAnywhere)
		float TimeUntilDeletion;

	bool bAdjacentChunksNeedUpdate;


	AVoxelChunk* GetChunk(TransitionDirection Direction);

	void Delete();
};