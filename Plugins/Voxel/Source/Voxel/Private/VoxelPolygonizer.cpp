#include "VoxelPrivatePCH.h"
#include "VoxelPolygonizer.h"
#include "Transvoxel.h"
#include "VoxelData.h"
#include "VoxelData/Private/ValueOctree.h"
#include "VoxelMaterial.h"

DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ Cache"), STAT_CACHE, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ Main Iter"), STAT_MAIN_ITER, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ Transitions Iter"), STAT_TRANSITIONS_ITER, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ CreateSection"), STAT_CREATE_SECTION, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ Add transitions to Section"), STAT_ADD_TRANSITIONS_TO_SECTION, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ MajorColor"), STAT_MAJOR_COLOR, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ GetValueAndColor"), STAT_GETVALUEANDCOLOR, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelPolygonizer ~ Get2DValueAndColor"), STAT_GET2DVALUEANDCOLOR, STATGROUP_Voxel);

FVoxelPolygonizer::FVoxelPolygonizer(int Depth, FVoxelData* Data, FIntVector ChunkPosition, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, bool bComputeTransitions, bool bComputeCollisions)
	: Depth(Depth)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, ChunkHasHigherRes(ChunkHasHigherRes)
	, bComputeTransitions(bComputeTransitions)
	, bComputeCollisions(bComputeCollisions)
	, LastOctree(nullptr)
{

}

