#include "VoxelPrivatePCH.h"
#include "TransvoxelTools.h"
#include "Transvoxel.h"
#include <cmath>

void TransvoxelTools::RegularPolygonize(IRegularVoxel* Chunk, int X, int Y, int Z, short ValidityMask, Trigs& Triangles, int& TrianglesCount, Verts& Vertices, Props& Properties, Colors& Colors, int& VerticesCount, int Step)
{
	check(Chunk);
	const float Corner[8] = {
		Chunk->GetValue(X       * Step, Y       * Step, Z       * Step),
		Chunk->GetValue((X + 1) * Step, Y       * Step, Z       * Step),
		Chunk->GetValue(X       * Step, (Y + 1) * Step, Z       * Step),
		Chunk->GetValue((X + 1) * Step, (Y + 1) * Step, Z       * Step),
		Chunk->GetValue(X       * Step, Y       * Step, (Z + 1) * Step),
		Chunk->GetValue((X + 1) * Step, Y       * Step, (Z + 1) * Step),
		Chunk->GetValue(X       * Step, (Y + 1) * Step, (Z + 1) * Step),
		Chunk->GetValue((X + 1) * Step, (Y + 1) * Step, (Z + 1) * Step)
	};

	unsigned long CaseCode =
		(std::signbit(Corner[0]) << 0)
		| (std::signbit(Corner[1]) << 1)
		| (std::signbit(Corner[2]) << 2)
		| (std::signbit(Corner[3]) << 3)
		| (std::signbit(Corner[4]) << 4)
		| (std::signbit(Corner[5]) << 5)
		| (std::signbit(Corner[6]) << 6)
		| (std::signbit(Corner[7]) << 7);

	if (CaseCode != 0 && CaseCode != 511)
	{
		// Cell has a nontrivial triangulation
		FIntVector Positions[8] = {
			FIntVector(X    , Y    , Z) * Step,
			FIntVector(X + 1, Y    , Z) * Step,
			FIntVector(X    , Y + 1, Z) * Step,
			FIntVector(X + 1, Y + 1, Z) * Step,
			FIntVector(X    , Y    , Z + 1) * Step,
			FIntVector(X + 1, Y    , Z + 1) * Step,
			FIntVector(X    , Y + 1, Z + 1) * Step,
			FIntVector(X + 1, Y + 1, Z + 1) * Step
		};

		check(0 <= CaseCode && CaseCode < 256);
		unsigned char CellClass = regularCellClass[CaseCode];
		const unsigned short* VertexData = regularVertexData[CaseCode];
		check(0 <= CellClass && CellClass < 16);
		RegularCellData CellData = regularCellData[CellClass];

		TArray<int> VertexIndices;
		VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

		for (int i = 0; i < CellData.GetVertexCount(); i++)
		{
			int VertexIndex;
			const unsigned short EdgeCode = VertexData[i];

			// A: low point / B: high point
			const unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
			const unsigned short IndexVerticeB = EdgeCode & 0x0F;

			check(0 <= IndexVerticeA && IndexVerticeA < 8);
			check(0 <= IndexVerticeB && IndexVerticeB < 8);

			const float ValueAtA = Corner[IndexVerticeA];
			const float ValueAtB = Corner[IndexVerticeB];

			const FIntVector PositionA = Positions[IndexVerticeA];
			const FIntVector PositionB = Positions[IndexVerticeB];

			// Index of vertex on a generic cube (0, 1, 2 or 3)
			const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
			// Direction to go to use an already created vertex
			const short CacheDirection = EdgeCode >> 12;

			if (ValueAtB == 0)
			{
				// Vertex lies at the higher-numbered endpoint
				if ((IndexVerticeB == 7) || ((ValidityMask & CacheDirection) != CacheDirection))
				{
					// Vertex failed validity check
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(Chunk, Step, Vertices, Properties, Colors, VerticesCount, static_cast<FVector>(PositionB), PositionB, IsExact);
				}
				else
				{
					// Vertex already created
					VertexIndex = Chunk->LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((ValidityMask & CacheDirection) != CacheDirection)
				{
					// Validity check failed
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(Chunk, Step, Vertices, Properties, Colors, VerticesCount, static_cast<FVector>(PositionA), PositionA, IsExact);
				}
				else
				{
					// Reuse vertex
					VertexIndex = Chunk->LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
				}
			}
			else
			{
				// Vertex lies in the interior of the edge
				if ((ValidityMask & CacheDirection) != CacheDirection)
				{
					const bool bIsAlongX = (EdgeIndex == 2);
					const bool bIsAlongY = (EdgeIndex == 1);
					const bool bIsAlongZ = (EdgeIndex == 3);

					FBoolVector IsExact(!bIsAlongX, !bIsAlongY, !bIsAlongZ);

					// Validity check failed
					if (Step == 1)
					{
						// Full resolution
						check(ValueAtA - ValueAtB != 0);
						float t = ValueAtB / (ValueAtB - ValueAtA);
						FVector Q = t * static_cast<FVector>(PositionA) + (1 - t) * static_cast<FVector>(PositionB);
						VertexIndex = AddVertex(Chunk, Step, Vertices, Properties, Colors, VerticesCount, Q, PositionA, IsExact);
					}
					else
					{
						FVector Q;
						if (bIsAlongX)
						{
							// Edge along X axis
							Q = InterpolateX(Chunk, PositionA.X, PositionB.X, PositionA.Y, PositionA.Z);
						}
						else if (bIsAlongY)
						{
							// Edge along Y axis
							Q = InterpolateY(Chunk, PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z);
						}
						else if (bIsAlongZ)
						{
							// Edge along z axis
							Q = InterpolateZ(Chunk, PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z);
						}
						else
						{
							checkf(false, TEXT("Error in interpolation: case should not exist"));
						}
						VertexIndex = AddVertex(Chunk, Step, Vertices, Properties, Colors, VerticesCount, Q, PositionA, IsExact);
					}
				}
				else
				{
					VertexIndex = Chunk->LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
				}
			}

			// If own vertex, save it
			if (CacheDirection & 0x08)
			{
				Chunk->SaveVertex(X, Y, Z, EdgeIndex, VertexIndex, Properties.front());
			}
			VertexIndices[i] = VertexIndex;

		}

		// Add triangles
		for (int i = 0; i < 3 * CellData.GetTriangleCount(); i++)
		{
			Triangles.push_front(VertexIndices[CellData.vertexIndex[i]]);
		}
		TrianglesCount += 3 * CellData.GetTriangleCount();
	}
}

int TransvoxelTools::AddVertex(IRegularVoxel* Chunk, int Step, Verts& Vertices, Props& Properties, Colors& Colors, int& VerticesCount, FVector Vertex, FIntVector ExactPosition, FBoolVector IsExact)
{
	Properties.push_front(VertexProperties({
		IsExact.X && ExactPosition.X == 0,
		IsExact.X && ExactPosition.X == 16 * Step,
		IsExact.Y && ExactPosition.Y == 0,
		IsExact.Y && ExactPosition.Y == 16 * Step,
		IsExact.Z && ExactPosition.Z == 0,
		IsExact.Z && ExactPosition.Z == 16 * Step,
		Chunk->IsNormalOnly(Vertex)
	}));
	Colors.push_front(Chunk->GetColor(ExactPosition.X, ExactPosition.Y, ExactPosition.Z));
	Vertices.push_front(Vertex);
	VerticesCount++;
	return VerticesCount - 1;
}

FVector TransvoxelTools::InterpolateX(IRegularVoxel* Chunk, int MinX, int MaxX, int Y, int Z)
{
	// A: Min / B: Max
	const float ValueAtA = Chunk->GetValue(MinX, Y, Z);
	const float ValueAtB = Chunk->GetValue(MaxX, Y, Z);
	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);
		return t * FVector(MinX, Y, Z) + (1 - t) *  FVector(MaxX, Y, Z);
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);

		int xMiddle = (MaxX + MinX) / 2;
		if (std::signbit(ValueAtA) == std::signbit(Chunk->GetValue(xMiddle, Y, Z)))
		{
			// If min and middle have same sign
			return InterpolateX(Chunk, xMiddle, MaxX, Y, Z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(Chunk, MinX, xMiddle, Y, Z);
		}
	}
}

