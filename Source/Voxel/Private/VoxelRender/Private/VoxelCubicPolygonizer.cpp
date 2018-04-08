// Copyright 2018 Phyronnaz

#include "VoxelCubicPolygonizer.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "VoxelMaterial.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelCubicPolygonizer::CreateSection"), STAT_FVoxelCubicPolygonizer_CreateSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelCubicPolygonizer::CreateSection.BeginGet"), STAT_FVoxelCubicPolygonizer_CreateSection_BeginGet, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelCubicPolygonizer::CreateSection.Cache.GetValuesAndMaterials"), STAT_FVoxelCubicPolygonizer_CreateSection_Cache_GetValuesAndMaterials, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelCubicPolygonizer::CreateSection.Iter"), STAT_FVoxelCubicPolygonizer_CreateSection_Iter, STATGROUP_Voxel);

FVoxelCubicPolygonizer::FVoxelCubicPolygonizer(FVoxelData* Data, const FIntVector& ChunkPosition)
	: Data(Data)
	, ChunkPosition(ChunkPosition)
{
}

bool FVoxelCubicPolygonizer::CreateSection(FVoxelProcMeshSection& OutSection)
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelCubicPolygonizer_CreateSection, VOXEL_MULTITHREAD_STAT);

	OutSection.Reset();
	OutSection.bEnableCollision = true;
	OutSection.bSectionVisible = true;
	OutSection.SectionLocalBox = FBox(-FVector::OneVector, FVector::OneVector * (CHUNK_SIZE + 1));

	FIntVector ChunkDataSize((CHUNK_SIZE + 2), (CHUNK_SIZE + 2), (CHUNK_SIZE + 2));
	FIntBox Bounds(ChunkPosition - FIntVector(1, 1, 1), ChunkPosition - FIntVector(1, 1, 1) + ChunkDataSize);

	TArray<uint64> Octrees;
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelCubicPolygonizer_CreateSection_BeginGet, VOXEL_MULTITHREAD_STAT);
		Octrees = Data->BeginGet(Bounds);
	}

	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelCubicPolygonizer_CreateSection_Cache_GetValuesAndMaterials, VOXEL_MULTITHREAD_STAT);

		Data->GetValuesAndMaterials(CachedValues, CachedMaterials, ChunkPosition - FIntVector(1, 1, 1), FIntVector::ZeroValue, 1, ChunkDataSize, ChunkDataSize);
	}

	Data->EndGet(Octrees);

	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelCubicPolygonizer_CreateSection_Iter, VOXEL_MULTITHREAD_STAT);

		for (int X = 0; X < CHUNK_SIZE; X++)
		{
			for (int Y = 0; Y < CHUNK_SIZE; Y++)
			{
				for (int Z = 0; Z < CHUNK_SIZE; Z++)
				{
					const float Value = GetValue(X, Y, Z);

					if (Value <= 0)
					{
						if (GetValue(X - 1, Y, Z) > 0)
						{
							AddFace(X, Y, Z, XMin, OutSection);
						}
						if (GetValue(X + 1, Y, Z) > 0)
						{
							AddFace(X, Y, Z, XMax, OutSection);
						}
						if (GetValue(X, Y - 1, Z) > 0)
						{
							AddFace(X, Y, Z, YMin, OutSection);
						}
						if (GetValue(X, Y + 1, Z) > 0)
						{
							AddFace(X, Y, Z, YMax, OutSection);
						}
						if (GetValue(X, Y, Z - 1) > 0)
						{
							AddFace(X, Y, Z, ZMin, OutSection);
						}
						if (GetValue(X, Y, Z + 1) > 0)
						{
							AddFace(X, Y, Z, ZMax, OutSection);
						}
					}
				}
			}
		}
	}

	if (OutSection.ProcVertexBuffer.Num() < 3 || OutSection.ProcIndexBuffer.Num() == 0)
	{
		// Else physics thread crash
		OutSection.Reset();
	}

	return true;
}

