// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTransitionChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "Transvoxel.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "VoxelChunk.h"
#include <vector>

AVoxelTransitionChunk::AVoxelTransitionChunk()
{
	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;
}

void AVoxelTransitionChunk::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelTransitionChunk::Update()
{
	Equivalences.clear();
	VertexColors.clear();
	Vertices.clear();
	VerticesProperties.clear();
	TransitionTriangles.clear();
	RegularTriangles.clear();

	VerticesCount = 0;

	TransitionTrianglesCount = 0;
	RegularTrianglesCount = 0;

	/**
	 * Polygonize
	 */
	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			short validityMask = (x == 0 ? 0 : 1) + (y == 0 ? 0 : 2);
			TransvoxelTools::TransitionPolygonize(this, x, y, validityMask, TransitionTriangles, TransitionTrianglesCount, Vertices, VerticesProperties, VertexColors, VerticesCount);
		}
	}

	const int TransitionVerticesCount = VerticesCount;

	for (int z = -1; z < 1; z++)
	{
		for (int y = -1; y < 16; y++)
		{
			for (int x = -1; x < 16; x++)
			{
				short validityMask = (x == -1 ? 0 : 1) + (y == -1 ? 0 : 2) + (z == -1 ? 0 : 4);
				Props ref;
				Colors ref2;
				TransvoxelTools::RegularPolygonize(this, x, y, z, validityMask, RegularTriangles, RegularTrianglesCount, Vertices, ref, ref2, VerticesCount);
			}
		}
	}
	const int RegularVerticesCount = VerticesCount - TransitionVerticesCount;

	/**
	 * Compute equivalence
	 */
	TArray<int> EquivalenceArray;
	EquivalenceArray.SetNumUninitialized(VerticesCount);
	for (int i = 0; i < VerticesCount; i++)
	{
		EquivalenceArray[i] = i;
	}
	for (auto it = Equivalences.begin(); it != Equivalences.end(); ++it)
	{
		int to = *it;
		++it;
		int from = *it;
		EquivalenceArray[from] = to;
	}


	typedef std::tuple<int, VertexProperties> Translation;
	std::forward_list<Translation> TranslationStack;

	TArray<FVector> VerticesArray;
	VerticesArray.SetNumUninitialized(VerticesCount);

	// Add vertices for normals
	for (int i = 0; i < RegularVerticesCount; i++)
	{
		FVector Vertex = Vertices.front();
		Vertices.pop_front();
		VerticesArray[VerticesCount - 1 - i] = GetRealPosition(Vertex);
	}

	// Add real vertices
	int Width = 16 << Depth;
	for (int i = RegularVerticesCount; i < VerticesCount; i++)
	{
		FVector Vertex = Vertices.front();
		Vertices.pop_front();

		VertexProperties2D VP2D = VerticesProperties.front();
		VerticesProperties.pop_front();

		FVector RealPosition = GetRealPosition(Vertex);
		FBoolVector IsExact = GetRealIsExact(VP2D.IsXExact, VP2D.IsYExact, true);
		FIntVector RealExactPosition = GetRealPosition(VP2D.X, VP2D.Y, 0);

		if (VP2D.NeedTranslation)
		{
			VertexProperties VP = VertexProperties({
				IsExact.X && RealExactPosition.X == 0,
				IsExact.X && RealExactPosition.X == Width,
				IsExact.Y && RealExactPosition.Y == 0,
				IsExact.Y && RealExactPosition.Y == Width,
				IsExact.Z && RealExactPosition.Z == 0,
				IsExact.Z && RealExactPosition.Z == Width,
				false });
			TranslationStack.push_front(Translation(VerticesCount - 1 - i, VP));
		}
		VerticesArray[VerticesCount - 1 - i] = RealPosition;
	}

	TArray<int> TrianglesArray;
	TrianglesArray.SetNumUninitialized(TransitionTrianglesCount);
	TArray<FVector> NormalsArray;
	NormalsArray.SetNumZeroed(TransitionVerticesCount);
	TArray<FVector> TangentsArray;
	TangentsArray.SetNumZeroed(TransitionVerticesCount);

	// Add triangles
	int i = 0;
	for (auto it = TransitionTriangles.begin(); it != TransitionTriangles.end(); ++it)
	{
		TrianglesArray[i] = *it;
		i++;
	}

	// Compute normals
	for (auto it = RegularTriangles.begin(); it != RegularTriangles.end(); ++it)
	{
		int a = EquivalenceArray[*it];
		++it;
		int b = EquivalenceArray[*it];
		++it;
		int c = EquivalenceArray[*it];

		FVector A = VerticesArray[a];
		FVector B = VerticesArray[b];
		FVector C = VerticesArray[c];
		FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();
		//TODO: better tangents
		if (a < TransitionVerticesCount)
		{
			NormalsArray[a] += N;
			TangentsArray[a] += C - A;
		}
		if (b < TransitionVerticesCount)
		{
			NormalsArray[b] += N;
			TangentsArray[b] += C - A;
		}
		if (c < TransitionVerticesCount)
		{
			NormalsArray[c] += N;
			TangentsArray[c] += C - A;
		}
	}

	// Remove unused & translate vertices
	VerticesArray.SetNum(TransitionVerticesCount);
	for (auto it = TranslationStack.begin(); it != TranslationStack.end(); it++)
	{
		int i = std::get<0>(*it);
		VertexProperties VP = std::get<1>(*it);

		VerticesArray[i] = VoxelChunk->GetTranslated(VerticesArray[i], NormalsArray[i].GetSafeNormal(), VP);
	}

	// Compute additional normals
	for (auto it = TransitionTriangles.begin(); it != TransitionTriangles.end(); ++it)
	{
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;

		FVector A = VerticesArray[a];
		FVector B = VerticesArray[b];
		FVector C = VerticesArray[c];
		FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();
		//TODO: better tangents
		if (a < TransitionVerticesCount)
		{
			NormalsArray[a] += N;
			TangentsArray[a] += C - A;
		}
		if (b < TransitionVerticesCount)
		{
			NormalsArray[b] += N;
			TangentsArray[b] += C - A;
		}
		if (c < TransitionVerticesCount)
		{
			NormalsArray[c] += N;
			TangentsArray[c] += C - A;
		}
	}

	// Normalize & convert to FRuntimeMeshTangent
	TArray<FProcMeshTangent> RealTangentsArray;
	RealTangentsArray.SetNumUninitialized(VerticesCount);
	for (int i = 0; i < TransitionVerticesCount; i++)
	{
		RealTangentsArray[i] = FProcMeshTangent(TangentsArray[i].GetSafeNormal(), false);
		NormalsArray[i].Normalize();
	}

	// Vertex colors
	TArray<FColor> VertexColorsArray;
	VertexColorsArray.SetNumUninitialized(TransitionVerticesCount);
	for (int i = TransitionVerticesCount - 1; i >= 0; i--)
	{
		VertexColorsArray[i] = VertexColors.front();
		VertexColors.pop_front();
	}

	TArray<FVector2D> UV0;

	if (VerticesArray.Num() != 0)
	{
		/*if (PrimaryMesh->DoesSectionExist(0))
		{
			PrimaryMesh->UpdateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColorsArray, RealTangentsArray, ESectionUpdateFlags::MoveArrays);
		}
		else
		{*/
			PrimaryMesh->CreateMeshSection(0, VerticesArray, TrianglesArray, NormalsArray, UV0, VertexColorsArray, RealTangentsArray, false);
		//}
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
	return  World->GetValue(Position + GetRealPosition(x, y, 0));
}

