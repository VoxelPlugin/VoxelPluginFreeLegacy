#include "TransvoxelTools.h"
#include "Transvoxel.h"

void TransvoxelTools::RegularPolygonize(IRegularVoxel* chunk, int x, int y, int z, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props& properties, Colors& colors, int& verticesCount)
{
	check(chunk);
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

		TArray<int> VertexIndices;
		VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

		for (int i = 0; i < CellData.GetVertexCount(); i++)
		{
			int VertexIndex;
			const unsigned short EdgeCode = VertexData[i];

			// A: low point / B: high point
			const unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
			const unsigned short IndexVerticeB = EdgeCode & 0x0F;

			const signed char ValueAtA = Corner[IndexVerticeA];
			const signed char ValueAtB = Corner[IndexVerticeB];

			const FIntVector PositionA = Positions[IndexVerticeA];
			const FIntVector PositionB = Positions[IndexVerticeB];

			// Index of vertex on a generic cube (0, 1, 2 or 3)
			const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
			// Direction to go to use an already created vertex
			const short Direction = EdgeCode >> 12;

			if (ValueAtB == 0)
			{
				// Vertex lies at the higher-numbered endpoint
				if ((IndexVerticeB == 7) || ((validityMask & Direction) != Direction))
				{
					// Vertex failed validity check
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(chunk, Step, vertices, properties, colors, verticesCount, (FVector)PositionB, PositionB, IsExact);
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
				if ((validityMask & Direction) != Direction)
				{
					// Validity check failed
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(chunk, Step, vertices, properties, colors, verticesCount, (FVector)PositionA, PositionA, IsExact);
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
				if ((validityMask & Direction) != Direction)
				{
					bool IsAlongX = (EdgeIndex == 2);
					bool IsAlongY = (EdgeIndex == 1);
					bool IsAlongZ = (EdgeIndex == 3);

					FBoolVector IsExact(!IsAlongX, !IsAlongY, !IsAlongZ);

					// Validity check failed
					if (Step == 1)
					{
						// Full resolution
						check(ValueAtA - ValueAtB != 0);
						float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
						FVector Q = t * (FVector)PositionA + (1 - t) * (FVector)PositionB;
						VertexIndex = AddVertex(chunk, Step, vertices, properties, colors, verticesCount, Q, PositionA, IsExact);
					}
					else
					{
						FVector Q;
						if (IsAlongX)
						{
							// Edge along x axis
							Q = InterpolateX(chunk, PositionA.X, PositionB.X, PositionA.Y, PositionA.Z);
						}
						else if (IsAlongY)
						{
							// Edge along y axis
							Q = InterpolateY(chunk, PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z);
						}
						else if (IsAlongZ)
						{
							// Edge along z axis
							Q = InterpolateZ(chunk, PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z);
						}
						else
						{
							checkf(false, TEXT("Error in interpolation: case should not exist"));
						}
						VertexIndex = AddVertex(chunk, Step, vertices, properties, colors, verticesCount, Q, PositionA, IsExact);
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

int TransvoxelTools::AddVertex(IRegularVoxel* chunk, int step, Verts& vertices, Props& properties, Colors& colors, int& verticesCount, FVector vertex, FIntVector exactPosition, FBoolVector isExact)
{
	properties.push_front(VertexProperties({
		isExact.X && exactPosition.X == 0,
		isExact.X && exactPosition.X == 16 * step,
		isExact.Y && exactPosition.Y == 0,
		isExact.Y && exactPosition.Y == 16 * step,
		isExact.Z && exactPosition.Z == 0,
		isExact.Z && exactPosition.Z == 16 * step,
		chunk->IsNormalOnly(vertex)
	}));
	colors.push_front(chunk->GetColor(exactPosition.X, exactPosition.Y, exactPosition.Z));
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



void TransvoxelTools::TransitionPolygonize(ITransitionVoxel* chunk, int x, int y, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props2D& properties, Colors& colors, int& verticesCount)
{
	check(chunk);
	check(0 <= x && x < 16 && 0 <= y && y < 16);

	int Step = 1 << (chunk->GetDepth() - 1);
	signed char Corner[13] = {
		chunk->GetValue(2 * x       * Step, 2 * y       * Step),
		chunk->GetValue((2 * x + 1) * Step, 2 * y       * Step),
		chunk->GetValue((2 * x + 2) * Step, 2 * y       * Step),
		chunk->GetValue(2 * x       * Step, (2 * y + 1) * Step),
		chunk->GetValue((2 * x + 1) * Step, (2 * y + 1) * Step),
		chunk->GetValue((2 * x + 2) * Step, (2 * y + 1) * Step),
		chunk->GetValue(2 * x       * Step, (2 * y + 2) * Step),
		chunk->GetValue((2 * x + 1) * Step, (2 * y + 2) * Step),
		chunk->GetValue((2 * x + 2) * Step, (2 * y + 2) * Step),

		chunk->GetValue(2 * x       * Step, 2 * y       * Step),
		chunk->GetValue((2 * x + 2) * Step, 2 * y       * Step),
		chunk->GetValue(2 * x       * Step, (2 * y + 2) * Step),
		chunk->GetValue((2 * x + 2) * Step, (2 * y + 2) * Step)
	};

	for (int k = 0; k < 10; k++)
	{
		chunk->SaveVertex(x, y, k, Corner[k] - 20);
	}

	FIntVector Positions[13] = {
		FIntVector(2 * x    , 2 * y    , 0) * Step,
		FIntVector(2 * x + 1, 2 * y    , 0) * Step,
		FIntVector(2 * x + 2, 2 * y    , 0) * Step,
		FIntVector(2 * x    , 2 * y + 1, 0) * Step,
		FIntVector(2 * x + 1, 2 * y + 1, 0) * Step,
		FIntVector(2 * x + 2, 2 * y + 1, 0) * Step,
		FIntVector(2 * x    , 2 * y + 2, 0) * Step,
		FIntVector(2 * x + 1, 2 * y + 2, 0) * Step,
		FIntVector(2 * x + 2, 2 * y + 2, 0) * Step,

		FIntVector(2 * x    , 2 * y    , 1) * Step,
		FIntVector(2 * x + 2, 2 * y    , 1) * Step,
		FIntVector(2 * x    , 2 * y + 2, 1) * Step,
		FIntVector(2 * x + 2, 2 * y + 2, 1) * Step
	};

	unsigned long CaseCode = ((Corner[0] >> 7) & 0x01)
		| ((Corner[1] >> 6) & 0x02)
		| ((Corner[2] >> 5) & 0x04)
		| ((Corner[5] >> 4) & 0x08)
		| ((Corner[8] >> 3) & 0x10)
		| ((Corner[7] >> 2) & 0x20)
		| ((Corner[6] >> 1) & 0x40)
		| ((Corner[3] >> 0) & 0x80)
		| ((Corner[4] << 1) & 0x100);

	if (!(CaseCode == 0 || CaseCode == 511))
	{
		const unsigned char CellClass = transitionCellClass[CaseCode];
		const TransitionCellData CellData = transitionCellData[CellClass & 0x7F];
		const unsigned short* VertexData = transitionVertexData[CaseCode];
		const bool Flip = CellClass >> 7;

		TArray<int> VertexIndices;
		VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

		for (int i = 0; i < CellData.GetVertexCount(); i++)
		{
			int VertexIndex;
			const unsigned short EdgeCode = VertexData[i];

			// A: low point / B: high point
			const unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
			const unsigned short IndexVerticeB = EdgeCode & 0x0F;

			check(0 <= IndexVerticeA && IndexVerticeA < 13);
			check(0 <= IndexVerticeB && IndexVerticeB < 13);

			const signed char ValueAtA = Corner[IndexVerticeA];
			const signed char ValueAtB = Corner[IndexVerticeB];


			const FIntVector PositionA = Positions[IndexVerticeA];
			const FIntVector PositionB = Positions[IndexVerticeB];

			const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
			// Direction to go to use an already created vertex
			const short Direction = EdgeCode >> 12;

			if (ValueAtB == 0)
			{
				// Vertex lies at the higher-numbered endpoint
				if (((validityMask & Direction) != Direction))
				{
					// Vertex failed validity check
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(chunk, PositionB.Z == Step, Step, vertices, properties, colors, verticesCount, (FVector)PositionB, PositionB, IsExact);
				}
				else
				{
					// Vertex already created
					VertexIndex = chunk->LoadVertex(x, y, Direction, EdgeIndex);
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((validityMask & Direction) != Direction)
				{
					// Validity check failed
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(chunk, PositionA.Z == Step, Step, vertices, properties, colors, verticesCount, (FVector)PositionA, PositionA, IsExact);

				}
				else
				{
					// Reuse vertex
					VertexIndex = chunk->LoadVertex(x, y, Direction, EdgeIndex);
				}
			}
			else
			{
				// Vertex lies in the interior of the edge
				if ((validityMask & Direction) != Direction)
				{
					// Validity check failed
					bool IsAlongX = EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8;
					bool IsAlongY = EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9;

					FBoolVector IsExact(!IsAlongX, !IsAlongY, false);

					FVector Q;
					if (IsAlongX)
					{
						// Edge along x axis
						Q = InterpolateX(chunk, PositionA.X, PositionB.X, PositionA.Y);
					}
					else if (IsAlongY)
					{
						// Edge along y axis
						Q = InterpolateY(chunk, PositionA.X, PositionA.Y, PositionB.Y);
					}
					else
					{
						checkf(false, TEXT("Error in interpolation: case should not exist"));
					}
					VertexIndex = AddVertex(chunk, PositionA.Z == Step, Step, vertices, properties, colors, verticesCount, Q, PositionA, IsExact);
				}
				else
				{
					VertexIndex = chunk->LoadVertex(x, y, Direction, EdgeIndex);
				}
			}

			// If own vertex, save it
			if (Direction & 0x08)
			{
				chunk->SaveVertex(x, y, EdgeIndex, VertexIndex);
			}
			VertexIndices[i] = VertexIndex;
		}

		// Add triangles
		int AddedTrigsCount = 3 * CellData.GetTriangleCount();
		for (int i = 0; i < AddedTrigsCount; i++)
		{
			triangles.push_front(VertexIndices[CellData.vertexIndex[Flip ? (AddedTrigsCount - 1 - i) : i]]);
		}
		trianglesCount += AddedTrigsCount;
	}
}

int TransvoxelTools::AddVertex(ITransitionVoxel* chunk, bool isTranslated, int step, Verts& vertices, Props2D& properties, Colors& colors, int& verticesCount, FVector vertex, FIntVector exactPosition, FBoolVector isExact)
{
	properties.push_front(VertexProperties2D({
		exactPosition.X,
		exactPosition.Y,
		isExact.X,
		isExact.Y,
		isTranslated
	}));
	colors.push_front(chunk->GetColor(exactPosition.X, exactPosition.Y));
	vertices.push_front(FVector(vertex.X, vertex.Y, 0));
	verticesCount++;
	return verticesCount - 1;
}

FVector TransvoxelTools::InterpolateX(ITransitionVoxel* chunk, int xMin, int xMax, int y)
{
	// A: Min / B: Max
	signed char ValueAtA = chunk->GetValue(xMin, y);
	signed char ValueAtB = chunk->GetValue(xMax, y);
	if (xMax - xMin == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		FIntVector PositionA(xMin, y, 0);
		FIntVector PositionB(xMax, y, 0);
		return t * (FVector)PositionA + (1 - t) * (FVector)PositionB;
	}
	else
	{
		check((xMax + xMin) % 2 == 0);

		int xMiddle = (xMax + xMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (chunk->GetValue(xMiddle, y) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateX(chunk, xMiddle, xMax, y);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(chunk, xMin, xMiddle, y);
		}
	}
}

FVector TransvoxelTools::InterpolateY(ITransitionVoxel* chunk, int x, int yMin, int yMax)
{
	// A: Min / B: Max
	signed char ValueAtA = chunk->GetValue(x, yMin);
	signed char ValueAtB = chunk->GetValue(x, yMax);
	if (yMax - yMin == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = (float)ValueAtB / (float)(ValueAtB - ValueAtA);
		FIntVector PositionA(x, yMin, 0);
		FIntVector PositionB(x, yMax, 0);
		return t * (FVector)PositionA + (1 - t) * (FVector)PositionB;
	}
	else
	{
		check((yMax + yMin) % 2 == 0);

		int yMiddle = (yMax + yMin) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA & 0x80) == (chunk->GetValue(x, yMiddle) & 0x80))
		{
			// If min and middle have same sign
			return InterpolateY(chunk, x, yMiddle, yMax);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(chunk, x, yMin, yMiddle);
		}
	}
}
