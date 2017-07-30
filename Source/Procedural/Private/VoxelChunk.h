// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class AVoxelWorld;
class AVoxelCollisionChunk;
class URuntimeMeshComponent;

DECLARE_LOG_CATEGORY_EXTERN(VoxelChunkLog, Log, All);

UCLASS()
class AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	void Init(FIntVector position, int depth, AVoxelWorld* world);

	void Update(URuntimeMeshComponent* mesh = nullptr, bool bCreateCollision = false);

	void Unload();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		URuntimeMeshComponent* PrimaryMesh;

	UPROPERTY()
		AVoxelCollisionChunk* CollisionChunk;

	// Lower corner
	FIntVector Position;

	int Depth;
	
	AVoxelWorld* World;

	std::forward_list<FVector> Vertices;
	std::forward_list<int> Triangles;
	std::forward_list<int> NormalsTriangles;

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

	void Polygonise(int x, int y, int z);
	char GetValue(int x, int y, int z);
	bool HasChunkHigherRes(int x, int y, int z);
	FVector GetTranslated(FVector P);

	int AddVertex(FVector vertex);
	int LoadCachedVertex(int x, int y, int z, short direction, int edgeIndex);

	FVector InterpolateX(int xMin, int xMax, int y, int z);
	FVector InterpolateY(int x, int yMin, int yMax, int z);
	FVector InterpolateZ(int x, int y, int zMin, int zMax);

	friend AVoxelCollisionChunk;
};