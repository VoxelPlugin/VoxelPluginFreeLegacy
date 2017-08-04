// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransvoxelTools.h"
#include "ProceduralMeshComponent.h"
#include "VoxelChunk.generated.h"

class AVoxelWorld;
class AVoxelTransitionChunk;
class UProceduralMeshComponent;
class VoxelThread;

UCLASS()
class AVoxelChunk : public AActor, public IRegularVoxel
{
	GENERATED_BODY()

public:
	friend class VoxelThread;

	AVoxelChunk();
	~AVoxelChunk();

	void Init(FIntVector position, int depth, AVoxelWorld* world);

	void Update();

	void Unload();

	FVector GetTranslated(FVector P, FVector normal, VertexProperties properties);

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

	int Cache1[18][18][4];
	int Cache2[18][18][4];
	bool NewCacheIs1;

	bool XMinChunkHasHigherRes;
	bool XMaxChunkHasHigherRes;
	bool YMinChunkHasHigherRes;
	bool YMaxChunkHasHigherRes;
	bool ZMinChunkHasHigherRes;
	bool ZMaxChunkHasHigherRes;
	AVoxelTransitionChunk* XMinChunk;
	AVoxelTransitionChunk* XMaxChunk;
	AVoxelTransitionChunk* YMinChunk;
	AVoxelTransitionChunk* YMaxChunk;
	AVoxelTransitionChunk* ZMinChunk;
	AVoxelTransitionChunk* ZMaxChunk;

	bool HasChunkHigherRes(int x, int y, int z);

	// Inherited via IRegularVoxel
	virtual signed char GetValue(int x, int y, int z) override;
	virtual FColor GetColor(int x, int y, int z) override;
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index) override;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) override;
	virtual int GetDepth() override;
	virtual bool IsNormalOnly(FVector vertex) override;
};