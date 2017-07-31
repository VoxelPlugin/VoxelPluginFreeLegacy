// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include <tuple>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransvoxelTools.h"
#include "VoxelChunk.generated.h"

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<VertexProperties> Props;

class AVoxelWorld;
class AVoxelCollisionChunk;
class AVoxelTransitionChunk;
class URuntimeMeshComponent;

UCLASS()
class AVoxelChunk : public AActor, public IRegularVoxel
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	void Init(FIntVector position, int depth, AVoxelWorld* world);

	void Update(URuntimeMeshComponent* mesh = nullptr, bool bCreateCollision = false);

	void Unload();

	FVector GetTranslated(FVector P, FVector normal, VertexProperties properties);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		URuntimeMeshComponent* PrimaryMesh;

	UPROPERTY()
		AVoxelCollisionChunk* CollisionChunk;

	// Lower corner
	UPROPERTY(VisibleAnywhere)
		FIntVector Position;

	UPROPERTY(VisibleAnywhere)
		int Depth;

	AVoxelWorld* World;

	Verts Vertices;
	Trigs Triangles;
	Props VerticesProperties;

	int VerticesCount;
	int TrianglesCount;

	int Cache1[18][18][4];
	int Cache2[18][18][4];
	bool NewCacheIs1;

	bool bCollisionDirty;

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
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index) override;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) override;
	virtual int GetDepth() override;
	virtual bool IsNormalOnly(FVector vertex) override;

	friend AVoxelCollisionChunk;
};