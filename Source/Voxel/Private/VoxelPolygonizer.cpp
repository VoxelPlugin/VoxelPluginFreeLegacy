// Copyright 2018 Phyronnaz

#include "VoxelPolygonizer.h"
#include "VoxelPrivate.h"
#include "Transvoxel.h"
#include "VoxelData.h"
#include "VoxelMaterial.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk"), STAT_FVoxelPolygonizer_CreateChunk, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.BeginGet"), STAT_FVoxelPolygonizer_CreateChunk_BeginGet, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.IsEmpty"), STAT_FVoxelPolygonizer_CreateChunk_IsEmpty, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.Cache"), STAT_FVoxelPolygonizer_CreateChunk_Cache, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.Cache.GetValueAndMaterials"), STAT_FVoxelPolygonizer_CreateChunk_Cache_GetValueAndMaterials, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.Iter"), STAT_FVoxelPolygonizer_CreateChunk_Iter, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizer::CreateChunk.AddAlphaAdditionalVertices"), STAT_FVoxelPolygonizer_CreateChunk_AddAlphaAdditionalVertices, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForTransitions::CreateTransitions"), STAT_FVoxelPolygonizerForTransitions_CreateTransitions, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelPolygonizerForTransitions::CreateTransitions.BeginGet"), STAT_FVoxelPolygonizerForTransitions_CreateTransitions_BeginGet, STATGROUP_Voxel);

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline void GetGradient(T* Data, const FIntVector& P, FVector& Result)
{
	const int32& X = P.X;
	const int32& Y = P.Y;
	const int32& Z = P.Z;

	Result.X = Data->GetValue(X + 1, Y, Z) - Data->GetValue(X - 1, Y, Z);
	Result.Y = Data->GetValue(X, Y + 1, Z) - Data->GetValue(X, Y - 1, Z);
	Result.Z = Data->GetValue(X, Y, Z + 1) - Data->GetValue(X, Y, Z - 1);
}

template<class C>
inline void GetNormalImpl(const C* Data, const FVector& Position, const int& Step, FVector& Result)
{
	FIntVector A, B;
	GetClosestPoints(Position, A, B);

	FVector NA, NB;
	GetGradient(Data, A, NA);
	GetGradient(Data, B, NB);

	float t = (Position - (FVector)A).GetAbsMax() / Step;
	Result = (t * NB + (1 - t) * NA).GetSafeNormal();
}

FORCEINLINE void GetClosestPoints(const FVector& Point, FIntVector& A, FIntVector& B)
{
	A = FIntVector(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y), FMath::FloorToInt(Point.Z));
	B = FIntVector(FMath::CeilToInt(Point.X), FMath::CeilToInt(Point.Y), FMath::CeilToInt(Point.Z));

	check(FMath::Abs((B - A).X) + FMath::Abs((B - A).Y) + FMath::Abs((B - A).Z) <= 1);
}

//////////////////////////////////////////////////////////////////////////

#define ABORD_IF_NOT(bSuccess) \
if (!(bSuccess)) \
{ \
	Data->EndGet(Octrees); \
	return false; \
}


void FVoxelIntermediateChunk::Reset()
{
	IndexBuffer.Reset();
	VertexBuffer.Reset();
}

void FVoxelIntermediateChunk::InitSectionBuffers(TArray<FVoxelProcMeshVertex>& OutVertexBuffer, TArray<int32>& OutIndexBuffer, uint8 TransitionsMask) const
{
	OutVertexBuffer.SetNumUninitialized(VertexBuffer.Num());
	for (int Index = 0; Index < OutVertexBuffer.Num(); Index++)
	{
		FVoxelProcMeshVertex NewVertex;
		const FVoxelVertex& Vertex = VertexBuffer[Index];

		NewVertex.Position = GetTranslated(Vertex.Position, Vertex.Normal, TransitionsMask, LOD);
		NewVertex.Normal = Vertex.Normal;
		NewVertex.Color = Vertex.Color;

		OutVertexBuffer[Index] = NewVertex;
	}
	OutIndexBuffer = IndexBuffer;
}

FVector FVoxelIntermediateChunk::GetTranslated(const FVector& Vertex, const FVector& Normal, uint8 TransitionsMask, uint8 LOD)
{
	const int Step = 1 << LOD;
	const int Size = CHUNK_SIZE << LOD;

	double DeltaX = 0;
	double DeltaY = 0;
	double DeltaZ = 0;

	if ((Vertex.X < KINDA_SMALL_NUMBER && !(TransitionsMask & XMin)) || (Vertex.X > Size - KINDA_SMALL_NUMBER && !(TransitionsMask & XMax)) ||
		(Vertex.Y < KINDA_SMALL_NUMBER && !(TransitionsMask & YMin)) || (Vertex.Y > Size - KINDA_SMALL_NUMBER && !(TransitionsMask & YMax)) ||
		(Vertex.Z < KINDA_SMALL_NUMBER && !(TransitionsMask & ZMin)) || (Vertex.Z > Size - KINDA_SMALL_NUMBER && !(TransitionsMask & ZMax)))
	{
		return Vertex;
	}

	double TwoPowerK = 1 << LOD;
	double w = TwoPowerK / 4;

	if ((TransitionsMask & XMin) && Vertex.X < Step)
	{
		DeltaX = (1 - static_cast<double>(Vertex.X) / TwoPowerK) * w;
	}
	if ((TransitionsMask & XMax) && Vertex.X > (CHUNK_SIZE - 1) * Step)
	{
		DeltaX = (CHUNK_SIZE - 1 - static_cast<double>(Vertex.X) / TwoPowerK) * w;
	}
	if ((TransitionsMask & YMin) && Vertex.Y < Step)
	{
		DeltaY = (1 - static_cast<double>(Vertex.Y) / TwoPowerK) * w;
	}
	if ((TransitionsMask & YMax) && Vertex.Y > (CHUNK_SIZE - 1) * Step)
	{
		DeltaY = (CHUNK_SIZE - 1 - static_cast<double>(Vertex.Y) / TwoPowerK) * w;
	}
	if ((TransitionsMask & ZMin) && Vertex.Z < Step)
	{
		DeltaZ = (1 - static_cast<double>(Vertex.Z) / TwoPowerK) * w;
	}
	if ((TransitionsMask & ZMax) && Vertex.Z > (CHUNK_SIZE - 1) * Step)
	{
		DeltaZ = (CHUNK_SIZE - 1 - static_cast<double>(Vertex.Z) / TwoPowerK) * w;
	}

	FVector Q = FVector(
		(1 - Normal.X * Normal.X) * DeltaX - Normal.X * Normal.Y * DeltaY - Normal.X * Normal.Z * DeltaZ,
		-Normal.X * Normal.Y * DeltaX + (1 - Normal.Y * Normal.Y) * DeltaY - Normal.Y * Normal.Z * DeltaZ,
		-Normal.X * Normal.Z * DeltaX - Normal.Y * Normal.Z * DeltaY + (1 - Normal.Z * Normal.Z) * DeltaZ);

	return Vertex + Q;
}

