// Copyright 2019 Phyronnaz

#include "VoxelRender/Polygonizers/VoxelMCPolygonizer.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "Transvoxel.h"

#define checkError(x) if(!(x)) goto generatorerror;

//////////////////////////////////////////////////////////////////////////

inline void GetClosestPoints(const FVector& Point, FIntVector& A, FIntVector& B)
{
	A = FIntVector(FMath::FloorToInt(Point.X), FMath::FloorToInt(Point.Y), FMath::FloorToInt(Point.Z));
	B = FIntVector(FMath::CeilToInt(Point.X), FMath::CeilToInt(Point.Y), FMath::CeilToInt(Point.Z));

	checkVoxelSlow(FMath::Abs((B - A).X) + FMath::Abs((B - A).Y) + FMath::Abs((B - A).Z) <= 1);
}

template<class T>
inline FVector GetNormalImpl(T* Data, int32 QueryLOD, const FVector& LocalPosition, const FIntVector& Offset)
{
	FIntVector A, B;
	GetClosestPoints(LocalPosition, A, B);

	FVector NormalA = FVoxelDataUtilities::GetGradient(*Data, A + Offset, QueryLOD);
	FVector NormalB = FVoxelDataUtilities::GetGradient(*Data, B + Offset, QueryLOD);

	float Alpha = (LocalPosition - (FVector)A).GetAbsMax();
	return FMath::Lerp(NormalA, NormalB, Alpha).GetSafeNormal();
}

///////////////////////////////////////////////////////////////////////////////

inline FVector2D GetUVs(EVoxelUVConfig UVConfig, float UVScale, FVoxelData* Data, const FIntVector& ChunkPosition, const FVector& IntersectionPoint, const FVoxelMaterial& Material)
{
	if (UVConfig == EVoxelUVConfig::PackWorldUpInUVs)
	{
		FVector WorldUp = Data->WorldGenerator->GetUpVector(ChunkPosition + FVoxelUtilities::RoundToInt(IntersectionPoint)).GetSafeNormal();
		return FVector2D(WorldUp.X, WorldUp.Y);
	}
	else if (UVConfig == EVoxelUVConfig::GlobalUVs)
	{
		return FVector2D(ChunkPosition.X + IntersectionPoint.X, ChunkPosition.Y + IntersectionPoint.Y) / UVScale;
	}
	else if (UVConfig == EVoxelUVConfig::PerVoxelUVs)
	{
		return FVector2D::ZeroVector;
	}
	else if (UVConfig == EVoxelUVConfig::UseRGAsUVs)
	{
		return FVector2D(Material.GetR(), Material.GetG());
	}
	else
	{
		check(UVConfig == EVoxelUVConfig::CustomFVoxelMaterial);
		return Material.GetUVs();
	}
}

///////////////////////////////////////////////////////////////////////////////

static const FIntVector MCChunkDataSize = FIntVector(MC_EXTENDED_CHUNK_SIZE);

FIntBox FVoxelMCPolygonizer::GetValuesBounds() const
{
	return FIntBox(ChunkPosition, ChunkPosition + MCChunkDataSize * Step);
}

FIntBox FVoxelMCPolygonizer::GetLockedBounds() const
{
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition + FIntVector(Step) + MCChunkDataSize * Step);
}