void FVoxelPolygonizer::CreateSection(FProcMeshSection& OutSection)
{
	Data->BeginGet();
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
								// -1: offset because of normals computations
								const int X = 3 * CubeX + LocalX - 1;
								const int Y = 3 * CubeY + LocalY - 1;
								const int Z = 3 * CubeZ + LocalZ - 1;

								const uint64 ONE = 1;
								uint64 CurrentBit = ONE << (LocalX + 4 * LocalY + 4 * 4 * LocalZ);

								float CurrentValue;
								FVoxelMaterial CurrentMaterial;
								Data->GetValueAndMaterial(X * Step() + ChunkPosition.X, Y * Step() + ChunkPosition.Y, Z * Step() + ChunkPosition.Z, CurrentValue, CurrentMaterial, LastOctree);

								if (X + 1 < 18 && Y + 1 < 18 && Z + 1 < 18) // Getting value out of this chunk for the "continue" optimization after
								{
									CachedValues[(X + 1) + 18 * (Y + 1) + 18 * 18 * (Z + 1)] = CurrentValue;
									CachedMaterials[(X + 1) + 18 * (Y + 1) + 18 * 18 * (Z + 1)] = CurrentMaterial;
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
		SCOPE_CYCLE_COUNTER(STAT_MAIN_ITER);
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
									  (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) != 0) << 0)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 0)))) != 0) << 1)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) != 0) << 2)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 0)))) != 0) << 3)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) != 0) << 4)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 0) + 4 * 4 * (LocalZ + 1)))) != 0) << 5)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 0) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) != 0) << 6)
									| (static_cast<bool>((CurrentCube & (ONE << ((LocalX + 1) + 4 * (LocalY + 1) + 4 * 4 * (LocalZ + 1)))) != 0) << 7);

								if (CaseCode != 0 && CaseCode != 511)
								{
									// Cell has a nontrivial triangulation

									// -1: offset because of normals computations
									const int X = 3 * CubeX + LocalX - 1;
									const int Y = 3 * CubeY + LocalY - 1;
									const int Z = 3 * CubeZ + LocalZ - 1;

									short ValidityMask = (X != -1) + 2 * (Y != -1) + 4 * (Z != -1);

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

									GetValueAndMaterial(Step() * (X + 0), Step() * (Y + 0), Step() * (Z + 0), CornerValues[0], CornerMaterials[0]);
									GetValueAndMaterial(Step() * (X + 1), Step() * (Y + 0), Step() * (Z + 0), CornerValues[1], CornerMaterials[1]);
									GetValueAndMaterial(Step() * (X + 0), Step() * (Y + 1), Step() * (Z + 0), CornerValues[2], CornerMaterials[2]);
									GetValueAndMaterial(Step() * (X + 1), Step() * (Y + 1), Step() * (Z + 0), CornerValues[3], CornerMaterials[3]);
									GetValueAndMaterial(Step() * (X + 0), Step() * (Y + 0), Step() * (Z + 1), CornerValues[4], CornerMaterials[4]);
									GetValueAndMaterial(Step() * (X + 1), Step() * (Y + 0), Step() * (Z + 1), CornerValues[5], CornerMaterials[5]);
									GetValueAndMaterial(Step() * (X + 0), Step() * (Y + 1), Step() * (Z + 1), CornerValues[6], CornerMaterials[6]);
									GetValueAndMaterial(Step() * (X + 1), Step() * (Y + 1), Step() * (Z + 1), CornerValues[7], CornerMaterials[7]);

									check(CaseCode == (
										((CornerValues[0] > 0) << 0)
										| ((CornerValues[1] > 0) << 1)
										| ((CornerValues[2] > 0) << 2)
										| ((CornerValues[3] > 0) << 3)
										| ((CornerValues[4] > 0) << 4)
										| ((CornerValues[5] > 0) << 5)
										| ((CornerValues[6] > 0) << 6)
										| ((CornerValues[7] > 0) << 7)));

									const FVoxelMaterial CellMaterial = CornerMaterials[0];

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
											FVoxelMaterial Material;
											float Value;
											GetValueAndMaterial(
												(X - static_cast<bool>((CacheDirection & 0x01) != 0)) * Step(),
												(Y - static_cast<bool>((CacheDirection & 0x02) != 0)) * Step(),
												(Z - static_cast<bool>((CacheDirection & 0x04) != 0)) * Step(),
												Value,
												Material
											);

											ForceVertexCreation = !CellMaterial.HasSameIndexesAs(Material);
										}

										if ((ValidityMask & CacheDirection) != CacheDirection || ForceVertexCreation)
										{
											// If we are on one the lower edges of the chunk, or precedent color is not the same as current one

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

												const float	AlphaAtA = CornerMaterials[IndexVerticeA].Alpha;
												const float AlphaAtB = CornerMaterials[IndexVerticeB].Alpha;

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
											Vertices.push_front(Q);
											Colors.push_front(FVoxelMaterial(CellMaterial.Index1, CellMaterial.Index2, Alpha).ToFColor());
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
	Data->EndGet();


	if (VerticesSize == 0)
	{
		// Early exit
		OutSection.Reset();
		return;
	}

	TArray<int32> AllToFiltered;
	int32 FilteredVertexCount;
	int32 FilteredTriangleCount;
	{
		SCOPE_CYCLE_COUNTER(STAT_CREATE_SECTION);
		// Create section
		OutSection.Reset();
		OutSection.bEnableCollision = bComputeCollisions;
		OutSection.bSectionVisible = true;

		OutSection.ProcVertexBuffer.SetNumUninitialized(VerticesSize);
		OutSection.ProcIndexBuffer.SetNumUninitialized(TrianglesSize);

		// We create 2 vertex arrays: one with all the vertex (AllVertex), and one without vertex that are only for normal computation (FilteredVertex) which is the Section array
		// To do so we create bijection between the 2 arrays (AllToFiltered)
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

			if ((Vertex.X < -KINDA_SMALL_NUMBER) || (Vertex.X > Size() + KINDA_SMALL_NUMBER) ||
				(Vertex.Y < -KINDA_SMALL_NUMBER) || (Vertex.Y > Size() + KINDA_SMALL_NUMBER) ||
				(Vertex.Z < -KINDA_SMALL_NUMBER) || (Vertex.Z > Size() + KINDA_SMALL_NUMBER))
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
		FilteredVertexCount = FilteredVertexIndex;
		OutSection.ProcVertexBuffer.SetNum(FilteredVertexCount);

		// Normal array to compute normals while iterating over triangles
		TArray<FVector> Normals;
		Normals.SetNumZeroed(FilteredVertexCount); // Zeroed because +=

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
		FilteredTriangleCount = FilteredTriangleIndex;;
		OutSection.ProcIndexBuffer.SetNum(FilteredTriangleCount);


		// Apply normals
		for (int32 i = 0; i < FilteredVertexCount; i++)
		{
			FProcMeshVertex& ProcMeshVertex = OutSection.ProcVertexBuffer[i];
			ProcMeshVertex.Normal = Normals[i].GetSafeNormal();
			ProcMeshVertex.Position = bComputeTransitions ? GetTranslated(ProcMeshVertex.Position, ProcMeshVertex.Normal) : ProcMeshVertex.Position;
		}
	}

	check(Vertices.empty());
	check(Triangles.empty());

	if (bComputeTransitions)
	{
		const int OldVerticesSize = VerticesSize;
		const int OldTrianglesSize = TrianglesSize;

		Data->BeginGet();
		{
			SCOPE_CYCLE_COUNTER(STAT_TRANSITIONS_ITER);

			for (int DirectionIndex = 0; DirectionIndex < 6; DirectionIndex++)
			{
				auto Direction = (TransitionDirection)DirectionIndex;

				if (ChunkHasHigherRes[Direction])
				{
					for (int X = 0; X < 16; X++)
					{
						for (int Y = 0; Y < 16; Y++)
						{
							const int HalfStep = Step() / 2;

							float CornerValues[9];
							FVoxelMaterial CornerMaterials[9];

							Get2DValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[0], CornerMaterials[0]);
							Get2DValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[1], CornerMaterials[1]);
							Get2DValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 0) * HalfStep, CornerValues[2], CornerMaterials[2]);
							Get2DValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[3], CornerMaterials[3]);
							Get2DValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[4], CornerMaterials[4]);
							Get2DValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 1) * HalfStep, CornerValues[5], CornerMaterials[5]);
							Get2DValueAndMaterial(Direction, (2 * X + 0) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[6], CornerMaterials[6]);
							Get2DValueAndMaterial(Direction, (2 * X + 1) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[7], CornerMaterials[7]);
							Get2DValueAndMaterial(Direction, (2 * X + 2) * HalfStep, (2 * Y + 2) * HalfStep, CornerValues[8], CornerMaterials[8]);

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

								const FVoxelMaterial CellMaterial = CornerMaterials[0];

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
									const unsigned short EdgeCode = VertexData[i];

									// A: low point / B: high point
									const unsigned short IndexVerticeA = (EdgeCode >> 4) & 0x0F;
									const unsigned short IndexVerticeB = EdgeCode & 0x0F;

									check(0 <= IndexVerticeA && IndexVerticeA < 13);
									check(0 <= IndexVerticeB && IndexVerticeB < 13);

									const FIntVector PositionA = Positions[IndexVerticeA];
									const FIntVector PositionB = Positions[IndexVerticeB];

									const short EdgeIndex = (EdgeCode >> 8) & 0x0F;
									// Direction to go to use an already created vertex
									const short CacheDirection = EdgeCode >> 12;

									if ((ValidityMask & CacheDirection) != CacheDirection && EdgeIndex != 8 && EdgeIndex != 9)
									{
										// Validity check failed or EdgeIndex == 8 | 9 (always use Cache2D)
										const bool bIsAlongX = EdgeIndex == 3 || EdgeIndex == 4 || EdgeIndex == 8;
										const bool bIsAlongY = EdgeIndex == 5 || EdgeIndex == 6 || EdgeIndex == 9;


										FVector Q;
										uint8 Alpha;

										if (bIsAlongX)
										{
											// Edge along X axis
											InterpolateX2D(Direction, PositionA.X, PositionB.X, PositionA.Y, Q, Alpha);
										}
										else if (bIsAlongY)
										{
											// Edge along Y axis
											InterpolateY2D(Direction, PositionA.X, PositionA.Y, PositionB.Y, Q, Alpha);
										}
										else
										{
											Alpha = 0;
											checkf(false, TEXT("Error in interpolation: case should not exist"));
										}

										VertexIndex = VerticesSize;
										Vertices.push_front(Q);
										Colors.push_front(FVoxelMaterial(CellMaterial.Index1, CellMaterial.Index2, Alpha).ToFColor());
										VerticesSize++;

										// If own vertex, save it
										if (CacheDirection & 0x08)
										{
											SaveVertex2D(Direction, X, Y, EdgeIndex, VertexIndex);
										}
									}
									else
									{
										VertexIndex = LoadVertex2D(Direction, X, Y, CacheDirection, EdgeIndex);
									}

									VertexIndices[i] = VertexIndex;
								}

								// Add triangles
								int n = 3 * CellData.GetTriangleCount();
								for (int i = 0; i < n; i++)
								{
									Triangles.push_front(VertexIndices[CellData.vertexIndex[bFlip ? (n - 1 - i) : i]]);
								}
								TrianglesSize += n;
							}
						}
					}
				}
			}
		}
		Data->EndGet();

		{
			SCOPE_CYCLE_COUNTER(STAT_ADD_TRANSITIONS_TO_SECTION);

			const int32 TransitionsVerticesSize = VerticesSize - OldVerticesSize;
			const int32 TransitionTrianglesSize = TrianglesSize - OldTrianglesSize;

			OutSection.ProcVertexBuffer.AddUninitialized(TransitionsVerticesSize);
			OutSection.ProcIndexBuffer.AddUninitialized(TransitionTrianglesSize);

			TArray<FVector> TransitionVertex;
			TransitionVertex.SetNumUninitialized(TransitionsVerticesSize);

			for (int TransitionVertexIndex = TransitionsVerticesSize - 1; TransitionVertexIndex >= 0; TransitionVertexIndex--)
			{
				FVector Vertex = Vertices.front();
				FColor Color = Colors.front();
				Vertices.pop_front();
				Colors.pop_front();

				FProcMeshVertex& ProcMeshVertex = OutSection.ProcVertexBuffer[FilteredVertexCount + TransitionVertexIndex];
				ProcMeshVertex.Position = Vertex;
				ProcMeshVertex.Tangent = FProcMeshTangent();
				ProcMeshVertex.Color = Color;
				ProcMeshVertex.UV0 = FVector2D::ZeroVector;
				OutSection.SectionLocalBox += Vertex;

				TransitionVertex[TransitionVertexIndex] = Vertex;
			}

			// Normal array to compute normals while iterating over triangles
			TArray<FVector> Normals;
			Normals.SetNumUninitialized(TransitionsVerticesSize);

			int32 TransitionTriangleIndex = 0;
			while (!Triangles.empty())
			{
				int32 A = Triangles.front();
				// OldVerticesSize - FilteredVertexCount because we need to offset index due to vertex removal for normals
				int32 FA = A < OldVerticesSize ? AllToFiltered[OldVerticesSize - 1 - A] : A - (OldVerticesSize - FilteredVertexCount);
				Triangles.pop_front();

				int32 B = Triangles.front();
				int32 FB = B < OldVerticesSize ? AllToFiltered[OldVerticesSize - 1 - B] : B - (OldVerticesSize - FilteredVertexCount);
				Triangles.pop_front();

				int32 C = Triangles.front();
				int32 FC = C < OldVerticesSize ? AllToFiltered[OldVerticesSize - 1 - C] : C - (OldVerticesSize - FilteredVertexCount);
				Triangles.pop_front();

				check(FA != -1 && FB != -1 && FC != -1);

				OutSection.ProcIndexBuffer[FilteredTriangleCount + TransitionTriangleIndex] = FC;
				OutSection.ProcIndexBuffer[FilteredTriangleCount + TransitionTriangleIndex + 1] = FB;
				OutSection.ProcIndexBuffer[FilteredTriangleCount + TransitionTriangleIndex + 2] = FA;
				TransitionTriangleIndex += 3;

				FVector Normal;
				if (A < OldVerticesSize)
				{
					Normal = OutSection.ProcVertexBuffer[FA].Normal;
				}
				else if (B < OldVerticesSize)
				{
					Normal = OutSection.ProcVertexBuffer[FB].Normal;
				}
				else if (C < OldVerticesSize)
				{
					Normal = OutSection.ProcVertexBuffer[FC].Normal;
				}


				if (A >= OldVerticesSize)
				{
					Normals[FA - FilteredVertexCount] = Normal;
				}
				if (B >= OldVerticesSize)
				{
					Normals[FB - FilteredVertexCount] = Normal;
				}
				if (C >= OldVerticesSize)
				{
					Normals[FC - FilteredVertexCount] = Normal;
				}
			}


			// Apply normals
			for (int32 i = 0; i < TransitionsVerticesSize; i++)
			{
				FProcMeshVertex& ProcMeshVertex = OutSection.ProcVertexBuffer[i + FilteredVertexCount];
				ProcMeshVertex.Normal = Normals[i].GetSafeNormal();
			}
		}
	}
}

