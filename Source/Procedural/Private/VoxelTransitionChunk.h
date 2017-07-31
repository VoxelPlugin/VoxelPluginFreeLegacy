// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransvoxelTools.h"
#include "VoxelTransitionChunk.generated.h"

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<VertexProperties> Props;
typedef std::forward_list<VertexProperties2D> Props2D;

class AVoxelWorld;
class AVoxelChunk;
class URuntimeMeshComponent;

enum TransitionDirectionEnum { XMin, XMax, YMin, YMax, ZMin, ZMax };

UCLASS()
class AVoxelTransitionChunk : public AActor, public ITransitionVoxel
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

	Verts Vertices;
	Trigs Triangles;
	Props2D VerticesProperties;

	int VerticesCount;
	int TrianglesCount;

	int Cache1[16][10];
	int Cache2[16][10];
	bool NewCacheIs1;

	// Inherited via ITransitionVoxel
	virtual signed char GetValue(int x, int y) override;
	virtual void SaveVertex(int x, int y, short edgeIndex, int index) override;
	virtual int LoadVertex(int x, int y, short direction, short edgeIndex) override;
	virtual int GetDepth() override;
	virtual FIntVector GetRealPosition(int x, int y) override;
	virtual FBoolVector GetRealIsExact(bool xIsExact, bool yIsExact) override;
};
