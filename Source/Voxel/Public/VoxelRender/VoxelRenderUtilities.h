// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "VoxelConfigEnums.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelDebug/VoxelStats.h"

class AVoxelWorld;
struct FVoxelRendererSettings;
struct FVoxelRenderChunkSettings;

class VOXEL_API FVoxelRenderUtilities
{
public:
	inline static FVector GetTranslated(const FVector& Vertex, const FVector& Normal, uint8 TransitionsMask, uint8 LOD)
	{
		const int32 Step = 1 << LOD;
		const int32 Size = CHUNK_SIZE << LOD;
		
		const float LowerBound = Step;
		const float UpperBound = (CHUNK_SIZE - 1) * Step;
		
		if ((LowerBound <= Vertex.X && Vertex.X <= UpperBound) &&
			(LowerBound <= Vertex.Y && Vertex.Y <= UpperBound) &&
			(LowerBound <= Vertex.Z && Vertex.Z <= UpperBound))
		{
			// Fast exit
			return Vertex;
		}
		
		if ((Vertex.X == 0.f && !(TransitionsMask & XMin)) || (Vertex.X == Size && !(TransitionsMask & XMax)) ||
	  	    (Vertex.Y == 0.f && !(TransitionsMask & YMin)) || (Vertex.Y == Size && !(TransitionsMask & YMax)) ||
	  	    (Vertex.Z == 0.f && !(TransitionsMask & ZMin)) || (Vertex.Z == Size && !(TransitionsMask & ZMax)))
		{
			// Can't translate when on a corner
			return Vertex;
		}

		FVector Delta(0.f);

		if ((TransitionsMask & XMin) && Vertex.X < LowerBound)
		{
			Delta.X = LowerBound - Vertex.X;
		}
		if ((TransitionsMask & XMax) && Vertex.X > UpperBound)
		{
			Delta.X = UpperBound - Vertex.X;
		}
		if ((TransitionsMask & YMin) && Vertex.Y < LowerBound)
		{
			Delta.Y = LowerBound - Vertex.Y;
		}
		if ((TransitionsMask & YMax) && Vertex.Y > UpperBound)
		{
			Delta.Y = UpperBound - Vertex.Y;
		}
		if ((TransitionsMask & ZMin) && Vertex.Z < LowerBound)
		{
			Delta.Z = LowerBound - Vertex.Z;
		}
		if ((TransitionsMask & ZMax) && Vertex.Z > UpperBound)
		{
			Delta.Z = UpperBound - Vertex.Z;
		}

		Delta /= 4;

		FVector Q = FVector(
			(1 - Normal.X * Normal.X) * Delta.X -      Normal.Y * Normal.X  * Delta.Y -      Normal.Z * Normal.X  * Delta.Z,
			   - Normal.X * Normal.Y  * Delta.X + (1 - Normal.Y * Normal.Y) * Delta.Y -      Normal.Z * Normal.Y  * Delta.Z,
			   - Normal.X * Normal.Z  * Delta.X -      Normal.Y * Normal.Z  * Delta.Y + (1 - Normal.Z * Normal.Z) * Delta.Z);

		return Vertex + Q;
	}

