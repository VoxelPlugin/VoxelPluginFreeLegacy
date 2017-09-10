#include "VoxelPrivatePCH.h"
#include "VoxelRender.h"
#include "Transvoxel.h"

DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ Cache"), STAT_CACHE, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ Iter"), STAT_ITER, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ CreateSection"), STAT_CREATE_SECTION, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ MajorColor"), STAT_MAJOR_COLOR, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelRender ~ GetValueAndColor"), STAT_GETVALUEANDCOLOR, STATGROUP_Voxel);

VoxelRender::VoxelRender(int Depth, VoxelData* Data, FIntVector ChunkPosition) : Depth(Depth), Data(Data), ChunkPosition(ChunkPosition - FIntVector(Step(), Step(), Step())) // Translate chunk for normals computation =
{

}

void VoxelRender::CreateSection(FProcMeshSection& OutSection)
{
	{
		SCOPE_CYCLE_COUNTER(STAT_CACHE);
		// Cache signs
		for (int CubeX = 0; CubeX < 6; CubeX++)
		{
			for (int CubeY = 0; CubeY < 6; CubeY++)
			{
				for (int CubeZ = 0; CubeZ < 6; CubeZ++)
				{
					uint64& CurrentCube = CachedSigns[CubeX + 6 * CubeY + 6 * 6 * CubeZ];
					CurrentCube = 0;
					for (int LocalX = 0; LocalX < 4; LocalX++)
					{
						for (int LocalY = 0; LocalY < 4; LocalY++)
						{
							for (int LocalZ = 0; LocalZ < 4; LocalZ++)
							{
								const int X = 3 * CubeX + LocalX;
								const int Y = 3 * CubeY + LocalY;
								const int Z = 3 * CubeZ + LocalZ;

								const uint64 ONE = 1;
								uint64 CurrentBit = ONE << (LocalX + 4 * LocalY + 4 * 4 * LocalZ);

								float CurrentValue;
								FColor CurrentColor;
								Data->GetValueAndColor(X * Step() + ChunkPosition.X, Y * Step() + ChunkPosition.Y, Z * Step() + ChunkPosition.Z, CurrentValue, CurrentColor);

								if (X < 18 && Y < 18 && Z < 18) // Getting value out of this chunk for the "continue" optimization after
								{
									CachedValues[X + 18 * Y + 18 * 18 * Z] = CurrentValue;
									CachedColors[X + 18 * Y + 18 * 18 * Z] = CurrentColor;
								}

								bool Sign = CurrentValue > 0;
								CurrentCube = CurrentCube | (CurrentBit * Sign);
							}
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

	{
		SCOPE_CYCLE_COUNTER(STAT_ITER);
		// Iterate over cubes
		for (int CubeX = 0; CubeX < 6; CubeX++)
		{
			for (int CubeY = 0; CubeY < 6; CubeY++)
			{
				for (int CubeZ = 0; CubeZ < 6; CubeZ++)
				{
					uint64 CurrentCube = CachedSigns[CubeX + 6 * CubeY + 6 * 6 * CubeZ];
					if (CurrentCube == 0 || CurrentCube == /*MAXUINT64*/ ((uint64)~((uint64)0)))
					{
						continue;
					}
					for (int LocalX = 0; LocalX < 3 - (CubeX == 5); LocalX++)
					{
						for (int LocalY = 0; LocalY < 3 - (CubeY == 5); LocalY++)
						{
							for (int LocalZ = 0; LocalZ < 3 - (CubeZ == 5); LocalZ++)
							{
								const uint64 ONE = 1;
								unsigned long CaseCode =
									(static_cast<bool>(CurrentCube   & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) << 0)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) << 1)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) << 2)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) << 3)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) << 4)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) << 5)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) << 6)
									| (static_cast<bool>(CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) << 7);

								if (CaseCode != 0 && CaseCode != 511)
								{
									// Cell has a nontrivial triangulation

									const int X = 3 * CubeX + LocalX;
									const int Y = 3 * CubeY + LocalY;
									const int Z = 3 * CubeZ + LocalZ;

									short ValidityMask = (X != 0) + 2 * (Y != 0) + 4 * (Z != 0);

									const FIntVector CornerPositions[8] = {
										FIntVector(X + 0, Y + 0, Z + 0) * Step(),
										FIntVector(X + 1, Y + 0, Z + 0) * Step(),
										FIntVector(X + 0, Y + 1, Z + 0) * Step(),
										FIntVector(X + 1, Y + 1, Z + 0) * Step(),
										FIntVector(X + 0, Y + 0, Z + 1) * Step(),
										FIntVector(X + 1, Y + 0, Z + 1) * Step(),
										FIntVector(X + 0, Y + 1, Z + 1) * Step(),
										FIntVector(X + 1, Y + 1, Z + 1) * Step()
									};

									float CornerValues[8];

									FColor CornerColors[8];

									GetValueAndColor(Step() * (X + 0), Step() * (Y + 0), Step() * (Z + 0), CornerValues[0], CornerColors[0]);
									GetValueAndColor(Step() * (X + 1), Step() * (Y + 0), Step() * (Z + 0), CornerValues[1], CornerColors[1]);
									GetValueAndColor(Step() * (X + 0), Step() * (Y + 1), Step() * (Z + 0), CornerValues[2], CornerColors[2]);
									GetValueAndColor(Step() * (X + 1), Step() * (Y + 1), Step() * (Z + 0), CornerValues[3], CornerColors[3]);
									GetValueAndColor(Step() * (X + 0), Step() * (Y + 0), Step() * (Z + 1), CornerValues[4], CornerColors[4]);
									GetValueAndColor(Step() * (X + 1), Step() * (Y + 0), Step() * (Z + 1), CornerValues[5], CornerColors[5]);
									GetValueAndColor(Step() * (X + 0), Step() * (Y + 1), Step() * (Z + 1), CornerValues[6], CornerColors[6]);
									GetValueAndColor(Step() * (X + 1), Step() * (Y + 1), Step() * (Z + 1), CornerValues[7], CornerColors[7]);

									check(CaseCode == (
										((CornerValues[0] > 0) << 0)
										| ((CornerValues[1] > 0) << 1)
										| ((CornerValues[2] > 0) << 2)
										| ((CornerValues[3] > 0) << 3)
										| ((CornerValues[4] > 0) << 4)
										| ((CornerValues[5] > 0) << 5)
										| ((CornerValues[6] > 0) << 6)
										| ((CornerValues[7] > 0) << 7)));

									// Too slow
									//const FColor& CellColor = GetMajorColor(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, Step());

									const FColor& CellColor = CornerColors[0];

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
										bool ForceVertexCreation = false;
										if ((ValidityMask & CacheDirection) == CacheDirection) // Avoid accessing to invalid value
										{
											FColor Color;
											float Value;
											GetValueAndColor(X - static_cast<bool>(CacheDirection & 0x01) * Step(),
															 Y - static_cast<bool>(CacheDirection & 0x02) * Step(),
															 Z - static_cast<bool>(CacheDirection & 0x04) * Step(),
															 Value, Color);

											ForceVertexCreation = (CellColor != Color);
										}

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

												const float	AlphaAtA = CornerColors[IndexVerticeA].B;
												const float AlphaAtB = CornerColors[IndexVerticeB].B;

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
													Alpha = 0;
													checkf(false, TEXT("Error in interpolation: case should not exist"));
												}
											}
											VertexIndex = VerticesSize;
											Q -= FVector(Step(), Step(), Step()); // Translate back (see constructor)
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
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CREATE_SECTION);
		// Create section
		OutSection = FProcMeshSection();
		OutSection.bEnableCollision = (Depth == 0);
		OutSection.bSectionVisible = true;
		OutSection.ProcVertexBuffer.SetNumUninitialized(VerticesSize);
		OutSection.ProcIndexBuffer.SetNumUninitialized(TrianglesSize);

		// We create 2 vertex arrays: one with all the vertex (AllVertex), and one without vertex that are only for normal computation (FilteredVertex) which is the Section array
		// To do so we create bijection between the 2 arrays
		TArray<int32> AllToFiltered;
		AllToFiltered.SetNumUninitialized(VerticesSize);

		TArray<FVector> AllVertex;
		AllVertex.SetNumUninitialized(VerticesSize);

		int32 AllVertexIndex = 0;
		int32 FilteredVertexIndex = 0;
		for (int i = VerticesSize - 1; i >= 0; i--)
		{
			FVector Vertex = Vertices.front();
			FColor Color = Colors.front();
			Vertices.pop_front();
			Colors.pop_front();

			if ((Vertex.X < -KINDA_SMALL_NUMBER) || (Vertex.X > Width() + KINDA_SMALL_NUMBER) ||
				(Vertex.Y < -KINDA_SMALL_NUMBER) || (Vertex.Y > Width() + KINDA_SMALL_NUMBER) ||
				(Vertex.Z < -KINDA_SMALL_NUMBER) || (Vertex.Z > Width() + KINDA_SMALL_NUMBER))
			{
				// For normals only
				AllToFiltered[AllVertexIndex] = -1;
			}
			else
			{
				AllToFiltered[AllVertexIndex] = FilteredVertexIndex;

				FProcMeshVertex& ProcMeshVertex = OutSection.ProcVertexBuffer[FilteredVertexIndex];
				ProcMeshVertex.Position = Vertex;
				ProcMeshVertex.Tangent = FProcMeshTangent();
				ProcMeshVertex.Color = Color;
				ProcMeshVertex.UV0 = FVector2D::ZeroVector;
				OutSection.SectionLocalBox += Vertex;

				FilteredVertexIndex++;
			}

			AllVertex[AllVertexIndex] = Vertex;
			AllVertexIndex++;
		}
		const int32 FilteredVertexCount = FilteredVertexIndex;
		OutSection.ProcVertexBuffer.SetNum(FilteredVertexCount);

		// Normal array to compute normals while iterating over triangles
		TArray<FVector> Normals;
		Normals.SetNumZeroed(FilteredVertexCount); // Init because +=

		int32 FilteredTriangleIndex = 0;
		while (!Triangles.empty())
		{
			int32 A = Triangles.front();
			int32 FA = AllToFiltered[VerticesSize - 1 - A]; // Invert triangles because AllVertex and OutSection.ProcVertexBuffer are inverted
			Triangles.pop_front();

			int32 B = Triangles.front();
			int32 FB = AllToFiltered[VerticesSize - 1 - B];
			Triangles.pop_front();

			int32 C = Triangles.front();
			int32 FC = AllToFiltered[VerticesSize - 1 - C];
			Triangles.pop_front();

			if (FA != -1 && FB != -1 && FC != -1)
			{
				// If all vertex of this triangle are not for normal only, this is a valid triangle
				// FilteredVertexCount - X because vertices are reversed
				OutSection.ProcIndexBuffer[FilteredTriangleIndex] = FC;
				OutSection.ProcIndexBuffer[FilteredTriangleIndex + 1] = FB;
				OutSection.ProcIndexBuffer[FilteredTriangleIndex + 2] = FA;
				FilteredTriangleIndex += 3;
			}

			FVector PA = AllVertex[VerticesSize - 1 - A];
			FVector PB = AllVertex[VerticesSize - 1 - B];
			FVector PC = AllVertex[VerticesSize - 1 - C];

			FVector Normal = FVector::CrossProduct(PB - PA, PC - PA).GetSafeNormal();
			if (FA != -1)
			{
				Normals[FA] += Normal;
			}
			if (FB != -1)
			{
				Normals[FB] += Normal;
			}
			if (FC != -1)
			{
				Normals[FC] += Normal;
			}
		}
		const int32 FilteredTriangleCount = FilteredTriangleIndex;;
		OutSection.ProcIndexBuffer.SetNum(FilteredTriangleCount);


		// Apply normals
		for (int32 i = 0; i < FilteredVertexCount; i++)
		{
			OutSection.ProcVertexBuffer[i].Normal = Normals[i].GetSafeNormal();
		}
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


