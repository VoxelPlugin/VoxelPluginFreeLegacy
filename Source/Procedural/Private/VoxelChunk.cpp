// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Transvoxel.h"
#include <vector>


// Sets default values
AVoxelChunk::AVoxelChunk()
{
	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RootComponent = PrimaryMesh;
}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelChunk::Init(int x, int y, int z, int depth, AVoxelWorld* world)
{
	X = x;
	Y = y;
	Z = z;
	Depth = depth;
	World = world;

	FString name = FString::FromInt(x) + ", " + FString::FromInt(y) + ", " + FString::FromInt(z);
	FVector relativeLocation = FVector(x, y, z);

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorLabel(name);
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector * (1 << Depth));
	PrimaryMesh->SetMaterial(0, World->VoxelMaterial);
}

void AVoxelChunk::Update()
{
	/**
	* Initialize
	*/
	Vertices.clear();
	Triangles.clear();
	VerticesCount = 0;
	TrianglesCount = 0;

	/**
	* Polygonize
	*/
	for (int z = 0; z < 16; z++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{

				Polygonise(x, y, z);
			}
		}
		NewCacheIs1 = !NewCacheIs1;
	}

	/**
	* Compute normals + tangents & final arrays
	*/
	TArray<FVector> VerticesArray;
	VerticesArray.SetNumUninitialized(VerticesCount);

	TArray<FVector> NormalsArray;
	NormalsArray.Init(FVector::ZeroVector, VerticesCount);

	TArray<int> TrianglesArray;
	TrianglesArray.SetNumUninitialized(TrianglesCount);

	TArray<FVector> TangentVectorsArray;
	TangentVectorsArray.SetNumUninitialized(VerticesCount);

	TArray<FProcMeshTangent> TangentsArray;
	TangentsArray.SetNumUninitialized(VerticesCount);


	int i = 0;
	for (auto it = Vertices.begin(); it != Vertices.end(); ++it)
	{
		VerticesArray[VerticesCount - 1 - i] = *it;
		i++;
	}
	i = 0;
	for (auto it = Triangles.begin(); it != Triangles.end(); ++it)
	{
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;

		TrianglesArray[i] = a;
		TrianglesArray[i + 1] = b;
		TrianglesArray[i + 2] = c;
		i += 3;

		FVector A = VerticesArray[a];
		FVector B = VerticesArray[b];
		FVector C = VerticesArray[c];
		FVector n = FVector::CrossProduct(C - A, B - A);
		// surface = norm(n) / 2
		// We want: normals += n / norm(n) * surface
		// <=> normals += n / 2
		// <=> normals += n because normals are normalized
		n.Normalize();
		NormalsArray[a] += n;
		NormalsArray[b] += n;
		NormalsArray[c] += n;
		TangentVectorsArray[a] += B + C - 2 * A;
		TangentVectorsArray[b] += B + C - 2 * A;
		TangentVectorsArray[c] += B + C - 2 * A;
	}

	for (int i = 0; i < TangentVectorsArray.Num(); i++)
	{
		TangentsArray[i] = FProcMeshTangent(TangentVectorsArray[i].GetSafeNormal(), false);
		NormalsArray[i].Normalize();
	}


	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;

	if (TangentsArray.Num() != VerticesArray.Num() || NormalsArray.Num() != VerticesArray.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error tangents or normals"));
	}

	PrimaryMesh->CreateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, TangentsArray, true);
}

