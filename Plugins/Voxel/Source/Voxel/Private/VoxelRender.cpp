#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"
#include "Transvoxel.h"


VoxelRender::VoxelRender(int Depth, ValueOctree* Octree, FIntVector ChunkPosition) : Depth(Depth), Octree(Octree), ChunkPosition(ChunkPosition)
{

}

void VoxelRender::CreateSection(FProcMeshSection& OutSection)
{
	// Cache signs
	for (int CubeX = 0; CubeX < 5; CubeX++)
	{
		for (int CubeY = 0; CubeY < 5; CubeY++)
		{
			for (int CubeZ = 0; CubeZ < 5; CubeZ++)
			{
				uint64& CurrentCube = Signs[CubeX + 5 * CubeY + 5 * 5 * CubeZ];
				for (int LocalX = 0; LocalX < 4; LocalX++)
				{
					for (int LocalY = 0; LocalY < 4; LocalY++)
					{
						for (int LocalZ = 0; LocalZ < 4; LocalZ++)
						{
							int CurrentBit = LocalX + 4 * LocalY + 4 * 4 * LocalZ;
							bool Sign = Octree->GetValue(FIntVector(3 * CubeX + LocalX, 3 * CubeY + LocalY, 3 * CubeZ + LocalZ)) > 0;
							CurrentCube = CurrentCube & (0xFFFFFFFFFFFFFFFF - CurrentBit) | (CurrentBit * Sign);
						}
					}
				}
			}
		}
	}

	// Create forward lists
	std::forward_list<FVector> Vertices;
	std::forward_list<FColor> Colors;
	std::forward_list<int32> Triangles;
	int VerticesSize = 0;
	int TrianglesSize = 0;

	// Iterate over cubes
	for (int CubeX = 0; CubeX < 5; CubeX++)
	{
		for (int CubeY = 0; CubeY < 5; CubeY++)
		{
			for (int CubeZ = 0; CubeZ < 5; CubeZ++)
			{
				uint64& CurrentCube = Signs[CubeX + 5 * CubeY + 5 * 5 * CubeZ];
				for (int LocalX = 0; LocalX < 3; LocalX++)
				{
					for (int LocalY = 0; LocalY < 3; LocalY++)
					{
						for (int LocalZ = 0; LocalZ < 3; LocalZ++)
						{
							short ValidityMask = (CubeX + LocalX == 0) + 2 * (CubeY + LocalY == 0) + 4 * (CubeZ + LocalZ == 0);
							unsigned long CaseCode =
								((CurrentCube & ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0))) << 0)
								| ((CurrentCube & ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0))) << 1)
								| ((CurrentCube & ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0))) << 2)
								| ((CurrentCube & ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0))) << 3)
								| ((CurrentCube & ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1))) << 4)
								| ((CurrentCube & ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1))) << 5)
								| ((CurrentCube & ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1))) << 6)
								| ((CurrentCube & ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1))) << 7);

							if (CaseCode != 0 && CaseCode != 511)
							{
								// Cell has a nontrivial triangulation

								int X = 3 * CubeX + LocalX;
								int Y = 3 * CubeY + LocalY;
								int Z = 3 * CubeZ + LocalZ;

								const FIntVector CornerPositions[8] = {
									FIntVector(X    , Y    , Z) * Step(),
									FIntVector(X + 1, Y    , Z) * Step(),
									FIntVector(X    , Y + 1, Z) * Step(),
									FIntVector(X + 1, Y + 1, Z) * Step(),
									FIntVector(X    , Y    , Z + 1) * Step(),
									FIntVector(X + 1, Y    , Z + 1) * Step(),
									FIntVector(X    , Y + 1, Z + 1) * Step(),
									FIntVector(X + 1, Y + 1, Z + 1) * Step()
								};

								const float CornerValues[8] = {
									Octree->GetValue(CornerPositions[0] + ChunkPosition),
									Octree->GetValue(CornerPositions[1] + ChunkPosition),
									Octree->GetValue(CornerPositions[2] + ChunkPosition),
									Octree->GetValue(CornerPositions[3] + ChunkPosition),
									Octree->GetValue(CornerPositions[4] + ChunkPosition),
									Octree->GetValue(CornerPositions[5] + ChunkPosition),
									Octree->GetValue(CornerPositions[6] + ChunkPosition),
									Octree->GetValue(CornerPositions[7] + ChunkPosition)
								};

								const float CornerAlphas[8] = {
									Octree->GetColor(CornerPositions[0] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[1] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[2] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[3] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[4] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[5] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[6] + ChunkPosition).B,
									Octree->GetColor(CornerPositions[7] + ChunkPosition).B
								};

								const FColor& CellColor = GetMajorColor(CornerPositions[0], Step());

								check(0 <= CaseCode && CaseCode < 256);
								unsigned char CellClass = Transvoxel::regularCellClass[CaseCode];
								const unsigned short* VertexData = Transvoxel::regularVertexData[CaseCode];
								check(0 <= CellClass && CellClass < 16);
								Transvoxel::RegularCellData CellData = Transvoxel::regularCellData[CellClass];

								// Indices of the vertices used in this cube
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

									const FIntVector PositionA = CornerPositions[IndexVerticeA];
									const FIntVector PositionB = CornerPositions[IndexVerticeB];

									// Index of vertex on a generic cube (0, 1, 2 or 3 in the transvoxel paper, but it's always != 0 so we substract 1 to have 0, 1, or 2)
									const short EdgeIndex = ((EdgeCode >> 8) & 0x0F) - 1;
									check(0 <= EdgeIndex && EdgeIndex < 3);

									// Direction to go to use an already created vertex: 
									// first bit:  x is different
									// second bit: y is different
									// third bit:  z is different
									// fourth bit: vertex isn't cached
									const short CacheDirection = EdgeCode >> 12;

									// Force vertex creation when colors are different
									const bool ForceVertexCreation = (CellColor != GetMajorColor(FIntVector(X - static_cast<bool>(CacheDirection & 0x01) * Step(),
																											Y - static_cast<bool>(CacheDirection & 0x02) * Step(),
																											Z - static_cast<bool>(CacheDirection & 0x04) * Step()), Step()));

									if ((ValidityMask & CacheDirection) != CacheDirection || ForceVertexCreation)
									{
										// If we are on the edge of the chunk, or precedent color is not the same as current one

										const bool bIsAlongX = (EdgeIndex == 1);
										const bool bIsAlongY = (EdgeIndex == 0);
										const bool bIsAlongZ = (EdgeIndex == 2);

										FVector Q;
										uint8 Alpha;
										if (Step() == 1)
										{
											// Full resolution

											const float ValueAtA = CornerValues[IndexVerticeA];
											const float ValueAtB = CornerValues[IndexVerticeB];

											const float	AlphaAtA = CornerAlphas[IndexVerticeA];
											const float AlphaAtB = CornerAlphas[IndexVerticeB];

											check(ValueAtA - ValueAtB != 0);
											const float t = ValueAtB / (ValueAtB - ValueAtA);

											Q = t * static_cast<FVector>(PositionA) + (1 - t) * static_cast<FVector>(PositionB);
											Alpha = t * AlphaAtA + (1 - t) * AlphaAtB;
										}
										else
										{
											// Interpolate

											if (bIsAlongX)
											{
												InterpolateX(PositionA.X, PositionB.X, PositionA.Y, PositionA.Z, Q, Alpha);
											}
											else if (bIsAlongY)
											{
												InterpolateY(PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z, Q, Alpha);
											}
											else if (bIsAlongZ)
											{
												InterpolateZ(PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z, Q, Alpha);
											}
											else
											{
												checkf(false, TEXT("Error in interpolation: case should not exist"));
											}
										}
										VertexIndex = VerticesSize;

										Vertices.push_front(Q);
										Colors.push_front(FColor(CellColor.R, CellColor.G, Alpha, 0));
										VerticesSize++;
									}
									else
									{
										VertexIndex = LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
									}

									// If own vertex, save it
									if (CacheDirection & 0x08)
									{
										SaveVertex(X, Y, Z, EdgeIndex, VertexIndex);
									}

									VertexIndices[i] = VertexIndex;
								}

								// Add triangles
								// 3 vertex per triangle
								int n = 3 * CellData.GetTriangleCount();
								for (int i = 0; i < n; i++)
								{
									Triangles.push_front(VertexIndices[CellData.vertexIndex[i]]);
								}
								TrianglesSize += n;
							}
						}
					}
				}
			}
		}
	}

	OutSection = FProcMeshSection();
	OutSection.bEnableCollision = true;
	OutSection.bSectionVisible = true;
	OutSection.ProcVertexBuffer.SetNumUninitialized(VerticesSize);
	OutSection.ProcIndexBuffer.SetNumUninitialized(TrianglesSize);

	for (int i = VerticesSize - 1; i >= 0; i++)
	{
		FProcMeshVertex& Vertex = OutSection.ProcVertexBuffer[i];

		Vertex.Position = Vertices.front();
		Vertex.Normal = FVector::ZeroVector;
		Vertex.Tangent = FProcMeshTangent();
		Vertex.Color = Colors.front();
		Vertex.UV0 = FVector2D::ZeroVector;

		Vertices.pop_front();
		Colors.pop_front();
	}

	for (int i = TrianglesSize - 1; i >= 0; i++)
	{
		OutSection.ProcIndexBuffer[i] = Triangles.front();
		Triangles.pop_front();
	}
}

