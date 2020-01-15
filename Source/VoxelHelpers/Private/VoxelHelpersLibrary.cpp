// Copyright 2020 Phyronnaz

#include "VoxelHelpersLibrary.h"
#include "ProceduralMeshComponent.h"

void UVoxelHelpersLibrary::CreateProcMeshPlane(UProceduralMeshComponent* Mesh, int32 SizeX, int32 SizeY, float Step)
{
	if (!Mesh)
	{
		return;
	}
	
	TArray<int32> Indices;
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<FVector2D> TextureCoordinates;
	
	const int32 NumVertices = (SizeX + 1) * (SizeY + 1);
	
	Positions.Reserve(NumVertices);
	Normals.Reserve(NumVertices);
	TextureCoordinates.Reserve(NumVertices);
	
	Indices.Reserve(SizeX * SizeY * 6);
	
	for (int32 X = 0; X <= SizeX; X++)
	{
		for (int32 Y = 0; Y <= SizeY; Y++)
		{
			Positions.Add(FVector(X, Y, 0) * Step);
			Normals.Add(FVector(0, 0, 1));
			TextureCoordinates.Add(FVector2D(X / float(SizeX), Y / float(SizeY)));

			if (X < SizeX && Y < SizeY)
			{
				const auto Index = [&](int32 U, int32 V) { return V + (SizeY + 1) * U; };
				const int32 A = Index(X + 0, Y + 0);
				const int32 B = Index(X + 1, Y + 0);
				const int32 C = Index(X + 0, Y + 1);
				const int32 D = Index(X + 1, Y + 1);
				
				Indices.Add(C);
				Indices.Add(D);
				Indices.Add(A);

				Indices.Add(D);
				Indices.Add(B);
				Indices.Add(A);
			}
		}
	}

	Mesh->CreateMeshSection(0, Positions, Indices, Normals, TextureCoordinates, {}, {}, false);
}