void AVoxelChunk::Polygonise(int x, int y, int z)
{
	signed char corner[8] = {
		GetValue(x    , y    , z),
		GetValue(x + 1, y    , z),
		GetValue(x    , y + 1, z),
		GetValue(x + 1, y + 1, z),
		GetValue(x    , y    , z + 1),
		GetValue(x + 1, y    , z + 1),
		GetValue(x    , y + 1, z + 1),
		GetValue(x + 1, y + 1, z + 1)
	};

	FVector positions[8] = {
		FVector(x    , y    , z),
		FVector(x + 1, y    , z),
		FVector(x    , y + 1, z),
		FVector(x + 1, y + 1, z),
		FVector(x    , y    , z + 1),
		FVector(x + 1, y    , z + 1),
		FVector(x    , y + 1, z + 1),
		FVector(x + 1, y + 1, z + 1)
	};

	unsigned long caseCode = ((corner[0] >> 7) & 0x01)
		| ((corner[1] >> 6) & 0x02)
		| ((corner[2] >> 5) & 0x04)
		| ((corner[3] >> 4) & 0x08)
		| ((corner[4] >> 3) & 0x10)
		| ((corner[5] >> 2) & 0x20)
		| ((corner[6] >> 1) & 0x40)
		| (corner[7] & 0x80);


	if ((caseCode ^ ((corner[7] >> 7) & 0xFF)) != 0)
	{
		// Cell has a nontrivial triangulation
		unsigned char cellClass = regularCellClass[caseCode];
		RegularCellData cellData = regularCellData[cellClass];
		const unsigned short* vertexData = regularVertexData[caseCode];
		short validityMask = (x == 0 ? 0 : 1) + (y == 0 ? 0 : 2) + (z == 0 ? 0 : 4);

		std::vector<int> vertexIndices(cellData.GetVertexCount());

		auto newCache = NewCacheIs1 ? Cache1 : Cache2;
		auto oldCache = NewCacheIs1 ? Cache2 : Cache1;

		for (int i = 0; i < cellData.GetVertexCount(); i++)
		{
			int verticeIndex;
			unsigned short edgeCode = vertexData[i];
			unsigned short v0 = (edgeCode >> 4) & 0x0F;
			unsigned short v1 = edgeCode & 0x0F;
			long d0 = corner[v0];
			long d1 = corner[v1];
			FVector P0 = positions[v0];
			FVector P1 = positions[v1];

			short edgeIndex = (edgeCode >> 8) & 0x0F;
			short direction = edgeCode >> 12;

			long t = (d1 << 8) / (d1 - d0);
			if ((t & 0x00FF) != 0)
			{
				// Vertex lies in the interior of the edge
				if ((validityMask & direction) != direction)
				{
					long u = 0x0100 - t;
					FVector Q = (t * P0 + u * P1) / 256;
					Vertices.push_front(Q);
					verticeIndex = VerticesCount;
					VerticesCount++;
					newCache[x][y][edgeIndex] = verticeIndex;
				}
				else
				{
					bool xIsDifferent = direction & 0x01;
					bool yIsDifferent = direction & 0x02;
					bool zIsDifferent = direction & 0x04;
					verticeIndex = (zIsDifferent ? oldCache : newCache)[x - (xIsDifferent ? 1 : 0)][y - (yIsDifferent ? 1 : 0)][edgeIndex];
				}
			}
			else if (t == 0)
			{
				// Vertex lies at the higher-numbered endpoint.
				if (v1 == 7 || ((validityMask & direction) != direction))
				{
					// This cell owns the vertex or is along minimal boundaries
					Vertices.push_front(P1);
					verticeIndex = VerticesCount;
					VerticesCount++;
					newCache[x][y][edgeIndex] = verticeIndex;
				}
				else
				{
					bool xIsDifferent = direction & 0x01;
					bool yIsDifferent = direction & 0x02;
					bool zIsDifferent = direction & 0x04;
					verticeIndex = (zIsDifferent ? oldCache : newCache)[x - (xIsDifferent ? 1 : 0)][y - (yIsDifferent ? 1 : 0)][edgeIndex];
				}
			}
			else
			{
				// Vertex lies at the lower-numbered endpoint.
				// Always try to reuse corner vertex from a preceding cell.
				if ((validityMask & direction) != direction)
				{
					Vertices.push_front(P0);
					verticeIndex = VerticesCount;
					VerticesCount++;
					newCache[x][y][edgeIndex] = verticeIndex;
				}
				else
				{
					bool xIsDifferent = direction & 0x01;
					bool yIsDifferent = direction & 0x02;
					bool zIsDifferent = direction & 0x04;
					verticeIndex = (zIsDifferent ? oldCache : newCache)[x - (xIsDifferent ? 1 : 0)][y - (yIsDifferent ? 1 : 0)][edgeIndex];
				}
			}

			vertexIndices[i] = verticeIndex;
		}

		// Add triangles
		for (int i = 0; i < 3 * cellData.GetTriangleCount(); i++)
		{
			Triangles.push_front(vertexIndices[cellData.vertexIndex[i]]);
		}
		TrianglesCount += 3 * cellData.GetTriangleCount();
	}
}

char AVoxelChunk::GetValue(int x, int y, int z)
{
	return World->GetValue(X + x * (1 + Depth), Y + y * (1 + Depth), Z + z * (1 + Depth));
}