int VoxelRender::Width()
{
	return 16 << Depth;
}

int VoxelRender::Step()
{
	return 1 << Depth;
}


FColor VoxelRender::GetMajorColor(FIntVector LowerCorner, uint32 CellWidth)
{
	check(Depth > 0);

	FColor Colors[8];
	if (CellWidth == 1)
	{
		Colors[0] = Octree->GetColor(LowerCorner);
		Colors[1] = Octree->GetColor(LowerCorner + FIntVector(1, 0, 0));
		Colors[2] = Octree->GetColor(LowerCorner + FIntVector(0, 1, 0));
		Colors[3] = Octree->GetColor(LowerCorner + FIntVector(1, 1, 0));
		Colors[4] = Octree->GetColor(LowerCorner + FIntVector(0, 0, 1));
		Colors[5] = Octree->GetColor(LowerCorner + FIntVector(1, 0, 1));
		Colors[6] = Octree->GetColor(LowerCorner + FIntVector(0, 1, 1));
		Colors[7] = Octree->GetColor(LowerCorner + FIntVector(1, 1, 1));
	}
	else
	{
		uint32 HalfWidth = CellWidth / 2;
		Colors[0] = GetMajorColor(LowerCorner, Depth - 1);
		Colors[1] = GetMajorColor(LowerCorner + FIntVector(HalfWidth, 0, 0), Depth - 1);
		Colors[2] = GetMajorColor(LowerCorner + FIntVector(0, HalfWidth, 0), Depth - 1);
		Colors[3] = GetMajorColor(LowerCorner + FIntVector(HalfWidth, HalfWidth, 0), Depth - 1);
		Colors[4] = GetMajorColor(LowerCorner + FIntVector(0, 0, HalfWidth), Depth - 1);
		Colors[5] = GetMajorColor(LowerCorner + FIntVector(HalfWidth, 0, HalfWidth), Depth - 1);
		Colors[6] = GetMajorColor(LowerCorner + FIntVector(0, HalfWidth, HalfWidth), Depth - 1);
		Colors[7] = GetMajorColor(LowerCorner + FIntVector(HalfWidth, HalfWidth, HalfWidth), Depth - 1);
	}
	// Reground same colors and count them
	FColor SingleColors[8];
	uint8 SingleColorsCount[8];
	uint8 NumberOfDifferentColors = 0;

	// Add Colors to the lists
	for (int i = 1; i < 8; i++)
	{
		bool AlreadyInList = false;
		for (int j = 0; j < NumberOfDifferentColors; j++)
		{
			if (Colors[i] == SingleColors[j])
			{
				SingleColorsCount[j]++;
				AlreadyInList = true;
				break;
			}
		}
		if (!AlreadyInList)
		{
			SingleColors[NumberOfDifferentColors] = Colors[i];
			SingleColorsCount[NumberOfDifferentColors] = 1;
			NumberOfDifferentColors++;
		}
	}
	check(NumberOfDifferentColors != 0);

	// Get max
	uint8 MaxCount = 0;
	uint8 MaxIndex;
	for (int i = 0; i < NumberOfDifferentColors; i++)
	{
		if (SingleColorsCount[i] > MaxCount)
		{
			MaxCount = SingleColorsCount[i];
			MaxIndex = i;
		}
	}

	return SingleColors[MaxIndex];
}