///////////////////////////////////////////////////////////////////////////////

FVoxelPolygonizer::FVoxelPolygonizer(
	int LOD,
	FVoxelData* Data,
	const FIntVector& ChunkPosition,
	bool bCreateAdditionalVerticesForMaterialsTransitions,
	bool bEnableNormals)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, bCreateAdditionalVerticesForMaterialsTransitions(bCreateAdditionalVerticesForMaterialsTransitions)
	, bEnableNormals(bEnableNormals)
{

}

bool FVoxelPolygonizer::CreateChunk(FVoxelIntermediateChunk& OutChunk)
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk, VOXEL_MULTITHREAD_STAT);

	OutChunk.Reset();
	OutChunk.LOD = LOD;

	for (int i = 0; i < CHUNK_SIZE + 1; i++)
	{
		for (int j = 0; j < CHUNK_SIZE + 1; j++)
		{
			for (int k = 0; k < CHUNK_SIZE + 1; k++)
			{
				IntegerCoordinates[i][j][k] = -1;
			}
		}
	}
	
	FIntVector ChunkDataSize(CHUNK_SIZE + 3, CHUNK_SIZE + 3, CHUNK_SIZE + 3);
	const FIntBox Bounds = FIntBox(ChunkPosition - FIntVector(1, 1, 1), ChunkPosition + (ChunkDataSize - FIntVector(1, 1, 1)) * Step() + FIntVector(2, 2, 2)); // -1: for normals; +2 normals + max of intbox excluded

	TArray<uint64> Octrees;
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_BeginGet, VOXEL_MULTITHREAD_STAT);
		Octrees = Data->BeginGet(Bounds);
	}

	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_IsEmpty, VOXEL_MULTITHREAD_STAT);
		if (Data->IsEmpty(ChunkPosition, Step(), ChunkDataSize))
		{
			Data->EndGet(Octrees);
			return true;
		}
	}

	{
		{
			CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_Cache, VOXEL_MULTITHREAD_STAT);

			{
				CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_Cache_GetValueAndMaterials, VOXEL_MULTITHREAD_STAT);

				Data->GetValuesAndMaterials(CachedValues, CachedMaterials, ChunkPosition - FIntVector(Step(), Step(), Step()), FIntVector::ZeroValue, Step(), ChunkDataSize, ChunkDataSize);
			}

			// Cache signs
			for (int CubeX = 0; CubeX < CUBE_COUNT; CubeX++)
			{
				for (int CubeY = 0; CubeY < CUBE_COUNT; CubeY++)
				{
					for (int CubeZ = 0; CubeZ < CUBE_COUNT; CubeZ++)
					{
						uint64& CurrentCube = CachedSigns[CubeX + CUBE_COUNT * CubeY + CUBE_COUNT * CUBE_COUNT * CubeZ];
						CurrentCube = 0;
						for (int LocalX = 0; LocalX < 4 - (CubeX == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalX++)
						{
							for (int LocalY = 0; LocalY < 4 - (CubeY == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalY++)
							{
								for (int LocalZ = 0; LocalZ < 4 - (CubeZ == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalZ++)
								{
									const int X = 3 * CubeX + LocalX;
									const int Y = 3 * CubeY + LocalY;
									const int Z = 3 * CubeZ + LocalZ;

									const uint64 ONE = 1;
									uint64 CurrentBit = ONE << (LocalX + 4 * LocalY + 4 * 4 * LocalZ);


									check(0 <= X + 1 && X + 1 < CHUNK_SIZE + 3);
									check(0 <= Y + 1 && Y + 1 < CHUNK_SIZE + 3);
									check(0 <= Z + 1 && Z + 1 < CHUNK_SIZE + 3);
									float CurrentValue = CachedValues[(X + 1) + (CHUNK_SIZE + 3) * (Y + 1) + (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3) * (Z + 1)];

									bool Sign = CurrentValue > 0;
									CurrentCube = CurrentCube | (CurrentBit * Sign);
								}
							}
						}
					}
				}
			}
		}

		if (LOD == 0)
		{
			// We won't use it after
			Data->EndGet(Octrees);
		}

		TArray<FVoxelVertex>& Vertices = OutChunk.VertexBuffer;
		TArray<int32>& Triangles = OutChunk.IndexBuffer;

		{
			CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_Iter, VOXEL_MULTITHREAD_STAT);

			// Iterate over cubes
			for (int CubeX = 0; CubeX < CUBE_COUNT; CubeX++)
			{
				for (int CubeY = 0; CubeY < CUBE_COUNT; CubeY++)
				{
					for (int CubeZ = 0; CubeZ < CUBE_COUNT; CubeZ++)
					{
						uint64 CurrentCube = CachedSigns[CubeX + CUBE_COUNT * CubeY + CUBE_COUNT * CUBE_COUNT * CubeZ];
						if (CurrentCube == 0 || CurrentCube == /*MAXUINT64*/ ((uint64)~((uint64)0)))
						{
							continue;
						}
						for (int LocalX = 0; LocalX < 3 - (CubeX == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalX++)
						{
							for (int LocalY = 0; LocalY < 3 - (CubeY == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalY++)
							{
								for (int LocalZ = 0; LocalZ < 3 - (CubeZ == CUBE_COUNT - 1 ? END_CUBE_OFFSET : 0); LocalZ++)
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

										FVoxelMaterial CornerMaterials[8];

										GetValueAndMaterialFromCache(X + 0, Y + 0, Z + 0, CornerValues[0], CornerMaterials[0]);
										GetValueAndMaterialFromCache(X + 1, Y + 0, Z + 0, CornerValues[1], CornerMaterials[1]);
										GetValueAndMaterialFromCache(X + 0, Y + 1, Z + 0, CornerValues[2], CornerMaterials[2]);
										GetValueAndMaterialFromCache(X + 1, Y + 1, Z + 0, CornerValues[3], CornerMaterials[3]);
										GetValueAndMaterialFromCache(X + 0, Y + 0, Z + 1, CornerValues[4], CornerMaterials[4]);
										GetValueAndMaterialFromCache(X + 1, Y + 0, Z + 1, CornerValues[5], CornerMaterials[5]);
										GetValueAndMaterialFromCache(X + 0, Y + 1, Z + 1, CornerValues[6], CornerMaterials[6]);
										GetValueAndMaterialFromCache(X + 1, Y + 1, Z + 1, CornerValues[7], CornerMaterials[7]);

										if (UNLIKELY(FMath::IsNaN(CornerValues[0]) ||
											FMath::IsNaN(CornerValues[1]) ||
											FMath::IsNaN(CornerValues[2]) ||
											FMath::IsNaN(CornerValues[3]) ||
											FMath::IsNaN(CornerValues[4]) ||
											FMath::IsNaN(CornerValues[5]) ||
											FMath::IsNaN(CornerValues[6]) ||
											FMath::IsNaN(CornerValues[7])))
										{
											ABORD_IF_NOT(false);
										}

										check(CaseCode == (
											((CornerValues[0] > 0) << 0)
											| ((CornerValues[1] > 0) << 1)
											| ((CornerValues[2] > 0) << 2)
											| ((CornerValues[3] > 0) << 3)
											| ((CornerValues[4] > 0) << 4)
											| ((CornerValues[5] > 0) << 5)
											| ((CornerValues[6] > 0) << 6)
											| ((CornerValues[7] > 0) << 7)));

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
											int VertexIndex = -1;
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

											const bool bIsVertexCached = ((ValidityMask & CacheDirection) == CacheDirection);

											if (bIsVertexCached)
											{
												check(!(CacheDirection & 0x08));
												VertexIndex = LoadVertex(X, Y, Z, CacheDirection, EdgeIndex);
											}
											else
											{
												// We are on one the lower edges of the chunk. Compute vertex

												FVector IntersectionPoint;
												FColor IntersectionColor;
												// Compute IntersectionPoint and IntersectionColor
												{
													uint8 Alpha;

													const bool bIsAlongX = (EdgeIndex == 1);
													const bool bIsAlongY = (EdgeIndex == 0);
													const bool bIsAlongZ = (EdgeIndex == 2);

													if (Step() == 1)
													{
														// Full resolution

														const float& ValueAtA = CornerValues[IndexVerticeA];
														const float& ValueAtB = CornerValues[IndexVerticeB];

														const float& AlphaAtA = CornerMaterials[IndexVerticeA].Alpha;
														const float& AlphaAtB = CornerMaterials[IndexVerticeB].Alpha;

														ABORD_IF_NOT(ValueAtA - ValueAtB != 0)
															check(ValueAtA - ValueAtB != 0);
														const float t = ValueAtB / (ValueAtB - ValueAtA);

														ABORD_IF_NOT(!FMath::IsNaN(t))

															if (bIsAlongX)
															{
																IntersectionPoint = FVector(t * PositionA.X + (1 - t) * PositionB.X, PositionB.Y, PositionB.Z);
															}
															else if (bIsAlongY)
															{
																IntersectionPoint = FVector(PositionB.X, t * PositionA.Y + (1 - t) * PositionB.Y, PositionB.Z);
															}
															else
															{
																check(bIsAlongZ);
																IntersectionPoint = FVector(PositionB.X, PositionB.Y, t * PositionA.Z + (1 - t) * PositionB.Z);
															}

														Alpha = t * AlphaAtA + (1 - t) * AlphaAtB;
													}
													else
													{
														// Interpolate

														bool bSuccess;

														if (bIsAlongX)
														{
															bSuccess = InterpolateX(PositionA.X, PositionB.X, PositionA.Y, PositionA.Z, IntersectionPoint, Alpha);
														}
														else if (bIsAlongY)
														{
															bSuccess = InterpolateY(PositionA.X, PositionA.Y, PositionB.Y, PositionA.Z, IntersectionPoint, Alpha);
														}
														else
														{
															check(bIsAlongZ);
															bSuccess = InterpolateZ(PositionA.X, PositionA.Y, PositionA.Z, PositionB.Z, IntersectionPoint, Alpha);
														}

														ABORD_IF_NOT(bSuccess)
													}
													// Use the material of the point inside
													FVoxelMaterial Material = (CornerValues[IndexVerticeA] <= 0) ? CornerMaterials[IndexVerticeA] : CornerMaterials[IndexVerticeB];
#if !MATERIALS_ARE_COLOR
													Material.Alpha = Alpha;
#endif
													IntersectionColor = Material.ToFColor();
												}

												bool bCreateVertex = true;

												// Check if vertice is exactly on the grid. Causes issues with tessellation
												if (FMath::Abs(IntersectionPoint.X / Step() - FMath::RoundToInt(IntersectionPoint.X / Step())) < KINDA_SMALL_NUMBER &&
													FMath::Abs(IntersectionPoint.Y / Step() - FMath::RoundToInt(IntersectionPoint.Y / Step())) < KINDA_SMALL_NUMBER &&
													FMath::Abs(IntersectionPoint.Z / Step() - FMath::RoundToInt(IntersectionPoint.Z / Step())) < KINDA_SMALL_NUMBER)
												{
													// Vertex is on the grid

													IntersectionPoint.X = FMath::RoundToInt(IntersectionPoint.X);
													IntersectionPoint.Y = FMath::RoundToInt(IntersectionPoint.Y);
													IntersectionPoint.Z = FMath::RoundToInt(IntersectionPoint.Z);

													const int IX = FMath::RoundToInt(IntersectionPoint.X / Step());
													const int IY = FMath::RoundToInt(IntersectionPoint.Y / Step());
													const int IZ = FMath::RoundToInt(IntersectionPoint.Z / Step());

													check(0 <= IX && IX < CHUNK_SIZE + 1);
													check(0 <= IY && IY < CHUNK_SIZE + 1);
													check(0 <= IZ && IZ < CHUNK_SIZE + 1);

													if (IntegerCoordinates[IX][IY][IZ] == -1)
													{
														// Not cached. Cache
														IntegerCoordinates[IX][IY][IZ] = Vertices.Num(); // VertexIndex == Vertices.Num()
													}
													else
													{
														VertexIndex = IntegerCoordinates[IX][IY][IZ];
														bCreateVertex = false;
													}
												}

												if (bCreateVertex)
												{
													VertexIndex = Vertices.Num();

													FVoxelVertex Vertex;
													Vertex.Position = IntersectionPoint;
													Vertex.Color = IntersectionColor;
													GetNormal(IntersectionPoint, Vertex.Normal);
													Vertices.Add(Vertex);
												}

												// Save vertex if not on edge
												if (CacheDirection & 0x08)
												{
													SaveVertex(X, Y, Z, EdgeIndex, VertexIndex);
												}
											}

											VertexIndices[i] = VertexIndex;
										}

										// Add triangles
										// 3 vertex per triangle
										int n = 3 * CellData.GetTriangleCount();
										for (int i = 0; i < n; i++)
										{
											Triangles.Add(VertexIndices[CellData.vertexIndex[i]]);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (LOD > 0)
		{
			Data->EndGet(Octrees);
		}
	}

	if(bCreateAdditionalVerticesForMaterialsTransitions)
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizer_CreateChunk_AddAlphaAdditionalVertices, VOXEL_MULTITHREAD_STAT);

		const int32 TriangleCount = OutChunk.IndexBuffer.Num();
		for (int32 TriangleIndex = 0; TriangleIndex < TriangleCount; TriangleIndex += 3)
		{
			int32 IndexOfA = OutChunk.IndexBuffer[TriangleIndex];
			int32 IndexOfB = OutChunk.IndexBuffer[TriangleIndex + 1];
			int32 IndexOfC = OutChunk.IndexBuffer[TriangleIndex + 2];

			TArray<int32> Vertices = { IndexOfA, IndexOfB, IndexOfC };
			TArray<int32> Indexes = { TriangleIndex, TriangleIndex + 1, TriangleIndex + 2 };

			FVoxelMaterial MatA = FVoxelMaterial(OutChunk.VertexBuffer[IndexOfA].Color);
			FVoxelMaterial MatB = FVoxelMaterial(OutChunk.VertexBuffer[IndexOfB].Color);
			FVoxelMaterial MatC = FVoxelMaterial(OutChunk.VertexBuffer[IndexOfC].Color);

			uint8 AloneIndex;
			if (!MatA.HasSameIndexesAs(MatB) && !MatB.HasSameIndexesAs(MatC) && !MatC.HasSameIndexesAs(MatA))
			{
				// Not exact, but gives good result
				AloneIndex = 0;
			}
			else if (MatA.HasSameIndexesAs(MatB) && MatB.HasSameIndexesAs(MatC) && MatC.HasSameIndexesAs(MatA))
			{
				// Nothing to do
				continue;
			}
			else if (MatA.HasSameIndexesAs(MatB) && !(MatC.Alpha == MatA.Alpha || MatC.Alpha == MatB.Alpha))
			{
				// C alone, A = B
				AloneIndex = 2;
			}
			else if (MatA.HasSameIndexesAs(MatC) && !(MatB.Alpha == MatA.Alpha || MatB.Alpha == MatC.Alpha))
			{
				// B alone, A = C
				AloneIndex = 1;
			}
			else if (MatB.HasSameIndexesAs(MatC) && !(MatA.Alpha == MatB.Alpha || MatA.Alpha == MatC.Alpha))
			{
				// A alone, B = C
				AloneIndex = 0;
			}
			else
			{
				// Same alpha
				continue;
			}
			uint8 OtherIndex1 = (AloneIndex + 1) % 3;
			uint8 OtherIndex2 = (AloneIndex + 2) % 3;

			const int32& IndexOfU = Vertices[AloneIndex];
			const int32& IndexOfV = Vertices[OtherIndex1];
			const int32& IndexOfW = Vertices[OtherIndex2];

			const FVoxelVertex& U = OutChunk.VertexBuffer[IndexOfU];
			const FVoxelVertex& V = OutChunk.VertexBuffer[IndexOfV];
			const FVoxelVertex& W = OutChunk.VertexBuffer[IndexOfW];

			/*
				   U
				/	 \
			  X	______ Y
			 /	\______ \
			/		   \_\
		   W ____________ V

			*/

			FVoxelVertex X;
			{
				X.Position = (U.Position + W.Position) / 2;
				X.Normal = (U.Normal + W.Normal).GetSafeNormal();
			}

			FVoxelVertex Y;
			{
				Y.Position = (U.Position + V.Position) / 2;
				Y.Normal = (U.Normal + V.Normal).GetSafeNormal();
			}

			// Build (U Y X)
			{
				FVoxelVertex NewVertex1 = Y;
				FVoxelVertex NewVertex2 = X;

				NewVertex1.Color = U.Color;
				NewVertex2.Color = U.Color;

				OutChunk.VertexBuffer.Add(NewVertex1);
				OutChunk.IndexBuffer[Indexes[OtherIndex1]] = OutChunk.VertexBuffer.Num() - 1;
				OutChunk.VertexBuffer.Add(NewVertex2);
				OutChunk.IndexBuffer[Indexes[OtherIndex2]] = OutChunk.VertexBuffer.Num() - 1;
			}

			// Build (V X Y)
			{
				FVoxelVertex NewVertex1 = X;
				FVoxelVertex NewVertex2 = Y;

				NewVertex1.Color = V.Color;
				NewVertex2.Color = V.Color;

				OutChunk.IndexBuffer.Add(IndexOfV);

				OutChunk.VertexBuffer.Add(NewVertex1);
				OutChunk.IndexBuffer.Add(OutChunk.VertexBuffer.Num() - 1);

				OutChunk.VertexBuffer.Add(NewVertex2);
				OutChunk.IndexBuffer.Add(OutChunk.VertexBuffer.Num() - 1);
			}

			// Build (W X V)
			{
				FVoxelVertex NewVertex1 = X;

				NewVertex1.Color = W.Color;

				OutChunk.IndexBuffer.Add(IndexOfW);

				OutChunk.VertexBuffer.Add(NewVertex1);
				OutChunk.IndexBuffer.Add(OutChunk.VertexBuffer.Num() - 1);

				OutChunk.IndexBuffer.Add(IndexOfV);
			}
		}
	}

	if (OutChunk.VertexBuffer.Num() < 3 || OutChunk.IndexBuffer.Num() == 0)
	{
		// Else physics thread crash
		OutChunk.Reset();
	}

	return true;
}

int FVoxelPolygonizer::Size() const
{
	return CHUNK_SIZE << LOD;
}

int FVoxelPolygonizer::Step() const
{
	return 1 << LOD;
}

void FVoxelPolygonizer::GetNormal(const FVector& Position, FVector& Result) const
{
	if (bEnableNormals ||
		Position.X < Step() ||
		Position.X >(CHUNK_SIZE - 1) * Step() ||
		Position.Y < Step() ||
		Position.Y >(CHUNK_SIZE - 1) * Step() ||
		Position.Z < Step() ||
		Position.Z >(CHUNK_SIZE - 1) * Step())
	{
		if (LOD == 0)
		{
			GetNormalImpl(this, Position, Step(), Result);
		}
		else
		{
			GetNormalImpl(Data, Position + (FVector)ChunkPosition, Step(), Result);
		}
	}
	else
	{
		Result = FVector::ZeroVector;
	}
}

void FVoxelPolygonizer::GetValueAndMaterialNoCache(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	check(LOD > 0);
	Data->GetValueAndMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, OutValue, OutMaterial);
}

void FVoxelPolygonizer::GetValueAndMaterialFromCache(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	check(0 <= X + 1 && X + 1 < CHUNK_SIZE + 3);
	check(0 <= Y + 1 && Y + 1 < CHUNK_SIZE + 3);
	check(0 <= Z + 1 && Z + 1 < CHUNK_SIZE + 3);

	int Index = (X + 1) + (CHUNK_SIZE + 3) * (Y + 1) + (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3) * (Z + 1);

	OutValue = CachedValues[Index];
	OutMaterial = CachedMaterials[Index];
}

float FVoxelPolygonizer::GetValue(int X, int Y, int Z) const
{
	check(LOD == 0);
	check(-1 <= X && X < CHUNK_SIZE + 2 && -1 <= Y && Y < CHUNK_SIZE + 2 && -1 <= Z && Z < CHUNK_SIZE + 2);

	return CachedValues[(X + 1) + (CHUNK_SIZE + 3) * (Y + 1) + (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3) * (Z + 1)];
}

void FVoxelPolygonizer::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index)
{
	// +1: normals offset
	check(0 <= X && X < CHUNK_SIZE);
	check(0 <= Y && Y < CHUNK_SIZE);
	check(0 <= Z && Z < CHUNK_SIZE);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	Cache[X][Y][Z][EdgeIndex] = Index;
}

int FVoxelPolygonizer::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	bool XIsDifferent = static_cast<bool>((Direction & 0x01) != 0);
	bool YIsDifferent = static_cast<bool>((Direction & 0x02) != 0);
	bool ZIsDifferent = static_cast<bool>((Direction & 0x04) != 0);

	// +1: normals offset
	check(0 <= X - XIsDifferent && X - XIsDifferent < CHUNK_SIZE);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < CHUNK_SIZE);
	check(0 <= Z - ZIsDifferent && Z - ZIsDifferent < CHUNK_SIZE);
	check(0 <= EdgeIndex && EdgeIndex < 3);


	check(Cache[X - XIsDifferent][Y - YIsDifferent][Z - ZIsDifferent][EdgeIndex] >= 0);
	return Cache[X - XIsDifferent][Y - YIsDifferent][Z - ZIsDifferent][EdgeIndex];
}

bool FVoxelPolygonizer::InterpolateX(int MinX, int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	while (MaxX - MinX != 1)
	{
		checkf((MaxX + MinX) % 2 == 0, TEXT("Invalid World Generator"));
		const int MiddleX = (MaxX + MinX) / 2;

		float ValueAtA;
		FVoxelMaterial MaterialAtA;
		GetValueAndMaterialNoCache(MinX, Y, Z, ValueAtA, MaterialAtA);

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterialNoCache(MiddleX, Y, Z, ValueAtMiddle, MaterialAtMiddle);

		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			MinX = MiddleX;
		}
		else
		{
			// If max and middle have same sign
			MaxX = MiddleX;
		}
	}

	// A: Min / B: Max
	float ValueAtA, ValueAtB;
	FVoxelMaterial MaterialAtA, MaterialAtB;
	GetValueAndMaterialNoCache(MinX, Y, Z, ValueAtA, MaterialAtA);
	GetValueAndMaterialNoCache(MaxX, Y, Z, ValueAtB, MaterialAtB);

	if (ValueAtA - ValueAtB == 0)
	{
		return false;
	}
	checkf(ValueAtA - ValueAtB != 0, TEXT("Invalid World Generator"));
	const float t = ValueAtB / (ValueAtB - ValueAtA);
	
	if (FMath::IsNaN(t))
	{
		return false;
	}

	OutVector = FVector(t * MinX + (1 - t) * MaxX, Y, Z);
	OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;

	return true;
}