FVector TransvoxelTools::InterpolateY(IRegularVoxel* Chunk, int X, int MinY, int MaxY, int Z)
{
	// A: Min / B: Max
	const float ValueAtA = Chunk->GetValue(X, MinY, Z);
	const float ValueAtB = Chunk->GetValue(X, MaxY, Z);
	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);
		return t * FVector(X, MinY, Z) + (1 - t) *  FVector(X, MaxY, Z);
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);

		int yMiddle = (MaxY + MinY) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if (std::signbit(ValueAtA) == std::signbit(Chunk->GetValue(X, yMiddle, Z)))
		{
			// If min and middle have same sign
			return InterpolateY(Chunk, X, yMiddle, MaxY, Z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(Chunk, X, MinY, yMiddle, Z);
		}
	}
}

FVector TransvoxelTools::InterpolateZ(IRegularVoxel* Chunk, int X, int Y, int MinZ, int MaxZ)
{
	// A: Min / B: Max
	const float ValueAtA = Chunk->GetValue(X, Y, MinZ);
	const float ValueAtB = Chunk->GetValue(X, Y, MaxZ);
	if (MaxZ - MinZ == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);
		return t * FVector(X, Y, MinZ) + (1 - t) *  FVector(X, Y, MaxZ);
	}
	else
	{
		check((MaxZ + MinZ) % 2 == 0);

		int zMiddle = (MaxZ + MinZ) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if (std::signbit(ValueAtA) == std::signbit(Chunk->GetValue(X, Y, zMiddle)))
		{
			// If min and middle have same sign
			return InterpolateZ(Chunk, X, Y, zMiddle, MaxZ);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateZ(Chunk, X, Y, MinZ, zMiddle);
		}
	}
}