FColor AVoxelTransitionChunk::GetColor(int x, int y)
{
	return  World->GetColor(Position + GetRealPosition(x, y, 0));
}

FVector AVoxelTransitionChunk::GetRealPosition(FVector vertex)
{
	int width = 16 << Depth;
	float x = vertex.X;
	float y = vertex.Y;
	float z = vertex.Z;
	switch (TransitionDirection)
	{
	case XMin:
		return FVector(z, x, y);
	case XMax:
		return FVector(width - z, width - x, y);
	case YMin:
		return FVector(width - x, z, y);
	case YMax:
		return FVector(width - x, width - z, width - y);
	case ZMin:
		return FVector(y, width - x, z);
	case ZMax:
		return FVector(width - y, width - x, width - z);
	default:
		check(false);
		return FVector(0, 0, 0);
	}
}

FIntVector AVoxelTransitionChunk::GetRealPosition(int x, int y, int z)
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

FIntVector AVoxelTransitionChunk::InverseGetRealPosition(int x, int y, int z)
{
	int width = 16 << Depth;
	switch (TransitionDirection)
	{
	case XMin:
		// return FIntVector(z, x, y);
		return FIntVector(y, z, x);
	case XMax:
		// return FIntVector(width - z, width - x, y);
		return FIntVector(width - y, z, width - x);
	case YMin:
		// return FIntVector(width - x, z, y);
		return FIntVector(width - x, z, y);
	case YMax:
		// return FIntVector(width - x, width - z, width - y);
		return FIntVector(width - x, width - z, width - y);
	case ZMin:
		// return FIntVector(y, width - x, z);
		return FIntVector(width - y, x, z);
	case ZMax:
		// return FIntVector(width - y, width - x, width - z);
		return FIntVector(width - y, width - x, width - z);
	default:
		check(false);
		return FIntVector::ZeroValue;
	}
}