int FVoxelPolygonizer::Size()
{
	return 16 << Depth;
}

int FVoxelPolygonizer::Step()
{
	return 1 << Depth;
}


//FColor VoxelPolygonizer::GetMajorColor(int X, int Y, int Z, uint32 CellWidth)
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


void FVoxelPolygonizer::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial)
{
	SCOPE_CYCLE_COUNTER(STAT_GETVALUEANDCOLOR);
	if (LIKELY((X % Step() == 0) && (Y % Step() == 0) && (Z % Step() == 0)))
	{
		OutValue = CachedValues[(X / Step() + 1) + 18 * (Y / Step() + 1) + 18 * 18 * (Z / Step() + 1)];
		OutMaterial = CachedMaterials[(X / Step() + 1) + 18 * (Y / Step() + 1) + 18 * 18 * (Z / Step() + 1)];
	}
	else
	{
		Data->GetValueAndMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, OutValue, OutMaterial, LastOctree);
	}
}

void FVoxelPolygonizer::Get2DValueAndMaterial(TransitionDirection Direction, int X, int Y, float& OutValue, FVoxelMaterial& OutMaterial)
{
	SCOPE_CYCLE_COUNTER(STAT_GET2DVALUEANDCOLOR);
	int GX, GY, GZ;
	Local2DToGlobal(Size(), Direction, X, Y, 0, GX, GY, GZ);

	GetValueAndMaterial(GX, GY, GZ, OutValue, OutMaterial);
}