void VoxelRender::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index)
{
	auto NewCache = bNewCacheIs1 ? Cache1 : Cache2;

	check(0 <= X && X < 16);
	check(0 <= Y && Y < 16);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	NewCache[X][Y][EdgeIndex] = Index;
}

int VoxelRender::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	auto NewCache = bNewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = bNewCacheIs1 ? Cache2 : Cache1;

	bool XIsDifferent = Direction & 0x01;
	bool YIsDifferent = Direction & 0x02;
	bool ZIsDifferent = Direction & 0x04;

	check(0 <= X - XIsDifferent && X - XIsDifferent < 16);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < 16);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	return (ZIsDifferent ? OldCache : NewCache)[X - XIsDifferent][Y - YIsDifferent][EdgeIndex];
}


void VoxelRender::InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	const float ValueAtA = Octree->GetValue(FIntVector(MinX, Y, Z));
	const float ValueAtB = Octree->GetValue(FIntVector(MaxX, Y, Z));

	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(MinX, Y, Z) + (1 - t) *  FVector(MaxX, Y, Z);
		OutAlpha = t * Octree->GetColor(FIntVector(MinX, Y, Z)).B + (1 - t) * Octree->GetColor(FIntVector(MaxX, Y, Z)).B;
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);

		int xMiddle = (MaxX + MinX) / 2;
		if ((ValueAtA > 0) == (Octree->GetValue(FIntVector(xMiddle, Y, Z)) > 0))
		{
			// If min and middle have same sign
			return InterpolateX(xMiddle, MaxX, Y, Z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(MinX, xMiddle, Y, Z);
		}
	}
}

