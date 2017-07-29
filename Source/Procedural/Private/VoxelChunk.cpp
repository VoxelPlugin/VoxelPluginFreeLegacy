// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Transvoxel.h"
#include "DrawDebugHelpers.h"
#include "VoxelCollisionChunk.h"
#include <vector>


// Sets default values
AVoxelChunk::AVoxelChunk() : bCollisionDirty(true)
{
	// Create primary  mesh
	PrimaryMesh = CreateDefaultSubobject<URuntimeMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;
}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

	CollisionChunk = GetWorld()->SpawnActor<AVoxelCollisionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	CollisionChunk->VoxelChunk = this;
	CollisionChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	CollisionChunk->SetActorLabel("VoxelCollisionChunkActor");
	CollisionChunk->SetActorRelativeLocation(FVector::ZeroVector);
	CollisionChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	CollisionChunk->SetActorRelativeScale3D(FVector::OneVector);
}

void AVoxelChunk::Init(FIntVector position, int depth, AVoxelWorld* world)
{
	Position = position;
	Depth = depth;
	World = world;

	FString name = FString::FromInt(position.X) + ", " + FString::FromInt(position.Y) + ", " + FString::FromInt(position.Z);
	FVector relativeLocation = FVector(position.X, position.Y, position.Z);

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorLabel(name);
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);
	PrimaryMesh->SetMaterial(0, World->VoxelMaterial);
	PrimaryMesh->bCastShadowAsTwoSided = true;
}

void AVoxelChunk::Update(URuntimeMeshComponent* mesh, bool bCreateCollision)
{
	if (mesh == nullptr)
	{
		mesh = PrimaryMesh;
	}

	/**
	* Initialize
	*/
	Vertices.clear();
	Triangles.clear();
	NormalsTriangles.clear();
	VerticesCount = 0;
	TrianglesCount = 0;

	/**
	* Polygonize
	*/
	for (int z = -1; z < 17; z++)
	{
		for (int y = -1; y < 17; y++)
		{
			for (int x = -1; x < 17; x++)
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

	TArray<FRuntimeMeshTangent> TangentsArray;
	TangentsArray.SetNumUninitialized(VerticesCount);


	int i = 0;
	for (auto it = Vertices.begin(); it != Vertices.end(); ++it)
	{
		VerticesArray[VerticesCount - 1 - i] = *it;
		i++;
	}
	i = 0;

	// Real triangles
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

	// Triangles used only for normals
	for (auto it = NormalsTriangles.begin(); it != NormalsTriangles.end(); ++it)
	{
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;

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
		TangentsArray[i] = FRuntimeMeshTangent(TangentVectorsArray[i].GetSafeNormal());
		NormalsArray[i].Normalize();
	}


	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;

	if (TangentsArray.Num() != VerticesArray.Num() || NormalsArray.Num() != VerticesArray.Num())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error tangents or normals"));
	}

	if (VerticesArray.Num() != 0)
	{
		if (mesh->DoesSectionExist(0))
		{
			mesh->UpdateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, TangentsArray, ESectionUpdateFlags::MoveArrays);
		}
		else
		{
			mesh->CreateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, TangentsArray, bCreateCollision, EUpdateFrequency::Frequent);
		}
	}
	bCollisionDirty = true;
}

void AVoxelChunk::Unload()
{
	if (this->IsValidLowLevel())
	{
		if (!this->IsPendingKill())
		{
			if (!this->CollisionChunk->IsPendingKill())
			{
				this->CollisionChunk->Destroy();
			}
			this->Destroy();
		}
	}
}


