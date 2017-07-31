// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VertexProperties.h"
#include "VoxelTransitionChunk.generated.h"

class AVoxelWorld;
class AVoxelChunk;
class URuntimeMeshComponent;

enum TransitionDirectionEnum { XMin, XMax, YMin, YMax, ZMin, ZMax };

UCLASS()
class AVoxelTransitionChunk : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelTransitionChunk();

	void Update();

	void Init(AVoxelWorld* world, AVoxelChunk* chunk, FIntVector position, int depth, TransitionDirectionEnum transitionDirection);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
		URuntimeMeshComponent* PrimaryMesh;

	UPROPERTY()
		AVoxelChunk* VoxelChunk;

	UPROPERTY(VisibleAnywhere)
		FIntVector Position;

	UPROPERTY(VisibleAnywhere)
		int Depth;

	AVoxelWorld* World;

	TransitionDirectionEnum TransitionDirection;

	std::forward_list<FVector> Vertices;
	std::forward_list<int> Triangles;
	std::forward_list<VertexProperties> VerticesProperties;

	int VerticesCount;
	int TrianglesCount;

	int Cache1[16][10];
	int Cache2[16][10];
	bool NewCacheIs1;

	void Polygonise(int x, int y);
	char GetValue(int x, int y);

	int AddVertex(FVector vertex, int z);
	int LoadCachedVertex(int x, int y, short direction, int edgeIndex);

	FIntVector GetRotated(int x, int y, int z);
	FIntVector GetRotated(FIntVector position);

	FVector InterpolateX(int xMin, int xMax, int y, int z);
	FVector InterpolateY(int x, int yMin, int yMax, int z);
};
