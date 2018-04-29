// Copyright 2018 Phyronnaz

#include "VoxelPolygonizerForCollisions.h"
#include "VoxelPrivate.h"
#include "Transvoxel.h"
#include "VoxelData.h"
#include "VoxelMaterial.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForCollisions::CreateSection"), STAT_FVoxelPolygonizerForCollisions_CreateSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForCollisions::CreateSection.BeginGet"), STAT_FVoxelPolygonizerForCollisions_CreateSection_BeginGet, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForCollisions::CreateSection.Cache"), STAT_FVoxelPolygonizerForCollisions_CreateSection_Cache, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForCollisions::CreateSection.Cache.GetValuesAndMaterials"), STAT_FVoxelPolygonizerForCollisions_CreateSection_Cache_GetValuesAndMaterials, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForCollisions::CreateSection.Iter"), STAT_FVoxelPolygonizerForCollisions_CreateSection_Iter, STATGROUP_Voxel);

FVoxelPolygonizerForCollisions::FVoxelPolygonizerForCollisions(FVoxelData* Data, const FIntVector& ChunkPosition, bool bEnableRender)
	: Data(Data)
	, ChunkPosition(ChunkPosition)
	, bEnableRender(bEnableRender)
{
}

bool FVoxelPolygonizerForCollisions::CreateSection(FVoxelProcMeshSection& OutSection)
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForCollisions_CreateSection, VOXEL_MULTITHREAD_STAT);

	OutSection.Reset();
	OutSection.bEnableCollision = true;
	OutSection.bSectionVisible = bEnableRender;
	OutSection.SectionLocalBox = FBox(-FVector::OneVector * 1000, FVector::OneVector * 1000);
	
	FIntVector ChunkDataSize(CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1);
	FIntBox Bounds(ChunkPosition, ChunkPosition + ChunkDataSize);

	TArray<uint64> Octrees;
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForCollisions_CreateSection_BeginGet, VOXEL_MULTITHREAD_STAT);
		Octrees = Data->BeginGet(Bounds);
	}

	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForCollisions_CreateSection_Cache, VOXEL_MULTITHREAD_STAT);

		{
			CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForCollisions_CreateSection_Cache_GetValuesAndMaterials, VOXEL_MULTITHREAD_STAT);

			Data->GetValuesAndMaterials(CachedValues, nullptr, ChunkPosition, FIntVector::ZeroValue, 1, ChunkDataSize, ChunkDataSize);
		}

		// Cache signs
		for (int CubeX = 0; CubeX < CUBE_COUNT_FC; CubeX++)
		{
			for (int CubeY = 0; CubeY < CUBE_COUNT_FC; CubeY++)
			{
				for (int CubeZ = 0; CubeZ < CUBE_COUNT_FC; CubeZ++)
				{
					uint64& CurrentCube = CachedSigns[CubeX + CUBE_COUNT_FC * CubeY + CUBE_COUNT_FC * CUBE_COUNT_FC * CubeZ];
					CurrentCube = 0;
					for (int LocalX = 0; LocalX < 4 - (CubeX == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalX++)
					{
						for (int LocalY = 0; LocalY < 4 - (CubeY == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalY++)
						{
							for (int LocalZ = 0; LocalZ < 4 - (CubeZ == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalZ++)
							{
								const int X = 3 * CubeX + LocalX;
								const int Y = 3 * CubeY + LocalY;
								const int Z = 3 * CubeZ + LocalZ;

								const uint64 ONE = 1;
								uint64 CurrentBit = ONE << (LocalX + 4 * LocalY + 4 * 4 * LocalZ);

								float CurrentValue = GetValue(X, Y, Z);

								bool Sign = CurrentValue > 0;
								CurrentCube = CurrentCube | (CurrentBit * Sign);
							}
						}
					}
				}
			}
		}
	}

	Data->EndGet(Octrees);

	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForCollisions_CreateSection_Iter, VOXEL_MULTITHREAD_STAT);

		// Iterate over cubes
		for (int CubeX = 0; CubeX < CUBE_COUNT_FC; CubeX++)
		{
			for (int CubeY = 0; CubeY < CUBE_COUNT_FC; CubeY++)
			{
				for (int CubeZ = 0; CubeZ < CUBE_COUNT_FC; CubeZ++)
				{
					uint64 CurrentCube = CachedSigns[CubeX + CUBE_COUNT_FC * CubeY + CUBE_COUNT_FC * CUBE_COUNT_FC * CubeZ];
					if (CurrentCube == 0 || CurrentCube == /*MAXUINT64*/ ((uint64)~((uint64)0)))
					{
						continue;
					}
					for (int LocalX = 0; LocalX < 3 - (CubeX == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalX++)
					{
						for (int LocalY = 0; LocalY < 3 - (CubeY == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalY++)
						{
							for (int LocalZ = 0; LocalZ < 3 - (CubeZ == CUBE_COUNT_FC - 1 ? END_CUBE_OFFSET_FC : 0); LocalZ++)
							{
								const uint64 ONE = 1;
								unsigned long CaseCode =
									(static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) != 0) << 0)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) != 0) << 1)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) != 0) << 2)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) != 0) << 3)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) != 0) << 4)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) != 0) << 5)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) != 0) << 6)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) != 0) << 7);

								if (CaseCode != 0 && CaseCode != 255)
								{
									// Cell has a nontrivial triangulation

									const int X = 3 * CubeX + LocalX;
									const int Y = 3 * CubeY + LocalY;
									const int Z = 3 * CubeZ + LocalZ;


									float CornerValues[8] = {
										GetValue(X + 0, Y + 0, Z + 0),
										GetValue(X + 1, Y + 0, Z + 0),
										GetValue(X + 0, Y + 1, Z + 0),
										GetValue(X + 1, Y + 1, Z + 0),
										GetValue(X + 0, Y + 0, Z + 1),
										GetValue(X + 1, Y + 0, Z + 1),
										GetValue(X + 0, Y + 1, Z + 1),
										GetValue(X + 1, Y + 1, Z + 1)
									};

									const FIntVector CornerPositions[8] = {
										FIntVector(X + 0, Y + 0, Z + 0),
										FIntVector(X + 1, Y + 0, Z + 0),
										FIntVector(X + 0, Y + 1, Z + 0),
										FIntVector(X + 1, Y + 1, Z + 0),
										FIntVector(X + 0, Y + 0, Z + 1),
										FIntVector(X + 1, Y + 0, Z + 1),
										FIntVector(X + 0, Y + 1, Z + 1),
										FIntVector(X + 1, Y + 1, Z + 1)
									};

									short ValidityMask = (X != 0) + 2 * (Y != 0) + 4 * (Z != 0);

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

										if ((ValidityMask & CacheDirection) != CacheDirection)
										{
											// If we are on one the lower edges of the chunk, or precedent color is not the same as current one
											const float ValueAtA = CornerValues[IndexVerticeA];
											const float ValueAtB = CornerValues[IndexVerticeB];

											if (ValueAtB - ValueAtA == 0)
											{
												return false;
											}
											check(ValueAtB - ValueAtA != 0);
											const float t = ValueAtB / (ValueAtB - ValueAtA);

											if (FMath::IsNaN(t))
											{
												return false;
											}

											FVector Q = t * static_cast<FVector>(PositionA) + (1 - t) * static_cast<FVector>(PositionB);


											VertexIndex = OutSection.ProcVertexBuffer.Num();

											FVoxelProcMeshVertex Vertex;
											Vertex.Position = Q;
											OutSection.ProcVertexBuffer.Add(Vertex);

											// If own vertex, save it
											if (CacheDirection & 0x08)
											{
												SaveVertex(X, Y, Z, EdgeIndex, VertexIndex);
											}
										}
										else
										{
											VertexIndex = LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
										}

										VertexIndices[i] = VertexIndex;
									}

									// Add triangles
									// 3 vertex per triangle
									int n = 3 * CellData.GetTriangleCount();
									for (int i = 0; i < n; i++)
									{
										OutSection.ProcIndexBuffer.Add(VertexIndices[CellData.vertexIndex[i]]);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (OutSection.ProcVertexBuffer.Num() < 3 || OutSection.ProcIndexBuffer.Num() == 0)
	{
		// Else physics thread crash
		OutSection.Reset();
	}

	// TODO: Speed up using raw ptrs
	// Else sweeps crash PhysX
	for (int I = 0; I < OutSection.ProcIndexBuffer.Num();)
	{
		const FVector& A = OutSection.ProcVertexBuffer[OutSection.ProcIndexBuffer[I]].Position;
		I++;
		const FVector& B = OutSection.ProcVertexBuffer[OutSection.ProcIndexBuffer[I]].Position;
		I++;
		const FVector& C = OutSection.ProcVertexBuffer[OutSection.ProcIndexBuffer[I]].Position;
		I++;

		if (A == B || A == C || B == C)
		{
			I -= 3;
			OutSection.ProcIndexBuffer.RemoveAt(I, 3, false);
		}
	}

	return true;
}