void AVoxelChunk::Polygonise(int x, int y, int z)
{
	int Step = 1 << Depth;
	signed char Corner[8] = {
		GetValue(x       * Step, y       * Step, z       * Step),
		GetValue((x + 1) * Step, y       * Step, z       * Step),
		GetValue(x       * Step, (y + 1) * Step, z       * Step),
		GetValue((x + 1) * Step, (y + 1) * Step, z       * Step),
		GetValue(x       * Step, y       * Step, (z + 1) * Step),
		GetValue((x + 1) * Step, y       * Step, (z + 1) * Step),
		GetValue(x       * Step, (y + 1) * Step, (z + 1) * Step),
		GetValue((x + 1) * Step, (y + 1) * Step, (z + 1) * Step)
	};

	FVector Positions[8] = {
		Step * FVector(x    , y    , z),
		Step * FVector(x + 1, y    , z),
		Step * FVector(x    , y + 1, z),
		Step * FVector(x + 1, y + 1, z),
		Step * FVector(x    , y    , z + 1),
		Step * FVector(x + 1, y    , z + 1),
		Step * FVector(x    , y + 1, z + 1),
		Step * FVector(x + 1, y + 1, z + 1)
	};

	unsigned long CaseCode = ((Corner[0] >> 7) & 0x01)
		| ((Corner[1] >> 6) & 0x02)
		| ((Corner[2] >> 5) & 0x04)
		| ((Corner[3] >> 4) & 0x08)
		| ((Corner[4] >> 3) & 0x10)
		| ((Corner[5] >> 2) & 0x20)
		| ((Corner[6] >> 1) & 0x40)
		| (Corner[7] & 0x80);


	if ((CaseCode ^ ((Corner[7] >> 7) & 0xFF)) != 0)
	{
		// Cell has a nontrivial triangulation
		unsigned char CellClass = regularCellClass[CaseCode];
		RegularCellData CellData = regularCellData[CellClass];
		const unsigned short* VertexData = regularVertexData[CaseCode];
		// Check if precedent cell exist
		short ValidityMask = (x == -1 ? 0 : 1) + (y == -1 ? 0 : 2) + (z == -1 ? 0 : 4);

		std::vector<int> VertexIndices(CellData.GetVertexCount());

		auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
		auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

		for (int i = 0; i < CellData.GetVertexCount(); i++)
		{
			int VerticeIndex;
			unsigned short EdgeCode = VertexData[i];

			// A: low point / B: high point
			unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
			unsigned short IndexVerticeB = EdgeCode & 0x0F;

			signed char ValueAtA = Corner[IndexVerticeA];
			signed char ValueAtB = Corner[IndexVerticeB];

			FVector PositionA = Positions[IndexVerticeA];
			FVector PositionB = Positions[IndexVerticeB];

			// Index of vertex on a generic cube (0, 1, 2 or 3)
			short EdgeIndex = (EdgeCode >> 8) & 0x0F;
			// Direction to go to use an already created vertex
			short Direction = EdgeCode >> 12;

			if (ValueAtB == 0)
			{
				// Vertex lies at the higher-numbered endpoint
				if ((IndexVerticeB == 7) || ((ValidityMask & Direction) != Direction))
				{
					// Vertex failed validity check (needs to be created, but not cached)
					Vertices.push_front(PositionB);
					VerticeIndex = VerticesCount;
					VerticesCount++;
				}
				else
				{
					// Vertex already created
					bool xIsDifferent = Direction & 0x01;
					bool yIsDifferent = Direction & 0x02;
					bool zIsDifferent = Direction & 0x04;
					VerticeIndex = (zIsDifferent ? OldCache : NewCache)[1 + x - (xIsDifferent ? 1 : 0)][1 + y - (yIsDifferent ? 1 : 0)][EdgeIndex];
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((ValidityMask & Direction) != Direction)
				{
					// Validity check failed
					Vertices.push_front(PositionA);
					VerticeIndex = VerticesCount;
					VerticesCount++;
				}
				else
				{
					// Reuse vertex
					bool xIsDifferent = Direction & 0x01;
					bool yIsDifferent = Direction & 0x02;
					bool zIsDifferent = Direction & 0x04;
					VerticeIndex = (zIsDifferent ? OldCache : NewCache)[1 + x - (xIsDifferent ? 1 : 0)][1 + y - (yIsDifferent ? 1 : 0)][EdgeIndex];
				}
			}
			else
			{
				// Vertex lies in the interior of the edge
				if ((ValidityMask & Direction) != Direction)
				{
					// Validity check failed
					FVector Q;
					if (Step == 1)
					{
						// Full resolution
						float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
						Q = t * PositionA + (1 - t) * PositionB;
					}
					else
					{
						// Cube vertex are counted in binary order, so
						// deltaX = index % 2
						// deltaY = (index // 2) % 2
						// deltaZ = (index // 4) % 2

						int ADeltaX = IndexVerticeA % 2;
						int ADeltaY = (IndexVerticeA / 2) % 2;
						int ADeltaZ = IndexVerticeA / 4;

						if (EdgeIndex == 2)
						{
							// Edge along x axis
							int BDeltaX = IndexVerticeB % 2;
							Q = InterpolateX(Step * (x + ADeltaX), Step * (x + BDeltaX), Step * (y + ADeltaY), Step * (z + ADeltaZ));
						}
						else if (EdgeIndex == 1)
						{
							// Edge along y axis
							int BDeltaY = (IndexVerticeB / 2) % 2;
							Q = InterpolateY(Step * (x + ADeltaX), Step * (y + ADeltaY), Step * (y + BDeltaY), Step * (z + ADeltaZ));
						}
						else if (EdgeIndex == 3)
						{
							// Edge along z axis
							int BDeltaZ = IndexVerticeB / 4;
							Q = InterpolateZ(Step * (x + ADeltaX), Step * (y + ADeltaY), Step * (z + ADeltaZ), Step * (z + BDeltaZ));
						}
						else
						{
							checkf(false, TEXT("Error in interpolation: case should not exist"));
						}
					}
					Vertices.push_front(Q);
					VerticeIndex = VerticesCount;
					VerticesCount++;
				}
				else
				{
					// Reuse vertex
					bool xIsDifferent = Direction & 0x01;
					bool yIsDifferent = Direction & 0x02;
					bool zIsDifferent = Direction & 0x04;
					VerticeIndex = (zIsDifferent ? OldCache : NewCache)[1 + x - (xIsDifferent ? 1 : 0)][1 + y - (yIsDifferent ? 1 : 0)][EdgeIndex];
				}
			}

			// If own vertex, save it
			if (Direction & 0x08)
			{
				NewCache[1 + x][1 + y][EdgeIndex] = VerticeIndex;
			}
			VertexIndices[i] = VerticeIndex;

		}

		// Add triangles
		bool UseNormalsTriangles = (x == -1 || y == -1 || z == -1 || x == 16 || y == 16 || z == 16);
		for (int i = 0; i < 3 * CellData.GetTriangleCount(); i++)
		{
			(UseNormalsTriangles ? NormalsTriangles : Triangles).push_front(VertexIndices[CellData.vertexIndex[i]]);
		}
		if (!UseNormalsTriangles)
		{
			TrianglesCount += 3 * CellData.GetTriangleCount();
		}
	}
}

char AVoxelChunk::GetValue(int x, int y, int z)
{
	return World->GetValue(Position + FIntVector(x, y, z));
}

FVector AVoxelChunk::InterpolateX(int xMin, int xMax, int y, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = GetValue(xMin, y, z);
	signed char ValueAtB = GetValue(xMax, y, z);
	if (xMax - xMin == 1)
	{
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(xMin, y, z) + (1 - t) *  FVector(xMax, y, z);
	}
	else
	{
		check((xMax + xMin) % 2 == 0);

		int xMiddle = (xMax + xMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (GetValue(xMiddle, y, z) & 0x80))
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

FVector AVoxelChunk::InterpolateY(int x, int yMin, int yMax, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = GetValue(x, yMin, z);
	signed char ValueAtB = GetValue(x, yMax, z);
	if (yMax - yMin == 1)
	{
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(x, yMin, z) + (1 - t) *  FVector(x, yMax, z);
	}
	else
	{
		check((yMax + yMin) % 2 == 0);

		int yMiddle = (yMax + yMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (GetValue(x, yMiddle, z) & 0x80))
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

FVector AVoxelChunk::InterpolateZ(int x, int y, int zMin, int zMax)
{
	// A: Min / B: Max
	signed char ValueAtA = GetValue(x, y, zMin);
	signed char ValueAtB = GetValue(x, y, zMax);
	if (zMax - zMin == 1)
	{
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(x, y, zMin) + (1 - t) *  FVector(x, y, zMax);
	}
	else
	{
		check((zMax + zMin) % 2 == 0);

		int zMiddle = (zMax + zMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (GetValue(x, y, zMiddle) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateZ(x, y, zMiddle, zMax);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateZ(x, y, zMin, zMiddle);
		}
	}
}