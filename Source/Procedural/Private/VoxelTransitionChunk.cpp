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
			Polygonise(x, y);
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
		VerticesArray[i] = Vertex;
		Vertices.pop_front();
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

void AVoxelTransitionChunk::Init(AVoxelWorld* world, FIntVector position, int depth, TransitionDirectionEnum transitionDirection)
{
	check(world);

	World = world;
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

int AVoxelTransitionChunk::AddVertex(FVector vertex)
{
	Vertices.push_front(vertex);
	VerticesCount++;
	return VerticesCount - 1;
}

int AVoxelTransitionChunk::LoadCachedVertex(int x, int y, short direction, int edgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	return (yIsDifferent ? OldCache : NewCache)[x - (xIsDifferent ? 1 : 0)][edgeIndex];
}

void AVoxelTransitionChunk::Polygonise(int x, int y)
{
	int Step = 1 << Depth;
	signed char Corner[9] = {
		GetValue(x       * Step, y       * Step),
		GetValue((x + 1) * Step, y       * Step),
		GetValue((x + 2) * Step, y       * Step),
		GetValue(x       * Step, (y + 1) * Step),
		GetValue((x + 1) * Step, (y + 1) * Step),
		GetValue((x + 2) * Step, (y + 1) * Step),
		GetValue(x       * Step, (y + 2) * Step),
		GetValue((x + 1) * Step, (y + 2) * Step),
		GetValue((x + 2) * Step, (y + 2) * Step)
	};

	FIntVector Positions[13] = {
		FIntVector(x    , y    , 0) * Step,
		FIntVector(x + 1, y    , 0) * Step,
		FIntVector(x + 2, y    , 0) * Step,
		FIntVector(x    , y + 1, 0) * Step,
		FIntVector(x + 1, y + 1, 0) * Step,
		FIntVector(x + 2, y + 1, 0) * Step,
		FIntVector(x    , y + 2, 0) * Step,
		FIntVector(x + 1, y + 2, 0) * Step,
		FIntVector(x + 2, y + 2, 0) * Step,
		FIntVector(x    , y    , 1) * Step,
		FIntVector(x + 1, y    , 1) * Step,
		FIntVector(x    , y + 1, 1) * Step,
		FIntVector(x + 1, y + 1, 1) * Step
	};

	unsigned long CaseCode = ((Corner[0] >> 7) & 0x01)
		| ((Corner[1] >> 6) & 0x02)
		| ((Corner[2] >> 5) & 0x04)
		| ((Corner[5] >> 4) & 0x08)
		| ((Corner[8] >> 3) & 0x10)
		| ((Corner[7] >> 2) & 0x20)
		| ((Corner[6] >> 1) & 0x40)
		| (Corner[3] & 0x80)
		| ((Corner[4] << 1) & 0x100);

	if (CaseCode == 0 || CaseCode == 511)
	{
		return;
	}

	unsigned char CellClass = transitionCellClass[CaseCode];
	TransitionCellData CellData = transitionCellData[CellClass & 0x7F];
	const unsigned short* VertexData = transitionVertexData[CaseCode];
	// Check if precedent cell exist
	short ValidityMask = (x == 0 ? 0 : 1) + (y == 0 ? 0 : 2);

	std::vector<int> VertexIndices(CellData.GetVertexCount());

	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;

	for (int i = 0; i < CellData.GetVertexCount(); i++)
	{
		int VertexIndex;
		unsigned short EdgeCode = VertexData[i];

		// A: low point / B: high point
		unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
		unsigned short IndexVerticeB = EdgeCode & 0x0F;

		signed char ValueAtA = Corner[IndexVerticeA];
		signed char ValueAtB = Corner[IndexVerticeB];

		FIntVector PositionA = Positions[IndexVerticeA];
		FIntVector PositionB = Positions[IndexVerticeB];

		short EdgeIndex;
		short Direction;

		if (ValueAtB == 0)
		{
			// Vertex lies at the higher-numbered endpoint
			EdgeIndex = transitionCornerData[IndexVerticeB] & 0x0F;
			Direction = transitionCornerData[IndexVerticeB] >> 4;

			if (((ValidityMask & Direction) != Direction))
			{
				// Vertex failed validity check
				VertexIndex = AddVertex((FVector)GetRotated(PositionB));
			}
			else
			{
				// Vertex already created
				VertexIndex = LoadCachedVertex(x, y, Direction, EdgeIndex);
			}
		}
		else if (ValueAtA == 0)
		{
			EdgeIndex = transitionCornerData[IndexVerticeA] & 0x0F;
			Direction = transitionCornerData[IndexVerticeA] >> 4;

			// Vertex lies at the lower-numbered endpoint
			if ((ValidityMask & Direction) != Direction)
			{
				// Validity check failed
				VertexIndex = AddVertex((FVector)GetRotated(PositionA));
			}
			else
			{
				// Reuse vertex
				VertexIndex = LoadCachedVertex(x, y, Direction, EdgeIndex);
			}
		}
		else
		{
			EdgeIndex = (EdgeCode >> 8) & 0x0F;
			Direction = EdgeCode >> 12;

			// Vertex lies in the interior of the edge
			if ((ValidityMask & Direction) != Direction)
			{
				// Validity check failed
				if (Step == 1)
				{
					// Full resolution
					float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
					VertexIndex = AddVertex(t * (FVector)GetRotated(PositionA) + (1 - t) * (FVector)GetRotated(PositionB));
				}
				else
				{
					FVector Q;
					if (EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8)
					{
						// Edge along x axis
						Q = InterpolateX(PositionA.X, PositionB.X, PositionA.Y, PositionA.Z);
					}
					else if (EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9)
					{
						// Edge along y axis
						Q = InterpolateY(PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z);
					}
					else
					{
						checkf(false, TEXT("Error in interpolation: case should not exist"));
					}
					VertexIndex = AddVertex(Q);
				}
			}
			else
			{
				VertexIndex = LoadCachedVertex(x, y, Direction, EdgeIndex);
			}
		}

		// If own vertex, save it
		if (Direction & 0x08)
		{
			NewCache[x][EdgeIndex] = VertexIndex;
		}
		VertexIndices[i] = VertexIndex;

	}

	// Add triangles
	for (int i = 0; i < 3 * CellData.GetTriangleCount(); i++)
	{
		Triangles.push_front(VertexIndices[CellData.vertexIndex[i]]);
	}
	TrianglesCount += 3 * CellData.GetTriangleCount();
}


char AVoxelTransitionChunk::GetValue(int x, int y)
{
	return World->GetValue(Position + GetRotated(x, y, 0));
}

FVector AVoxelTransitionChunk::InterpolateX(int xMin, int xMax, int y, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = GetValue(xMin, y);
	signed char ValueAtB = GetValue(xMax, y);
	if (xMax - xMin == 1)
	{
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * (FVector)GetRotated(xMin, y, z) + (1 - t) *  (FVector)GetRotated(xMax, y, z);
	}
	else
	{
		check((xMax + xMin) % 2 == 0);

		int xMiddle = (xMax + xMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (GetValue(xMiddle, y) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateX(xMiddle, xMax, y, z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(xMin, xMiddle, y, z);
		}
	}
}

FVector AVoxelTransitionChunk::InterpolateY(int x, int yMin, int yMax, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = GetValue(x, yMin);
	signed char ValueAtB = GetValue(x, yMax);
	if (yMax - yMin == 1)
	{
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * (FVector)GetRotated(x, yMin, z) + (1 - t) *  (FVector)GetRotated(x, yMax, z);
	}
	else
	{
		check((yMax + yMin) % 2 == 0);

		int yMiddle = (yMax + yMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (GetValue(x, yMiddle) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateY(x, yMiddle, yMax, z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(x, yMin, yMiddle, z);
		}
	}
}

FIntVector AVoxelTransitionChunk::GetRotated(int x, int y, int z)
{
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

FIntVector AVoxelTransitionChunk::GetRotated(FIntVector position)
{
	return GetRotated(position.X, position.Y, position.Z);
}