//FColor VoxelRender::GetMajorColor(int X, int Y, int Z, uint32 CellWidth)
//{
//	// Too slow
//	check(false);
//
//	SCOPE_CYCLE_COUNTER(STAT_MAJOR_COLOR);
//
//	FColor Colors[8];
//	if (CellWidth == 1)
//	{
//		Colors[0] = GetColor(X + 0, Y + 0, Z + 0);
//		Colors[1] = GetColor(X + 1, Y + 0, Z + 0);
//		Colors[2] = GetColor(X + 0, Y + 1, Z + 0);
//		Colors[3] = GetColor(X + 1, Y + 1, Z + 0);
//		Colors[4] = GetColor(X + 0, Y + 0, Z + 1);
//		Colors[5] = GetColor(X + 1, Y + 0, Z + 1);
//		Colors[6] = GetColor(X + 0, Y + 1, Z + 1);
//		Colors[7] = GetColor(X + 1, Y + 1, Z + 1);
//	}
//	else
//	{
//		uint32 HalfWidth = CellWidth / 2;
//		Colors[0] = GetMajorColor(X + 000000000, Y + 000000000, Z + 000000000, HalfWidth);
//		Colors[1] = GetMajorColor(X + HalfWidth, Y + 000000000, Z + 000000000, HalfWidth);
//		Colors[2] = GetMajorColor(X + 000000000, Y + HalfWidth, Z + 000000000, HalfWidth);
//		Colors[3] = GetMajorColor(X + HalfWidth, Y + HalfWidth, Z + 000000000, HalfWidth);
//		Colors[4] = GetMajorColor(X + 000000000, Y + 000000000, Z + HalfWidth, HalfWidth);
//		Colors[5] = GetMajorColor(X + HalfWidth, Y + 000000000, Z + HalfWidth, HalfWidth);
//		Colors[6] = GetMajorColor(X + 000000000, Y + HalfWidth, Z + HalfWidth, HalfWidth);
//		Colors[7] = GetMajorColor(X + HalfWidth, Y + HalfWidth, Z + HalfWidth, HalfWidth);
//	}
//	// Reground same colors and count them
//	FColor SingleColors[8];
//	uint8 SingleColorsCount[8];
//	uint8 NumberOfDifferentColors = 0;
//
//	// Add Colors to the lists
//	for (int i = 1; i < 8; i++)
//	{
//		bool AlreadyInList = false;
//		for (int j = 0; j < NumberOfDifferentColors; j++)
//		{
//			if (Colors[i] == SingleColors[j])
//			{
//				SingleColorsCount[j]++;
//				AlreadyInList = true;
//				break;
//			}
//		}
//		if (!AlreadyInList)
//		{
//			SingleColors[NumberOfDifferentColors] = Colors[i];
//			SingleColorsCount[NumberOfDifferentColors] = 1;
//			NumberOfDifferentColors++;
//		}
//	}
//	check(NumberOfDifferentColors != 0);
//
//	// Get max
//	uint8 MaxCount = 0;
//	uint8 MaxIndex = -1;
//	for (int i = 0; i < NumberOfDifferentColors; i++)
//	{
//		if (SingleColorsCount[i] > MaxCount)
//		{
//			MaxCount = SingleColorsCount[i];
//			MaxIndex = i;
//		}
//	}
//	check(MaxIndex >= 0);
//	return SingleColors[MaxIndex];
////}