void FVoxelPolygonizer::SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index)
{
	// +1: normals offset
	check(0 <= X + 1 && X + 1 < 17);
	check(0 <= Y + 1 && Y + 1 < 17);
	check(0 <= Z + 1 && Z + 1 < 17);
	check(0 <= EdgeIndex && EdgeIndex < 3);

	Cache[X + 1][Y + 1][Z + 1][EdgeIndex] = Index;
}

int FVoxelPolygonizer::LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex)
{
	bool XIsDifferent = static_cast<bool>((Direction & 0x01) != 0);
	bool YIsDifferent = static_cast<bool>((Direction & 0x02) != 0);
	bool ZIsDifferent = static_cast<bool>((Direction & 0x04) != 0);

	// +1: normals offset
	check(0 <= X - XIsDifferent + 1 && X - XIsDifferent + 1 < 17);
	check(0 <= Y - YIsDifferent + 1 && Y - YIsDifferent + 1 < 17);
	check(0 <= Z - ZIsDifferent + 1 && Z - ZIsDifferent + 1 < 17);
	check(0 <= EdgeIndex && EdgeIndex < 3);


	check(Cache[X - XIsDifferent + 1][Y - YIsDifferent + 1][Z - ZIsDifferent + 1][EdgeIndex] >= 0);
	return Cache[X - XIsDifferent + 1][Y - YIsDifferent + 1][Z - ZIsDifferent + 1][EdgeIndex];
}


