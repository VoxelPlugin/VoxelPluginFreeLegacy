// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
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

	void Unload();

	int GetDepth();

	signed char GetValue(int x, int y, int z);

	FColor GetColor(int x, int y, int z);

	bool HasChunkHigherRes(int x, int y, int z);

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

	bool bNeedDeletion;
	float TimeUntilDeletion;

	void Delete();
};