float FVoxelPolygonizerForCollisions::GetValue(int X, int Y, int Z)
{
	check(0 <= X && X < CHUNKSIZE_FC + 1);
	check(0 <= Y && Y < CHUNKSIZE_FC + 1);
	check(0 <= Z && Z < CHUNKSIZE_FC + 1);

	return CachedValues[X + (CHUNKSIZE_FC + 1) * Y + (CHUNKSIZE_FC + 1) * (CHUNKSIZE_FC + 1) * Z];
}

void FVoxelPolygonizerForCollisions::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index)
{
	check(0 <= X && X < CHUNKSIZE_FC);
	check(0 <= Y && Y < CHUNKSIZE_FC);
	check(0 <= Z && Z < CHUNKSIZE_FC);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	Cache[X][Y][Z][EdgeIndex] = Index;
}

int FVoxelPolygonizerForCollisions::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	bool XIsDifferent = static_cast<bool>((Direction & 0x01) != 0);
	bool YIsDifferent = static_cast<bool>((Direction & 0x02) != 0);
	bool ZIsDifferent = static_cast<bool>((Direction & 0x04) != 0);

	check(0 <= X - XIsDifferent && X - XIsDifferent < CHUNKSIZE_FC);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < CHUNKSIZE_FC);
	check(0 <= Z - ZIsDifferent && Z - ZIsDifferent < CHUNKSIZE_FC);
	check(0 <= EdgeIndex && EdgeIndex < 3);


	check(Cache[X - XIsDifferent][Y - YIsDifferent][Z - ZIsDifferent][EdgeIndex] >= 0);
	return Cache[X - XIsDifferent][Y - YIsDifferent][Z - ZIsDifferent][EdgeIndex];
}