float FVoxelCubicPolygonizer::GetValue(int X, int Y, int Z) const
{
	check(IsInBounds(X, Y, Z));
	return CachedValues[(X + 1) + (Y + 1) * (CHUNK_SIZE + 2) + (Z + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];
}

FVoxelMaterial FVoxelCubicPolygonizer::GetMaterial(int X, int Y, int Z) const
{
	check(IsInBounds(X, Y, Z));
	return CachedMaterials[(X + 1) + (Y + 1) * (CHUNK_SIZE + 2) + (Z + 1) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];
}

bool FVoxelCubicPolygonizer::IsInBounds(int X, int Y, int Z) const
{
	return -1 <= X && -1 <= Y && -1 <= Z && X < (CHUNK_SIZE + 2) && Y < (CHUNK_SIZE + 2) && Z < (CHUNK_SIZE + 2);
}

void FVoxelCubicPolygonizer::AddFace(int X, int Y, int Z, EVoxelDirection Direction, FVoxelProcMeshSection& Section)
{
	TArray<FVoxelProcMeshVertex>& Vertices = Section.ProcVertexBuffer;
	TArray<int32>& Indices = Section.ProcIndexBuffer;

	FVector AP, BP, CP, DP;
	bool bReversed;

	switch (Direction)
	{
	case XMin:
		AP = FVector(-0.5, -0.5, 0.5);
		BP = FVector(-0.5, 0.5, 0.5);
		CP = FVector(-0.5, 0.5, -0.5);
		DP = FVector(-0.5, -0.5, -0.5);
		bReversed = false;
		break;
	case XMax:
		AP = FVector(0.5, -0.5, 0.5);
		BP = FVector(0.5, 0.5, 0.5);
		CP = FVector(0.5, 0.5, -0.5);
		DP = FVector(0.5, -0.5, -0.5);
		bReversed = true;
		break;
	case YMin:
		AP = FVector(0.5, -0.5, -0.5);
		BP = FVector(0.5, -0.5, 0.5);
		CP = FVector(-0.5, -0.5, 0.5);
		DP = FVector(-0.5, -0.5, -0.5);
		bReversed = false;
		break;
	case YMax:
		AP = FVector(0.5, 0.5, -0.5);
		BP = FVector(0.5, 0.5, 0.5);
		CP = FVector(-0.5, 0.5, 0.5);
		DP = FVector(-0.5, 0.5, -0.5);
		bReversed = true;
		break;
	case ZMin:
		AP = FVector(-0.5, 0.5, -0.5);
		BP = FVector(0.5, 0.5, -0.5);
		CP = FVector(0.5, -0.5, -0.5);
		DP = FVector(-0.5, -0.5, -0.5);
		bReversed = false;
		break;
	case ZMax:
		AP = FVector(-0.5, 0.5, 0.5);
		BP = FVector(0.5, 0.5, 0.5);
		CP = FVector(0.5, -0.5, 0.5);
		DP = FVector(-0.5, -0.5, 0.5);
		bReversed = true;
		break;
	default:
		AP = FVector::ZeroVector;
		BP = FVector::ZeroVector;
		CP = FVector::ZeroVector;
		DP = FVector::ZeroVector;
		check(false);
	}

	FVector P(X, Y, Z);
	FColor Color = GetMaterial(X, Y, Z).ToFColor();

	int A = Vertices.Num();
	{
		FVoxelProcMeshVertex AV;
		AV.Color = Color;
		AV.Position = AP + P;
		Vertices.Add(AV);
	}

	int B = Vertices.Num();
	{
		FVoxelProcMeshVertex BV;
		BV.Color = Color;
		BV.Position = BP + P;
		Vertices.Add(BV);
	}

	int C = Vertices.Num();
	{
		FVoxelProcMeshVertex CV;
		CV.Color = Color;
		CV.Position = CP + P;
		Vertices.Add(CV);
	}

	int D = Vertices.Num();
	{
		FVoxelProcMeshVertex DV;
		DV.Color = Color;
		DV.Position = DP + P;
		Vertices.Add(DV);
	}


	if (!bReversed)
	{
		Indices.Add(D);
		Indices.Add(B);
		Indices.Add(A);
		
		Indices.Add(D);
		Indices.Add(C);
		Indices.Add(B);
	}
	else
	{
		Indices.Add(A);
		Indices.Add(B);
		Indices.Add(D);

		Indices.Add(B);
		Indices.Add(C);
		Indices.Add(D);
	}
}