void FVoxelPolygonizer::SaveVertex2D(TransitionDirection Direction, int X, int Y, short EdgeIndex, int Index)
{
	if (EdgeIndex == 8 || EdgeIndex == 9)
	{
		// Never save those
		check(false);
	}

	check(0 <= X && X < 17);
	check(0 <= Y && Y < 17);
	check(0 <= EdgeIndex && EdgeIndex < 7);

	Cache2D[Direction][X][Y][EdgeIndex] = Index;
}

int FVoxelPolygonizer::LoadVertex2D(TransitionDirection Direction, int X, int Y, short CacheDirection, short EdgeIndex)
{
	bool XIsDifferent = static_cast<bool>((CacheDirection & 0x01) != 0);
	bool YIsDifferent = static_cast<bool>((CacheDirection & 0x02) != 0);

	if (EdgeIndex == 8 || EdgeIndex == 9)
	{
		int Index;
		switch (Direction)
		{
		case XMin:
			Index = EdgeIndex == 8 ? 0 : 2;
			break;
		case XMax:
			Index = EdgeIndex == 8 ? 2 : 0;
			break;
		case YMin:
			Index = EdgeIndex == 8 ? 2 : 1;
			break;
		case YMax:
			Index = EdgeIndex == 8 ? 1 : 2;
			break;
		case ZMin:
			Index = EdgeIndex == 8 ? 1 : 0;
			break;
		case ZMax:
			Index = EdgeIndex == 8 ? 0 : 1;
			break;
		default:
			Index = 0;
			check(false);
			break;
		}
		int GX, GY, GZ;

		Local2DToGlobal(14, Direction, X - XIsDifferent, Y - YIsDifferent, -1, GX, GY, GZ);

		return LoadVertex(GX, GY, GZ, 0, Index);
	}

	check(0 <= X - XIsDifferent && X - XIsDifferent < 17);
	check(0 <= Y - YIsDifferent && Y - YIsDifferent < 17);
	check(0 <= EdgeIndex && EdgeIndex < 7);

	check(Cache2D[Direction][X - XIsDifferent][Y - YIsDifferent][EdgeIndex] >= 0);
	return Cache2D[Direction][X - XIsDifferent][Y - YIsDifferent][EdgeIndex];
}