void TransvoxelTools::TransitionPolygonize(ITransitionVoxel* Chunk, int X, int Y, short ValidityMask, Trigs& Triangles, int& TrianglesCount, Verts& Vertices, Props2D& Properties, Colors& Colors, int& VerticesCount, const int Step)
{
	check(Chunk);
	check(0 <= X && X < 16 && 0 <= Y && Y < 16);

	const int HalfStep = Step / 2;

	const float Corner[13] = {
		Chunk->GetValue2D(2 * X       * HalfStep, 2 * Y       * HalfStep),
		Chunk->GetValue2D((2 * X + 1) * HalfStep, 2 * Y       * HalfStep),
		Chunk->GetValue2D((2 * X + 2) * HalfStep, 2 * Y       * HalfStep),
		Chunk->GetValue2D(2 * X       * HalfStep, (2 * Y + 1) * HalfStep),
		Chunk->GetValue2D((2 * X + 1) * HalfStep, (2 * Y + 1) * HalfStep),
		Chunk->GetValue2D((2 * X + 2) * HalfStep, (2 * Y + 1) * HalfStep),
		Chunk->GetValue2D(2 * X       * HalfStep, (2 * Y + 2) * HalfStep),
		Chunk->GetValue2D((2 * X + 1) * HalfStep, (2 * Y + 2) * HalfStep),
		Chunk->GetValue2D((2 * X + 2) * HalfStep, (2 * Y + 2) * HalfStep),

		Chunk->GetValue2D(2 * X       * HalfStep, 2 * Y       * HalfStep),
		Chunk->GetValue2D((2 * X + 2) * HalfStep, 2 * Y       * HalfStep),
		Chunk->GetValue2D(2 * X       * HalfStep, (2 * Y + 2) * HalfStep),
		Chunk->GetValue2D((2 * X + 2) * HalfStep, (2 * Y + 2) * HalfStep)
	};

	unsigned long CaseCode =
		(std::signbit(Corner[0]) << 0)
		| (std::signbit(Corner[1]) << 1)
		| (std::signbit(Corner[2]) << 2)
		| (std::signbit(Corner[5]) << 3)
		| (std::signbit(Corner[8]) << 4)
		| (std::signbit(Corner[7]) << 5)
		| (std::signbit(Corner[6]) << 6)
		| (std::signbit(Corner[3]) << 7)
		| (std::signbit(Corner[4]) << 8);

	if (!(CaseCode == 0 || CaseCode == 511))
	{
		FIntVector Positions[13] = {
			FIntVector(2 * X    , 2 * Y    , 0) * HalfStep,
			FIntVector(2 * X + 1, 2 * Y    , 0) * HalfStep,
			FIntVector(2 * X + 2, 2 * Y    , 0) * HalfStep,
			FIntVector(2 * X    , 2 * Y + 1, 0) * HalfStep,
			FIntVector(2 * X + 1, 2 * Y + 1, 0) * HalfStep,
			FIntVector(2 * X + 2, 2 * Y + 1, 0) * HalfStep,
			FIntVector(2 * X    , 2 * Y + 2, 0) * HalfStep,
			FIntVector(2 * X + 1, 2 * Y + 2, 0) * HalfStep,
			FIntVector(2 * X + 2, 2 * Y + 2, 0) * HalfStep,

			FIntVector(2 * X    , 2 * Y    , 1) * HalfStep,
			FIntVector(2 * X + 2, 2 * Y    , 1) * HalfStep,
			FIntVector(2 * X    , 2 * Y + 2, 1) * HalfStep,
			FIntVector(2 * X + 2, 2 * Y + 2, 1) * HalfStep
		};

		check(0 <= CaseCode && CaseCode < 512);
		const unsigned char CellClass = transitionCellClass[CaseCode];
		const unsigned short* VertexData = transitionVertexData[CaseCode];
		check(0 <= (CellClass & 0x7F) && (CellClass & 0x7F) < 56);
		const TransitionCellData CellData = transitionCellData[CellClass & 0x7F];
		const bool bFlip = CellClass >> 7;

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

			const float ValueAtA = Corner[IndexVerticeA];
			const float ValueAtB = Corner[IndexVerticeB];


			const FIntVector PositionA = Positions[IndexVerticeA];
			const FIntVector PositionB = Positions[IndexVerticeB];

			const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
			// Direction to go to use an already created vertex
			const short CacheDirection = EdgeCode >> 12;

			if (ValueAtB == 0)
			{
				// Vertex lies at the higher-numbered endpoint
				if (((ValidityMask & CacheDirection) != CacheDirection))
				{
					// Vertex failed validity check
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(Chunk, PositionB.Z == HalfStep, HalfStep, Vertices, Properties, Colors, VerticesCount, static_cast<FVector>(PositionB), PositionB, IsExact);
				}
				else
				{
					// Vertex already created
					VertexIndex = Chunk->LoadVertex2D(X, Y, CacheDirection, EdgeIndex);
				}
			}
			else if (ValueAtA == 0)
			{
				// Vertex lies at the lower-numbered endpoint
				if ((ValidityMask & CacheDirection) != CacheDirection)
				{
					// Validity check failed
					FBoolVector IsExact(true, true, true);
					VertexIndex = AddVertex(Chunk, PositionA.Z == HalfStep, HalfStep, Vertices, Properties, Colors, VerticesCount, static_cast<FVector>(PositionA), PositionA, IsExact);

				}
				else
				{
					// Reuse vertex
					VertexIndex = Chunk->LoadVertex2D(X, Y, CacheDirection, EdgeIndex);
				}
			}
			else
			{
				// Vertex lies in the interior of the edge
				if ((ValidityMask & CacheDirection) != CacheDirection)
				{
					// Validity check failed
					const bool bIsAlongX = EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8;
					const bool bIsAlongY = EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9;

					FBoolVector IsExact(!bIsAlongX, !bIsAlongY, false);

					FVector Q;
					if (bIsAlongX)
					{
						// Edge along X axis
						Q = InterpolateX2D(Chunk, PositionA.X, PositionB.X, PositionA.Y);
					}
					else if (bIsAlongY)
					{
						// Edge along Y axis
						Q = InterpolateY2D(Chunk, PositionA.X, PositionA.Y, PositionB.Y);
					}
					else
					{
						checkf(false, TEXT("Error in interpolation: case should not exist"));
					}
					VertexIndex = AddVertex(Chunk, PositionA.Z == HalfStep, HalfStep, Vertices, Properties, Colors, VerticesCount, Q, PositionA, IsExact);
				}
				else
				{
					VertexIndex = Chunk->LoadVertex2D(X, Y, CacheDirection, EdgeIndex);
				}
			}

			// If own vertex, save it
			if (CacheDirection & 0x08)
			{
				Chunk->SaveVertex2D(X, Y, EdgeIndex, VertexIndex);
			}
			VertexIndices[i] = VertexIndex;
		}

		// Add triangles
		int AddedTrigsCount = 3 * CellData.GetTriangleCount();
		for (int i = 0; i < AddedTrigsCount; i++)
		{
			Triangles.push_front(VertexIndices[CellData.vertexIndex[bFlip ? (AddedTrigsCount - 1 - i) : i]]);
		}
		TrianglesCount += AddedTrigsCount;
	}
}