bool FVoxelPolygonizer::InterpolateY(const int X, int MinY, int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	while (MaxY - MinY != 1)
	{
		checkf((MaxY + MinY) % 2 == 0, TEXT("Invalid World Generator"));
		const int MiddleY = (MaxY + MinY) / 2;

		float ValueAtA;
		FVoxelMaterial MaterialAtA;
		GetValueAndMaterialNoCache(X, MinY, Z, ValueAtA, MaterialAtA);

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterialNoCache(X, MiddleY, Z, ValueAtMiddle, MaterialAtMiddle);

		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			MinY = MiddleY;
		}
		else
		{
			// If max and middle have same sign
			MaxY = MiddleY;
		}
	}

	// A: Min / B: Max
	float ValueAtA, ValueAtB;
	FVoxelMaterial MaterialAtA, MaterialAtB;
	GetValueAndMaterialNoCache(X, MinY, Z, ValueAtA, MaterialAtA);
	GetValueAndMaterialNoCache(X, MaxY, Z, ValueAtB, MaterialAtB);
	
	if (ValueAtA - ValueAtB == 0)
	{
		return false;
	}
	checkf(ValueAtA - ValueAtB != 0, TEXT("Invalid World Generator"));
	const float t = ValueAtB / (ValueAtB - ValueAtA);

	if (FMath::IsNaN(t))
	{
		return false;
	}

	OutVector = FVector(X, t * MinY + (1 - t) * MaxY, Z);
	OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;

	return true;
}