void FVoxelPolygonizer::InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FVoxelMaterial MaterialAtA;
	FVoxelMaterial MaterialAtB;
	GetValueAndMaterial(MinX, Y, Z, ValueAtA, MaterialAtA);
	GetValueAndMaterial(MaxX, Y, Z, ValueAtB, MaterialAtB);

	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(MinX, Y, Z) + (1 - t) *  FVector(MaxX, Y, Z);
		OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);
		int xMiddle = (MaxX + MinX) / 2;

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterial(xMiddle, Y, Z, ValueAtMiddle, MaterialAtMiddle);
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

void FVoxelPolygonizer::InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FVoxelMaterial MaterialAtA;
	FVoxelMaterial MaterialAtB;
	GetValueAndMaterial(X, MinY, Z, ValueAtA, MaterialAtA);
	GetValueAndMaterial(X, MaxY, Z, ValueAtB, MaterialAtB);

	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, MinY, Z) + (1 - t) *  FVector(X, MaxY, Z);
		OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);
		int yMiddle = (MaxY + MinY) / 2;

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterial(X, yMiddle, Z, ValueAtMiddle, MaterialAtMiddle);
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

void FVoxelPolygonizer::InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FVoxelMaterial MaterialAtA;
	FVoxelMaterial MaterialAtB;
	GetValueAndMaterial(X, Y, MinZ, ValueAtA, MaterialAtA);
	GetValueAndMaterial(X, Y, MaxZ, ValueAtB, MaterialAtB);

	if (MaxZ - MinZ == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		OutVector = t * FVector(X, Y, MinZ) + (1 - t) *  FVector(X, Y, MaxZ);
		OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;
	}
	else
	{
		check((MaxZ + MinZ) % 2 == 0);
		int zMiddle = (MaxZ + MinZ) / 2;

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		GetValueAndMaterial(X, Y, zMiddle, ValueAtMiddle, MaterialAtMiddle);
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



void FVoxelPolygonizer::InterpolateX2D(TransitionDirection Direction, const int MinX, const int MaxX, const int Y, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FVoxelMaterial MaterialAtA;
	FVoxelMaterial MaterialAtB;
	Get2DValueAndMaterial(Direction, MinX, Y, ValueAtA, MaterialAtA);
	Get2DValueAndMaterial(Direction, MaxX, Y, ValueAtB, MaterialAtB);

	if (MaxX - MinX == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		int GMinX, GMaxX, GMinY, GMaxY, GMinZ, GMaxZ;
		Local2DToGlobal(Size(), Direction, MinX, Y, 0, GMinX, GMinY, GMinZ);
		Local2DToGlobal(Size(), Direction, MaxX, Y, 0, GMaxX, GMaxY, GMaxZ);

		OutVector = t * FVector(GMinX, GMinY, GMinZ) + (1 - t) *  FVector(GMaxX, GMaxY, GMaxZ);
		OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;
	}
	else
	{
		check((MaxX + MinX) % 2 == 0);
		int xMiddle = (MaxX + MinX) / 2;

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		Get2DValueAndMaterial(Direction, xMiddle, Y, ValueAtMiddle, MaterialAtMiddle);
		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			return InterpolateX2D(Direction, xMiddle, MaxX, Y, OutVector, OutAlpha);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateX2D(Direction, MinX, xMiddle, Y, OutVector, OutAlpha);
		}
	}
}

