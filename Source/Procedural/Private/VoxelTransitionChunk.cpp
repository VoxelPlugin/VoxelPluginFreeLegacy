// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTransitionChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "Transvoxel.h"
#include "RuntimeMeshComponent.h"
#include "VoxelChunk.h"
#include <vector>

AVoxelTransitionChunk::AVoxelTransitionChunk()
{
	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<URuntimeMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;
}

void AVoxelTransitionChunk::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelTransitionChunk::Update()
{
	Vertices.clear();
	Triangles.clear();
	VerticesCount = 0;
	TrianglesCount = 0;

	/**
	 * Polygonize
	 */
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			short validityMask = (x == 0 ? 0 : 1) + (y == 0 ? 0 : 2);
			TransvoxelTools::TransitionPolygonize(this, x, y, validityMask, Triangles, TrianglesCount, Vertices, VerticesProperties, VerticesCount);
		}

		NewCacheIs1 = !NewCacheIs1;
	}

	/**
	 * Compute normals + tangents & final arrays
	 */
	TArray<FVector> VerticesArray;
	VerticesArray.SetNumUninitialized(VerticesCount);

	// Fill arrays
	for (int i = VerticesCount - 1; i >= 0; i--)
	{
		FVector Vertex = Vertices.front();
		VertexProperties2D Properties = VerticesProperties.front();

		VerticesArray[i] = Vertex + (Properties.NeedTranslation ? FVector::ForwardVector : FVector::ZeroVector);// (Properties.NeedTranslation ? VoxelChunk->GetTranslated(Vertex, FVector::UpVector, *(struct VertexProperties*) &Properties) : Vertex);

		Vertices.pop_front();
		VerticesProperties.pop_front();
	}

	TArray<int> TrianglesArray;
	TrianglesArray.SetNumUninitialized(TrianglesCount);

	// Add triangles
	int i = 0;
	for (auto it = Triangles.begin(); it != Triangles.end(); ++it)
	{
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;

		// Add triangles
		TrianglesArray[i] = a;
		TrianglesArray[i + 1] = b;
		TrianglesArray[i + 2] = c;
		i += 3;
	}

	TArray<FVector> NormalsArray;
	NormalsArray.Init(FVector::UpVector, VerticesCount);
	TArray<FRuntimeMeshTangent> TangentsArray;
	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;

	if (VerticesArray.Num() != 0)
	{
		if (PrimaryMesh->DoesSectionExist(0))
		{
			PrimaryMesh->UpdateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, TangentsArray, ESectionUpdateFlags::MoveArrays);
		}
		else
		{
			PrimaryMesh->CreateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, TangentsArray, false, EUpdateFrequency::Frequent);
		}
	}
}

void AVoxelTransitionChunk::Init(AVoxelWorld* world, AVoxelChunk* chunk, FIntVector position, int depth, TransitionDirectionEnum transitionDirection)
{
	check(world);
	check(chunk);

	World = world;
	VoxelChunk = chunk;
	Position = position;
	Depth = depth;
	TransitionDirection = transitionDirection;

	FVector relativeLocation = (FVector)position;

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);
	PrimaryMesh->SetMaterial(0, world->VoxelMaterial);
	PrimaryMesh->bCastShadowAsTwoSided = true;
}

signed char AVoxelTransitionChunk::GetValue(int x, int y)
{
	return World->GetValue(Position + GetRealPosition(x, y));
}

void AVoxelTransitionChunk::SaveVertex(int x, int y, short edgeIndex, int index)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	NewCache[x][edgeIndex] = index;
}

int AVoxelTransitionChunk::LoadVertex(int x, int y, short direction, short edgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;

	check(0 <= x - (xIsDifferent ? 1 : 0) && x - (xIsDifferent ? 1 : 0) < 16);
	check(0 <= edgeIndex && edgeIndex < 10);

	return (yIsDifferent ? OldCache : NewCache)[x - (xIsDifferent ? 1 : 0)][edgeIndex];
}

int AVoxelTransitionChunk::GetDepth()
{
	return Depth;
}

FIntVector AVoxelTransitionChunk::GetRealPosition(int x, int y)
{
	int z = 0;
	int width = 16 << Depth;
	switch (TransitionDirection)
	{
	case XMin:
		return FIntVector(z, x, y);
	case XMax:
		return FIntVector(width - z, width - x, y);
	case YMin:
		return FIntVector(width - x, z, y);
	case YMax:
		return FIntVector(width - x, width - z, width - y);
	case ZMin:
		return FIntVector(y, width - x, z);
	case ZMax:
		return FIntVector(width - y, width - x, width - z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}

FBoolVector AVoxelTransitionChunk::GetRealIsExact(bool xIsExact, bool yIsExact)
{
	int width = 16 << Depth;
	switch (TransitionDirection)
	{
	case XMin:
		return FBoolVector(true, xIsExact, yIsExact);
	case XMax:
		return FBoolVector(true, xIsExact, yIsExact);
	case YMin:
		return FBoolVector(xIsExact, true, yIsExact);
	case YMax:
		return FBoolVector(xIsExact, true, yIsExact);
	case ZMin:
		return FBoolVector(yIsExact, xIsExact, true);
	case ZMax:
		return FBoolVector(yIsExact, xIsExact, true);
	default:
		check(false);
		return FBoolVector(false, false, false);
	}
}