int TransvoxelTools::AddVertex(ITransitionVoxel* Chunk, bool bIsTranslated, int Step, Verts& Vertices, Props2D& Properties, Colors& Colors, int& VerticesCount, FVector Vertex, FIntVector ExactPosition, FBoolVector IsExact)
{
	Properties.push_front(VertexProperties2D({
		ExactPosition.X,
		ExactPosition.Y,
		IsExact.X,
		IsExact.Y,
		bIsTranslated,
		Chunk->GetDirection()
	}));
	Colors.push_front(Chunk->GetColor2D(ExactPosition.X, ExactPosition.Y));
	Vertices.push_front(FVector(Vertex.X, Vertex.Y, 0));
	VerticesCount++;
	return VerticesCount - 1;
}

FVector TransvoxelTools::InterpolateX2D(ITransitionVoxel* Chunk, int MinX, int MaxX, int Y)
{
	// A: Min / B: Max
	const float ValueAtA = Chunk->GetValue2D(MinX, Y);
	const float ValueAtB = Chunk->GetValue2D(MaxX, Y);
	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);
		FIntVector PositionA(MinX, Y, 0);
		FIntVector PositionB(MaxX, Y, 0);
		return t * static_cast<FVector>(PositionA) + (1 - t) * static_cast<FVector>(PositionB);
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);

		int xMiddle = (MaxX + MinX) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if (std::signbit(ValueAtA) == std::signbit(Chunk->GetValue2D(xMiddle, Y)))
		{
			// If min and middle have same sign
			return InterpolateX2D(Chunk, xMiddle, MaxX, Y);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX2D(Chunk, MinX, xMiddle, Y);
		}
	}
}