bool FVoxelMCPolygonizer::CreateChunk()
{
	Stats.StartStat("GetValuesAndMaterials");
	Data->GetValuesAndMaterials(CachedValues, nullptr, FVoxelWorldGeneratorQueryZone(GetValuesBounds(), MCChunkDataSize, LOD), LOD);

	Stats.StartStat("GetMap");
	MapAccelerator = MakeUnique<FVoxelDataUtilities::MapAccelerator>(GetLockedBounds(), Data);

	Stats.StartStat("Iteration");
	
	TArray<FLocalVoxelVertex> Vertices;
	TArray<uint32> Indices;

	uint32 Index = 0;
	for (int32 LZ = 0; LZ < CHUNK_SIZE; LZ++)
	{
		for (int32 LY = 0; LY < CHUNK_SIZE; LY++)
		{
			for (int32 LX = 0; LX < CHUNK_SIZE; LX++)
			{
				{
					GetCurrentCache()[GetCacheIndex(0, LX, LY)] = -1; // Set EdgeIndex 0 to -1 if the cell isn't voxelized, eg all corners = 0

					uint32 CubeIndices[8];
					CubeIndices[0] = Index;
					CubeIndices[1] = Index + 1;
					CubeIndices[2] = Index + MC_EXTENDED_CHUNK_SIZE;
					CubeIndices[3] = Index + 1 + MC_EXTENDED_CHUNK_SIZE;
					CubeIndices[4] = Index + MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE;
					CubeIndices[5] = Index + 1 + MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE;
					CubeIndices[6] = Index + MC_EXTENDED_CHUNK_SIZE + MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE;
					CubeIndices[7] = Index + 1 + MC_EXTENDED_CHUNK_SIZE + MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE;

					checkVoxelSlow(CubeIndices[0] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[1] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[2] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[3] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[4] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[5] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[6] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);
					checkVoxelSlow(CubeIndices[7] < MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE);

					unsigned long CaseCode =
						(CachedValues[CubeIndices[0]].IsEmpty() << 0) |
						(CachedValues[CubeIndices[1]].IsEmpty() << 1) |
						(CachedValues[CubeIndices[2]].IsEmpty() << 2) |
						(CachedValues[CubeIndices[3]].IsEmpty() << 3) |
						(CachedValues[CubeIndices[4]].IsEmpty() << 4) |
						(CachedValues[CubeIndices[5]].IsEmpty() << 5) |
						(CachedValues[CubeIndices[6]].IsEmpty() << 6) |
						(CachedValues[CubeIndices[7]].IsEmpty() << 7);

					if (CaseCode != 0 && CaseCode != 255)
					{
						// Cell has a nontrivial triangulation

						short ValidityMask = (LX != 0) + 2 * (LY != 0) + 4 * (LZ != 0);

						checkVoxelSlow(0 <= CaseCode && CaseCode < 256);
						const uint8 CellClass = Transvoxel::regularCellClass[CaseCode];
						const uint16* VertexData = Transvoxel::regularVertexData[CaseCode];
						checkVoxelSlow(0 <= CellClass && CellClass < 16);
						Transvoxel::RegularCellData CellData = Transvoxel::regularCellData[CellClass];

						// Indices of the vertices used in this cube
						TArray<int32, TFixedAllocator<16>> VertexIndices;
						VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

						for (int32 I = 0; I < CellData.GetVertexCount(); I++)
						{
							int32 VertexIndex = -2;
							const uint16 EdgeCode = VertexData[I];

							// A: low point / B: high point
							const uint8 LocalIndexA = (EdgeCode >> 4) & 0x0F;
							const uint8 LocalIndexB = EdgeCode & 0x0F;

							checkVoxelSlow(0 <= LocalIndexA && LocalIndexA < 8);
							checkVoxelSlow(0 <= LocalIndexB && LocalIndexB < 8);

							const uint32 IndexA = CubeIndices[LocalIndexA];
							const uint32 IndexB = CubeIndices[LocalIndexB];

							const FVoxelValue& ValueAtA = CachedValues[IndexA];
							const FVoxelValue& ValueAtB = CachedValues[IndexB];

							checkVoxelSlow(ValueAtA.IsEmpty() != ValueAtB.IsEmpty());

							uint8 EdgeIndex = ((EdgeCode >> 8) & 0x0F);
							checkVoxelSlow(1 <= EdgeIndex && EdgeIndex < 4);

							// Direction to go to use an already created vertex: 
							// first bit:  x is different
							// second bit: y is different
							// third bit:  z is different
							// fourth bit: vertex isn't cached
							uint8 CacheDirection = EdgeCode >> 12;

							if (ValueAtA.IsNull())
							{
								EdgeIndex = 0;
								CacheDirection = LocalIndexA ^ 7;
							}
							if (ValueAtB.IsNull())
							{
								checkVoxelSlow(!ValueAtA.IsNull());
								EdgeIndex = 0;
								CacheDirection = LocalIndexB ^ 7;
							}

							const bool bIsVertexCached = ((ValidityMask & CacheDirection) == CacheDirection) && CacheDirection; // CacheDirection == 0 => LocalIndexB = 0 (as only B can be = 7) and ValueAtB = 0

							if (bIsVertexCached)
							{
								checkVoxelSlow(!(CacheDirection & 0x08));

								bool XIsDifferent = !!(CacheDirection & 0x01);
								bool YIsDifferent = !!(CacheDirection & 0x02);
								bool ZIsDifferent = !!(CacheDirection & 0x04);
								
								VertexIndex = (ZIsDifferent ? GetOldCache() : GetCurrentCache())[GetCacheIndex(EdgeIndex, LX - XIsDifferent, LY - YIsDifferent)];
								checkVoxelSlow(-1 <= VertexIndex && VertexIndex < Vertices.Num());
							}

							if (!bIsVertexCached || VertexIndex == -1)
							{
								// We are on one the lower edges of the chunk. Compute vertex
							
								const FIntVector PositionA((LX + (LocalIndexA & 0x01)) * Step, (LY + ((LocalIndexA & 0x02) >> 1)) * Step, (LZ + ((LocalIndexA & 0x04) >> 2)) * Step);
								const FIntVector PositionB((LX + (LocalIndexB & 0x01)) * Step, (LY + ((LocalIndexB & 0x02) >> 1)) * Step, (LZ + ((LocalIndexB & 0x04) >> 2)) * Step);

								FVector IntersectionPoint;
								FVoxelMaterial Material;

								if (EdgeIndex == 0)
								{
									if (ValueAtA.IsNull())
									{
										IntersectionPoint = FVector(PositionA);
										Material = GetMaterialNoCache(PositionA);
									}
									else 
									{
										checkVoxelSlow(ValueAtB.IsNull());
										IntersectionPoint = FVector(PositionB);
										Material = GetMaterialNoCache(PositionB);
									}
								}
								else if (LOD == 0)
								{
									// Full resolution

									bool bSuccess;
									const float Alpha = ValueAtA.ThisDividedByThisMinusA(ValueAtB, bSuccess);
									checkError(bSuccess);
									
									switch (EdgeIndex)
									{
									case 2: // X
										IntersectionPoint = FVector(FMath::Lerp<float>(PositionA.X, PositionB.X, Alpha), PositionA.Y, PositionA.Z);
										break;
									case 1: // Y
										IntersectionPoint = FVector(PositionA.X, FMath::Lerp<float>(PositionA.Y, PositionB.Y, Alpha), PositionA.Z);
										break;
									case 3: // Z
										IntersectionPoint = FVector(PositionA.X, PositionA.Y, FMath::Lerp<float>(PositionA.Z, PositionB.Z, Alpha));
										break;
									}

									// Use the material of the point inside
									// TODO: DO THIS AT THE END!!!!!!!
									Material = GetMaterialNoCache(!ValueAtA.IsEmpty() ? PositionA : PositionB);
								}
								else
								{
									// Interpolate

									const bool bIsAlongX = (EdgeIndex == 2);
									const bool bIsAlongY = (EdgeIndex == 1);
									const bool bIsAlongZ = (EdgeIndex == 3);

									checkVoxelSlow(!bIsAlongX || (PositionA.Y == PositionB.Y && PositionA.Z == PositionB.Z));
									checkVoxelSlow(!bIsAlongY || (PositionA.X == PositionB.X && PositionA.Z == PositionB.Z));
									checkVoxelSlow(!bIsAlongZ || (PositionA.X == PositionB.X && PositionA.Y == PositionB.Y));

									int32 Min = bIsAlongX ? PositionA.X : bIsAlongY ? PositionA.Y : PositionA.Z;
									int32 Max = bIsAlongX ? PositionB.X : bIsAlongY ? PositionB.Y : PositionB.Z;

									FVoxelValue ValueAtACopy = ValueAtA;
									FVoxelValue ValueAtBCopy = ValueAtB;

									while (Max - Min != 1)
									{
										checkError((Max + Min) % 2 == 0);
										const int32 Middle = (Max + Min) / 2;

										FVoxelValue ValueAtMiddle = GetValueNoCache(bIsAlongX ? Middle : PositionA.X, bIsAlongY ? Middle : PositionA.Y, bIsAlongZ ? Middle : PositionA.Z);

										if (ValueAtACopy.IsEmpty() == ValueAtMiddle.IsEmpty())
										{
											// If min and middle have same sign
											Min = Middle;
											ValueAtACopy = ValueAtMiddle;
										}
										else
										{
											// If max and middle have same sign
											Max = Middle;
											ValueAtBCopy = ValueAtMiddle;
										}

										checkError(Min <= Max);
									}

									bool bSuccess;
									const float Alpha = ValueAtACopy.ThisDividedByThisMinusA(ValueAtBCopy, bSuccess);
									checkError(bSuccess);

									float R = FMath::Lerp<float>(Min, Max, Alpha);

									IntersectionPoint = FVector(bIsAlongX ? R : PositionA.X, bIsAlongY ? R : PositionA.Y, bIsAlongZ ? R : PositionA.Z);

									// Get intersection material
									if (!ValueAtACopy.IsEmpty())
									{
										// TODO: DO THIS AT THE END!!!!!!!
										checkVoxelSlow(ValueAtBCopy.IsEmpty());
										Material = GetMaterialNoCache(bIsAlongX ? Min : PositionA.X, bIsAlongY ? Min : PositionA.Y, bIsAlongZ ? Min : PositionA.Z);
									}
									else
									{
										checkVoxelSlow(!ValueAtBCopy.IsEmpty());
										Material = GetMaterialNoCache(bIsAlongX ? Max : PositionA.X, bIsAlongY ? Max : PositionA.Y, bIsAlongZ ? Max : PositionA.Z);
									}
								}

								VertexIndex = Vertices.Num();

								FLocalVoxelVertex Vertex(NormalConfig);
								Vertex.Position = IntersectionPoint;
								Vertex.Material = Material;
								Vertex.UVs = GetUVs(UVConfig, UVScale, Data, ChunkPosition, IntersectionPoint, Material);

								if (NormalConfig == EVoxelNormalConfig::GradientNormal
									|| Vertex.Position.X < Step
									|| Vertex.Position.Y < Step
									|| Vertex.Position.Z < Step
									|| Vertex.Position.X > (CHUNK_SIZE - 1) * Step
									|| Vertex.Position.Y > (CHUNK_SIZE - 1) * Step
									|| Vertex.Position.Z > (CHUNK_SIZE - 1) * Step)
								{
									Vertex.GradientNormal = GetNormal(Vertex.Position);
									// Required for continuity & transitions
									Vertex.bForceUseGradient = true;
								}

								Vertices.Add(Vertex);

								checkVoxelSlow((ValueAtB.IsNull() && LocalIndexB == 7) == !CacheDirection);
								checkVoxelSlow(CacheDirection || EdgeIndex == 0);

								// Save vertex if not on edge
								if (CacheDirection & 0x08 || !CacheDirection) // ValueAtB.IsNull() && LocalIndexB == 7 => !CacheDirection
								{
									GetCurrentCache()[GetCacheIndex(EdgeIndex, LX, LY)] = VertexIndex;
								}
							}

							VertexIndices[I] = VertexIndex;
							checkVoxelSlow(0 <= VertexIndex && VertexIndex < Vertices.Num());
						}

						// Add triangles
						// 3 vertex per triangle
						const int32 N = 3 * CellData.GetTriangleCount();
						for (int32 I = 0; I < N; I += 3)
						{
							const int32& IndexA = VertexIndices[CellData.vertexIndex[I]];
							const int32& IndexB = VertexIndices[CellData.vertexIndex[I + 1]];
							const int32& IndexC = VertexIndices[CellData.vertexIndex[I + 2]];
							FLocalVoxelVertex& A = Vertices[IndexA];
							FLocalVoxelVertex& B = Vertices[IndexB];
							FLocalVoxelVertex& C = Vertices[IndexC];
							if (A.Position != B.Position && B.Position != C.Position && C.Position != A.Position)
							{
								// Else sweeps crash PhysX
								Indices.Add(IndexA);
								Indices.Add(IndexB);
								Indices.Add(IndexC);

								if (NormalConfig == EVoxelNormalConfig::MeshNormal)
								{
									FVector Normal = FVector::CrossProduct(C.Position - A.Position, B.Position - A.Position).GetSafeNormal();
									A.NormalSum += Normal;
									B.NormalSum += Normal;
									C.NormalSum += Normal;
								}
							}
						}
					}
				}

				Index++;

				if (LX == CHUNK_SIZE - 1)
				{
					Index += 1;
				}
			}
			if (LY == CHUNK_SIZE - 1)
			{
				Index += MC_EXTENDED_CHUNK_SIZE;
			}
		}
		if (LZ == CHUNK_SIZE - 1)
		{
			Index += MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE;
		}
		bCurrentCacheIs0 = !bCurrentCacheIs0;
	}

	Stats.StartStat("UnlockRead");
	Data->Unlock<EVoxelLockType::Read>(Octrees);

	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArrays(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), Chunk, Stats);

	return true;

