// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include <tuple>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunk.generated.h"

class AVoxelWorld;
class AVoxelCollisionChunk;
class URuntimeMeshComponent;

struct VertexProperties
{
	bool IsNearXMin;
	bool IsNearXMax;

	bool IsNearYMin;
	bool IsNearYMax;

	bool IsNearZMin;
	bool IsNearZMax;

	bool IsNormalOnly;
};

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
	// Is near: XMin, XMax, YMin, YMax, ZMin, ZMax + Is only for normals computation
	std::forward_list<VertexProperties> VerticesProperties;

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
	FVector GetTranslated(FVector P, FVector normal, VertexProperties properties);

	int AddVertex(FVector vertex, FIntVector exactPosition, bool xIsExact = true, bool yIsExact = true, bool zIsExact = true);
	int LoadCachedVertex(int x, int y, int z, short direction, int edgeIndex);

	FVector InterpolateX(int xMin, int xMax, int y, int z);
	FVector InterpolateY(int x, int yMin, int yMax, int z);
	FVector InterpolateZ(int x, int y, int zMin, int zMax);

	friend AVoxelCollisionChunk;
};