bool FVoxelPolygonizer::InterpolateZ(const int X, const int Y, int MinZ, int MaxZ, FVector& OutVector, uint8& OutAlpha)
{
	while (MaxZ - MinZ != 1)
	{
		checkf((MaxZ + MinZ) % 2 == 0, TEXT("Invalid World Generator"));
		const int MiddleZ = (MaxZ + MinZ) / 2;

		float ValueAtA;
		FVoxelMaterial MaterialAtA;
		GetValueAndMaterialNoCache(X, Y, MinZ, ValueAtA, MaterialAtA);

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterialNoCache(X, Y, MiddleZ, ValueAtMiddle, MaterialAtMiddle);

		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			MinZ = MiddleZ;
		}
		else
		{
			// If max and middle have same sign
			MaxZ = MiddleZ;
		}
	}

	// A: Min / B: Max
	float ValueAtA, ValueAtB;
	FVoxelMaterial MaterialAtA, MaterialAtB;
	GetValueAndMaterialNoCache(X, Y, MinZ, ValueAtA, MaterialAtA);
	GetValueAndMaterialNoCache(X, Y, MaxZ, ValueAtB, MaterialAtB);
	
	if (ValueAtA - ValueAtB == 0)
	{
		return false;
	}
	checkf(ValueAtA - ValueAtB != 0, TEXT("Invalid World Generator"));
	const float t = ValueAtB / (ValueAtB - ValueAtA);

	if (FMath::IsNaN(t))
	{
		return false;
	}

	OutVector = FVector(X, Y, t * MinZ + (1 - t) * MaxZ);
	OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

