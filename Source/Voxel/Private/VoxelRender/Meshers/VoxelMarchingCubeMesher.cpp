// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelMarchingCubeMesher.h"
#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "Transvoxel.h"
#include "HAL/IConsoleManager.h"

#define checkError(x) if(!(x)) { return false; }

static TAutoConsoleVariable<int32> CVarEnableUniqueUVs(
	TEXT("voxel.mesher.UniqueUVs"),
	0,
	TEXT("If true, will duplicate the vertices to assign to each triangle in a chunk a unique part of the UV space"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarRandomizeTangents(
	TEXT("voxel.mesher.RandomizeTangents"),
	0,
	TEXT("If true, will randomize voxel tangents to help debug materials that should not be using them"),
	ECVF_Default);

class FMarchingCubeHelpers
{
public:
	template<typename T>
	static TArray<FVoxelMesherVertex> CreateMesherVertices(TArray<T>& Vertices)
	{
		VOXEL_FUNCTION_COUNTER();

		TArray<FVoxelMesherVertex> MesherVertices;
		MesherVertices.SetNumUninitialized(Vertices.Num());
		for (int32 Index = 0; Index < Vertices.Num(); Index++)
		{
			auto& Vertex = Vertices[Index];
			auto& MesherVertex = MesherVertices[Index];
			MesherVertex.Position = Vertex.Position;
		}
		return MesherVertices;
	}
	
	template<typename T, typename TMesher>
	static void ComputeMaterials(TMesher& Mesher, TArray<FVoxelMesherVertex>& MesherVertices, TArray<T>& Vertices)
	{
		VOXEL_FUNCTION_COUNTER();
	
		const auto GetMaterial = [&](const FIntVector& P)
		{
			return Mesher.Accelerator->GetMaterial(
				P.X + Mesher.ChunkPosition.X,
				P.Y + Mesher.ChunkPosition.Y, 
				P.Z + Mesher.ChunkPosition.Z, 
				Mesher.LOD);
		};
		if (Mesher.Settings.bInterpolateColors || Mesher.Settings.bInterpolateUVs)
		{
			for (int32 Index = 0; Index < Vertices.Num(); Index++)
			{
				auto& Vertex = Vertices[Index];
				auto& MesherVertex = MesherVertices[Index];

				const auto PositionA = FVoxelUtilities::FloorToInt(MesherVertex.Position);
				const auto PositionB = FVoxelUtilities::CeilToInt(MesherVertex.Position);

				const auto MaterialA = GetMaterial(PositionA);
				const auto MaterialB = GetMaterial(PositionB);
				
				ensureVoxelSlowNoSideEffects(Vertex.MaterialPosition == PositionA || Vertex.MaterialPosition == PositionB);
				MesherVertex.Material = Vertex.MaterialPosition == PositionA ? MaterialA : MaterialB;
				
				const FVector Difference = MesherVertex.Position - FVector(PositionA);
				const float Alpha = Difference.X + Difference.Y + Difference.Z;
				ensureVoxelSlowNoSideEffects(0 <= Alpha && Alpha <= 1);

				if (Mesher.Settings.bInterpolateUVs)
				{
					MesherVertex.Material.SetU0(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetU0(), MaterialB.GetU0(), Alpha)));
					MesherVertex.Material.SetV0(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetV0(), MaterialB.GetV0(), Alpha)));
					MesherVertex.Material.SetU1(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetU1(), MaterialB.GetU1(), Alpha)));
					MesherVertex.Material.SetV1(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetV1(), MaterialB.GetV1(), Alpha)));
					MesherVertex.Material.SetU2(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetU2(), MaterialB.GetU2(), Alpha)));
					MesherVertex.Material.SetV2(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetV2(), MaterialB.GetV2(), Alpha)));
					MesherVertex.Material.SetU3(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetU3(), MaterialB.GetU3(), Alpha)));
					MesherVertex.Material.SetV3(FVoxelUtilities::ClampToUINT8(FMath::Lerp(MaterialA.GetV3(), MaterialB.GetV3(), Alpha)));
				}
				
				if (Mesher.Settings.bInterpolateColors)
				{
					if (Mesher.Settings.MaterialConfig == EVoxelMaterialConfig::RGB)
					{
						MesherVertex.Material.SetColor(FMath::Lerp(MaterialA.GetLinearColor(), MaterialB.GetLinearColor(), Alpha));
					}
					else if (Mesher.Settings.MaterialConfig == EVoxelMaterialConfig::SingleIndex)
					{
						MesherVertex.Material.SetSingleIndex_DataA_AsFloat(FMath::Lerp(MaterialA.GetSingleIndex_DataA_AsFloat(), MaterialB.GetSingleIndex_DataA_AsFloat(), Alpha));
						MesherVertex.Material.SetSingleIndex_DataB_AsFloat(FMath::Lerp(MaterialA.GetSingleIndex_DataB_AsFloat(), MaterialB.GetSingleIndex_DataB_AsFloat(), Alpha));
						MesherVertex.Material.SetSingleIndex_DataC_AsFloat(FMath::Lerp(MaterialA.GetSingleIndex_DataC_AsFloat(), MaterialB.GetSingleIndex_DataC_AsFloat(), Alpha));
					}
					else
					{
						checkVoxelSlow(Mesher.Settings.MaterialConfig == EVoxelMaterialConfig::DoubleIndex);
						MesherVertex.Material.SetDoubleIndex_Blend_AsFloat(FMath::Lerp(MaterialA.GetDoubleIndex_Blend_AsFloat(), MaterialB.GetDoubleIndex_Blend_AsFloat(), Alpha));
						MesherVertex.Material.SetDoubleIndex_Data_AsFloat(FMath::Lerp(MaterialA.GetDoubleIndex_Blend_AsFloat(), MaterialB.GetDoubleIndex_Blend_AsFloat(), Alpha));
					}
				}
			}
		}
		else
		{
			for (int32 Index = 0; Index < Vertices.Num(); Index++)
			{
				auto& Vertex = Vertices[Index];
				auto& MesherVertex = MesherVertices[Index];

				MesherVertex.Material = GetMaterial(Vertex.MaterialPosition);
			}
		}
	}

	static void FixupTangents(TArray<FVoxelMesherVertex>& MesherVertices)
	{
		if (CVarRandomizeTangents.GetValueOnAnyThread() != 0)
		{
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Tangent.TangentX = FVector(FMath::FRandRange(-1, 1), FMath::FRandRange(-1, 1), FMath::FRandRange(-1, 1)).GetSafeNormal();
			}
		}
	}
	
	static void ComputeNormals(FVoxelMarchingCubeMesher& Mesher, TArray<FVoxelMesherVertex>& MesherVertices, TArray<uint32>& Indices)
	{
		VOXEL_FUNCTION_COUNTER();

		const auto GetGradient = [&](const FVector& Position)
		{
			if (Mesher.LOD == 0)
			{
				// For LOD 0, we used the cached data
				// One downside: might not look as great as using the float value from the generator
				// However, the generator at LOD 0 should have small enough values to generate nice normals from them even if they are FVoxelValues
				// Additionally, computing the normals from the generator often requires the interpolation fix at LOD 0,
				// which ends up doing the same as what we are doing here but a lot slower (eg 50ms of 54ms total taken to compute normals!)
				return FVoxelDataUtilities::GetGradientFromGetValue<v_flt>(
					FVoxelDataUtilities::MakeBilinearInterpolatedData(Mesher),
					Position.X,
					Position.Y,
					Position.Z,
					0,
					1);
			}
			else
			{
				return FVoxelDataUtilities::GetGradientFromGetFloatValue<v_flt>(
					*Mesher.Accelerator,
					v_flt(Position.X) + Mesher.ChunkPosition.X,
					v_flt(Position.Y) + Mesher.ChunkPosition.Y,
					v_flt(Position.Z) + Mesher.ChunkPosition.Z,
					Mesher.LOD,
					Mesher.Step);
			}
		};
		
		if (Mesher.Settings.NormalConfig == EVoxelNormalConfig::GradientNormal)
		{
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Normal = GetGradient(Vertex.Position);
				Vertex.Tangent = FVoxelProcMeshTangent();
			}
		}
		else if (Mesher.Settings.NormalConfig == EVoxelNormalConfig::MeshNormal)
		{
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Normal = FVector(0, 0, 0);
				Vertex.Tangent = FVoxelProcMeshTangent();
			}
			for (int32 Index = 0; Index < Indices.Num(); Index += 3)
			{
				auto& A = MesherVertices[Indices[Index + 0]];
				auto& B = MesherVertices[Indices[Index + 1]];
				auto& C = MesherVertices[Indices[Index + 2]];
				const FVector Normal = FVector::CrossProduct(C.Position - A.Position, B.Position - A.Position).GetSafeNormal();
				A.Normal += Normal;
				B.Normal += Normal;
				C.Normal += Normal;
			}
			for (auto& Vertex : MesherVertices)
			{
				if (Vertex.Position.X < Mesher.Step ||
					Vertex.Position.Y < Mesher.Step ||
					Vertex.Position.Z < Mesher.Step ||
					Vertex.Position.X > (RENDER_CHUNK_SIZE - 1) * Mesher.Step ||
					Vertex.Position.Y > (RENDER_CHUNK_SIZE - 1) * Mesher.Step ||
					Vertex.Position.Z > (RENDER_CHUNK_SIZE - 1) * Mesher.Step)
				{
					// Can't use mesh normals on edges, as it looks like crap because of the missing neighbor vertices
					Vertex.Normal = GetGradient(Vertex.Position);
				}
				else
				{
					Vertex.Normal.Normalize();
				}
			}
		}
		else
		{
			check(Mesher.Settings.NormalConfig == EVoxelNormalConfig::NoNormal);
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Normal = FVector(0, 0, 0);
				Vertex.Tangent = FVoxelProcMeshTangent();
			}
		}

		FixupTangents(MesherVertices);
	}
	static void ComputeNormals(FVoxelMarchingCubeTransitionsMesher& Mesher, TArray<FVoxelMesherVertex>& MesherVertices)
	{
		VOXEL_FUNCTION_COUNTER();

		if (Mesher.Settings.NormalConfig == EVoxelNormalConfig::GradientNormal || Mesher.Settings.NormalConfig == EVoxelNormalConfig::MeshNormal)
		{
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Normal = FVoxelDataUtilities::GetGradientFromGetFloatValue<v_flt>(
					*Mesher.Accelerator,
					v_flt(Vertex.Position.X) + Mesher.ChunkPosition.X,
					v_flt(Vertex.Position.Y) + Mesher.ChunkPosition.Y,
					v_flt(Vertex.Position.Z) + Mesher.ChunkPosition.Z,
					Mesher.LOD,
					Mesher.Step);;
				Vertex.Tangent = FVoxelProcMeshTangent();
			}
		}
		else
		{
			check(Mesher.Settings.NormalConfig == EVoxelNormalConfig::NoNormal);
			for (auto& Vertex : MesherVertices)
			{
				Vertex.Normal = FVector(0, 0, 0);
				Vertex.Tangent = FVoxelProcMeshTangent();
			}
		}

		FixupTangents(MesherVertices);
	}
	
	template<typename TMesher>
	static void ComputeUVs(TMesher& Mesher, TArray<FVoxelMesherVertex>& MesherVertices)
	{
		VOXEL_FUNCTION_COUNTER();

		for (auto& Vertex : MesherVertices)
		{
			Vertex.TextureCoordinate = FVoxelMesherUtilities::GetUVs(Mesher, Vertex.Position);
		}
	}
};

FIntBox FVoxelMarchingCubeMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FIntBox(ChunkPosition, ChunkPosition + CHUNK_SIZE_WITH_END_EDGE * Step);
}

FIntBox FVoxelMarchingCubeMesher::GetBoundsToLock() const
{
	// We need to lock for the normals (also work with LOD 0)
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition + FIntVector(Step) + CHUNK_SIZE_WITH_END_EDGE * Step);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	
TVoxelSharedPtr<FVoxelChunkMesh> FVoxelMarchingCubeMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	VOXEL_FUNCTION_COUNTER();

	struct FLocalVertex
	{
		FVector Position;
		FIntVector MaterialPosition;

		FLocalVertex() = default;
		FORCEINLINE FLocalVertex(const FVector& Position, const FIntVector& MaterialPosition)
			: Position(Position)
			, MaterialPosition(MaterialPosition)
		{
		}
	};
	
	TArray<uint32> Indices;
	TArray<FLocalVertex> Vertices;
	CreateGeometryTemplate(Times, Indices, Vertices);

	FVoxelMesherUtilities::SanitizeMesh(Indices, Vertices);

	TArray<FVoxelMesherVertex> MesherVertices = FMarchingCubeHelpers::CreateMesherVertices(Vertices);

	MESHER_TIME_MATERIALS(MesherVertices.Num(), FMarchingCubeHelpers::ComputeMaterials(*this, MesherVertices, Vertices));
	MESHER_TIME(Normals, FMarchingCubeHelpers::ComputeNormals(*this, MesherVertices, Indices));

	UnlockData();

	MESHER_TIME(UVs, FMarchingCubeHelpers::ComputeUVs(*this, MesherVertices));

	if (CVarEnableUniqueUVs.GetValueOnAnyThread() != 0)
	{
		{
			TArray<FVoxelMesherVertex> NewVertices;
			NewVertices.Reserve(Indices.Num());
			TArray<uint32> NewIndices;
			NewIndices.Reserve(Indices.Num());

			for (uint32 Index : Indices)
			{
				NewIndices.Add(NewVertices.Num());
				NewVertices.Add(MesherVertices[Index]);
			}

			MesherVertices = MoveTemp(NewVertices);
			Indices = MoveTemp(NewIndices);
		}
		
		const int32 NumTriangles = Indices.Num() / 3;
		const int32 NumSquares = FVoxelUtilities::DivideCeil(NumTriangles, 2);
		const int32 NumSquaresPerRow = FMath::CeilToInt(FMath::Sqrt(NumSquares));
		check(NumSquares <= NumSquaresPerRow * NumSquaresPerRow);

		const int32 TextureUVSize = 1;
		const float SquareUVSize = TextureUVSize / float(NumSquaresPerRow);

		const auto AddTriangle = [&Indices, &MesherVertices](int32 TriangleIndex, const FVector2D& A, const FVector2D& B, const FVector2D& C)
		{
			const int32 IndexA = Indices[3 * TriangleIndex + 0];
			const int32 IndexB = Indices[3 * TriangleIndex + 1];
			const int32 IndexC = Indices[3 * TriangleIndex + 2];
			MesherVertices[IndexA].TextureCoordinate = A;
			MesherVertices[IndexB].TextureCoordinate = B;
			MesherVertices[IndexC].TextureCoordinate = C;
		};

		int32 Row = 0;
		int32 Column = 0;
		for (int32 TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex += 2)
		{
			/**
			 * A B
			 * C D
			 */
			const FVector2D A = { (Column + 0) * SquareUVSize, (Row + 0) * SquareUVSize };
			const FVector2D B = { (Column + 1) * SquareUVSize, (Row + 0) * SquareUVSize };
			const FVector2D C = { (Column + 0) * SquareUVSize, (Row + 1) * SquareUVSize };
			const FVector2D D = { (Column + 1) * SquareUVSize, (Row + 1) * SquareUVSize };

			AddTriangle(TriangleIndex, A, B, D);

			if (TriangleIndex + 1 < NumTriangles)
			{
				AddTriangle(TriangleIndex + 1, A, D, C);
			}
			
			Column++;
			if (Column == NumSquaresPerRow)
			{
				Column = 0;
				Row++;
			}
		}
	}

	return MESHER_TIME_RETURN(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(
		Settings,
		MoveTemp(Indices),
		MoveTemp(MesherVertices)));
}