void VoxelRender::GetValueAndColor(int X, int Y, int Z, float& OutValue, FColor& OutColor)
{
	SCOPE_CYCLE_COUNTER(STAT_GETVALUEANDCOLOR);
	if (LIKELY((X % Step() == 0) && (Y % Step() == 0) && (Z % Step() == 0)))
	{
		OutValue = CachedValues[X / Step() + 18 * Y / Step() + 18 * 18 * Z / Step()];
		OutColor = CachedColors[X / Step() + 18 * Y / Step() + 18 * 18 * Z / Step()];
	}
	else
	{
		Data->GetValueAndColor(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, OutValue, OutColor);
	}
}


void VoxelRender::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index)
{
	check(0 <= X && X < 17);
	check(0 <= Y && Y < 17);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	Cache[X][Y][Z][EdgeIndex] = Index;
}

int VoxelRender::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	bool XIsDifferent = Direction & 0x01;
	bool YIsDifferent = Direction & 0x02;
	bool ZIsDifferent = Direction & 0x04;

	check(0 <= X - XIsDifferent && X - XIsDifferent < 17);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < 17);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	return Cache[X - XIsDifferent][Y - YIsDifferent][Z - ZIsDifferent][EdgeIndex];
}


void VoxelRender::InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FColor ColorAtA;
	FColor ColorAtB;
	GetValueAndColor(MinX, Y, Z, ValueAtA, ColorAtA);
	GetValueAndColor(MaxX, Y, Z, ValueAtB, ColorAtB);

	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(MinX, Y, Z) + (1 - t) *  FVector(MaxX, Y, Z);
		OutAlpha = t * ColorAtA.B + (1 - t) * ColorAtB.B;
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);
		int xMiddle = (MaxX + MinX) / 2;

		float ValueAtMiddle;
		FColor ColorAtMiddle;
		GetValueAndColor(xMiddle, Y, Z, ValueAtMiddle, ColorAtMiddle);
		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			return InterpolateX(xMiddle, MaxX, Y, Z, OutVector, OutAlpha);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX(MinX, xMiddle, Y, Z, OutVector, OutAlpha);
		}
	}
}