FVoxelPolygonizerForTransitions::FVoxelPolygonizerForTransitions(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
{

}

bool FVoxelPolygonizerForTransitions::CreateTransitions(TArray<FVoxelProcMeshVertex>& OutVertexBuffer, TArray<int32>& OutIndexBuffer, uint8 TransitionsMask)
{
	if (!TransitionsMask)
	{
		return true;
	}

	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForTransitions_CreateTransitions, VOXEL_MULTITHREAD_STAT);
	
	FIntVector ChunkDataSize(CHUNK_SIZE + 1, CHUNK_SIZE + 1, CHUNK_SIZE + 1);
	const FIntBox Bounds = FIntBox(ChunkPosition - FIntVector(1, 1, 1), ChunkPosition + (ChunkDataSize - FIntVector(1, 1, 1)) * Step() + FIntVector(2, 2, 2)); // -1: for normals; +2 normals + max of intbox excluded

	TArray<uint64> Octrees;
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FVoxelPolygonizerForTransitions_CreateTransitions_BeginGet, VOXEL_MULTITHREAD_STAT);
		Octrees = Data->BeginGet(Bounds);
	}

	int DirectionIndex = -1;
	for (auto Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
	{
		DirectionIndex++;

		if (TransitionsMask & Direction)
		{
			for (int X = 0; X < CHUNK_SIZE; X++)
			{
				for (int Y = 0; Y < CHUNK_SIZE; Y++)
				{
					const int HalfStep = Step() / 2;

					float CornerValues[9];
					FVoxelMaterial CornerMaterials[9];

					GetValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[0], CornerMaterials[0]);
					GetValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[1], CornerMaterials[1]);
					GetValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[2], CornerMaterials[2]);
					GetValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[3], CornerMaterials[3]);
					GetValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[4], CornerMaterials[4]);
					GetValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[5], CornerMaterials[5]);
					GetValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[6], CornerMaterials[6]);
					GetValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[7], CornerMaterials[7]);
					GetValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[8], CornerMaterials[8]);

					unsigned long CaseCode =
						(static_cast<bool>(CornerValues[0] > 0) << 0)
						| (static_cast<bool>(CornerValues[1] > 0) << 1)
						| (static_cast<bool>(CornerValues[2] > 0) << 2)
						| (static_cast<bool>(CornerValues[5] > 0) << 3)
						| (static_cast<bool>(CornerValues[8] > 0) << 4)
						| (static_cast<bool>(CornerValues[7] > 0) << 5)
						| (static_cast<bool>(CornerValues[6] > 0) << 6)
						| (static_cast<bool>(CornerValues[3] > 0) << 7)
						| (static_cast<bool>(CornerValues[4] > 0) << 8);

					if (!(CaseCode == 0 || CaseCode == 511))
					{
						short ValidityMask = (X != 0) + 2 * (Y != 0);
						
						FIntVector Positions[13] = {
							FIntVector(2 * X + 0, 2 * Y + 0, 0) * HalfStep,
							FIntVector(2 * X + 1, 2 * Y + 0, 0) * HalfStep,
							FIntVector(2 * X + 2, 2 * Y + 0, 0) * HalfStep,
							FIntVector(2 * X + 0, 2 * Y + 1, 0) * HalfStep,
							FIntVector(2 * X + 1, 2 * Y + 1, 0) * HalfStep,
							FIntVector(2 * X + 2, 2 * Y + 1, 0) * HalfStep,
							FIntVector(2 * X + 0, 2 * Y + 2, 0) * HalfStep,
							FIntVector(2 * X + 1, 2 * Y + 2, 0) * HalfStep,
							FIntVector(2 * X + 2, 2 * Y + 2, 0) * HalfStep,

							FIntVector(2 * X + 0, 2 * Y + 0, 1) * HalfStep,
							FIntVector(2 * X + 2, 2 * Y + 0, 1) * HalfStep,
							FIntVector(2 * X + 0, 2 * Y + 2, 1) * HalfStep,
							FIntVector(2 * X + 2, 2 * Y + 2, 1) * HalfStep
						};

						check(0 <= CaseCode && CaseCode < 512);
						const unsigned char CellClass = Transvoxel::transitionCellClass[CaseCode];
						const unsigned short* VertexData = Transvoxel::transitionVertexData[CaseCode];
						check(0 <= (CellClass & 0x7F) && (CellClass & 0x7F) < 56);
						const Transvoxel::TransitionCellData CellData = Transvoxel::transitionCellData[CellClass & 0x7F];
						const bool bFlip = ((CellClass >> 7) != 0);

						TArray<int> VertexIndices;
						VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

						for (int i = 0; i < CellData.GetVertexCount(); i++)
						{
							int VertexIndex;
							const unsigned short& EdgeCode = VertexData[i];

							// A: low point / B: high point
							const unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
							const unsigned short IndexVerticeB = EdgeCode & 0x0F;

							check(0 <= IndexVerticeA && IndexVerticeA < 13);
							check(0 <= IndexVerticeB && IndexVerticeB < 13);

							const FIntVector& PositionA = Positions[IndexVerticeA];
							const FIntVector& PositionB = Positions[IndexVerticeB];

							const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
							// Direction to go to use an already created vertex
							const short CacheDirection = EdgeCode >> 12;

							if ((ValidityMask & CacheDirection) != CacheDirection)
							{
								// Validity check failed
								const bool bIsAlongX = EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8;
								const bool bIsAlongY = EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9;


								FVector Q;
								uint8 Alpha;
								bool bSuccess;

								if (bIsAlongX)
								{
									// Edge along X axis
									bSuccess = InterpolateX(Direction, PositionA.X, PositionB.X, PositionA.Y, Q, Alpha);
								}
								else if (bIsAlongY)
								{
									// Edge along Y axis
									bSuccess = InterpolateY(Direction, PositionA.X, PositionA.Y, PositionB.Y, Q, Alpha);
								}
								else
								{
									bSuccess = false;
									Alpha = 0;
									checkf(false, TEXT("Error in interpolation: case should not exist"));
								}

								ABORD_IF_NOT(bSuccess)

								VertexIndex = OutVertexBuffer.Num();

								FVoxelProcMeshVertex Vertex;
								GetNormal(Q, Vertex.Normal);
								Vertex.Position = (EdgeIndex == 8 || EdgeIndex == 9) ? FVoxelIntermediateChunk::GetTranslated(Q, Vertex.Normal, TransitionsMask, LOD) : Q;

								// Use the material of the point inside
								const TArray<int> Perm = { 0, 2, 6, 8 };
								int ValidIndexA = IndexVerticeA < 9 ? IndexVerticeA : Perm[IndexVerticeA - 9];
								int ValidIndexB = IndexVerticeB < 9 ? IndexVerticeB : Perm[IndexVerticeB - 9];
								check(0 <= ValidIndexA && ValidIndexA < 9 && 0 <= ValidIndexB && ValidIndexB < 9);
								FVoxelMaterial Material = (CornerValues[ValidIndexA] <= 0) ? CornerMaterials[ValidIndexA] : CornerMaterials[ValidIndexB];

								Material.Alpha = Alpha;
								Vertex.Color = Material.ToFColor();
								OutVertexBuffer.Add(Vertex);

								// If own vertex, save it
								if (CacheDirection & 0x08)
								{
									SaveVertex2D(DirectionIndex, X, Y, EdgeIndex, VertexIndex);
								}
							}
							else
							{
								VertexIndex = LoadVertex2D(DirectionIndex, X, Y, CacheDirection, EdgeIndex);
							}

							VertexIndices[i] = VertexIndex;
						}

						// Add triangles
						int n = 3 * CellData.GetTriangleCount();
						for (int i = 0; i < n; i++)
						{
							OutIndexBuffer.Add(VertexIndices[CellData.vertexIndex[bFlip ? (n - 1 - i) : i]]);
						}
					}
				}
			}
		}
	}

	Data->EndGet(Octrees);

	return true;
}