	template<typename TVertex, typename TAllocatorA, typename TAllocatorB>
	static void ConvertArrays(
		int32 LOD,
		EVoxelMaterialConfig MaterialConfig,
		FVoxelMeshProcessingParameters MeshParameters,
		TArray<uint32, TAllocatorA>&& Indices,
		TArray<TVertex, TAllocatorB>&& Vertices,
		FVoxelChunk& OutChunk, 
		FVoxelStatsElement& Stats)
	{
		Stats.SetNumIndices(Indices.Num());
		Stats.SetNumVertices(Vertices.Num());
		Stats.SetMaterialConfig(MaterialConfig);

		if (MaterialConfig == EVoxelMaterialConfig::RGB)
		{
			OutChunk.bSingleBuffers = true;
			FVoxelChunkBuffers& Buffers = OutChunk.SingleBuffers;

			Buffers.SetIndices(MoveTemp(Indices));
			Buffers.LoadVerticesFromArray(Vertices);
		}
		else if (MaterialConfig == EVoxelMaterialConfig::DoubleIndex)
		{
			struct FDoubleIndex
			{
				uint8 IndexA;
				uint8 IndexB;

				FDoubleIndex() = default;
				FDoubleIndex(uint8 IndexA, uint8 IndexB)
					: IndexA(IndexA)
					, IndexB(IndexB)
				{
				}
				FDoubleIndex(const FVoxelMaterial& Material)
					: IndexA(Material.GetIndexA())
					, IndexB(Material.GetIndexB())
				{
				}

				inline bool Equals(uint8 InIndexA, uint8 InIndexB) const
				{
					return IndexA == InIndexA && IndexB == InIndexB;
				}
			};
			struct FDoubleIndexBlend : public FDoubleIndex
			{
				uint8 Blend;

				FDoubleIndexBlend() = default;
				FDoubleIndexBlend(uint8 IndexA, uint8 IndexB, uint8 Blend)
					: FDoubleIndex(IndexA, IndexB)
					, Blend(Blend)
				{
				}
				FDoubleIndexBlend(const FVoxelMaterial& Material)
					: FDoubleIndex(Material)
					, Blend(Material.GetBlend())
				{
				}
				
				static inline FDoubleIndexBlend GetBest(const FDoubleIndexBlend& A, const FDoubleIndexBlend& B, const FDoubleIndexBlend& C)
				{
					uint8 Strengths[256];
					TArray<uint8, TFixedAllocator<6>> UsedIds;

					struct FHelper
					{
						static void AddIndex(
							uint8 Index,
							uint8 Strength,
							uint8 Strengths[256],
							TArray<uint8, TFixedAllocator<6>>& UsedIds)
						{
							if (!UsedIds.Contains(Index))
							{
								UsedIds.Add(Index);
								Strengths[Index] = 0;
							}
							Strengths[Index] = FMath::Max(Strengths[Index], Strength);
						}
						static void AddDoubleIndexBlend(
							const FDoubleIndexBlend& A,
							uint8 Strengths[256],
							TArray<uint8, TFixedAllocator<6>>& UsedIds)
						{
							AddIndex(A.IndexA, 255 - A.Blend, Strengths, UsedIds);
							AddIndex(A.IndexB, A.Blend, Strengths, UsedIds);
						}
					};

					FHelper::AddDoubleIndexBlend(A, Strengths, UsedIds);
					FHelper::AddDoubleIndexBlend(B, Strengths, UsedIds);
					FHelper::AddDoubleIndexBlend(C, Strengths, UsedIds);

					uint8 MaxIndexA = 0;
					int32 MaxIndexAStrength = -1;
					uint8 MaxIndexB = 0;
					int32 MaxIndexBStrength = -1;
					for (uint8 Id : UsedIds)
					{
						uint8 Strength = Strengths[Id];
						if (Strength >= MaxIndexAStrength)
						{
							MaxIndexB = MaxIndexA;
							MaxIndexBStrength = MaxIndexAStrength;
							MaxIndexA = Id;
							MaxIndexAStrength = Strength;
						}
						else if (Strength > MaxIndexBStrength)
						{
							MaxIndexB = Id;
							MaxIndexBStrength = Strength;
						}
					}
					checkVoxelSlow(MaxIndexAStrength >= 0);
					if (MaxIndexBStrength < 0)
					{
						MaxIndexB = MaxIndexA;
						MaxIndexBStrength = 0;
					}

					int32 Strength = ((255 - MaxIndexAStrength) + MaxIndexBStrength) / 2;
					checkVoxelSlow(0 <= Strength && Strength < 256);
					return FDoubleIndexBlend(MaxIndexA, MaxIndexB, Strength);
				}

				inline uint8 GetBlendFor(const FDoubleIndexBlend& Other) const
				{
					// Check if we're equal
					if (Other.Equals(IndexA, IndexB))
					{
						return Blend;
					}
					// Check if we're equal, but with reversed indices
					if (Other.Equals(IndexB, IndexA))
					{
						return 255 - Blend;
					}
					// Check if we have 2 same indices: in that case check if our single index is in Other
					if (IndexA == IndexB)
					{
						if (IndexA == Other.IndexA)
						{
							return 0;
						}
						if (IndexA == Other.IndexB)
						{
							return 255;
						}
					}
					else
					{
						// If we have different indices, check if our blend is maximal/minimal and if so if the corresponding index is in Other
						if (Blend == 0)
						{
							if (IndexA == Other.IndexA)
							{
								return 0;
							}
							if (IndexA == Other.IndexB)
							{
								return 255;
							}
						}
						if (Blend == 255)
						{
							if (IndexB == Other.IndexA)
							{
								return 0;
							}
							if (IndexB == Other.IndexB)
							{
								return 255;
							}
						}
					}
					// We don't have any info on the other index blend, so return Other
					return Other.Blend;
				}
			};

			OutChunk.bSingleBuffers = false;

			TMap<FVoxelBlendedMaterial, TMap<int32, int32>> IndicesMaps;

			for (int32 I = 0; I < Indices.Num(); I += 3)
			{
				int32 IndexA = Indices[I];
				int32 IndexB = Indices[I + 1];
				int32 IndexC = Indices[I + 2];

				const TVertex& VertexA = Vertices[IndexA];
				const TVertex& VertexB = Vertices[IndexB];
				const TVertex& VertexC = Vertices[IndexC];

				// Same indices on the whole triangle, only blends change
				FDoubleIndex DoubleIndex;
				uint8 BlendA;
				uint8 BlendB;
				uint8 BlendC;
				{
					FDoubleIndexBlend MaterialA(VertexA.Material);
					FDoubleIndexBlend MaterialB(VertexB.Material);
					FDoubleIndexBlend MaterialC(VertexC.Material);

					FDoubleIndexBlend MaxBlend = FDoubleIndexBlend::GetBest(MaterialA, MaterialB, MaterialC);
					DoubleIndex = MaxBlend;

					BlendA = MaterialA.GetBlendFor(MaxBlend);
					BlendB = MaterialB.GetBlendFor(MaxBlend);
					BlendC = MaterialC.GetBlendFor(MaxBlend);
				}

				FVoxelBlendedMaterial Material;
				if (BlendA == 0 && BlendB == 0 && BlendC == 0)
				{
					Material = FVoxelBlendedMaterial(DoubleIndex.IndexA);
				}
				else if (BlendA == 255 && BlendB == 255 && BlendC == 255)
				{
					Material = FVoxelBlendedMaterial(DoubleIndex.IndexB);
				}
				else
				{
					uint8 Min = FMath::Min(DoubleIndex.IndexA, DoubleIndex.IndexB);
					uint8 Max = FMath::Max(DoubleIndex.IndexA, DoubleIndex.IndexB);
					Material = Min == Max ? FVoxelBlendedMaterial(Min) : FVoxelBlendedMaterial(Min, Max);
					if (Min != DoubleIndex.IndexA)
					{
						// They were swapped, change the blends accordingly
						BlendA = 255 - BlendA;
						BlendB = 255 - BlendB;
						BlendC = 255 - BlendC;
					}
				}

				FVoxelChunkBuffers& Buffer = OutChunk.Map.FindOrAdd(Material);
				TMap<int32, int32>& IndicesMap = IndicesMaps.FindOrAdd(Material);

				struct FHelper
				{
					inline static void AddVertex(
						const int32 Index,
						const uint8 Blend,
						const TVertex& Vertex,
						FVoxelChunkBuffers& Buffer,
						TMap<int32, int32>& IndicesMap)
					{
						int32 FinalIndex;
						if (int32* FinalIndexPtr = IndicesMap.Find(Index))
						{
							FinalIndex = *FinalIndexPtr;
						}
						else
						{
							auto ProcVertex = Vertex.GetProcMeshVertex();
							ProcVertex.Color.R = Blend;
							FinalIndex = IndicesMap.Add(Index, Buffer.AddVertex(ProcVertex));
						}
						Buffer.AddIndex(FinalIndex);
					}
				};

				FHelper::AddVertex(IndexA, BlendA, VertexA, Buffer, IndicesMap);
				FHelper::AddVertex(IndexB, BlendB, VertexB, Buffer, IndicesMap);
				FHelper::AddVertex(IndexC, BlendC, VertexC, Buffer, IndicesMap);
			}
		}
		else
		{
			check(MaterialConfig == EVoxelMaterialConfig::SingleIndex);

			OutChunk.bSingleBuffers = false;

			TMap<FVoxelBlendedMaterial, TMap<int32, int32>> SingleIndicesMaps;

			for (int32 I = 0; I < Indices.Num(); I += 3)
			{
				int32 IndexA = Indices[I];
				int32 IndexB = Indices[I + 1];
				int32 IndexC = Indices[I + 2];

				const TVertex& VertexA = Vertices[IndexA];
				const TVertex& VertexB = Vertices[IndexB];
				const TVertex& VertexC = Vertices[IndexC];

				uint8 MaterialIndexA = VertexA.Material.GetIndex();
				uint8 MaterialIndexB = VertexB.Material.GetIndex();
				uint8 MaterialIndexC = VertexC.Material.GetIndex();

				if (MaterialIndexA == MaterialIndexB && MaterialIndexA == MaterialIndexC)
				{
					FVoxelBlendedMaterial Material(MaterialIndexA);
					FVoxelChunkBuffers& Buffer = OutChunk.Map.FindOrAdd(Material);
					auto& SingleIndicesMap = SingleIndicesMaps.FindOrAdd(Material);

					int32* FinalIndexARef = SingleIndicesMap.Find(IndexA);
					int32 FinalIndexA = FinalIndexARef ? *FinalIndexARef : SingleIndicesMap.Add(IndexA, Buffer.AddVertex(VertexA.GetProcMeshVertex()));

					int32* FinalIndexBRef = SingleIndicesMap.Find(IndexB);
					int32 FinalIndexB = FinalIndexBRef ? *FinalIndexBRef : SingleIndicesMap.Add(IndexB, Buffer.AddVertex(VertexB.GetProcMeshVertex()));

					int32* FinalIndexCRef = SingleIndicesMap.Find(IndexC);
					int32 FinalIndexC = FinalIndexCRef ? *FinalIndexCRef : SingleIndicesMap.Add(IndexC, Buffer.AddVertex(VertexC.GetProcMeshVertex()));

					Buffer.AddIndex(FinalIndexA);
					Buffer.AddIndex(FinalIndexB);
					Buffer.AddIndex(FinalIndexC);
				}
				else
				{
					FVoxelBlendedMaterial Material;

					auto NewVertexA = VertexA.GetProcMeshVertex();
					auto NewVertexB = VertexB.GetProcMeshVertex();
					auto NewVertexC = VertexC.GetProcMeshVertex();

					FColor& ColorA = NewVertexA.Color;
					FColor& ColorB = NewVertexB.Color;
					FColor& ColorC = NewVertexC.Color;

					if (MaterialIndexA != MaterialIndexB && MaterialIndexA != MaterialIndexC && MaterialIndexB != MaterialIndexC)
					{
						int32 Min = FMath::Min(MaterialIndexA, FMath::Min(MaterialIndexB, MaterialIndexC));
						int32 Max = FMath::Max(MaterialIndexA, FMath::Max(MaterialIndexB, MaterialIndexC));
						int32 Med = MaterialIndexA + MaterialIndexB + MaterialIndexC - Min - Max;

						Material = FVoxelBlendedMaterial(Min, Med, Max);

						ColorA.R = MaterialIndexA == Material.Index0 ? 255 : 0;
						ColorB.R = MaterialIndexB == Material.Index0 ? 255 : 0;
						ColorC.R = MaterialIndexC == Material.Index0 ? 255 : 0;

						ColorA.G = MaterialIndexA == Material.Index1 ? 255 : 0;
						ColorB.G = MaterialIndexB == Material.Index1 ? 255 : 0;
						ColorC.G = MaterialIndexC == Material.Index1 ? 255 : 0;
					}
					else
					{
						int32 Min = FMath::Min(MaterialIndexA, FMath::Min(MaterialIndexB, MaterialIndexC));
						int32 Max = FMath::Max(MaterialIndexA, FMath::Max(MaterialIndexB, MaterialIndexC));

						Material = FVoxelBlendedMaterial(Min, Max);

						ColorA.R = MaterialIndexA == Material.Index1 ? 255 : 0;
						ColorB.R = MaterialIndexB == Material.Index1 ? 255 : 0;
						ColorC.R = MaterialIndexC == Material.Index1 ? 255 : 0;
					}

					FVoxelChunkBuffers& Buffer = OutChunk.Map.FindOrAdd(Material);

					Buffer.AddIndex(Buffer.GetNumVertices());
					Buffer.AddIndex(Buffer.GetNumVertices() + 1);
					Buffer.AddIndex(Buffer.GetNumVertices() + 2);

					Buffer.AddVertex(NewVertexA);
					Buffer.AddVertex(NewVertexB);
					Buffer.AddVertex(NewVertexC);
				}
			}
		}

		if (MeshParameters.bBuildAdjacency)
		{
			Stats.StartStat("Building Tessellation");
			OutChunk.IterateOnBuffers([](auto& Buffer) { Buffer.BuildAdjacency(); });
		}
		if (MeshParameters.bOptimizeIndices)
		{
			Stats.StartStat("Optimizing Indices");
			OutChunk.IterateOnBuffers([](auto& Buffer) { Buffer.OptimizeIndices(); });
		}
		
		Stats.StartStat("Shrinking buffers");
		OutChunk.IterateOnBuffers([](auto& Buffer) { Buffer.Shrink(); });
	}

	static float GetWorldCurrentTime(UWorld* World);
	static void StartMeshDithering(UVoxelProceduralMeshComponent* Mesh, const FVoxelRendererSettings& Settings);
	static void ResetDithering(UVoxelProceduralMeshComponent* Mesh);
	static void HideMesh(UVoxelProceduralMeshComponent* Mesh);
	static void CreateMeshSectionFromChunks(
		int32 LOD, 
		uint64 Priority,
		bool bShouldFade, 
		const FVoxelRendererSettings& RendererSettings, 
		const FVoxelRenderChunkSettings& ChunkSettings, 
		UVoxelProceduralMeshComponent* Mesh, 
		FVoxelChunkMaterials& ChunkMaterials, 
		const TSharedPtr<FVoxelChunk>& MainChunk, 
		uint8 TransitionsMask = 0, 
		const TSharedPtr<FVoxelChunk>& TransitionChunk = TSharedPtr<FVoxelChunk>());
	static bool DebugInvisibleChunks();
};
