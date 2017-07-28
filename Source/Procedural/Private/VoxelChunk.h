// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "VoxelChunk.generated.h"

class AVoxelWorld;


UCLASS()
class AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	void Init(FIntVector position, int depth, AVoxelWorld* world);
	void Update();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UProceduralMeshComponent* PrimaryMesh;

	// Lower corner
	FIntVector Position;
	int Depth;
	AVoxelWorld* World;

	std::forward_list<FVector> Vertices;
	std::forward_list<int> Triangles;

	int VerticesCount;
	int TrianglesCount;

	int Cache1[16][16][4];
	int Cache2[16][16][4];
	bool NewCacheIs1;


	void Polygonise(int x, int y, int z);
	char GetValue(int x, int y, int z);
	FVector InterpolateX(int xMin, int xMax, int y, int z);
	FVector InterpolateY(int x, int yMin, int yMax, int z);
	FVector InterpolateZ(int x, int y, int zMin, int zMax);

};