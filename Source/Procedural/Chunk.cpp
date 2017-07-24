#include "Chunk.h"
#include "World.h"
#include <cmath>
#include <algorithm>
#include "AssertionMacros.h"
#include "Transvoxel.cpp"
#include "EngineGlobals.h"
#include "Engine.h"

Chunk::Chunk(World* w, int posX, int posY, int posZ) : world(w), X(posX), Y(posY), Z(posZ)
{

}




void Chunk::Process()
{
	/**
	 * Initialize
	 */
	vertices.clear();
	triangles.clear();
	verticesCount = 0;
	trianglesCount = 0;

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
		newCacheIs1 = !newCacheIs1;
	}

	/**
	 * Compute normals + tangents & final arrays
	 */
	Vertices.SetNumUninitialized(verticesCount);
	Normals.SetNum(verticesCount);
	Triangles.SetNumUninitialized(trianglesCount);

	Tangents.SetNumUninitialized(verticesCount);
	tangents.SetNumUninitialized(verticesCount);

	int i = 0;
	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
		Vertices[verticesCount - 1 - i] = *it;
		i++;
	}
	i = 0;
	for (auto it = triangles.begin(); it != triangles.end(); ++it)
	{
		int a = *it;
		++it;
		int b = *it;
		++it;
		int c = *it;

		Triangles[i] = a;
		Triangles[i + 1] = b;
		Triangles[i + 2] = c;
		i += 3;

		FVector A = Vertices[a];
		FVector B = Vertices[b];
		FVector C = Vertices[c];
		FVector n = FVector::CrossProduct(C - A, B - A);
		// surface = norm(n) / 2
		// We want: normals += n / norm(n) * surface
		// <=> normals += n / 2
		// <=> normals += n because normals are normalized
		n.Normalize();
		Normals[a] += n;
		Normals[b] += n;
		Normals[c] += n;
		tangents[a] += B + C - 2 * A;
		tangents[b] += B + C - 2 * A;
		tangents[c] += B + C - 2 * A;
	}

	for (int i = 0; i < tangents.Num(); i++)
	{
		Tangents[i] = FProcMeshTangent(tangents[i].SafeNormal(), false);
		Normals[i].Normalize();
	}
}

void Chunk::Polygonise(int x, int y, int z)
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

		auto newCache = newCacheIs1 ? cache1 : cache2;
		auto oldCache = newCacheIs1 ? cache2 : cache1;

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
					vertices.push_front(Q);
					verticeIndex = verticesCount;
					verticesCount++;
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
					vertices.push_front(P1);
					verticeIndex = verticesCount;
					verticesCount++;
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
					vertices.push_front(P0);
					verticeIndex = verticesCount;
					verticesCount++;
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
			triangles.push_front(vertexIndices[cellData.vertexIndex[i]]);
		}
		trianglesCount += 3 * cellData.GetTriangleCount();
	}
}



char Chunk::GetValue(int x, int y, int z)
{
	return world->GetValue(X + x, Y + y, Z + z);
}