FVector TransvoxelTools::InterpolateY2D(ITransitionVoxel* Chunk, int X, int MinY, int MaxY)
{
	// A: Min / B: Max
	const float ValueAtA = Chunk->GetValue2D(X, MinY);
	const float ValueAtB = Chunk->GetValue2D(X, MaxY);
	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);
		FIntVector PositionA(X, MinY, 0);
		FIntVector PositionB(X, MaxY, 0);
		return t * static_cast<FVector>(PositionA) + (1 - t) * static_cast<FVector>(PositionB);
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);

		int yMiddle = (MaxY + MinY) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if (std::signbit(ValueAtA) == std::signbit(Chunk->GetValue2D(X, yMiddle)))
		{
			// If min and middle have same sign
			return InterpolateY2D(Chunk, X, yMiddle, MaxY);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY2D(Chunk, X, MinY, yMiddle);
		}
	}
}

FVector TransvoxelTools::GetTranslated(FVector V, FVector N, VertexProperties P, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, int Depth)
{
	// If an adjacent block is rendered at the same resolution, return primary position
	if ((P.IsNearXMin && !ChunkHasHigherRes[XMin]) || (P.IsNearXMax && !ChunkHasHigherRes[XMax]) ||
		(P.IsNearYMin && !ChunkHasHigherRes[YMin]) || (P.IsNearYMax && !ChunkHasHigherRes[YMax]) ||
		(P.IsNearZMin && !ChunkHasHigherRes[ZMin]) || (P.IsNearZMax && !ChunkHasHigherRes[ZMax]))
	{
		return V;
	}


	double DeltaX = 0;
	double DeltaY = 0;
	double DeltaZ = 0;

	double TwoPowerK = 1 << Depth;
	double w = TwoPowerK / 4;

	if ((P.IsNearXMin && ChunkHasHigherRes[XMin]) || (P.IsNearXMax && ChunkHasHigherRes[XMax]))
	{
		if (V.X < TwoPowerK)
		{
			DeltaX = (1 - static_cast<double>(V.X) / TwoPowerK) * w;
		}
		else if (V.X > TwoPowerK * (16 - 1))
		{
			DeltaX = (16 - 1 - static_cast<double>(V.X) / TwoPowerK) * w;
		}
	}
	if ((P.IsNearYMin && ChunkHasHigherRes[YMin]) || (P.IsNearYMax && ChunkHasHigherRes[YMax]))
	{
		if (V.Y < TwoPowerK)
		{
			DeltaY = (1 - static_cast<double>(V.Y) / TwoPowerK) * w;
		}
		else if (V.Y > TwoPowerK * (16 - 1))
		{
			DeltaY = (16 - 1 - static_cast<double>(V.Y) / TwoPowerK) * w;
		}
	}
	if ((P.IsNearZMin && ChunkHasHigherRes[ZMin]) || (P.IsNearZMax && ChunkHasHigherRes[ZMax]))
	{
		if (V.Z < TwoPowerK)
		{
			DeltaZ = (1 - static_cast<double>(V.Z) / TwoPowerK) * w;
		}
		else if (V.Z > TwoPowerK * (16 - 1))
		{
			DeltaZ = (16 - 1 - static_cast<double>(V.Z) / TwoPowerK) * w;
		}
	}

	FVector Q = FVector(
		(1 - N.X * N.X) * DeltaX - N.X * N.Y * DeltaY - N.X * N.Z * DeltaZ,
		-N.X * N.Y * DeltaX + (1 - N.Y * N.Y) * DeltaY - N.Y * N.Z * DeltaZ,
		-N.X * N.Z * DeltaX - N.Y * N.Z * DeltaY + (1 - N.Z * N.Z) * DeltaZ);

	return V + Q;
}
