// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Transvoxel.h"
#include "DrawDebugHelpers.h"
#include "VoxelCollisionChunk.h"
#include "RuntimeMeshComponent.h"
#include "IntVectorExtension.h"
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
	check(world);

	Position = position;
	Depth = depth;
	World = world;

	FString name = FString::FromInt(position.X) + ", " + FString::FromInt(position.Y) + ", " + FString::FromInt(position.Z);
	FVector relativeLocation = (FVector)position;

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorLabel(name);
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);
	PrimaryMesh->SetMaterial(0, world->VoxelMaterial);
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

	int Step = 1 << Depth;
	XMinChunkHasHigherRes = HasChunkHigherRes(-Step, 0, 0);
	XMaxChunkHasHigherRes = HasChunkHigherRes(Step * 16, 0, 0);
	YMinChunkHasHigherRes = HasChunkHigherRes(0, -Step, 0);
	YMaxChunkHasHigherRes = HasChunkHigherRes(0, Step * 16, 0);
	ZMinChunkHasHigherRes = HasChunkHigherRes(0, 0, -Step);
	ZMaxChunkHasHigherRes = HasChunkHigherRes(0, 0, Step * 16);

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
	TArray<int> BijectionArray;
	TArray<int> InverseBijectionArray;
	TArray<VertexProperties> VerticesPropertiesArray;

	VerticesArray.SetNumUninitialized(VerticesCount);
	BijectionArray.SetNumUninitialized(VerticesCount);
	InverseBijectionArray.SetNumUninitialized(VerticesCount);
	VerticesPropertiesArray.SetNumUninitialized(VerticesCount);

	// Fill arrays
	int cleanedIndex = 0;
	for (int i = VerticesCount - 1; i >= 0; i--)
	{
		FVector Vertex = Vertices.front();
		auto Properties = VerticesProperties.front();

		VerticesArray[i] = Vertex;
		VerticesPropertiesArray[i] = Properties;
		if (!Properties.IsNormalOnly)
		{
			InverseBijectionArray[cleanedIndex] = i;
			BijectionArray[i] = cleanedIndex;
			cleanedIndex++;
		}
		else
		{
			BijectionArray[i] = -1;
		}

		Vertices.pop_front();
		VerticesProperties.pop_front();
	}
	const int RealVerticesCount = cleanedIndex;

	TArray<int> TrianglesArray;
	TArray<FVector> NormalsArray;
	TArray<FVector> TangentsArray;
	TrianglesArray.SetNumUninitialized(TrianglesCount);
	NormalsArray.Init(FVector::ZeroVector, RealVerticesCount);
	TangentsArray.Init(FVector::ZeroVector, RealVerticesCount);

	// Compute normals from real triangles & Add triangles
	int i = 0;
	for (auto it = Triangles.begin(); it != Triangles.end(); ++it)
	{
		int a = *it;
		int ba = BijectionArray[a];
		++it;
		int b = *it;
		int bb = BijectionArray[b];
		++it;
		int c = *it;
		int bc = BijectionArray[c];

		// Add triangles
		TrianglesArray[i] = ba;
		TrianglesArray[i + 1] = bb;
		TrianglesArray[i + 2] = bc;
		i += 3;

		// Add normals & tangents
		FVector A = VerticesArray[a];
		FVector B = VerticesArray[b];
		FVector C = VerticesArray[c];
		FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();
		NormalsArray[ba] += N;
		NormalsArray[bb] += N;
		NormalsArray[bc] += N;
		// TODO: better tangents
		TangentsArray[ba] += C - A;
		TangentsArray[bb] += C - A;
		TangentsArray[bc] += C - A;
	}

	// Triangles used only for normals
	for (auto it = NormalsTriangles.begin(); it != NormalsTriangles.end(); ++it)
	{
		int a = *it;
		int ba = BijectionArray[a];
		++it;
		int b = *it;
		int bb = BijectionArray[b];
		++it;
		int c = *it;
		int bc = BijectionArray[c];

		// Add normals & tangents
		FVector A = VerticesArray[a];
		FVector B = VerticesArray[b];
		FVector C = VerticesArray[c];
		FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();
		if (ba != -1)
		{
			NormalsArray[ba] += N;
			TangentsArray[ba] += C - A;
		}
		if (bb != -1)
		{
			NormalsArray[bb] += N;
			TangentsArray[bb] += C - A;
		}
		if (bc != -1)
		{
			NormalsArray[bc] += N;
			TangentsArray[bc] += C - A;
		}
	}

	// Normalize & convert to FRuntimeMeshTangent
	TArray<FRuntimeMeshTangent> RealTangentsArray;
	RealTangentsArray.SetNumUninitialized(RealVerticesCount);
	for (int i = 0; i < RealVerticesCount; i++)
	{
		RealTangentsArray[i] = FRuntimeMeshTangent(TangentsArray[i].GetSafeNormal());
		NormalsArray[i].Normalize();
	}

	// Compute final vertice array
	TArray<FVector> CleanedVerticesArray;
	CleanedVerticesArray.SetNumUninitialized(RealVerticesCount);
	for (int i = 0; i < RealVerticesCount; i++)
	{
		int j = InverseBijectionArray[i];
		CleanedVerticesArray[i] = GetTranslated(VerticesArray[j], NormalsArray[i], VerticesPropertiesArray[j]);
	}


	TArray<FVector2D> UV0;
	TArray<FColor> VertexColors;

	check(RealTangentsArray.Num() == CleanedVerticesArray.Num() && NormalsArray.Num() == CleanedVerticesArray.Num());

	if (VerticesArray.Num() != 0)
	{
		if (mesh->DoesSectionExist(0))
		{
			mesh->UpdateMeshSection(0, CleanedVerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, RealTangentsArray, ESectionUpdateFlags::MoveArrays);
		}
		else
		{
			mesh->CreateMeshSection(0, CleanedVerticesArray, TrianglesArray, NormalsArray, UV0, VertexColors, RealTangentsArray, bCreateCollision, EUpdateFrequency::Frequent);
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

int AVoxelChunk::AddVertex(FVector vertex, FIntVector exactPosition, bool xIsExact, bool yIsExact, bool zIsExact)
{
	int Step = 1 << Depth;
	VerticesProperties.push_front(VertexProperties({
		xIsExact && exactPosition.X == 0,
		xIsExact && exactPosition.X == 16 * Step,
		yIsExact && exactPosition.Y == 0,
		yIsExact && exactPosition.Y == 16 * Step,
		zIsExact && exactPosition.Z == 0,
		zIsExact && exactPosition.Z == 16 * Step,
		(xIsExact && (exactPosition.X == -1 * Step || exactPosition.X == 17 * Step)) ||
		(yIsExact && (exactPosition.Y == -1 * Step || exactPosition.Y == 17 * Step)) ||
		(zIsExact && (exactPosition.Z == -1 * Step || exactPosition.Z == 17 * Step))
	}));
	Vertices.push_front(vertex);
	VerticesCount++;
	return VerticesCount - 1;
}

int AVoxelChunk::LoadCachedVertex(int x, int y, int z, short direction, int edgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	bool zIsDifferent = direction & 0x04;
	return (zIsDifferent ? OldCache : NewCache)[1 + x - (xIsDifferent ? 1 : 0)][1 + y - (yIsDifferent ? 1 : 0)][edgeIndex];
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

	FIntVector Positions[8] = {
		Step * FIntVector(x    , y    , z),
		Step * FIntVector(x + 1, y    , z),
		Step * FIntVector(x    , y + 1, z),
		Step * FIntVector(x + 1, y + 1, z),
		Step * FIntVector(x    , y    , z + 1),
		Step * FIntVector(x + 1, y    , z + 1),
		Step * FIntVector(x    , y + 1, z + 1),
		Step * FIntVector(x + 1, y + 1, z + 1)
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
					VertexIndex = AddVertex((FVector)PositionB, PositionB);
				}
				else
				{
					// Vertex already created
					VertexIndex = LoadCachedVertex(x, y, z, Direction, EdgeIndex);
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((ValidityMask & Direction) != Direction)
				{
					// Validity check failed
					VertexIndex = AddVertex((FVector)PositionA, PositionA);
				}
				else
				{
					// Reuse vertex
					VertexIndex = LoadCachedVertex(x, y, z, Direction, EdgeIndex);
				}
			}
			else
			{
				// Vertex lies in the interior of the edge
				if ((ValidityMask & Direction) != Direction)
				{
					// Validity check failed
					if (Step == 1)
					{
						// Full resolution
						float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
						VertexIndex = AddVertex(t * (FVector)PositionA + (1 - t) *(FVector)PositionB, PositionA, EdgeIndex != 2, EdgeIndex != 1, EdgeIndex != 3);
					}
					else
					{
						FVector Q;
						if (EdgeIndex == 2)
						{
							// Edge along x axis
							Q = InterpolateX(PositionA.X, PositionB.X, PositionA.Y, PositionA.Z);
						}
						else if (EdgeIndex == 1)
						{
							// Edge along y axis
							Q = InterpolateY(PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z);
						}
						else if (EdgeIndex == 3)
						{
							// Edge along z axis
							Q = InterpolateZ(PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z);
						}
						else
						{
							checkf(false, TEXT("Error in interpolation: case should not exist"));
						}
						VertexIndex = AddVertex(Q, PositionA, EdgeIndex != 2, EdgeIndex != 1, EdgeIndex != 3);
					}
				}
				else
				{
					VertexIndex = LoadCachedVertex(x, y, z, Direction, EdgeIndex);
				}
			}

			// If own vertex, save it
			if (Direction & 0x08)
			{
				NewCache[1 + x][1 + y][EdgeIndex] = VertexIndex;
			}
			VertexIndices[i] = VertexIndex;

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

bool AVoxelChunk::HasChunkHigherRes(int x, int y, int z)
{
	FIntVector P = Position + FIntVector(x, y, z);
	if (World->IsInWorld(P))
	{
		return Depth > World->GetDepthAt(P);
	}
	else
	{
		return false;
	}
}

FVector AVoxelChunk::GetTranslated(FVector V, FVector N, VertexProperties P)
{
	// If an adjacent block is rendered at the same resolution, return primary position
	if ((P.IsNearXMin && !XMinChunkHasHigherRes) || (P.IsNearXMax && !XMaxChunkHasHigherRes) ||
		(P.IsNearYMin && !YMinChunkHasHigherRes) || (P.IsNearYMax && !YMaxChunkHasHigherRes) ||
		(P.IsNearZMin && !ZMinChunkHasHigherRes) || (P.IsNearZMax && !ZMaxChunkHasHigherRes))
	{
		return V;
	}


	float DeltaX = 0;
	float DeltaY = 0;
	float DeltaZ = 0;

	float TwoPowerK = 1 << Depth;
	float w = TwoPowerK / 4;

	if ((P.IsNearXMin && XMinChunkHasHigherRes) || (P.IsNearXMax && XMaxChunkHasHigherRes))
	{
		if (V.X < TwoPowerK)
		{
			DeltaX = (1 - V.X / TwoPowerK) * w;
		}
		else if (V.X > TwoPowerK * (16 - 1))
		{
			DeltaX = (16 - 1 - V.X / TwoPowerK) * w;
		}
	}
	if ((P.IsNearYMin && YMinChunkHasHigherRes) || (P.IsNearYMax && YMaxChunkHasHigherRes))
	{
		if (V.Y < TwoPowerK)
		{
			DeltaY = (1 - V.Y / TwoPowerK) * w;
		}
		else if (V.Y > TwoPowerK * (16 - 1))
		{
			DeltaY = (16 - 1 - V.Y / TwoPowerK) * w;
		}
	}
	if ((P.IsNearZMin && ZMinChunkHasHigherRes) || (P.IsNearZMax && ZMaxChunkHasHigherRes))
	{
		if (V.Z < TwoPowerK)
		{
			DeltaZ = (1 - V.Z / TwoPowerK) * w;
		}
		else if (V.Z > TwoPowerK * (16 - 1))
		{
			DeltaZ = (16 - 1 - V.Z / TwoPowerK) * w;
		}
	}

	FVector Q = FVector(
		(1 - N.X * N.X) * DeltaX - N.X * N.Y * DeltaY - N.X * N.Z * DeltaZ,
		-N.X * N.Y * DeltaX + (1 - N.Y * N.Y) * DeltaY - N.Y * N.Z * DeltaZ,
		-N.X * N.Z * DeltaX - N.Y * N.Z * DeltaY + (1 - N.Z * N.Z) * DeltaZ);
	return V + Q;
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