int FVoxelPolygonizerForTransitions::Size() const
{
	return CHUNK_SIZE << LOD;
}

int FVoxelPolygonizerForTransitions::Step() const
{
	return 1 << LOD;
}

void FVoxelPolygonizerForTransitions::SaveVertex2D(int Direction, int X, int Y, short EdgeIndex, int Index)
{
	check(0 <= X && X < CHUNK_SIZE + 1);
	check(0 <= Y && Y < CHUNK_SIZE + 1);
	check(0 <= EdgeIndex && EdgeIndex < 10);

	Cache2D[Direction][X][Y][EdgeIndex] = Index;
}

int FVoxelPolygonizerForTransitions::LoadVertex2D(int Direction, int X, int Y, short CacheDirection, short EdgeIndex) const
{
	bool XIsDifferent = static_cast<bool>((CacheDirection & 0x01) != 0);
	bool YIsDifferent = static_cast<bool>((CacheDirection & 0x02) != 0);

	check(0 <= X - XIsDifferent && X - XIsDifferent < CHUNK_SIZE + 1);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < CHUNK_SIZE + 1);
	check(0 <= EdgeIndex && EdgeIndex < 10);

	check(Cache2D[Direction][X - XIsDifferent][Y - YIsDifferent][EdgeIndex] >= 0);
	return Cache2D[Direction][X - XIsDifferent][Y - YIsDifferent][EdgeIndex];
}

