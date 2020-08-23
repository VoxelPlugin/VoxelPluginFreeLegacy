// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelSurfaceNetMesher.h"
#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelDataIncludes.h"

/**
 * This code is based on an original implementation kindly provided by Dexyfex
 * You can check out his website here: https://dexyfex.com/
 */

struct FVoxelSurfaceNetFullVertex : FVoxelMesherVertex
{
	static constexpr bool bComputeParentPosition = true;
	static constexpr bool bComputeNormal = true;
	static constexpr bool bComputeTextureCoordinate = true;
	static constexpr bool bComputeMaterial = true;
	
	FORCEINLINE void SetPosition(const FVector& InPosition)
	{
		Position = InPosition;
	}
	FORCEINLINE void SetParentPosition(const FVector& InParentPosition)
	{
		Tangent = FVoxelProcMeshTangent(InParentPosition, false);
	}
	FORCEINLINE void SetNormal(const FVector& InNormal)
	{
		Normal = InNormal;
	}
	FORCEINLINE void SetTextureCoordinate(const FVector2D& InTextureCoordinate)
	{
		TextureCoordinate = InTextureCoordinate;
	}
	FORCEINLINE void SetMaterial(const FVoxelMaterial& InMaterial)
	{
		Material = InMaterial;
	}
};
static_assert(sizeof(FVoxelSurfaceNetFullVertex) == sizeof(FVoxelMesherVertex), "");

struct FVoxelSurfaceNetGeometryVertex : FVector
{
	static constexpr bool bComputeParentPosition = false;
	static constexpr bool bComputeNormal = false;
	static constexpr bool bComputeTextureCoordinate = false;
	static constexpr bool bComputeMaterial = false;
	
	FORCEINLINE void SetPosition(const FVector& InPosition)
	{
		static_cast<FVector&>(*this) = InPosition;
	}
	FORCEINLINE void SetParentPosition(const FVector& InParentPosition)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetNormal(const FVector& InNormal)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetTextureCoordinate(const FVector2D& InTextureCoordinate)
	{
		checkVoxelSlow(false);
	}
	FORCEINLINE void SetMaterial(const FVoxelMaterial& InMaterial)
	{
		checkVoxelSlow(false);
	}
};
static_assert(sizeof(FVoxelSurfaceNetGeometryVertex) == sizeof(FVector), "");

FVoxelIntBox FVoxelSurfaceNetMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FVoxelIntBox(ChunkPosition, ChunkPosition + SN_EXTENDED_CHUNK_SIZE * Step);
}

FVoxelIntBox FVoxelSurfaceNetMesher::GetBoundsToLock() const
{
	return GetBoundsToCheckIsEmptyOn();
}

inline float SampleIsoValue(const float Values[8], const FVector& Offset)
{
	// TODO use FVoxelUtilities::TrilinearInterpolation
	const FVector i = FVector(1.0f) - Offset;
	const float x1 = Values[0] * i.X + Values[1] * Offset.X;
	const float x2 = Values[2] * i.X + Values[3] * Offset.X;
	const float x3 = Values[4] * i.X + Values[5] * Offset.X;
	const float x4 = Values[6] * i.X + Values[7] * Offset.X;
	const float y1 = x1 * i.Y + x2 * Offset.Y;
	const float y2 = x3 * i.Y + x4 * Offset.Y;
	const float z1 = y1 * i.Z + y2 * Offset.Z;
	return z1;
}

inline FVector GetNormal(const float Values[8], const FVector& Offset)
{
	const float MaxX = SampleIsoValue(Values, Offset + FVector(+1, 0, 0));
	const float MinX = SampleIsoValue(Values, Offset + FVector(-1, 0, 0));
	const float MaxY = SampleIsoValue(Values, Offset + FVector(0, +1, 0));
	const float MinY = SampleIsoValue(Values, Offset + FVector(0, -1, 0));
	const float MaxZ = SampleIsoValue(Values, Offset + FVector(0, 0, +1));
	const float MinZ = SampleIsoValue(Values, Offset + FVector(0, 0, -1));
	return FVector(MaxX - MinX, MaxY - MinY, MaxZ - MinZ).GetSafeNormal();
}

