// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelThread.h"
#include <forward_list>
#include "TransvoxelTools.h"
#include "VoxelChunk.h"

VoxelThread::VoxelThread(AVoxelChunk* voxelChunk) : VoxelChunk(voxelChunk)
{
}

VoxelThread::~VoxelThread()
{
}

void VoxelThread::DoWork()
{

	std::forward_list<FVector> Vertices;
	std::forward_list<FColor> VertexColors;
	std::forward_list<int> Triangles;
	std::forward_list<VertexProperties> VerticesProperties;

	int VerticesCount = 0;
	int TrianglesCount = 0;

	/*if (XMinChunkHasHigherRes)
	{
	XMinChunk->Update();
	}
	if (XMaxChunkHasHigherRes)
	{
	XMaxChunk->Update();
	}
	if (YMinChunkHasHigherRes)
	{
	YMinChunk->Update();
	}
	if (YMaxChunkHasHigherRes)
	{
	YMaxChunk->Update();
	}
	if (ZMinChunkHasHigherRes)
	{
	ZMinChunk->Update();
	}
	if (ZMaxChunkHasHigherRes)
	{
	ZMaxChunk->Update();
	}*/

	/**
	* Polygonize
	*/
	for (int z = -1; z < 17; z++)
	{
		for (int y = -1; y < 17; y++)
		{
			for (int x = -1; x < 17; x++)
			{
				short validityMask = (x == -1 ? 0 : 1) + (y == -1 ? 0 : 2) + (z == -1 ? 0 : 4);
				TransvoxelTools::RegularPolygonize(VoxelChunk, x, y, z, validityMask, Triangles, TrianglesCount, Vertices, VerticesProperties, VertexColors, VerticesCount);
			}
		}
		VoxelChunk->NewCacheIs1 = !VoxelChunk->NewCacheIs1;
	}

	// Temporary arrays of all the vertices
	TArray<FVector> VerticesArray;
	TArray<VertexProperties> VerticesPropertiesArray;
	// Array to get the final vertex index knowing its index in VerticesArray
	TArray<int> BijectionArray;
	// Inverse of Bijection (final index -> index in VerticesArray)
	TArray<int> InverseBijectionArray;

	// Reserve memory
	VerticesArray.SetNumUninitialized(VerticesCount);
	BijectionArray.SetNumUninitialized(VerticesCount);
	InverseBijectionArray.SetNumUninitialized(VerticesCount);
	VerticesPropertiesArray.SetNumUninitialized(VerticesCount);

	// Fill arrays
	int cleanedIndex = 0;
	for (int i = VerticesCount - 1; i >= 0; i--)
	{
		FVector Vertex = Vertices.front();
		VertexProperties Properties = VerticesProperties.front();

		VerticesArray[i] = Vertex;
		VerticesPropertiesArray[i] = Properties;

		if (!Properties.IsNormalOnly)
		{
			// If real vertex
			InverseBijectionArray[cleanedIndex] = i;
			BijectionArray[i] = cleanedIndex;
			cleanedIndex++;
		}
		else
		{
			// If only for normals
			BijectionArray[i] = -1;
		}

		Vertices.pop_front();
		VerticesProperties.pop_front();
	}
	const int RealVerticesCount = cleanedIndex;

	if (RealVerticesCount != 0)
	{
		// Create Section
		FProcMeshSection& Section = VoxelChunk->Section;
		Section.Reset();
		Section.bEnableCollision = VoxelChunk->Depth == 0;
		Section.bSectionVisible = true;
		Section.ProcVertexBuffer.SetNumUninitialized(RealVerticesCount);
		Section.ProcIndexBuffer.SetNumUninitialized(TrianglesCount);

		// Init normals & tangents
		for (int i = 0; i < RealVerticesCount; i++)
		{
			Section.ProcVertexBuffer[i].Normal = FVector::ZeroVector;
			Section.ProcVertexBuffer[i].Tangent.TangentX = FVector::ZeroVector;
		}

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

			FVector A = VerticesArray[a];
			FVector B = VerticesArray[b];
			FVector C = VerticesArray[c];
			FVector N = FVector::CrossProduct(C - A, B - A).GetSafeNormal();

			// Add triangles
			if (ba != -1 && bb != -1 && bc != -1)
			{
				Section.ProcIndexBuffer[i] = ba;
				Section.ProcIndexBuffer[i + 1] = bb;
				Section.ProcIndexBuffer[i + 2] = bc;
				i += 3;
			}

			//TODO: better tangents
			if (ba != -1)
			{
				Section.ProcVertexBuffer[ba].Normal += N;
				Section.ProcVertexBuffer[ba].Tangent.TangentX += C - A;
			}
			if (bb != -1)
			{
				Section.ProcVertexBuffer[bb].Normal += N;
				Section.ProcVertexBuffer[bb].Tangent.TangentX += C - A;
			}
			if (bc != -1)
			{
				Section.ProcVertexBuffer[bc].Normal += N;
				Section.ProcVertexBuffer[bc].Tangent.TangentX += C - A;
			}
		}
		// Shrink triangles
		Section.ProcIndexBuffer.SetNumUninitialized(i);


		// Normalize normals + tangents & translate vertices on edges
		for (int i = 0; i < RealVerticesCount; i++)
		{
			Section.ProcVertexBuffer[i].Normal.Normalize();
			Section.ProcVertexBuffer[i].Tangent.TangentX.Normalize();
			int j = InverseBijectionArray[i];
			Section.ProcVertexBuffer[i].Position = VoxelChunk->GetTranslated(VerticesArray[j], Section.ProcVertexBuffer[i].Normal, VerticesPropertiesArray[j]);
			Section.SectionLocalBox += Section.ProcVertexBuffer[i].Position;
		}

		// Set vertex colors
		for (int i = VerticesCount - 1; i >= 0; i--)
		{
			int  j = BijectionArray[i];
			if (j != -1)
			{
				Section.ProcVertexBuffer[j].Color = VertexColors.front();
			}
			VertexColors.pop_front();
		}
	}
}