void VoxelRender::InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	const float ValueAtA = Octree->GetValue(FIntVector(X, MinY, Z));
	const float ValueAtB = Octree->GetValue(FIntVector(X, MaxY, Z));

	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, MinY, Z) + (1 - t) *  FVector(X, MaxY, Z);
		OutAlpha = t * Octree->GetColor(FIntVector(X, MinY, Z)).B + (1 - t) * Octree->GetColor(FIntVector(X, MaxY, Z)).B;
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);

		int yMiddle = (MaxY + MinY) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA > 0) == (Octree->GetValue(FIntVector(X, yMiddle, Z)) > 0))
		{
			// If min and middle have same sign
			return InterpolateY(X, yMiddle, MaxY, Z);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(X, MinY, yMiddle, Z);
		}
	}
}

void VoxelRender::InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	const float ValueAtA = Octree->GetValue(FIntVector(X, Y, MinZ));
	const float ValueAtB = Octree->GetValue(FIntVector(X, Y, MaxZ));

	if (MaxZ - MinZ == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, Y, MinZ) + (1 - t) *  FVector(X, Y, MaxZ);
		OutAlpha = t * Octree->GetColor(FIntVector(X, Y, MinZ)).B + (1 - t) * Octree->GetColor(FIntVector(X, Y, MaxZ)).B;
	}
	else
	{
		check((MaxZ + MinZ) % 2 == 0);

		int zMiddle = (MaxZ + MinZ) / 2;
		// Sign of a char: char & 0x80 (char are 8 bits)
		if ((ValueAtA > 0) == (Octree->GetValue(FIntVector(X, Y, zMiddle)) > 0))
		{
			// If min and middle have same sign
			return InterpolateZ(X, Y, zMiddle, MaxZ);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateZ(X, Y, MinZ, zMiddle);
		}
	}
}


