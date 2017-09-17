// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelMeshAsset.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "basetsd.h"

#define VOXELIZER_IMPLEMENTATION
//#define VOXELIZER_DEBUG // Only if assertions need to be checked
#include "voxelizer.h"


AVoxelMeshAsset::AVoxelMeshAsset() : ValueMultiplier(1), MaxResolution(10000)
{
	DebugLineBatch = CreateDefaultSubobject<ULineBatchComponent>(FName("LineBatch"));
};

void AVoxelMeshAsset::Import()
{
	DebugLineBatch->Flush();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(StaticMeshComponent->GetStaticMesh(), 0, 0, Vertices, Triangles, Normals, UVs, Tangents);


	vx_mesh_t* mesh;
	vx_mesh_t* result;

	mesh = vx_mesh_alloc(Vertices.Num(), Triangles.Num());

	for (int i = 0; i < Vertices.Num(); i++)
	{
		mesh->vertices[i] = { Vertices[i].X, Vertices[i].Y, Vertices[i].Z };
		mesh->normals[i] = { Normals[i].X, Normals[i].Y, Normals[i].Z };
	}

	for (int i = 0; i < Triangles.Num(); i++)
	{
		mesh->indices[i] = Triangles[i];
	}

	// Precision factor to reduce "holes" artifact
	float precision = 0.01;

	// Run voxelization
	const int VoxelSize = 10;

	result = vx_voxelize(mesh, VoxelSize, VoxelSize, VoxelSize, precision);

	int32 MinX = MAXINT32;
	int32 MinY = MAXINT32;
	int32 MinZ = MAXINT32;
	int32 MaxX = MININT32;
	int32 MaxY = MININT32;
	int32 MaxZ = MININT32;

	for (int i = 0; i < result->nvertices; i++)
	{
		vx_vertex V = result->vertices[i];

		int X = FMath::RoundToInt((V.x - VoxelSize / 2.f) / (float)VoxelSize);
		int Y = FMath::RoundToInt((V.y - VoxelSize / 2.f) / (float)VoxelSize);
		int Z = FMath::RoundToInt((V.z - VoxelSize / 2.f) / (float)VoxelSize);

		MinX = FMath::Min(MinX, X);
		MaxX = FMath::Max(MaxX, X);

		MinY = FMath::Min(MinY, Y);
		MaxY = FMath::Max(MaxY, Y);

		MinZ = FMath::Min(MinZ, Z);
		MaxZ = FMath::Max(MaxZ, Z);
	}

	Size.X = MaxX - MinX + 1;
	Size.Y = MaxY - MinY + 1;
	Size.Z = MaxY - MinZ + 1;
	IsInside.SetNumZeroed(Size.X * Size.Y * Size.Z);

	for (int i = 0; i < result->nvertices; i++)
	{
		vx_vertex V = result->vertices[i];

		int X = FMath::RoundToInt((V.x - VoxelSize / 2.f) / (float)VoxelSize);
		int Y = FMath::RoundToInt((V.y - VoxelSize / 2.f) / (float)VoxelSize);
		int Z = FMath::RoundToInt((V.z - VoxelSize / 2.f) / (float)VoxelSize);

		IsInside[(X - MinX) + Size.X * (Y - MinY) + Size.X * Size.Y * (Z - MinZ)] = true;
	}

	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			bool bHasBeenThroughFirstWall = false;
			bool bHasBeenThroughInside = false;
			bool bHasBeenThroughSecondWall = false;
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				if (IsInside[Index])
				{
					if (!bHasBeenThroughSecondWall)
					{
						if (!bHasBeenThroughFirstWall)
						{
							bHasBeenThroughFirstWall = true;
						}
						else if (bHasBeenThroughInside)
						{
							bHasBeenThroughSecondWall = true;
						}
					}
				}
				else
				{
					if (bHasBeenThroughFirstWall)
					{
						if (bHasBeenThroughSecondWall)
						{
							bHasBeenThroughFirstWall = false;
							bHasBeenThroughInside = false;
							bHasBeenThroughSecondWall = false;
						}
						else
						{
							bHasBeenThroughInside = true;
							IsInside[Index] = true;
						}
					}
				}
			}
		}
	}

	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				if (IsInside[Index])
				{
					DebugLineBatch->DrawPoint(FVector(X + MinX, Y + MinY, Z + MinZ) * VoxelSize, FColor::Red, 5, 0, 100);

					DebugLineBatch->DrawLine(FVector(X + MinX, Y + MinY, Z + MinZ) * VoxelSize, FVector(X + MinX + 1, Y + MinY, Z + MinZ) * VoxelSize, FColor::Green, 0, 0.1f, 100);
					DebugLineBatch->DrawLine(FVector(X + MinX, Y + MinY, Z + MinZ) * VoxelSize, FVector(X + MinX, Y + MinY + 1, Z + MinZ) * VoxelSize, FColor::Green, 0, 0.1f, 100);
					DebugLineBatch->DrawLine(FVector(X + MinX, Y + MinY, Z + MinZ) * VoxelSize, FVector(X + MinX, Y + MinY, Z + MinZ + 1) * VoxelSize, FColor::Green, 0, 0.1f, 100);
				}
			}
		}
	}

	vx_mesh_free(result);
	vx_mesh_free(mesh);

}

void AVoxelMeshAsset::ImportIntoWorld(AVoxelWorld* World, FIntVector Position, bool bAsync, bool bDebugPoints)
{
	/*check(World);
	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				FIntVector CurrentPosition = Position + FIntVector(X, Y, Z);

				if (bDebugPoints)
				{
					DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), 5, FColor::Red, false, 5);
				}

				if (IsInside[Index] || Values[Index] * World->GetValue(CurrentPosition) > 0)
				{
					World->SetValue(CurrentPosition, Values[Index] * ValueMultiplier);
					World->QueueUpdate(CurrentPosition, bAsync);
				}
			}
		}
	}*/
}
