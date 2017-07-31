#include "TransvoxelTools.h"
#include "Transvoxel.h"

void TransvoxelTools::RegularPolygonize(IRegularVoxel* chunk, int x, int y, int z, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props& properties, int& verticesCount)
{
	int Step = 1 << chunk->GetDepth();
	signed char Corner[8] = {
		chunk->GetValue(x       * Step, y       * Step, z       * Step),
		chunk->GetValue((x + 1) * Step, y       * Step, z       * Step),
		chunk->GetValue(x       * Step, (y + 1) * Step, z       * Step),
		chunk->GetValue((x + 1) * Step, (y + 1) * Step, z       * Step),
		chunk->GetValue(x       * Step, y       * Step, (z + 1) * Step),
		chunk->GetValue((x + 1) * Step, y       * Step, (z + 1) * Step),
		chunk->GetValue(x       * Step, (y + 1) * Step, (z + 1) * Step),
		chunk->GetValue((x + 1) * Step, (y + 1) * Step, (z + 1) * Step)
	};

	FIntVector Positions[8] = {
		FIntVector(x    , y    , z) * Step,
		FIntVector(x + 1, y    , z) * Step,
		FIntVector(x    , y + 1, z) * Step,
		FIntVector(x + 1, y + 1, z) * Step,
		FIntVector(x    , y    , z + 1) * Step,
		FIntVector(x + 1, y    , z + 1) * Step,
		FIntVector(x    , y + 1, z + 1) * Step,
		FIntVector(x + 1, y + 1, z + 1) * Step
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

		TArray<int> VertexIndices;
		VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

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
					// Vertex failed validity check
					VertexIndex = AddVertex(chunk, Step, vertices, properties, verticesCount, (FVector)PositionB, PositionB);
				}
				else
				{
					// Vertex already created
					VertexIndex = chunk->LoadVertex(x, y, z, Direction, EdgeIndex);
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((ValidityMask & Direction) != Direction)
				{
					// Validity check failed
					VertexIndex = AddVertex(chunk, Step, vertices, properties, verticesCount, (FVector)PositionA, PositionA);
				}
				else
				{
					// Reuse vertex
					VertexIndex = chunk->LoadVertex(x, y, z, Direction, EdgeIndex);
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
						check(ValueAtA - ValueAtB != 0);
						float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
						FVector Q = t * (FVector)PositionA + (1 - t) * (FVector)PositionB;
						VertexIndex = AddVertex(chunk, Step, vertices, properties, verticesCount, Q, PositionA, EdgeIndex != 2, EdgeIndex != 1, EdgeIndex != 3);
					}
					else
					{
						FVector Q;
						if (EdgeIndex == 2)
						{
							// Edge along x axis
							Q = InterpolateX(chunk, PositionA.X, PositionB.X, PositionA.Y, PositionA.Z);
						}
						else if (EdgeIndex == 1)
						{
							// Edge along y axis
							Q = InterpolateY(chunk, PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z);
						}
						else if (EdgeIndex == 3)
						{
							// Edge along z axis
							Q = InterpolateZ(chunk, PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z);
						}
						else
						{
							checkf(false, TEXT("Error in interpolation: case should not exist"));
						}
						VertexIndex = AddVertex(chunk, Step, vertices, properties, verticesCount, Q, PositionA, EdgeIndex != 2, EdgeIndex != 1, EdgeIndex != 3);
					}
				}
				else
				{
					VertexIndex = chunk->LoadVertex(x, y, z, Direction, EdgeIndex);
				}
			}

			// If own vertex, save it
			if (Direction & 0x08)
			{
				chunk->SaveVertex(x, y, z, EdgeIndex, VertexIndex);
			}
			VertexIndices[i] = VertexIndex;

		}

		// Add triangles
		for (int i = 0; i < 3 * CellData.GetTriangleCount(); i++)
		{
			triangles.push_front(VertexIndices[CellData.vertexIndex[i]]);
		}
		trianglesCount += 3 * CellData.GetTriangleCount();
	}
}

int TransvoxelTools::AddVertex(IRegularVoxel* chunk, int step, Verts& vertices, Props& properties, int& verticesCount, FVector vertex, FIntVector exactPosition, bool xIsExact, bool yIsExact, bool zIsExact)
{
	properties.push_front(VertexProperties({
		xIsExact && exactPosition.X == 0,
		xIsExact && exactPosition.X == 16 * step,
		yIsExact && exactPosition.Y == 0,
		yIsExact && exactPosition.Y == 16 * step,
		zIsExact && exactPosition.Z == 0,
		zIsExact && exactPosition.Z == 16 * step,
		chunk->IsNormalOnly(vertex)
	}));
	vertices.push_front(vertex);
	verticesCount++;
	return verticesCount - 1;
}

FVector TransvoxelTools::InterpolateX(IRegularVoxel* chunk, int xMin, int xMax, int y, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = chunk->GetValue(xMin, y, z);
	signed char ValueAtB = chunk->GetValue(xMax, y, z);
	if (xMax - xMin == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(xMin, y, z) + (1 - t) *  FVector(xMax, y, z);
	}
	else
	{
		check((xMax + xMin) % 2 == 0);

		int xMiddle = (xMax + xMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (chunk->GetValue(xMiddle, y, z) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateX(chunk, xMiddle, xMax, y, z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(chunk, xMin, xMiddle, y, z);
		}
	}
}

FVector TransvoxelTools::InterpolateY(IRegularVoxel* chunk, int x, int yMin, int yMax, int z)
{
	// A: Min / B: Max
	signed char ValueAtA = chunk->GetValue(x, yMin, z);
	signed char ValueAtB = chunk->GetValue(x, yMax, z);
	if (yMax - yMin == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(x, yMin, z) + (1 - t) *  FVector(x, yMax, z);
	}
	else
	{
		check((yMax + yMin) % 2 == 0);

		int yMiddle = (yMax + yMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (chunk->GetValue(x, yMiddle, z) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateY(chunk, x, yMiddle, yMax, z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(chunk, x, yMin, yMiddle, z);
		}
	}
}

FVector TransvoxelTools::InterpolateZ(IRegularVoxel* chunk, int x, int y, int zMin, int zMax)
{
	// A: Min / B: Max
	signed char ValueAtA = chunk->GetValue(x, y, zMin);
	signed char ValueAtB = chunk->GetValue(x, y, zMax);
	if (zMax - zMin == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		return t * FVector(x, y, zMin) + (1 - t) *  FVector(x, y, zMax);
	}
	else
	{
		check((zMax + zMin) % 2 == 0);

		int zMiddle = (zMax + zMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (chunk->GetValue(x, y, zMiddle) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateZ(chunk, x, y, zMiddle, zMax);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateZ(chunk, x, y, zMin, zMiddle);
		}
	}
}