void VoxelRender::InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FColor ColorAtA;
	FColor ColorAtB;
	GetValueAndColor(X, MinY, Z, ValueAtA, ColorAtA);
	GetValueAndColor(X, MaxY, Z, ValueAtB, ColorAtB);

	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, MinY, Z) + (1 - t) *  FVector(X, MaxY, Z);
		OutAlpha = t * ColorAtA.B + (1 - t) * ColorAtB.B;
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);
		int yMiddle = (MaxY + MinY) / 2;

		float ValueAtMiddle;
		FColor ColorAtMiddle;
		GetValueAndColor(X, yMiddle, Z, ValueAtMiddle, ColorAtMiddle);
		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			return InterpolateY(X, yMiddle, MaxY, Z, OutVector, OutAlpha);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY(X, MinY, yMiddle, Z, OutVector, OutAlpha);
		}
	}
}

void VoxelRender::InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FColor ColorAtA;
	FColor ColorAtB;
	GetValueAndColor(X, Y, MinZ, ValueAtA, ColorAtA);
	GetValueAndColor(X, Y, MaxZ, ValueAtB, ColorAtB);

	if (MaxZ - MinZ == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, Y, MinZ) + (1 - t) *  FVector(X, Y, MaxZ);
		OutAlpha = t * ColorAtA.B + (1 - t) * ColorAtB.B;
	}
	else
	{
		check((MaxZ + MinZ) % 2 == 0);
		int zMiddle = (MaxZ + MinZ) / 2;

		float ValueAtMiddle;
		FColor ColorAtMiddle;
		GetValueAndColor(X, Y, zMiddle, ValueAtMiddle, ColorAtMiddle);
		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			return InterpolateZ(X, Y, zMiddle, MaxZ, OutVector, OutAlpha);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateZ(X, Y, MinZ, zMiddle, OutVector, OutAlpha);
		}
	}
}