template<typename TVertex>
void FVoxelSurfaceNetMesher::CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<TVertex>& Vertices)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	TVoxelQueryZone<FVoxelValue> QueryZone(GetBoundsToCheckIsEmptyOn(), FIntVector(SN_EXTENDED_CHUNK_SIZE), LOD, CachedValues);
	MESHER_TIME_VALUES(SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE, Data.Get<FVoxelValue>(QueryZone, LOD));

	Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());

	constexpr uint32 EdgeIndexOffsets[12] =
	{
		0,
		SN_EXTENDED_CHUNK_SIZE * 3,
		SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		SN_EXTENDED_CHUNK_SIZE * 3 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		1,
		4,
		1 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		4 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		2,
		5,
		2 + SN_EXTENDED_CHUNK_SIZE * 3,
		5 + SN_EXTENDED_CHUNK_SIZE * 3
	};
	constexpr uint32 ParentEdgeIndexOffsetsMin[12] =
	{
		0,
		2 * SN_EXTENDED_CHUNK_SIZE * 3,
		2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		2 * SN_EXTENDED_CHUNK_SIZE * 3 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		1,
		7,
		1 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		7 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		2,
		8,
		2 + 2 * SN_EXTENDED_CHUNK_SIZE * 3,
		8 + 2 * SN_EXTENDED_CHUNK_SIZE * 3
	};
	constexpr uint32 ParentEdgeIndexOffsetsMax[12] =
	{
		3,
		2 * SN_EXTENDED_CHUNK_SIZE * 3 + 3,
		2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3 + 3,
		2 * SN_EXTENDED_CHUNK_SIZE * 3 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3 + 3,
		1 + SN_EXTENDED_CHUNK_SIZE * 3,
		7 + SN_EXTENDED_CHUNK_SIZE * 3,
		1 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3 + SN_EXTENDED_CHUNK_SIZE * 3,
		7 + 2 * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3 + SN_EXTENDED_CHUNK_SIZE * 3,
		2 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		8 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		2 + 2 * SN_EXTENDED_CHUNK_SIZE * 3 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3,
		8 + 2 * SN_EXTENDED_CHUNK_SIZE * 3 + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3
	};
	constexpr uint32 EdgeFirstCornerIndex[12] = { 0, 2, 4, 6, 0, 1, 4, 5, 0, 1, 2, 3 };
	constexpr uint32 EdgeSecondCornerIndex[12] = { 1, 3, 5, 7, 2, 3, 6, 7, 4, 5, 6, 7 };
	// TODO: Replace buffer by simple math
	const FVector Corners[8] =
	{
		{0,0,0},
		{1,0,0},
		{0,1,0},
		{1,1,0},
		{0,0,1},
		{1,0,1},
		{0,1,1},
		{1,1,1}
	};
	const FVector ParentCorners[8] =
	{
		{0,0,0},
		{2,0,0},
		{0,2,0},
		{2,2,0},
		{0,0,2},
		{2,0,2},
		{0,2,2},
		{2,2,2}
	};
	constexpr uint32 Offsets[3] = { 1, SN_EXTENDED_CHUNK_SIZE, SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE };
	const FIntVector icorners[3] = { {1,0,0},{0,1,0},{0,0,1} };

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Find Intersections");

		// find intersections and calculate the edge blend factors for all cells
		for (int32 LZ = 0; LZ < SN_EXTENDED_CHUNK_SIZE; LZ++)
		{
			for (int32 LY = 0; LY < SN_EXTENDED_CHUNK_SIZE; LY++)
			{
				for (int32 LX = 0; LX < SN_EXTENDED_CHUNK_SIZE; LX++)
				{
					const uint32 VoxelIndex = LX + LY * SN_EXTENDED_CHUNK_SIZE + LZ * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE;
					const FVoxelValue MinCornerValue = CachedValues[VoxelIndex];
					FIntVector MinCornerPosition = FIntVector(LX, LY, LZ) * Step;
					FIntVector MaxCornerPositions = FIntVector(LX + 1, LY + 1, LZ + 1);

					for (uint32 Direction = 0; Direction < 3; Direction++)
					{
						float Factor = -1; // empty value, valid factor should be between 0 and 1
						if (MaxCornerPositions[Direction] < SN_EXTENDED_CHUNK_SIZE) // don't go outside of the cached area
						{
							const FVoxelValue MaxCornerInDirectionValue = CachedValues[VoxelIndex + Offsets[Direction]];
							if (MinCornerValue.IsEmpty() != MaxCornerInDirectionValue.IsEmpty())
							{
								FVoxelValue MinValue = MinCornerValue;
								FVoxelValue MaxValue = MaxCornerInDirectionValue;

								if (LOD != 0)
								{
									// for LOD chunks, search along the edge for the actual intersecting segment
									FIntVector MinPosition = MinCornerPosition;
									FIntVector MaxPosition = MinCornerPosition + icorners[Direction] * Step;
									float c1 = 0;
									float c2 = 1;
									for (int iStep = Step; iStep > 1; iStep >>= 1)
									{
										const float cmid = (c1 + c2) * 0.5f;
										const FIntVector MidPosition = (MinPosition + MaxPosition) / 2;
										const FVoxelValue MidValue = MESHER_TIME_RETURN_VALUES(1, Accelerator->Get<FVoxelValue>(MidPosition + ChunkPosition, LOD));
										if (MinValue.IsEmpty() != MidValue.IsEmpty())//intersection is between c1 and cmid
										{
											c2 = cmid;
											MaxValue = MidValue;
											MaxPosition = MidPosition;
										}
										else //intersection is between cmid and c2
										{
											c1 = cmid;
											MinValue = MidValue;
											MinPosition = MidPosition;
										}
									}
									// TODO is this needed
									Factor = c1 + (c2 - c1) * MinValue.ToFloat() / (MinValue.ToFloat() - MaxValue.ToFloat());
								}
								else
								{
									Factor = MinValue.ToFloat() / (MinValue.ToFloat() - MaxValue.ToFloat());
								}
							}
						}
						EdgeFactors[3 * VoxelIndex + Direction] = Factor;
					}
					
					if (TVertex::bComputeMaterial)
					{
						// We need to find the min value that's a surface value
						if (LX < SN_EXTENDED_CHUNK_SIZE - 1 && 
							LY < SN_EXTENDED_CHUNK_SIZE - 1 &&
							LZ < SN_EXTENDED_CHUNK_SIZE - 1)
						{
							FVoxelValue VoxelValues[8];
							VoxelValues[0] = CachedValues[VoxelIndex];
							VoxelValues[1] = CachedValues[VoxelIndex + 1];
							VoxelValues[2] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE];
							VoxelValues[3] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE + 1];
							VoxelValues[4] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE];
							VoxelValues[5] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE + 1];
							VoxelValues[6] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE];
							VoxelValues[7] = CachedValues[VoxelIndex + SN_EXTENDED_CHUNK_SIZE + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE + 1];
							
							FVoxelValue MinValue = FVoxelValue::Empty();
							int32 MinValueIndex = 0;
							
							for (int32 Index = 0; Index < 8; Index++)
							{
								if (VoxelValues[Index] > MinValue)
								{
									continue;
								}
								bool bIsSurfaceValue = false;
								for (int32 Neighbor = 0; Neighbor < 3; Neighbor++)
								{
									const int32 NeighborIndex = Index ^ (1 << Neighbor);
									if (VoxelValues[Index].IsEmpty() != VoxelValues[NeighborIndex].IsEmpty())
									{
										bIsSurfaceValue = true;
										break;
									}
								}
								if (!bIsSurfaceValue)
								{
									continue;
								}
								MinValue = VoxelValues[Index];
								MinValueIndex = Index;
							}
							
							MaterialPositions[VoxelIndex] = 
							{
								LX + bool(MinValueIndex & 0x1),
								LY + bool(MinValueIndex & 0x2),
								LZ + bool(MinValueIndex & 0x4)
							};
						}
						else
						{
							MaterialPositions[VoxelIndex] = { LX, LY, LZ };
						}
					}
				}
			}
		}
	}

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Generate Vertices");

		// generate the vertices and store the indices for the next step
		for (uint32 LZ = 0; LZ < SN_CHUNK_SIZE; LZ++)
		{
			for (uint32 LY = 0; LY < SN_CHUNK_SIZE; LY++)
			{
				for (uint32 LX = 0; LX < SN_CHUNK_SIZE; LX++)
				{
					const uint32 VoxelIndex = LX + LY * SN_EXTENDED_CHUNK_SIZE + LZ * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE;
					const uint32 VertexIndex = LX + LY * SN_CHUNK_SIZE + LZ * SN_CHUNK_SIZE * SN_CHUNK_SIZE;

					uint32 VoxelIndices[8];
					VoxelIndices[0] = VoxelIndex;
					VoxelIndices[1] = VoxelIndex + 1;
					VoxelIndices[2] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE;
					VoxelIndices[3] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE + 1;
					VoxelIndices[4] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE;
					VoxelIndices[5] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE + 1;
					VoxelIndices[6] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE;
					VoxelIndices[7] = VoxelIndex + SN_EXTENDED_CHUNK_SIZE + SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE + 1;

					FVoxelValue VoxelValues[8];
					VoxelValues[0] = CachedValues[VoxelIndices[0]];
					VoxelValues[1] = CachedValues[VoxelIndices[1]];
					VoxelValues[2] = CachedValues[VoxelIndices[2]];
					VoxelValues[3] = CachedValues[VoxelIndices[3]];
					VoxelValues[4] = CachedValues[VoxelIndices[4]];
					VoxelValues[5] = CachedValues[VoxelIndices[5]];
					VoxelValues[6] = CachedValues[VoxelIndices[6]];
					VoxelValues[7] = CachedValues[VoxelIndices[7]];

					float VoxelFloats[8];
					VoxelFloats[0] = VoxelValues[0].ToFloat();
					VoxelFloats[1] = VoxelValues[1].ToFloat();
					VoxelFloats[2] = VoxelValues[2].ToFloat();
					VoxelFloats[3] = VoxelValues[3].ToFloat();
					VoxelFloats[4] = VoxelValues[4].ToFloat();
					VoxelFloats[5] = VoxelValues[5].ToFloat();
					VoxelFloats[6] = VoxelValues[6].ToFloat();
					VoxelFloats[7] = VoxelValues[7].ToFloat();

					const uint32 MarchingCubesCase =
						(VoxelValues[0].IsEmpty() << 0) |
						(VoxelValues[1].IsEmpty() << 1) |
						(VoxelValues[2].IsEmpty() << 2) |
						(VoxelValues[3].IsEmpty() << 3) |
						(VoxelValues[4].IsEmpty() << 4) |
						(VoxelValues[5].IsEmpty() << 5) |
						(VoxelValues[6].IsEmpty() << 6) |
						(VoxelValues[7].IsEmpty() << 7);

					const uint8 SurfaceNetsCase =
						(VoxelValues[3].IsEmpty() << 0) |
						(VoxelValues[5].IsEmpty() << 1) |
						(VoxelValues[6].IsEmpty() << 2) |
						(VoxelValues[7].IsEmpty() << 3);

					VertexSNCases[VertexIndex] = SurfaceNetsCase;

					if ((MarchingCubesCase == 0) || (MarchingCubesCase == 255)) //cell is empty
					{
						VertexIndices[VertexIndex] = -1;
						continue;
					}

					VertexIndices[VertexIndex] = Vertices.Num();


					const uint32 VoxelEdgeIndex = VoxelIndex * 3;
					FVector CrossingTotal = FVector(0, 0, 0);
					uint32 CrossingCount = 0;

					constexpr int32 RemoveFirstBit = 0xFFFE; // TODO: what if more than 64k vertices
					const FIntVector ParentPosition((LX & RemoveFirstBit), (LY & RemoveFirstBit), (LZ & RemoveFirstBit));
					const uint32 ParentVoxelIndex = ParentPosition.X + ParentPosition.Y * SN_EXTENDED_CHUNK_SIZE + ParentPosition.Z * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE;
					const uint32 ParentVoxelEdgeIndex = ParentVoxelIndex * 3;
					FVector ParentCrossingTotal = FVector(0, 0, 0);
					uint32 ParentCrossingCount = 0;

					for (uint32 Edge = 0; Edge < 12; Edge++)
					{
						// if this edge has a crossing, find the point and add it to the avg total, increment count
						const float EdgeFactor = EdgeFactors[VoxelEdgeIndex + EdgeIndexOffsets[Edge]];
						if (EdgeFactor >= 0)
						{
							const FVector MinPosition = Corners[EdgeFirstCornerIndex[Edge]];
							const FVector MaxPosition = Corners[EdgeSecondCornerIndex[Edge]];
							const FVector MidPosition = FMath::Lerp(MinPosition, MaxPosition, EdgeFactor); // blend between corners
							CrossingTotal += MidPosition;
							CrossingCount++;
						}

						// find crossings of parent cell
						const float ParentEdgeFactorMin = EdgeFactors[ParentVoxelEdgeIndex + ParentEdgeIndexOffsetsMin[Edge]];
						const float ParentEdgeFactorMax = EdgeFactors[ParentVoxelEdgeIndex + ParentEdgeIndexOffsetsMax[Edge]];
						if ((ParentEdgeFactorMin >= 0) || (ParentEdgeFactorMax >= 0))
						{
							const float ParentEdgeFactor =
								((ParentEdgeFactorMin >= 0) ? 0.5f * ParentEdgeFactorMin : 0.5f) +
								((ParentEdgeFactorMax >= 0) ? 0.5f * ParentEdgeFactorMax : 0);
							const FVector MinPosition = ParentCorners[EdgeFirstCornerIndex[Edge]];
							const FVector MaxPosition = ParentCorners[EdgeSecondCornerIndex[Edge]];
							const FVector MidPosition = FMath::Lerp(MinPosition, MaxPosition, ParentEdgeFactor); // blend between corners
							ParentCrossingTotal += MidPosition;
							ParentCrossingCount++;
						}
					}

					ensureVoxelSlowNoSideEffects(CrossingCount > 0);
					const FVector Offset = CrossingTotal / CrossingCount;

					const FVector ParentOffset = ParentCrossingCount == 0 ? FVector::ZeroVector : ParentCrossingTotal / ParentCrossingCount;


					const FIntVector CellPosition(LX * Step, LY * Step, LZ * Step);
					const FVector CornerPosition{ CellPosition };
					const FVector FinalPosition = CornerPosition + Offset * Step;

					const FIntVector ParentCellPosition = ParentPosition * Step;
					const FVector ParentCornerPosition{ ParentCellPosition };
					const FVector ParentFinalPosition = ParentCornerPosition + ParentOffset * Step;
					
					TVertex Vertex;
					Vertex.SetPosition(FinalPosition);
					if (TVertex::bComputeParentPosition)
					{
						Vertex.SetParentPosition((ParentFinalPosition - FinalPosition) / Step); // Divide by Step to avoid overflowing the tangent
					}
					if (TVertex::bComputeNormal)
					{
						Vertex.SetNormal(MESHER_TIME_RETURN(Normals, GetNormal(VoxelFloats, Offset)));
					}
					if (TVertex::bComputeMaterial)
					{
						Vertex.SetMaterial(MESHER_TIME_RETURN_MATERIALS(1, Accelerator->GetMaterial(MaterialPositions[VoxelIndex] * Step + ChunkPosition, LOD)));
					}
					if (TVertex::bComputeTextureCoordinate)
					{
						Vertex.SetTextureCoordinate(MESHER_TIME_RETURN(UVs, FVoxelMesherUtilities::GetUVs(*this, FinalPosition)));
					}
					Vertices.Add(Vertex);
				}
			}
		}
	}

	UnlockData();

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Generate Mesh");

		// generate the mesh indices
		for (uint32 LZ = 0; LZ < RENDER_CHUNK_SIZE; LZ++)
		{
			for (uint32 LY = 0; LY < RENDER_CHUNK_SIZE; LY++)
			{
				for (uint32 LX = 0; LX < RENDER_CHUNK_SIZE; LX++)
				{
					const uint32 VoxelIndex = LX + LY * SN_CHUNK_SIZE + LZ * SN_CHUNK_SIZE * SN_CHUNK_SIZE;
					const uint8 SurfaceNetCase = VertexSNCases[VoxelIndex];

					constexpr uint32 QuadsOffsets[8] = 
					{ 
						0, 
						SN_CHUNK_SIZE, 
						SN_CHUNK_SIZE + 1, 
						1, 
						SN_CHUNK_SIZE * SN_CHUNK_SIZE, 
						SN_CHUNK_SIZE * SN_CHUNK_SIZE + SN_CHUNK_SIZE,
						SN_CHUNK_SIZE * SN_CHUNK_SIZE + SN_CHUNK_SIZE + 1,
						SN_CHUNK_SIZE * SN_CHUNK_SIZE + 1 
					};
					constexpr uint32 QuadsPositions[6][4] =
					{
						{ 0, 1, 3, 2 },
						{ 0, 3, 4, 7 },
						{ 0, 4, 1, 5 },
						{ 0, 3, 1, 2 },
						{ 0, 4, 3, 7 },
						{ 0, 1, 4, 5 }
					};

					uint32 QuadsIndices[3] = { 0, 0, 0 }; //indexing into quads array
					uint32 QuadCount = 0;

					switch (SurfaceNetCase) //surface nets polygonization
					{
					case  0: QuadCount = 0; break;
					case  1: QuadCount = 1; QuadsIndices[0] = 0; break;
					case  2: QuadCount = 1; QuadsIndices[0] = 1; break;
					case  3: QuadCount = 2; QuadsIndices[0] = 0; QuadsIndices[1] = 1; break;
					case  4: QuadCount = 1; QuadsIndices[0] = 2; break;
					case  5: QuadCount = 2; QuadsIndices[0] = 0; QuadsIndices[1] = 2; break;
					case  6: QuadCount = 2; QuadsIndices[0] = 1; QuadsIndices[1] = 2; break;
					case  7: QuadCount = 3; QuadsIndices[0] = 0; QuadsIndices[1] = 1; QuadsIndices[2] = 2; break; //^^ forward cases
					case  8: QuadCount = 3; QuadsIndices[0] = 3; QuadsIndices[1] = 4; QuadsIndices[2] = 5; break; //vv complement cases
					case  9: QuadCount = 2; QuadsIndices[0] = 4; QuadsIndices[1] = 5; break;
					case 10: QuadCount = 2; QuadsIndices[0] = 3; QuadsIndices[1] = 5; break;
					case 11: QuadCount = 1; QuadsIndices[0] = 5; break;
					case 12: QuadCount = 2; QuadsIndices[0] = 3; QuadsIndices[1] = 4; break;
					case 13: QuadCount = 1; QuadsIndices[0] = 4; break;
					case 14: QuadCount = 1; QuadsIndices[0] = 3; break;
					case 15: QuadCount = 0; break;
					default: checkVoxelSlow(false);
					}

					for (uint32 QuadIndex = 0; QuadIndex < QuadCount; QuadIndex++)
					{
						const uint32* Positions = QuadsPositions[QuadsIndices[QuadIndex]];
						const uint32 Index0 = VertexIndices[VoxelIndex + QuadsOffsets[Positions[0]]];
						const uint32 Index1 = VertexIndices[VoxelIndex + QuadsOffsets[Positions[1]]];
						const uint32 Index2 = VertexIndices[VoxelIndex + QuadsOffsets[Positions[2]]];
						const uint32 Index3 = VertexIndices[VoxelIndex + QuadsOffsets[Positions[3]]];

						Indices.Add(Index0);
						Indices.Add(Index2);
						Indices.Add(Index3);

						Indices.Add(Index3);
						Indices.Add(Index1);
						Indices.Add(Index0);
					}
				}
			}
		}
	}
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelSurfaceNetMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	TArray<uint32> Indices;
	TArray<FVoxelSurfaceNetFullVertex> Vertices;
	CreateGeometryTemplate(Times, Indices, Vertices);

	FVoxelMesherUtilities::SanitizeMesh(Indices, Vertices);

	return MESHER_TIME_RETURN(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(
		Settings,
		LOD,
		MoveTemp(Indices),
		MoveTemp(reinterpret_cast<TArray<FVoxelMesherVertex>&>(Vertices))));
}

void FVoxelSurfaceNetMesher::CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices)
{
	CreateGeometryTemplate(Times, Indices, reinterpret_cast<TArray<FVoxelSurfaceNetGeometryVertex>&>(Vertices));
}