void FVoxelPolygonizer::InterpolateY2D(TransitionDirection Direction, const int X, const int MinY, const int MaxY, FVector& OutVector, uint8& OutAlpha)
{
	// A: Min / B: Max
	float ValueAtA;
	float ValueAtB;
	FVoxelMaterial MaterialAtA;
	FVoxelMaterial MaterialAtB;
	Get2DValueAndMaterial(Direction, X, MinY, ValueAtA, MaterialAtA);
	Get2DValueAndMaterial(Direction, X, MaxY, ValueAtB, MaterialAtB);

	if (MaxY - MinY == 1)
	{
		check(ValueAtA - ValueAtB != 0);
		float t = ValueAtB / (ValueAtB - ValueAtA);

		int GMinX, GMaxX, GMinY, GMaxY, GMinZ, GMaxZ;
		Local2DToGlobal(Size(), Direction, X, MinY, 0, GMinX, GMinY, GMinZ);
		Local2DToGlobal(Size(), Direction, X, MaxY, 0, GMaxX, GMaxY, GMaxZ);

		OutVector = t * FVector(GMinX, GMinY, GMinZ) + (1 - t) *  FVector(GMaxX, GMaxY, GMaxZ);
		OutAlpha = t * MaterialAtA.Alpha + (1 - t) * MaterialAtB.Alpha;
	}
	else
	{
		check((MaxY + MinY) % 2 == 0);
		int yMiddle = (MaxY + MinY) / 2;

		float ValueAtMiddle;
		FVoxelMaterial MaterialAtMiddle;
		Get2DValueAndMaterial(Direction, X, yMiddle, ValueAtMiddle, MaterialAtMiddle);
		if ((ValueAtA > 0) == (ValueAtMiddle > 0))
		{
			// If min and middle have same sign
			return InterpolateY2D(Direction, X, yMiddle, MaxY, OutVector, OutAlpha);
		}
		else
		{
			// If max and middle have same sign
			return InterpolateY2D(Direction, X, MinY, yMiddle, OutVector, OutAlpha);
		}
	}
}

void FVoxelPolygonizer::GlobalToLocal2D(int Size, TransitionDirection Direction, int GX, int GY, int GZ, int& OutLX, int& OutLY, int& OutLZ)
{
	const int S = Size;
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

void FVoxelPolygonizer::Local2DToGlobal(int Size, TransitionDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ)
{
	const int S = Size;
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


FVector FVoxelPolygonizer::GetTranslated(const FVector Vertex, const FVector Normal)
{
	double DeltaX = 0;
	double DeltaY = 0;
	double DeltaZ = 0;

	if ((Vertex.X < KINDA_SMALL_NUMBER && !ChunkHasHigherRes[XMin]) || (Vertex.X > Size() - KINDA_SMALL_NUMBER && !ChunkHasHigherRes[XMax]) ||
		(Vertex.Y < KINDA_SMALL_NUMBER && !ChunkHasHigherRes[YMin]) || (Vertex.Y > Size() - KINDA_SMALL_NUMBER && !ChunkHasHigherRes[YMax]) ||
		(Vertex.Z < KINDA_SMALL_NUMBER && !ChunkHasHigherRes[ZMin]) || (Vertex.Z > Size() - KINDA_SMALL_NUMBER && !ChunkHasHigherRes[ZMax]))
	{
		return Vertex;
	}

	double TwoPowerK = 1 << Depth;
	double w = TwoPowerK / 4;

	if (ChunkHasHigherRes[XMin] && Vertex.X < Step())
	{
		DeltaX = (1 - static_cast<double>(Vertex.X) / TwoPowerK) * w;
	}
	if (ChunkHasHigherRes[XMax] && Vertex.X > 15 * Step())
	{
		DeltaX = (16 - 1 - static_cast<double>(Vertex.X) / TwoPowerK) * w;
	}
	if (ChunkHasHigherRes[YMin] && Vertex.Y < Step())
	{
		DeltaY = (1 - static_cast<double>(Vertex.Y) / TwoPowerK) * w;
	}
	if (ChunkHasHigherRes[YMax] && Vertex.Y > 15 * Step())
	{
		DeltaY = (16 - 1 - static_cast<double>(Vertex.Y) / TwoPowerK) * w;
	}
	if (ChunkHasHigherRes[ZMin] && Vertex.Z < Step())
	{
		DeltaZ = (1 - static_cast<double>(Vertex.Z) / TwoPowerK) * w;
	}
	if (ChunkHasHigherRes[ZMax] && Vertex.Z > 15 * Step())
	{
		DeltaZ = (16 - 1 - static_cast<double>(Vertex.Z) / TwoPowerK) * w;
	}

	FVector Q = FVector(
		(1 - Normal.X * Normal.X) * DeltaX - Normal.X * Normal.Y * DeltaY - Normal.X * Normal.Z * DeltaZ,
		-Normal.X * Normal.Y * DeltaX + (1 - Normal.Y * Normal.Y) * DeltaY - Normal.Y * Normal.Z * DeltaZ,
		-Normal.X * Normal.Z * DeltaX - Normal.Y * Normal.Z * DeltaY + (1 - Normal.Z * Normal.Z) * DeltaZ);

	return Vertex + Q;
}