FBoolVector AVoxelTransitionChunk::GetRealIsExact(bool xIsExact, bool yIsExact, bool zIsExact)
{
	int width = 16 << Depth;
	switch (TransitionDirection)
	{
	case XMin:
		return FBoolVector(zIsExact, xIsExact, yIsExact);
	case XMax:
		return FBoolVector(zIsExact, xIsExact, yIsExact);
	case YMin:
		return FBoolVector(xIsExact, zIsExact, yIsExact);
	case YMax:
		return FBoolVector(xIsExact, zIsExact, yIsExact);
	case ZMin:
		return FBoolVector(yIsExact, xIsExact, zIsExact);
	case ZMax:
		return FBoolVector(yIsExact, xIsExact, zIsExact);
	default:
		check(false);
		return FBoolVector(false, false, false);
	}
}

FBoolVector AVoxelTransitionChunk::InverseGetRealIsExact(bool x, bool y, bool z)
{
	int width = 16 << Depth;
	switch (TransitionDirection)
	{
	case XMin:
		// return FIntVector(z, x, y);
		return FBoolVector(y, z, x);
	case XMax:
		// return FIntVector(width - z, width - x, y);
		return FBoolVector(y, z, x);
	case YMin:
		// return FIntVector(width - x, z, y);
		return FBoolVector(x, z, y);
	case YMax:
		// return FIntVector(width - x, width - z, width - y);
		return FBoolVector(x, z, y);
	case ZMin:
		// return FIntVector(y, width - x, z);
		return FBoolVector(y, x, z);
	case ZMax:
		// return FIntVector(width - y, width - x, width - z);
		return FBoolVector(y, x, z);
	default:
		check(false);
		return FBoolVector(false, false, false);
	}
}



void AVoxelTransitionChunk::SaveVertex(int x, int y, short edgeIndex, int index)
{
	check(0 <= x && 0 <= y && x < 16 && y < 16);
	check(0 <= edgeIndex && edgeIndex < 10);
	CacheTransvoxel[x][y][edgeIndex] = index;
}

int AVoxelTransitionChunk::LoadVertex(int x, int y, short direction, short edgeIndex)
{
	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	check(0 <= x - xIsDifferent && x - xIsDifferent < 16);
	check(0 <= y - yIsDifferent && y - yIsDifferent < 16);
	check(0 <= edgeIndex && edgeIndex < 10);
	check(CacheTransvoxel[x - xIsDifferent][y - yIsDifferent][edgeIndex] >= 0);

	return CacheTransvoxel[x - xIsDifferent][y - yIsDifferent][edgeIndex];
}

int AVoxelTransitionChunk::GetDepth()
{
	return Depth;
}



signed char AVoxelTransitionChunk::GetValue(int x, int y, int z)
{
	return World->GetValue(Position + GetRealPosition(x, y, z));
}

FColor AVoxelTransitionChunk::GetColor(int x, int y, int z)
{
	return World->GetColor(Position + GetRealPosition(x, y, z));
}

void AVoxelTransitionChunk::SaveVertex(int x, int y, int z, short edgeIndex, int index)
{
	check(0 <= 1 + x && 1 + x < 17);
	check(0 <= 1 + y && 1 + y < 17);
	check(0 <= edgeIndex && edgeIndex < 4);

	if (edgeIndex != 3 && z == -1 && x >= 0 && y >= 0)
	{
		int i;
		switch (edgeIndex)
		{
		case 2:
			i = 8;
			break;
		case 1:
			i = 9;
			break;
		default:
			check(false);
		}
		Equivalences.push_front(index);
		Equivalences.push_front(CacheTransvoxel[x][y][i]);
	}
	CacheLowRes[1 + z][1 + x][1 + y][edgeIndex] = index;
}

int AVoxelTransitionChunk::LoadVertex(int x, int y, int z, short direction, short edgeIndex)
{
	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	bool zIsDifferent = direction & 0x04;

	check(0 <= 1 + x - xIsDifferent && 1 + x - xIsDifferent < 17);
	check(0 <= 1 + y - yIsDifferent && 1 + y - yIsDifferent < 17);
	check(0 <= edgeIndex && edgeIndex < 4);

	return CacheLowRes[1 + z - zIsDifferent][1 + x - xIsDifferent][1 + y - yIsDifferent][edgeIndex];
}

bool AVoxelTransitionChunk::IsNormalOnly(FVector vertex)
{
	return false;
}