void FVoxelPolygonizerForTransitions::GetNormal(const FVector& Position, FVector& Result) const
{
	GetNormalImpl(Data, Position + (FVector)ChunkPosition, Step(), Result);
}

void FVoxelPolygonizerForTransitions::GetValueAndMaterial(EVoxelDirection Direction, int X, int Y, float& OutValue, FVoxelMaterial& OutMaterial) const
{
	int GX, GY, GZ;
	Local2DToGlobal(Size(), Direction, X, Y, 0, GX, GY, GZ);

	Data->GetValueAndMaterial(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, OutValue, OutMaterial);
}

void FVoxelPolygonizerForTransitions::GlobalToLocal2D(int Size, EVoxelDirection Direction, int GX, int GY, int GZ, int& OutLX, int& OutLY, int& OutLZ) const
{
	const int& S = Size;
	switch (Direction)
	{
	case XMin:
		OutLX = GY;
		OutLY = GZ;
		OutLZ = GX;
		break;
	case XMax:
		OutLX = GZ;
		OutLY = GY;
		OutLZ = S - GX;
		break;
	case YMin:
		OutLX = GZ;
		OutLY = GX;
		OutLZ = GY;
		break;
	case YMax:
		OutLX = GX;
		OutLY = GZ;
		OutLZ = S - GY;
		break;
	case ZMin:
		OutLX = GX;
		OutLY = GY;
		OutLZ = GZ;
		break;
	case ZMax:
		OutLX = GY;
		OutLY = GX;
		OutLZ = S - GZ;
		break;
	default:
		check(false)
			break;
	}
}

