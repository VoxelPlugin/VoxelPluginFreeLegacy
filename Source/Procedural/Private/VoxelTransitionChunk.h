// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <forward_list>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransvoxelTools.h"
#include "VoxelTransitionChunk.generated.h"

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<FColor> Colors;
typedef std::forward_list<VertexProperties> Props;
typedef std::forward_list<VertexProperties2D> Props2D;

class AVoxelWorld;
class AVoxelChunk;
class URuntimeMeshComponent;

enum TransitionDirectionEnum { XMin, XMax, YMin, YMax, ZMin, ZMax };

UCLASS()
class AVoxelTransitionChunk : public AActor, public ITransitionVoxel, public IRegularVoxel
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
	Colors VertexColors;
	Trigs TransitionTriangles;
	Trigs RegularTriangles;
	Props2D VerticesProperties;
	// Pairs of equivalent vertices (from, to)
	std::forward_list<int> Equivalences;

	int VerticesCount;

	int TransitionTrianglesCount;
	int RegularTrianglesCount;

	//int CacheHighRes[32][32][4];
	int CacheTransvoxel[16][16][10];
	int CacheLowRes[2][17][17][4];

	FVector GetRealPosition(FVector vertex);
	FIntVector GetRealPosition(int x, int y, int z);
	FIntVector InverseGetRealPosition(int x, int y, int z);
	FBoolVector GetRealIsExact(bool xIsExact, bool yIsExact, bool zIsExact);
	FBoolVector InverseGetRealIsExact(bool x, bool y, bool z);

	// Inherited via ITransitionVoxel
	virtual signed char GetValue(int x, int y) override;
	virtual FColor GetColor(int x, int y) override;
	virtual void SaveVertex(int x, int y, short edgeIndex, int index) override;
	virtual int LoadVertex(int x, int y, short direction, short edgeIndex) override;
	virtual int GetDepth() override;

	// Inherited via IRegularVoxel
	virtual signed char GetValue(int x, int y, int z) override;
	virtual FColor GetColor(int x, int y, int z) override;
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index) override;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) override;
	virtual bool IsNormalOnly(FVector vertex) override;
};
