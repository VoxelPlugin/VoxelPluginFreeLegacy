// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelThread.h"
#include <forward_list>
#include "TransvoxelTools.h"
#include "VoxelChunkStruct.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"

VoxelThread::VoxelThread(AVoxelChunk* voxelChunk) : VoxelChunk(voxelChunk), VoxelStruct(new VoxelChunkStruct(VoxelChunk))
{

}

VoxelThread::~VoxelThread()
{
	delete VoxelStruct;
}

void VoxelThread::DoWork()
{
	TSharedPtr<Verts> Vertices(new Verts());
	TSharedPtr<Colors> VertexColors(new Colors());
	TSharedPtr<Trigs> RegularTriangles(new Trigs());
	TSharedPtr<Trigs> TransitionTriangles(new Trigs());
	TSharedPtr<Props> VerticesProperties(new Props());
	TSharedPtr<Props2D> VerticesProperties2D(new Props2D());

	int VerticesCount = 0;
	int TrianglesCount = 0;

	const int Depth = VoxelChunk->GetDepth();
	const int Width = 16 << Depth;


	/*
	VerticesCount++;
	Vertices->push_front(FVector::UpVector * 100);
	VertexColors->push_front(FColor::Red);
	VerticesProperties->push_front(VertexProperties({ false, false, false, false, false, false, false }));

	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 18; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				VoxelStruct->Cache1[i][j][k] = 0;
				VoxelStruct->Cache2[i][j][k] = 0;
			}
		}
	}*/

	/**
	 * Transitions voxels
	 */
	if (Depth != 0)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				short validityMask = (x == 0 ? 0 : 1) + (y == 0 ? 0 : 2);
				for (int i = 0; i < 6; i++)
				{
					if (VoxelStruct->ChunkHasHigherRes[i])
					{
						TransitionDirection Direction = (TransitionDirection)i;
						VoxelStruct->CurrentDirection = Direction;
						TransvoxelTools::TransitionPolygonize(VoxelStruct, x, y, validityMask, *TransitionTriangles, TrianglesCount, *Vertices, *VerticesProperties2D, *VertexColors, VerticesCount, Direction);
					}
				}
			}
		}
	}

	/**
	 * Normal voxels
	 */
	for (int z = -1; z < 17; z++)
	{
		for (int y = -1; y < 17; y++)
		{
			for (int x = -1; x < 17; x++)
			{
				short validityMask = (x == -1 ? 0 : 1) + (y == -1 ? 0 : 2) + (z == -1 ? 0 : 4);
				TransvoxelTools::RegularPolygonize(VoxelStruct, x, y, z, validityMask, *RegularTriangles, TrianglesCount, *Vertices, *VerticesProperties, *VertexColors, VerticesCount);
			}
		}
		VoxelStruct->NewCacheIs1 = !VoxelStruct->NewCacheIs1;
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
	int i = 0;
	// Regular voxels
	while (!VerticesProperties->empty())
	{
		FVector Vertex = Vertices->front();
		VertexProperties Properties = VerticesProperties->front();

		int index = VerticesCount - 1 - i;
		VerticesArray[index] = Vertex;
		VerticesPropertiesArray[index] = Properties;

		if (!Properties.IsNormalOnly)
		{
			// If real vertex
			InverseBijectionArray[cleanedIndex] = index;
			BijectionArray[index] = cleanedIndex;
			cleanedIndex++;
		}
		else
		{
			// If only for normals
			BijectionArray[index] = -1;
		}

		Vertices->pop_front();
		VerticesProperties->pop_front();

		i++;
	}
	if (Depth != 0)
	{
		// Transition voxels
		while (!VerticesProperties2D->empty())
		{
			FVector Vertex = Vertices->front();
			VertexProperties2D Properties2D = VerticesProperties2D->front();

			int index = VerticesCount - 1 - i;

			FVector RealPosition = VoxelStruct->TransformPosition(Vertex, Properties2D.Direction);

			VertexProperties Properties;
			if (Properties2D.NeedTranslation)
			{
				FBoolVector IsExact = VoxelStruct->TransformPosition(FBoolVector(Properties2D.IsXExact, Properties2D.IsYExact, true), Properties2D.Direction);
				FIntVector RealExactPosition = VoxelStruct->TransformPosition(Properties2D.X, Properties2D.Y, 0, Properties2D.Direction);
				Properties = VertexProperties({
					IsExact.X && RealExactPosition.X == 0,
					IsExact.X && RealExactPosition.X == Width,
					IsExact.Y && RealExactPosition.Y == 0,
					IsExact.Y && RealExactPosition.Y == Width,
					IsExact.Z && RealExactPosition.Z == 0,
					IsExact.Z && RealExactPosition.Z == Width,
					false });
			}
			else
			{
				Properties = VertexProperties({
					false,
					false,
					false,
					false,
					false,
					false,
					false });
			}

			VerticesArray[index] = RealPosition;
			VerticesPropertiesArray[index] = Properties;

			InverseBijectionArray[cleanedIndex] = index;
			BijectionArray[index] = cleanedIndex;
			cleanedIndex++;

			Vertices->pop_front();
			VerticesProperties2D->pop_front();

			i++;
		}
	}
	const int RealVerticesCount = cleanedIndex;

	if (RealVerticesCount != 0)
	{
		if (Depth != 0)
		{
			// Update bijections arrays with equivalence list
			for (auto it = VoxelStruct->EquivalenceList.begin(); it != VoxelStruct->EquivalenceList.end(); ++it)
			{
				int from = *it;
				++it;
				int to = *it;

				int i = BijectionArray[to];
				check(i != -1);
				BijectionArray[from] = i;
				InverseBijectionArray[i] = from;
			}
		}

		// Create Section
		FProcMeshSection& Section = VoxelChunk->Section;
		Section.Reset();
		Section.bEnableCollision = (Depth == 0);
		Section.bSectionVisible = true;
		Section.ProcVertexBuffer.SetNumUninitialized(RealVerticesCount);
		Section.ProcIndexBuffer.SetNumUninitialized(TrianglesCount);

		// Init normals & tangents
		for (int i = 0; i < RealVerticesCount; i++)
		{
			Section.ProcVertexBuffer[i].Normal = FVector::ZeroVector;
			Section.ProcVertexBuffer[i].Tangent.TangentX = FVector::ZeroVector;
		}

		// Add regular triangles & compute normals from them
		int RealTrianglesIndex = 0;
		for (auto it = RegularTriangles->begin(); it != RegularTriangles->end(); ++it)
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
				Section.ProcIndexBuffer[RealTrianglesIndex] = ba;
				Section.ProcIndexBuffer[RealTrianglesIndex + 1] = bb;
				Section.ProcIndexBuffer[RealTrianglesIndex + 2] = bc;
				RealTrianglesIndex += 3;
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

		if (Depth != 0)
		{
			// Copy normals & tangents from equivalence list to transition vertex
			for (auto it = VoxelStruct->NormalsEquivalenceList.begin(); it != VoxelStruct->NormalsEquivalenceList.end(); ++it)
			{
				int from = *it;
				++it;
				int to = *it;

				int fromIndex = BijectionArray[from];
				int toIndex = BijectionArray[to];
				check(fromIndex != -1);
				check(toIndex != -1);

				// We want from to became to: copy normals of to to from
				Section.ProcVertexBuffer[fromIndex].Normal = Section.ProcVertexBuffer[toIndex].Normal;
				Section.ProcVertexBuffer[fromIndex].Tangent = Section.ProcVertexBuffer[toIndex].Tangent;
			}

			// Add transitions triangles & compute normals from them
			for (auto it = TransitionTriangles->begin(); it != TransitionTriangles->end(); ++it)
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
				if (ba != -1 && bb != -1 && bc != -1)
				{
					Section.ProcIndexBuffer[RealTrianglesIndex] = ba;
					Section.ProcIndexBuffer[RealTrianglesIndex + 1] = bb;
					Section.ProcIndexBuffer[RealTrianglesIndex + 2] = bc;
					RealTrianglesIndex += 3;
				}

				// Copy normals & tangents
				FVector N = FVector::ZeroVector;
				FProcMeshTangent T;

				if (ba != -1 && Section.ProcVertexBuffer[ba].Normal != FVector::ZeroVector)
				{
					N = Section.ProcVertexBuffer[ba].Normal;
					T = Section.ProcVertexBuffer[ba].Tangent;
				}
				else if (bb != -1 && Section.ProcVertexBuffer[bb].Normal != FVector::ZeroVector)
				{
					N = Section.ProcVertexBuffer[bb].Normal;
					T = Section.ProcVertexBuffer[bb].Tangent;
				}
				else if (bc != -1 && Section.ProcVertexBuffer[bc].Normal != FVector::ZeroVector)
				{
					N = Section.ProcVertexBuffer[bc].Normal;
					T = Section.ProcVertexBuffer[bc].Tangent;
				}

				if (ba != -1)
				{
					Section.ProcVertexBuffer[ba].Normal = N;
					Section.ProcVertexBuffer[ba].Tangent = T;
				}
				if (bb != -1)
				{
					Section.ProcVertexBuffer[bb].Normal = N;
					Section.ProcVertexBuffer[bb].Tangent = T;
				}
				if (bc != -1)
				{
					Section.ProcVertexBuffer[bc].Normal = N;
					Section.ProcVertexBuffer[bc].Tangent = T;
				}
			}
		}
		// Shrink triangles
		Section.ProcIndexBuffer.SetNumUninitialized(RealTrianglesIndex);


		// Normalize normals + tangents & translate vertices on edges
		for (int i = 0; i < RealVerticesCount; i++)
		{
			Section.ProcVertexBuffer[i].Normal.Normalize();
			Section.ProcVertexBuffer[i].Tangent.TangentX.Normalize();
			int j = InverseBijectionArray[i];
			Section.ProcVertexBuffer[i].Position = TransvoxelTools::GetTranslated(VerticesArray[j], Section.ProcVertexBuffer[i].Normal, VerticesPropertiesArray[j], VoxelStruct->ChunkHasHigherRes, Depth);
			Section.SectionLocalBox += Section.ProcVertexBuffer[i].Position;
		}

		// Set vertex colors
		for (int i = VerticesCount - 1; i >= 0; i--)
		{
			int j = BijectionArray[i];
			if (j != -1)
			{
				Section.ProcVertexBuffer[j].Color = VertexColors->front();
			}
			VertexColors->pop_front();
		}
	}
}