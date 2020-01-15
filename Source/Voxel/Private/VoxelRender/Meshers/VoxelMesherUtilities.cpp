// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkMesh.h"

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
	explicit FDoubleIndex(const FVoxelMaterial& Material)
		: IndexA(Material.GetDoubleIndex_IndexA())
		, IndexB(Material.GetDoubleIndex_IndexB())
	{
	}

	FORCEINLINE bool Equals(uint8 InIndexA, uint8 InIndexB) const
	{
		return IndexA == InIndexA && IndexB == InIndexB;
	}
};
struct FDoubleIndexBlend : FDoubleIndex
{
	uint8 Blend;

	FDoubleIndexBlend() = default;
	FDoubleIndexBlend(uint8 IndexA, uint8 IndexB, uint8 Blend)
		: FDoubleIndex(IndexA, IndexB)
		, Blend(Blend)
	{
	}
	explicit FDoubleIndexBlend(const FVoxelMaterial& Material)
		: FDoubleIndex(Material)
		, Blend(Material.GetDoubleIndex_Blend())
	{
	}

	static FDoubleIndexBlend GetBest(const FDoubleIndexBlend& A, const FDoubleIndexBlend& B, const FDoubleIndexBlend& C)
	{
		uint8 Strengths[256];
		TArray<uint8, TFixedAllocator<6>> UsedIds;

		const auto AddIndex = [&](uint8 Index, uint8 Strength)
		{
			if (!UsedIds.Contains(Index))
			{
				UsedIds.Add(Index);
				Strengths[Index] = 0;
			}
			Strengths[Index] = FMath::Max(Strengths[Index], Strength);
		};
		const auto AddDoubleIndexBlend = [&](const FDoubleIndexBlend& DoubleIndex)
		{
			AddIndex(DoubleIndex.IndexA, 255 - DoubleIndex.Blend);
			AddIndex(DoubleIndex.IndexB, DoubleIndex.Blend);
		};

		AddDoubleIndexBlend(A);
		AddDoubleIndexBlend(B);
		AddDoubleIndexBlend(C);

		uint8 MaxIndexA = 0;
		int32 MaxIndexAStrength = -1;
		uint8 MaxIndexB = 0;
		int32 MaxIndexBStrength = -1;
		for (uint8 Id : UsedIds)
		{
			const uint8 Strength = Strengths[Id];
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

		const int32 Strength = ((255 - MaxIndexAStrength) + MaxIndexBStrength) / 2;
		checkVoxelSlow(0 <= Strength && Strength < 256);
		return FDoubleIndexBlend(MaxIndexA, MaxIndexB, Strength);
	}

	FORCEINLINE uint8 GetBlendFor(const FDoubleIndexBlend& Other) const
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

struct FVoxelFinalMesherVertex
{
	FVector Position;
	FVector Normal;
	FVoxelProcMeshTangent Tangent;
	FColor Color;
	FVector2D TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES];

	FVoxelFinalMesherVertex() = default;
	FORCEINLINE explicit FVoxelFinalMesherVertex(const FVoxelMesherVertex& Vertex)
		: Position(Vertex.Position)
		, Normal(Vertex.Normal)
		, Tangent(Vertex.Tangent)
		, Color(Vertex.Material.GetColor())
	{
		TextureCoordinates[0] = Vertex.TextureCoordinate;
		for (int32 Index = 0; Index < NUM_VOXEL_TEXTURE_COORDINATES - 1; Index++)
		{
			TextureCoordinates[Index + 1] = Vertex.Material.GetUV_AsFloat(Index);
		}
	}
};

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelMesherUtilities::CreateChunkFromVertices(
	const FVoxelRendererSettings& Settings, 
	TArray<uint32>&& Indices, 
	TArray<FVoxelMesherVertex>&& Vertices)
{
	VOXEL_FUNCTION_COUNTER();

	auto Chunk = MakeVoxelShared<FVoxelChunkMesh>();
	
	Settings.DynamicSettings->DynamicSettingsLock.Lock();
	const auto MaterialCollectionMap = Settings.DynamicSettings->MaterialCollectionMap; // Copy shared ptr to be safe if the voxel world changes it
	const bool bGenerateBlendings = Settings.DynamicSettings->bGenerateBlendings;
	Settings.DynamicSettings->DynamicSettingsLock.Unlock();
	
	const auto GetMaterialKeyDouble = [&](uint8 IndexA, uint8 IndexB)
	{
		FVoxelBlendedMaterialSorted Sorted;
		if (IndexA < IndexB)
		{
			Sorted = { IndexA, IndexB };
		}
		else
		{
			Sorted = { IndexB, IndexA };
		}
		auto* Unsorted = MaterialCollectionMap->Find(Sorted);
		if (!Unsorted)
		{
			return FVoxelBlendedMaterialUnsorted(Sorted.Index0, Sorted.Index1);
		}
		else
		{
			return *Unsorted;
		}
	};
	const auto GetMaterialKeyTriple = [&](uint8 IndexA, uint8 IndexB, uint8 IndexC)
	{
		const int32 Min = FMath::Min3(IndexA, IndexB, IndexC);
		const int32 Max = FMath::Max3(IndexA, IndexB, IndexC);
		const int32 Med = IndexA + IndexB + IndexC - Min - Max;
		const FVoxelBlendedMaterialSorted Sorted(Min, Med, Max);
		auto* Unsorted = MaterialCollectionMap->Find(Sorted);
		if (!Unsorted)
		{
			return FVoxelBlendedMaterialUnsorted(Sorted.Index0, Sorted.Index1, Sorted.Index2);
		}
		else
		{
			return *Unsorted;
		}
	};
	
	if (Settings.MaterialConfig == EVoxelMaterialConfig::RGB)
	{
		Chunk->SetIsSingle(true);
		FVoxelChunkMeshBuffers& Buffers = Chunk->CreateSingleBuffers();
		Buffers.SetIndices(MoveTemp(Indices));
		Buffers.Reserve(Vertices.Num(), Settings.bRenderWorld);
		for (auto& Vertex : Vertices)
		{
			Buffers.AddVertex(FVoxelFinalMesherVertex(Vertex), Settings.bRenderWorld);
		}
	}
	else if (Settings.MaterialConfig == EVoxelMaterialConfig::DoubleIndex)
	{
		Chunk->SetIsSingle(false);
		TMap<FVoxelBlendedMaterialUnsorted, TMap<int32, int32>> IndicesMaps;
		for (int32 I = 0; I < Indices.Num(); I += 3)
		{
			int32 IndexA = Indices[I];
			int32 IndexB = Indices[I + 1];
			int32 IndexC = Indices[I + 2];
			const FVoxelMesherVertex& VertexA = Vertices[IndexA];
			const FVoxelMesherVertex& VertexB = Vertices[IndexB];
			const FVoxelMesherVertex& VertexC = Vertices[IndexC];
			
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
				DoubleIndex = FDoubleIndex(MaxBlend);
				BlendA = MaterialA.GetBlendFor(MaxBlend);
				BlendB = MaterialB.GetBlendFor(MaxBlend);
				BlendC = MaterialC.GetBlendFor(MaxBlend);
			}
			
			FVoxelBlendedMaterialUnsorted Material;
			if (DoubleIndex.IndexA == DoubleIndex.IndexB)
			{
				Material = FVoxelBlendedMaterialUnsorted(DoubleIndex.IndexA);
			}
			else if (BlendA == 0 && BlendB == 0 && BlendC == 0)
			{
				Material = FVoxelBlendedMaterialUnsorted(DoubleIndex.IndexA);
			}
			else if (BlendA == 255 && BlendB == 255 && BlendC == 255)
			{
				Material = FVoxelBlendedMaterialUnsorted(DoubleIndex.IndexB);
			}
			else
			{
				Material = GetMaterialKeyDouble(DoubleIndex.IndexA, DoubleIndex.IndexB);
				if (Material.Index0 != DoubleIndex.IndexA)
				{
					// They were swapped, change the blends accordingly
					BlendA = 255 - BlendA;
					BlendB = 255 - BlendB;
					BlendC = 255 - BlendC;
				}
			}

			FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(Material);
			TMap<int32, int32>& IndicesMap = IndicesMaps.FindOrAdd(Material);
			
			const auto AddVertex = [&](const int32 Index, const uint8 Blend, const FVoxelMesherVertex& Vertex)
			{
				FVoxelFinalMesherVertex FinalVertex{ Vertex };
				// We want to have G being the IndexA, B the IndexB and A the custom data
				FinalVertex.Color.B = FinalVertex.Color.G; // Index B
				FinalVertex.Color.G = FinalVertex.Color.R; // Index A
				FinalVertex.Color.R = Blend;
				
				int32 FinalIndex;
				int32* FinalIndexPtr = IndicesMap.Find(Index);
				if (FinalIndexPtr && Buffer.Colors[*FinalIndexPtr] == FinalVertex.Color) // As it's per triangle, can have different result
				{
					FinalIndex = *FinalIndexPtr;
				}
				else
				{
					FinalIndex = Buffer.AddVertex(FinalVertex, Settings.bRenderWorld);
					IndicesMap.Add(Index, FinalIndex);
				}
				Buffer.AddIndex(FinalIndex);
			};
			
			AddVertex(IndexA, BlendA, VertexA);
			AddVertex(IndexB, BlendB, VertexB);
			AddVertex(IndexC, BlendC, VertexC);
		}
	}
	else
	{
		check(Settings.MaterialConfig == EVoxelMaterialConfig::SingleIndex);
		Chunk->SetIsSingle(false);
		
		TMap<FVoxelBlendedMaterialUnsorted, TMap<int32, int32>> IndicesMaps;
		for (int32 I = 0; I < Indices.Num(); I += 3)
		{
			const int32 IndexA = Indices[I + 0];
			const int32 IndexB = Indices[I + 1];
			const int32 IndexC = Indices[I + 2];
			const FVoxelMesherVertex& VertexA = Vertices[IndexA];
			const FVoxelMesherVertex& VertexB = Vertices[IndexB];
			const FVoxelMesherVertex& VertexC = Vertices[IndexC];
			const uint8 MaterialIndexA = VertexA.Material.GetSingleIndex_Index();
			const uint8 MaterialIndexB = VertexB.Material.GetSingleIndex_Index();
			const uint8 MaterialIndexC = VertexC.Material.GetSingleIndex_Index();

			// Send Material.R and Material.G through Vertex.B and Vertex.A, as Material.A is useless (index)
			const auto FixColor = [](FColor& Color)
			{
				Color.B = Color.R;
				Color.A = Color.G;
			};
			const auto AddVertex = [&Settings](FVoxelChunkMeshBuffers& Buffer, TMap<int32, int32>& IndicesMap, int32 Index, const FVoxelFinalMesherVertex& Vertex)
			{
				int32 FinalIndex;
				if (int32* FinalIndexPtr = IndicesMap.Find(Index))
				{
					FinalIndex = *FinalIndexPtr;
					ensureVoxelSlow(!Settings.bRenderWorld || Buffer.Colors[FinalIndex] == Vertex.Color);
				}
				else
				{
					FinalIndex = Buffer.AddVertex(Vertex, Settings.bRenderWorld);
					IndicesMap.Add(Index, FinalIndex);
				}
				Buffer.AddIndex(FinalIndex);
			};
			
			if ((MaterialIndexA == MaterialIndexB && MaterialIndexA == MaterialIndexC) || !bGenerateBlendings)
			{
				const FVoxelBlendedMaterialUnsorted Material(MaterialIndexA);
				auto& Buffer = Chunk->FindOrAddBuffer(Material);
				auto& IndicesMap = IndicesMaps.FindOrAdd(Material);
				const auto AddVertexSingle = [&](int32 Index, const FVoxelMesherVertex& Vertex)
				{
					FVoxelFinalMesherVertex FinalVertex{ Vertex };
					FixColor(FinalVertex.Color);
					AddVertex(Buffer, IndicesMap, Index, FinalVertex);
				};
				AddVertexSingle(IndexA, VertexA);
				AddVertexSingle(IndexB, VertexB);
				AddVertexSingle(IndexC, VertexC);
			}
			else
			{
				FVoxelBlendedMaterialUnsorted Material;
				FVoxelFinalMesherVertex NewVertexA{ VertexA };
				FVoxelFinalMesherVertex NewVertexB{ VertexB };
				FVoxelFinalMesherVertex NewVertexC{ VertexC };
				FColor& ColorA = NewVertexA.Color;
				FColor& ColorB = NewVertexB.Color;
				FColor& ColorC = NewVertexC.Color;
				FixColor(ColorA);
				FixColor(ColorB);
				FixColor(ColorC);
				if (MaterialIndexA != MaterialIndexB && MaterialIndexA != MaterialIndexC && MaterialIndexB != MaterialIndexC)
				{
					Material = GetMaterialKeyTriple(MaterialIndexA, MaterialIndexB, MaterialIndexC);
					ColorA.R = MaterialIndexA == Material.Index0 ? 255 : 0;
					ColorB.R = MaterialIndexB == Material.Index0 ? 255 : 0;
					ColorC.R = MaterialIndexC == Material.Index0 ? 255 : 0;
					ColorA.G = MaterialIndexA == Material.Index1 ? 255 : 0;
					ColorB.G = MaterialIndexB == Material.Index1 ? 255 : 0;
					ColorC.G = MaterialIndexC == Material.Index1 ? 255 : 0;
				}
				else
				{
					Material = GetMaterialKeyDouble(
						FMath::Min3(MaterialIndexA, MaterialIndexB, MaterialIndexC),
						FMath::Max3(MaterialIndexA, MaterialIndexB, MaterialIndexC));
					ColorA.R = MaterialIndexA == Material.Index1 ? 255 : 0;
					ColorB.R = MaterialIndexB == Material.Index1 ? 255 : 0;
					ColorC.R = MaterialIndexC == Material.Index1 ? 255 : 0;
				}
				FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(Material);
				auto& IndicesMap = IndicesMaps.FindOrAdd(Material);
				AddVertex(Buffer, IndicesMap, IndexA, NewVertexA);
				AddVertex(Buffer, IndicesMap, IndexB, NewVertexB);
				AddVertex(Buffer, IndicesMap, IndexC, NewVertexC);
			}
		}
	}

	return Chunk;
}