void FVoxelPolygonizerForTransitions::Local2DToGlobal(int Size, EVoxelDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ) const
{
	const int& S = Size;
	switch (Direction)
	{
	case XMin:
		OutGX = LZ;
		OutGY = LX;
		OutGZ = LY;
		break;
	case XMax:
		OutGX = S - LZ;
		OutGY = LY;
		OutGZ = LX;
		break;
	case YMin:
		OutGX = LY;
		OutGY = LZ;
		OutGZ = LX;
		break;
	case YMax:
		OutGX = LX;
		OutGY = S - LZ;
		OutGZ = LY;
		break;
	case ZMin:
		OutGX = LX;
		OutGY = LY;
		OutGZ = LZ;
		break;
	case ZMax:
		OutGX = LY;
		OutGY = LX;
		OutGZ = S - LZ;
		break;
	default:
		check(false)
			break;
	}
}

bool FVoxelPolygonizerForTransitions::InterpolateX(EVoxelDirection Direction, int MinX, int MaxX, const int Y, FVector& OutVector, uint8& OutAlpha) const
{
	while (MaxX - MinX != 1)
	{
		checkf((MaxX + MinX) % 2 == 0, TEXT("Invalid World Generator"));
		int MiddleX = (MaxX + MinX) / 2;

		float ValueAtA;
		FVoxelMaterial MaterialAtA;
		GetValueAndMaterial(Direction, MinX, Y, ValueAtA, MaterialAtA);

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterial(Direction, MiddleX, Y, ValueAtMiddle, MaterialAtMiddle);

		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			MinX = MiddleX;
		}
		else
		{
			// If max and middle have same sign
			MaxX = MiddleX;
		}
	}

	// A: Min / B: Max
	float ValueAtA, ValueAtB;
	FVoxelMaterial MaterialAtA, MaterialAtB;
	GetValueAndMaterial(Direction, MinX, Y, ValueAtA, MaterialAtA);
	GetValueAndMaterial(Direction, MaxX, Y, ValueAtB, MaterialAtB);

	if (ValueAtA - ValueAtB == 0)
	{
		return false;
	}
	checkf(ValueAtA - ValueAtB != 0, TEXT("Invalid World Generator"));
	const float t = ValueAtB / (ValueAtB - ValueAtA);

	if (FMath::IsNaN(t))
	{
		return false;
	}

	int GMinX, GMaxX, GMinY, GMaxY, GMinZ, GMaxZ;
	Local2DToGlobal(Size(), Direction, MinX, Y, 0, GMinX, GMinY, GMinZ);
	Local2DToGlobal(Size(), Direction, MaxX, Y, 0, GMaxX, GMaxY, GMaxZ);

	OutVector.X = GMinX != GMaxX ? t * GMinX + (1 - t) * GMaxX : GMinX;
	OutVector.Y = GMinY != GMaxY ? t * GMinY + (1 - t) * GMaxY : GMinY;
	OutVector.Z = GMinZ != GMaxZ ? t * GMinZ + (1 - t) * GMaxZ : GMinZ;
	OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;

	return true;
}

bool FVoxelPolygonizerForTransitions::InterpolateY(EVoxelDirection Direction, const int X, int MinY, int MaxY, FVector& OutVector, uint8& OutAlpha) const
{
	while (MaxY - MinY != 1)
	{
		checkf((MaxY + MinY) % 2 == 0, TEXT("Invalid World Generator"));
		const int MiddleY = (MaxY + MinY) / 2;

		float ValueAtA;
		FVoxelMaterial MaterialAtA;
		GetValueAndMaterial(Direction, X, MinY, ValueAtA, MaterialAtA);

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterial(Direction, X, MiddleY, ValueAtMiddle, MaterialAtMiddle);

		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			MinY = MiddleY;
		}
		else
		{
			// If max and middle have same sign
			MaxY = MiddleY;
		}
	}

	// A: Min / B: Max
	float ValueAtA, ValueAtB;
	FVoxelMaterial MaterialAtA, MaterialAtB;
	GetValueAndMaterial(Direction, X, MinY, ValueAtA, MaterialAtA);
	GetValueAndMaterial(Direction, X, MaxY, ValueAtB, MaterialAtB);

	if (ValueAtA - ValueAtB == 0)
	{
		return false;
	}
	checkf(ValueAtA - ValueAtB != 0, TEXT("Invalid World Generator"));
	const float t = ValueAtB / (ValueAtB - ValueAtA);

	if (FMath::IsNaN(t))
	{
		return false;
	}

	int GMinX, GMaxX, GMinY, GMaxY, GMinZ, GMaxZ;
	Local2DToGlobal(Size(), Direction, X, MinY, 0, GMinX, GMinY, GMinZ);
	Local2DToGlobal(Size(), Direction, X, MaxY, 0, GMaxX, GMaxY, GMaxZ);
	
	OutVector.X = GMinX != GMaxX ? t * GMinX + (1 - t) * GMaxX : GMinX;
	OutVector.Y = GMinY != GMaxY ? t * GMinY + (1 - t) * GMaxY : GMinY;
	OutVector.Z = GMinZ != GMaxZ ? t * GMinZ + (1 - t) * GMaxZ : GMinZ;
	OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;

	return true;
}