generatorerror: 
	Data->Unlock<EVoxelLockType::Read>(Octrees);
	return false;
}

FVector FVoxelMCPolygonizer::GetNormal(const FVector& Position) const
{
	return GetNormalImpl(MapAccelerator.Get(), LOD, Position, ChunkPosition);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntBox FVoxelMCTransitionsPolygonizer::GetBounds() const
{
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition + FIntVector(Step) + MCChunkDataSize * Step);
}

bool FVoxelMCTransitionsPolygonizer::CreateTransitions()
{
	Stats.StartStat("GetMap");
	MapAccelerator = MakeUnique<FVoxelDataUtilities::MapAccelerator>(GetBounds(), Data);

	Stats.StartStat("Iteration");

#if VOXEL_DEBUG
	for (auto& Value : Cache2D)
	{
		Value = -100;
	}
#endif

	TArray<FLocalVoxelVertex> Vertices;
	TArray<uint32> Indices;

	int32 DirectionIndex = -1;
	for (auto& Direction : { XMin, XMax, YMin, YMax, ZMin, ZMax })
	{
		DirectionIndex++;

		if (TransitionsMask & Direction)
		{
			for (int32 LX = 0; LX < CHUNK_SIZE; LX++)
			{
				for (int32 LY = 0; LY < CHUNK_SIZE; LY++)
				{
					// Set EdgeIndex 0, 1, 2 and 7 to -1 for when the cell aren't polygonized (0 on all corners)
					Cache2D[GetCacheIndex(0, LX, LY)] = -1;
					Cache2D[GetCacheIndex(1, LX, LY)] = -1;
					Cache2D[GetCacheIndex(2, LX, LY)] = -1;
					Cache2D[GetCacheIndex(7, LX, LY)] = -1;

					FVoxelValue CornerValues[13];

					CornerValues[0] = GetValue(Direction, (2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
					CornerValues[1] = GetValue(Direction, (2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
					CornerValues[2] = GetValue(Direction, (2 * LX + 2) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
					CornerValues[3] = GetValue(Direction, (2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
					CornerValues[4] = GetValue(Direction, (2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
					CornerValues[5] = GetValue(Direction, (2 * LX + 2) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
					CornerValues[6] = GetValue(Direction, (2 * LX + 0) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);
					CornerValues[7] = GetValue(Direction, (2 * LX + 1) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);
					CornerValues[8] = GetValue(Direction, (2 * LX + 2) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);

					CornerValues[9] = GetValue(Direction, (LX + 0) * Step, (LY + 0) * Step, LOD);
					CornerValues[10] = GetValue(Direction, (LX + 1) * Step, (LY + 0) * Step, LOD);
					CornerValues[11] = GetValue(Direction, (LX + 0) * Step, (LY + 1) * Step, LOD);
					CornerValues[12] = GetValue(Direction, (LX + 1) * Step, (LY + 1) * Step, LOD);

					if (CornerValues[9].IsEmpty() != CornerValues[0].IsEmpty())
					{
						CornerValues[9] = CornerValues[0];
					}
					if (CornerValues[10].IsEmpty() != CornerValues[2].IsEmpty())
					{
						CornerValues[10] = CornerValues[2];
					}
					if (CornerValues[11].IsEmpty() != CornerValues[6].IsEmpty())
					{
						CornerValues[11] = CornerValues[6];
					}
					if (CornerValues[12].IsEmpty() != CornerValues[8].IsEmpty())
					{
						CornerValues[12] = CornerValues[8];
					}

					unsigned long CaseCode =
						(CornerValues[0].IsEmpty() << 0)
						| (CornerValues[1].IsEmpty() << 1)
						| (CornerValues[2].IsEmpty() << 2)
						| (CornerValues[5].IsEmpty() << 3)
						| (CornerValues[8].IsEmpty() << 4)
						| (CornerValues[7].IsEmpty() << 5)
						| (CornerValues[6].IsEmpty() << 6)
						| (CornerValues[3].IsEmpty() << 7)
						| (CornerValues[4].IsEmpty() << 8);

					if (!(CaseCode == 0 || CaseCode == 511))
					{
						short ValidityMask = (LX != 0) + 2 * (LY != 0);

						FIntVector Positions[13] = {
							FIntVector(2 * LX + 0, 2 * LY + 0, 0) * HalfStep,
							FIntVector(2 * LX + 1, 2 * LY + 0, 0) * HalfStep,
							FIntVector(2 * LX + 2, 2 * LY + 0, 0) * HalfStep,
							FIntVector(2 * LX + 0, 2 * LY + 1, 0) * HalfStep,
							FIntVector(2 * LX + 1, 2 * LY + 1, 0) * HalfStep,
							FIntVector(2 * LX + 2, 2 * LY + 1, 0) * HalfStep,
							FIntVector(2 * LX + 0, 2 * LY + 2, 0) * HalfStep,
							FIntVector(2 * LX + 1, 2 * LY + 2, 0) * HalfStep,
							FIntVector(2 * LX + 2, 2 * LY + 2, 0) * HalfStep,

							FIntVector(2 * LX + 0, 2 * LY + 0, 1) * HalfStep,
							FIntVector(2 * LX + 2, 2 * LY + 0, 1) * HalfStep,
							FIntVector(2 * LX + 0, 2 * LY + 2, 1) * HalfStep,
							FIntVector(2 * LX + 2, 2 * LY + 2, 1) * HalfStep
						};

						checkVoxelSlow(0 <= CaseCode && CaseCode < 512);
						const uint8 CellClass = Transvoxel::transitionCellClass[CaseCode];
						const uint16* VertexData = Transvoxel::transitionVertexData[CaseCode];
						checkVoxelSlow(0 <= (CellClass & 0x7F) && (CellClass & 0x7F) < 56);
						const Transvoxel::TransitionCellData CellData = Transvoxel::transitionCellData[CellClass & 0x7F];
						const bool bFlip = ((CellClass >> 7) != 0);

						TArray<int32> VertexIndices;
						VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

						for (int32 i = 0; i < CellData.GetVertexCount(); i++)
						{
							int32 VertexIndex = -1;
							const uint16& EdgeCode = VertexData[i];

							// A: low point / B: high point
							const uint8 IndexVerticeA = (EdgeCode >> 4) & 0x0F;
							const uint8 IndexVerticeB = EdgeCode & 0x0F;

							checkVoxelSlow(0 <= IndexVerticeA && IndexVerticeA < 13);
							checkVoxelSlow(0 <= IndexVerticeB && IndexVerticeB < 13);

							const FIntVector& PositionA = Positions[IndexVerticeA];
							const FIntVector& PositionB = Positions[IndexVerticeB];
								
							const FVoxelValue& ValueAtA = CornerValues[IndexVerticeA];
							const FVoxelValue& ValueAtB = CornerValues[IndexVerticeB];

							uint8 EdgeIndex = (EdgeCode >> 8) & 0x0F;
							checkVoxelSlow(EdgeIndex < 10);
							// Direction to go to use an already created vertex
							// First bit: x is different
							// Second bit: y is different
							// Third bit: interior edge, never cached
							// Fourth bit: own edge, need to create
							uint8 CacheDirection = EdgeCode >> 12;

							if (!(CacheDirection & 0x04)) // If not interior edge
							{
								static uint8 CacheDirectionMap[13] = {3, 2, 2, 1, 4, 8, 1, 8, 8, 3, 2, 1, 8};
								if (ValueAtA.IsNull())
								{
									static uint8 EdgeIndexMap[10] = {0, 1, 2, 0, 1, 0, 2, 7, 7, 7};
									EdgeIndex = EdgeIndexMap[EdgeIndex];
									CacheDirection = CacheDirectionMap[IndexVerticeA];
								}
								if (ValueAtB.IsNull())
								{
									checkVoxelSlow(!ValueAtA.IsNull());
									static uint8 EdgeIndexMap[10] = {0, 1, 2, 1, 0, 2, 0, 7, 7, 7};
									EdgeIndex = EdgeIndexMap[EdgeIndex];
									CacheDirection = CacheDirectionMap[IndexVerticeB];
								}
							}
							const bool bIsVertexCached = ((ValidityMask & CacheDirection) == CacheDirection);

							if (bIsVertexCached)
							{
								checkVoxelSlow(!(CacheDirection & 0x08) && !(CacheDirection & 0x04));

								bool XIsDifferent = !!(CacheDirection & 0x01);
								bool YIsDifferent = !!(CacheDirection & 0x02);
								
								VertexIndex = Cache2D[GetCacheIndex(EdgeIndex, LX - XIsDifferent, LY - YIsDifferent)];
								checkVoxelSlow(-1 <= VertexIndex && VertexIndex < Vertices.Num());
							}

							if (!bIsVertexCached || VertexIndex == -1)
							{
								FVector IntersectionPoint;
								FVoxelMaterial Material;
								
								const bool bIsLowResChunk = EdgeIndex == 7 || EdgeIndex == 8 || EdgeIndex == 9;

								if (EdgeIndex == 0 || EdgeIndex == 1 || EdgeIndex == 2 || EdgeIndex == 7)
								{
									if (ValueAtA.IsNull())
									{
										int32 GX, GY, GZ;
										Local2DToGlobal(Direction, PositionA.X, PositionA.Y, 0, GX, GY, GZ);
										IntersectionPoint = FVector(GX, GY, GZ);
										Material = GetMaterial(GX, GY, GZ);
									}
									else
									{
										checkVoxelSlow(ValueAtB.IsNull());
										int32 GX, GY, GZ;
										Local2DToGlobal(Direction, PositionB.X, PositionB.Y, 0, GX, GY, GZ);
										IntersectionPoint = FVector(GX, GY, GZ);
										Material = GetMaterial(GX, GY, GZ);
									}
								}
								else
								{
									const bool bIsAlongX = EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8;
									const bool bIsAlongY = EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9;

									checkVoxelSlow((bIsAlongX && !bIsAlongY) || (!bIsAlongX && bIsAlongY));

									int32 Min = bIsAlongX ? PositionA.X : PositionA.Y;
									int32 Max = bIsAlongX ? PositionB.X : PositionB.Y;
									
									FVoxelValue ValueAtACopy = ValueAtA;
									FVoxelValue ValueAtBCopy = ValueAtB;

									while (Max - Min != 1)
									{
										checkError((Max + Min) % 2 == 0);
										int32 Middle = (Max + Min) / 2;

										FVoxelValue ValueAtMiddle = GetValue(Direction, bIsAlongX ? Middle : PositionA.X, bIsAlongY ? Middle : PositionA.Y, bIsLowResChunk ? LOD : HalfLOD);

										if (ValueAtACopy.IsEmpty() == ValueAtMiddle.IsEmpty())
										{
											// If min and middle have same sign
											Min = Middle;
											ValueAtACopy = ValueAtMiddle;
										}
										else
										{
											// If max and middle have same sign
											Max = Middle;
											ValueAtBCopy = ValueAtMiddle;
										}

										checkError(Min <= Max);
									}

									bool bSuccess;
									const float Alpha = ValueAtBCopy.ThisDividedByThisMinusA(ValueAtACopy, bSuccess);
									checkError(bSuccess);

									int32 GMinX, GMaxX, GMinY, GMaxY, GMinZ, GMaxZ;
									Local2DToGlobal(Direction, bIsAlongX ? Min : PositionA.X, bIsAlongY ? Min : PositionA.Y, 0, GMinX, GMinY, GMinZ);
									Local2DToGlobal(Direction, bIsAlongX ? Max : PositionA.X, bIsAlongY ? Max : PositionA.Y, 0, GMaxX, GMaxY, GMaxZ);

									IntersectionPoint.X = GMinX != GMaxX ? Alpha * GMinX + (1 - Alpha) * GMaxX : GMinX;
									IntersectionPoint.Y = GMinY != GMaxY ? Alpha * GMinY + (1 - Alpha) * GMaxY : GMinY;
									IntersectionPoint.Z = GMinZ != GMaxZ ? Alpha * GMinZ + (1 - Alpha) * GMaxZ : GMinZ;

									// Get intersection material
									if (!ValueAtACopy.IsEmpty())
									{
										checkVoxelSlow(ValueAtBCopy.IsEmpty());
										Material = GetMaterial(GMinX, GMinY, GMinZ);
									}
									else
									{
										checkVoxelSlow(!ValueAtBCopy.IsEmpty());
										Material = GetMaterial(GMaxX, GMaxY, GMaxZ);
									}
								}

								VertexIndex = Vertices.Num();								

								FVector Normal = GetNormal(IntersectionPoint);
								FVector FinalPoint = bIsLowResChunk ? FVoxelRenderUtilities::GetTranslated(IntersectionPoint, Normal, TransitionsMask, LOD) : IntersectionPoint;

								FLocalVoxelVertex Vertex(NormalConfig);
								Vertex.Position = FinalPoint;
								Vertex.GradientNormal = Normal;
								Vertex.Material = Material;
								Vertex.UVs = GetUVs(UVConfig, UVScale, Data, ChunkPosition, FinalPoint, Material);

								Vertices.Add(Vertex);

								// If own vertex, save it
								if (CacheDirection & 0x08)
								{
									Cache2D[GetCacheIndex(EdgeIndex, LX, LY)] = VertexIndex;
								}
							}

							VertexIndices[i] = VertexIndex;
							checkVoxelSlow(0 <= VertexIndex && VertexIndex < Vertices.Num());
						}

						// Add triangles
						const int32 N = 3 * CellData.GetTriangleCount();
						for (int32 I = 0; I < N; I += 3)
						{
							const int32& IndexA = VertexIndices[CellData.vertexIndex[bFlip ? (N - 1 - (I + 0)) : (I + 0)]];
							const int32& IndexB = VertexIndices[CellData.vertexIndex[bFlip ? (N - 1 - (I + 1)) : (I + 1)]];
							const int32& IndexC = VertexIndices[CellData.vertexIndex[bFlip ? (N - 1 - (I + 2)) : (I + 2)]];
							FLocalVoxelVertex& A = Vertices[IndexA];
							FLocalVoxelVertex& B = Vertices[IndexB];
							FLocalVoxelVertex& C = Vertices[IndexC];
							if (A.Position != B.Position && B.Position != C.Position && C.Position != A.Position)
							{
								// Else sweeps crash PhysX
								Indices.Add(IndexA);
								Indices.Add(IndexB);
								Indices.Add(IndexC);
							}
						}
					}
				}
			}
		}
	}

	Stats.StartStat("UnlockRead");
	Data->Unlock<EVoxelLockType::Read>(Octrees);
	
	Stats.StartStat("ConvertArrays");
	FVoxelRenderUtilities::ConvertArrays(LOD, MaterialConfig, MeshParameters, MoveTemp(Indices), MoveTemp(Vertices), Chunk, Stats);

	return true;
	
generatorerror: 
	Data->Unlock<EVoxelLockType::Read>(Octrees);
	return false;
}

FVector FVoxelMCTransitionsPolygonizer::GetNormal(const FVector& Position) const
{
	return GetNormalImpl(MapAccelerator.Get(), LOD, Position, ChunkPosition);
}