void FVoxelMarchingCubeMesher::CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices)
{
	VOXEL_FUNCTION_COUNTER();

	struct FVectorVertex : FVector
	{
		FVectorVertex() = default;
		FORCEINLINE FVectorVertex(const FVector& Position, const FIntVector&)
			: FVector(Position)
		{
		}
	};
	CreateGeometryTemplate(Times, Indices, reinterpret_cast<TArray<FVectorVertex>&>(Vertices));
	UnlockData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool FVoxelMarchingCubeMesher::CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices)
{
	VOXEL_FUNCTION_COUNTER();

	const int32 DataSize = LOD == 0 ? CHUNK_SIZE_WITH_NORMALS : CHUNK_SIZE_WITH_END_EDGE;

	FIntBox BoundsToQuery(ChunkPosition, ChunkPosition + CHUNK_SIZE_WITH_END_EDGE * Step);
	if (LOD == 0)
	{
		// Account for normals
		BoundsToQuery = BoundsToQuery.Extend(1);
	}
	TVoxelQueryZone<FVoxelValue> QueryZone(BoundsToQuery, FIntVector(DataSize), LOD, CachedValues);
	MESHER_TIME_VALUES(DataSize * DataSize * DataSize, Data.Get<FVoxelValue>(QueryZone, LOD));
	
	Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());

	uint32 VoxelIndex = 0;
	if (LOD == 0) VoxelIndex += DataSize * DataSize; // Additional voxel for normals
	for (int32 LZ = 0; LZ < RENDER_CHUNK_SIZE; LZ++)
	{
		if (LOD == 0) VoxelIndex += DataSize; // Additional voxel for normals
		for (int32 LY = 0; LY < RENDER_CHUNK_SIZE; LY++)
		{
			if (LOD == 0) VoxelIndex += 1; // Additional voxel for normals
			for (int32 LX = 0; LX < RENDER_CHUNK_SIZE; LX++)
			{
				{
					CurrentCache[GetCacheIndex(0, LX, LY)] = -1; // Set EdgeIndex 0 to -1 if the cell isn't voxelized, eg all corners = 0

					uint32 CubeIndices[8];
					CubeIndices[0] = VoxelIndex;
					CubeIndices[1] = VoxelIndex + 1;
					CubeIndices[2] = VoxelIndex     + DataSize;
					CubeIndices[3] = VoxelIndex + 1 + DataSize;
					CubeIndices[4] = VoxelIndex                + DataSize * DataSize;
					CubeIndices[5] = VoxelIndex + 1            + DataSize * DataSize;
					CubeIndices[6] = VoxelIndex     + DataSize + DataSize * DataSize;
					CubeIndices[7] = VoxelIndex + 1 + DataSize + DataSize * DataSize;

					checkVoxelSlow(CubeIndices[0] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[1] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[2] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[3] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[4] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[5] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[6] < uint32(DataSize * DataSize * DataSize));
					checkVoxelSlow(CubeIndices[7] < uint32(DataSize * DataSize * DataSize));

					const uint32 CaseCode =
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

						const uint8 ValidityMask = (LX != 0) + 2 * (LY != 0) + 4 * (LZ != 0);

						checkVoxelSlow(0 <= CaseCode && CaseCode < 256);
						const uint8 CellClass = Transvoxel::regularCellClass[CaseCode];
						const uint16* RESTRICT VertexData = Transvoxel::regularVertexData[CaseCode];
						checkVoxelSlow(0 <= CellClass && CellClass < 16);
						Transvoxel::RegularCellData CellData = Transvoxel::regularCellData[CellClass];

						// Indices of the vertices used in this cube
						TStackArray<int32, 16> VertexIndices;
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
								
								VertexIndex = (ZIsDifferent ? OldCache : CurrentCache)[GetCacheIndex(EdgeIndex, LX - XIsDifferent, LY - YIsDifferent)];
								ensureVoxelSlowNoSideEffects(-1 <= VertexIndex && VertexIndex < Vertices.Num()); // Can happen if the generator is returning different values
							}

							if (!bIsVertexCached || VertexIndex == -1)
							{
								// We are on one the lower edges of the chunk. Compute vertex
							
								const FIntVector PositionA((LX + (LocalIndexA & 0x01)) * Step, (LY + ((LocalIndexA & 0x02) >> 1)) * Step, (LZ + ((LocalIndexA & 0x04) >> 2)) * Step);
								const FIntVector PositionB((LX + (LocalIndexB & 0x01)) * Step, (LY + ((LocalIndexB & 0x02) >> 1)) * Step, (LZ + ((LocalIndexB & 0x04) >> 2)) * Step);

								FVector IntersectionPoint;
								FIntVector MaterialPosition;

								if (EdgeIndex == 0)
								{
									if (ValueAtA.IsNull())
									{
										IntersectionPoint = FVector(PositionA);
										MaterialPosition = PositionA;
									}
									else 
									{
										checkVoxelSlow(ValueAtB.IsNull());
										IntersectionPoint = FVector(PositionB);
										MaterialPosition = PositionB;
									}
								}
								else if (LOD == 0)
								{
									// Full resolution

									const float Alpha = ValueAtA.ToFloat() / (ValueAtA.ToFloat() - ValueAtB.ToFloat());
									checkError(!FMath::IsNaN(Alpha) && FMath::IsFinite(Alpha));
									
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
									default:
										checkVoxelSlow(false);
									}

									// Use the material of the point inside
									MaterialPosition = !ValueAtA.IsEmpty() ? PositionA : PositionB;
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

										FVoxelValue ValueAtMiddle = MESHER_TIME_RETURN_VALUES(1, Accelerator->Get<FVoxelValue>(
											(bIsAlongX ? Middle : PositionA.X) + ChunkPosition.X,
											(bIsAlongY ? Middle : PositionA.Y) + ChunkPosition.Y,
											(bIsAlongZ ? Middle : PositionA.Z) + ChunkPosition.Z, LOD));

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

									const float Alpha = ValueAtACopy.ToFloat() / (ValueAtACopy.ToFloat() - ValueAtBCopy.ToFloat());
									checkError(!FMath::IsNaN(Alpha) && FMath::IsFinite(Alpha));

									const float R = FMath::Lerp<float>(Min, Max, Alpha);
									IntersectionPoint = FVector(
										bIsAlongX ? R : PositionA.X,
										bIsAlongY ? R : PositionA.Y,
										bIsAlongZ ? R : PositionA.Z);

									// Get intersection material
									if (!ValueAtACopy.IsEmpty())
									{
										checkVoxelSlow(ValueAtBCopy.IsEmpty());
										MaterialPosition = FIntVector(
											bIsAlongX ? Min : PositionA.X,
											bIsAlongY ? Min : PositionA.Y,
											bIsAlongZ ? Min : PositionA.Z);
									}
									else
									{
										checkVoxelSlow(!ValueAtBCopy.IsEmpty());
										MaterialPosition = FIntVector(
											bIsAlongX ? Max : PositionA.X,
											bIsAlongY ? Max : PositionA.Y,
											bIsAlongZ ? Max : PositionA.Z);
									}
								}

								VertexIndex = Vertices.Num();

								Vertices.Add(T(IntersectionPoint, MaterialPosition));

								checkVoxelSlow((ValueAtB.IsNull() && LocalIndexB == 7) == !CacheDirection);
								checkVoxelSlow(CacheDirection || EdgeIndex == 0);

								// Save vertex if not on edge
								if (CacheDirection & 0x08 || !CacheDirection) // ValueAtB.IsNull() && LocalIndexB == 7 => !CacheDirection
								{
									CurrentCache[GetCacheIndex(EdgeIndex, LX, LY)] = VertexIndex;
								}
							}

							VertexIndices[I] = VertexIndex;
							checkVoxelSlow(0 <= VertexIndex && VertexIndex < Vertices.Num());
						}

						// Add triangles
						// 3 vertex per triangle
						for (int32 Index = 0; Index < 3 * CellData.GetTriangleCount(); Index += 3)
						{
							Indices.Add(VertexIndices[CellData.vertexIndex[Index + 0]]);
							Indices.Add(VertexIndices[CellData.vertexIndex[Index + 1]]);
							Indices.Add(VertexIndices[CellData.vertexIndex[Index + 2]]);
						}
					}
				}

				VoxelIndex++;
			}
			VoxelIndex += 1; // End edge voxel
			if (LOD == 0) VoxelIndex += 1; // Additional voxel for normals
		}
		VoxelIndex += DataSize; // End edge voxel
		if (LOD == 0) VoxelIndex += DataSize; // Additional voxel for normals

		// Can't use Unreal Swap on restrict ptrs with clang
		std::swap(CurrentCache, OldCache);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE int32 FVoxelMarchingCubeMesher::GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY)
{
	checkVoxelSlow(0 <= LX && LX < RENDER_CHUNK_SIZE);
	checkVoxelSlow(0 <= LY && LY < RENDER_CHUNK_SIZE);
	checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < EDGE_INDEX_COUNT);
	return EdgeIndex + LX * EDGE_INDEX_COUNT + LY * EDGE_INDEX_COUNT * RENDER_CHUNK_SIZE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntBox FVoxelMarchingCubeTransitionsMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FIntBox(ChunkPosition, ChunkPosition + CHUNK_SIZE_WITH_END_EDGE * Step);
}

FIntBox FVoxelMarchingCubeTransitionsMesher::GetBoundsToLock() const
{
	return FIntBox(ChunkPosition - FIntVector(Step), ChunkPosition + FIntVector(Step) + CHUNK_SIZE_WITH_END_EDGE * Step);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool FVoxelMarchingCubeTransitionsMesher::CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices)
{
	VOXEL_FUNCTION_COUNTER();
	
	Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());

	bool bSuccess = true;
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::XMin>(Times, Indices, Vertices);
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::XMax>(Times, Indices, Vertices);
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::YMin>(Times, Indices, Vertices);
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::YMax>(Times, Indices, Vertices);
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::ZMin>(Times, Indices, Vertices);
	bSuccess &= CreateGeometryForDirection<EVoxelDirection::ZMax>(Times, Indices, Vertices);
	return bSuccess;
}

template<uint8 Direction, typename T>
bool FVoxelMarchingCubeTransitionsMesher::CreateGeometryForDirection(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices)
{
	if (!(TransitionsMask & Direction)) return true;
	
#if VOXEL_DEBUG
	for (auto& Value : Cache2D)
	{
		Value = -100;
	}
#endif

	for (int32 LX = 0; LX < RENDER_CHUNK_SIZE; LX++)
	{
		for (int32 LY = 0; LY < RENDER_CHUNK_SIZE; LY++)
		{
			// Set EdgeIndex 0, 1, 2 and 7 to -1 for when the cell aren't polygonized (0 on all corners)
			Cache2D[GetCacheIndex(0, LX, LY)] = -1;
			Cache2D[GetCacheIndex(1, LX, LY)] = -1;
			Cache2D[GetCacheIndex(2, LX, LY)] = -1;
			Cache2D[GetCacheIndex(7, LX, LY)] = -1;

			FVoxelValue CornerValues[13];

			{
				MESHER_TIME_SCOPE_VALUES(13);
				
				CornerValues[0] = GetValue<Direction>((2 * LX + 0) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
				CornerValues[1] = GetValue<Direction>((2 * LX + 1) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
				CornerValues[2] = GetValue<Direction>((2 * LX + 2) * HalfStep, (2 * LY + 0) * HalfStep, HalfLOD);
				CornerValues[3] = GetValue<Direction>((2 * LX + 0) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
				CornerValues[4] = GetValue<Direction>((2 * LX + 1) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
				CornerValues[5] = GetValue<Direction>((2 * LX + 2) * HalfStep, (2 * LY + 1) * HalfStep, HalfLOD);
				CornerValues[6] = GetValue<Direction>((2 * LX + 0) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);
				CornerValues[7] = GetValue<Direction>((2 * LX + 1) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);
				CornerValues[8] = GetValue<Direction>((2 * LX + 2) * HalfStep, (2 * LY + 2) * HalfStep, HalfLOD);

				CornerValues[9] = GetValue<Direction>((LX + 0) * Step, (LY + 0) * Step, LOD);
				CornerValues[10] = GetValue<Direction>((LX + 1) * Step, (LY + 0) * Step, LOD);
				CornerValues[11] = GetValue<Direction>((LX + 0) * Step, (LY + 1) * Step, LOD);
				CornerValues[12] = GetValue<Direction>((LX + 1) * Step, (LY + 1) * Step, LOD);
			}

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

			const uint32 CaseCode =
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
				const uint8 ValidityMask = (LX != 0) + 2 * (LY != 0);

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

				TArray<int32, TFixedAllocator<64>> VertexIndices; // Not sure how many indices max, let's just say 64
				VertexIndices.SetNumUninitialized(CellData.GetVertexCount());

				for (int32 i = 0; i < CellData.GetVertexCount(); i++)
				{
					int32 VertexIndex = -1;
					const uint16& EdgeCode = VertexData[i];

					// A: low point / B: high point
					const uint8 IndexVertexA = (EdgeCode >> 4) & 0x0F;
					const uint8 IndexVertexB = EdgeCode & 0x0F;

					checkVoxelSlow(0 <= IndexVertexA && IndexVertexA < 13);
					checkVoxelSlow(0 <= IndexVertexB && IndexVertexB < 13);

					const FIntVector& PositionA = Positions[IndexVertexA];
					const FIntVector& PositionB = Positions[IndexVertexB];
						
					const FVoxelValue& ValueAtA = CornerValues[IndexVertexA];
					const FVoxelValue& ValueAtB = CornerValues[IndexVertexB];

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
							CacheDirection = CacheDirectionMap[IndexVertexA];
						}
						if (ValueAtB.IsNull())
						{
							checkVoxelSlow(!ValueAtA.IsNull());
							static uint8 EdgeIndexMap[10] = {0, 1, 2, 1, 0, 2, 0, 7, 7, 7};
							EdgeIndex = EdgeIndexMap[EdgeIndex];
							CacheDirection = CacheDirectionMap[IndexVertexB];
						}
					}
					const bool bIsVertexCached = ((ValidityMask & CacheDirection) == CacheDirection);

					if (bIsVertexCached)
					{
						checkVoxelSlow(!(CacheDirection & 0x08) && !(CacheDirection & 0x04));

						const bool XIsDifferent = !!(CacheDirection & 0x01);
						const bool YIsDifferent = !!(CacheDirection & 0x02);
						
						VertexIndex = Cache2D[GetCacheIndex(EdgeIndex, LX - XIsDifferent, LY - YIsDifferent)];
						checkVoxelSlow(-1 <= VertexIndex && VertexIndex < Vertices.Num());
					}

					if (!bIsVertexCached || VertexIndex == -1)
					{
						FVector IntersectionPoint;
						FIntVector MaterialPosition;
						
						const bool bIsLowResChunk = EdgeIndex == 7 || EdgeIndex == 8 || EdgeIndex == 9;

						if (EdgeIndex == 0 || EdgeIndex == 1 || EdgeIndex == 2 || EdgeIndex == 7)
						{
							if (ValueAtA.IsNull())
							{
								const auto P = Local2DToGlobal<Direction>(PositionA.X, PositionA.Y, 0);
								IntersectionPoint = FVector(P);
								MaterialPosition = P;
							}
							else
							{
								checkVoxelSlow(ValueAtB.IsNull());
								const auto P = Local2DToGlobal<Direction>(PositionB.X, PositionB.Y, 0);
								IntersectionPoint = FVector(P);
								MaterialPosition = P;
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
								const int32 Middle = (Max + Min) / 2;

								FVoxelValue ValueAtMiddle = MESHER_TIME_RETURN_VALUES(1, GetValue<Direction>(
									bIsAlongX ? Middle : PositionA.X,
									bIsAlongY ? Middle : PositionA.Y,
									bIsLowResChunk ? LOD : HalfLOD));

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

							const float Alpha = ValueAtACopy.ToFloat() / (ValueAtACopy.ToFloat() - ValueAtBCopy.ToFloat());
							checkError(!FMath::IsNaN(Alpha) && FMath::IsFinite(Alpha));

							const FIntVector GlobalMin = Local2DToGlobal<Direction>(
								bIsAlongX ? Min : PositionA.X,
								bIsAlongY ? Min : PositionA.Y,
								0);
							const FIntVector GlobalMax = Local2DToGlobal<Direction>(
								bIsAlongX ? Max : PositionA.X,
								bIsAlongY ? Max : PositionA.Y,
								0);

							IntersectionPoint = FMath::Lerp(FVector(GlobalMin), FVector(GlobalMax), Alpha);

							// Get intersection material
							if (!ValueAtACopy.IsEmpty())
							{
								checkVoxelSlow(ValueAtBCopy.IsEmpty());
								MaterialPosition = GlobalMin;
							}
							else
							{
								checkVoxelSlow(!ValueAtBCopy.IsEmpty());
								MaterialPosition = GlobalMax;
							}
						}

						VertexIndex = Vertices.Num();								
						Vertices.Emplace(T(IntersectionPoint, MaterialPosition, bIsLowResChunk));

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
				// 3 vertex per triangle
				const int32 NumIndices = 3 * CellData.GetTriangleCount();
				if (bFlip)
				{
					for (int32 Index = 0; Index < NumIndices; Index += 3)
					{
						Indices.Add(VertexIndices[CellData.vertexIndex[NumIndices - 1 - (Index + 0)]]);
						Indices.Add(VertexIndices[CellData.vertexIndex[NumIndices - 1 - (Index + 1)]]);
						Indices.Add(VertexIndices[CellData.vertexIndex[NumIndices - 1 - (Index + 2)]]);
					}
				}
				else
				{
					for (int32 Index = 0; Index < NumIndices; Index += 3)
					{
						Indices.Add(VertexIndices[CellData.vertexIndex[Index + 0]]);
						Indices.Add(VertexIndices[CellData.vertexIndex[Index + 1]]);
						Indices.Add(VertexIndices[CellData.vertexIndex[Index + 2]]);
					}
				}
			}
		}
	}
	return true;
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelMarchingCubeTransitionsMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	VOXEL_FUNCTION_COUNTER();
	
	struct FLocalVertex
	{
		FVector Position;
		FIntVector MaterialPosition;
		bool bNeedToTranslateVertex;

		FLocalVertex() = default;
		FORCEINLINE FLocalVertex(const FVector& Position, const FIntVector& MaterialPosition, bool bNeedToTranslateVertex)
			: Position(Position)
			, MaterialPosition(MaterialPosition)
			, bNeedToTranslateVertex(bNeedToTranslateVertex)
		{
		}
	};

	TArray<uint32> Indices;
	TArray<FLocalVertex> Vertices;

	if (!CreateGeometryTemplate(Times, Indices, Vertices))
	{
		return {};
	}

	TArray<FVoxelMesherVertex> MesherVertices = FMarchingCubeHelpers::CreateMesherVertices(Vertices);

	MESHER_TIME_MATERIALS(MesherVertices.Num(), FMarchingCubeHelpers::ComputeMaterials(*this, MesherVertices, Vertices));
	MESHER_TIME(Normals, FMarchingCubeHelpers::ComputeNormals(*this, MesherVertices));

	UnlockData();

	MESHER_TIME(UVs, FMarchingCubeHelpers::ComputeUVs(*this, MesherVertices));

	{
		VOXEL_SCOPE_COUNTER("Translate Vertices");
		for (int32 Index = 0; Index < Vertices.Num(); Index++)
		{
			if (Vertices[Index].bNeedToTranslateVertex)
			{
				auto& Vertex = MesherVertices[Index];
				Vertex.Position = FVoxelMesherUtilities::GetTranslatedTransvoxel(Vertex.Position, Vertex.Normal, TransitionsMask, LOD);
			}
		}
	}

	// Important: sanitize AFTER translating!
	FVoxelMesherUtilities::SanitizeMesh(Indices, MesherVertices);

	return MESHER_TIME_RETURN(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(Settings, MoveTemp(Indices), MoveTemp(MesherVertices)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE int32 FVoxelMarchingCubeTransitionsMesher::GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY)
{
	checkVoxelSlow(0 <= LX && LX < RENDER_CHUNK_SIZE);
	checkVoxelSlow(0 <= LY && LY < RENDER_CHUNK_SIZE);
	checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < TRANSITION_EDGE_INDEX_COUNT);
	return EdgeIndex + LX * TRANSITION_EDGE_INDEX_COUNT + LY * TRANSITION_EDGE_INDEX_COUNT * RENDER_CHUNK_SIZE;
}

template<uint8 Direction>
FORCEINLINE FVoxelValue FVoxelMarchingCubeTransitionsMesher::GetValue(int32 X, int32 Y, int32 InLOD) const
{
	const FIntVector GlobalPosition = Local2DToGlobal<Direction>(X, Y, 0);
	return Accelerator->Get<FVoxelValue>(ChunkPosition + GlobalPosition, InLOD);
}

template<uint8 Direction>
FORCEINLINE FIntVector FVoxelMarchingCubeTransitionsMesher::Local2DToGlobal(int32 X, int32 Y, int32 Z) const
{
	switch (Direction)
	{
	case EVoxelDirection::XMin:
		return { Z, X, Y };
	case EVoxelDirection::XMax:
		return { Size - Z, Y, X };
	case EVoxelDirection::YMin:
		return { Y, Z, X };
	case EVoxelDirection::YMax:
		return { X, Size - Z, Y };
	case EVoxelDirection::ZMin:
		return { X, Y, Z };
	case EVoxelDirection::ZMax:
		return { Y, X, Size - Z };
	default:
		check(false);
		return {};
	}
}

#undef checkError