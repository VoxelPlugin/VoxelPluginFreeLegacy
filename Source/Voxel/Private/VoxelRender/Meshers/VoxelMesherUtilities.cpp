// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkMesh.h"

FORCEINLINE int32 AddVertexToBuffer(
	const FVoxelMesherVertex& Vertex, 
	FVoxelChunkMeshBuffers& Buffer, 
	bool bRenderWorld, 
	const FVector2D* AdditionalUV0 = nullptr,
	const FVector2D* AdditionalUV1 = nullptr,
	const FVector2D* AdditionalUV2 = nullptr)
{
	const int32 Index = Buffer.Positions.Emplace(Vertex.Position);
	if (bRenderWorld)
	{
		ensure(Index == Buffer.Normals.Emplace(Vertex.Normal));
		ensure(Index == Buffer.Tangents.Emplace(Vertex.Tangent));
		ensure(Index == Buffer.Colors.Emplace(Vertex.Material.GetColor()));
		ensure(Index == Buffer.TextureCoordinates[0].Emplace(Vertex.TextureCoordinate));
		for (int32 Tex = 0; Tex < NUM_VOXEL_TEXTURE_COORDINATES - 1; Tex++)
		{
			ensure(Index == Buffer.TextureCoordinates[Tex + 1].Emplace(Vertex.Material.GetUV_AsFloat(Tex)));
		}

		if (AdditionalUV0) ensure(Index == Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 0].Emplace(*AdditionalUV0));
		if (AdditionalUV1) ensure(Index == Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 1].Emplace(*AdditionalUV1));
		if (AdditionalUV2) ensure(Index == Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 2].Emplace(*AdditionalUV2));
	}
	return Index;
}

inline void ReserveBuffer(
	FVoxelChunkMeshBuffers& Buffer,
	int32 Num,
	bool bRenderWorld,
	bool bAdditionalUV0 = false,
	bool bAdditionalUV1 = false,
	bool bAdditionalUV2 = false)
{
	VOXEL_FUNCTION_COUNTER();
	
	Buffer.Positions.Reserve(Num);
	if (bRenderWorld)
	{
		Buffer.Normals.Reserve(Num);
		Buffer.Tangents.Reserve(Num);
		Buffer.Colors.Reserve(Num);
		
		Buffer.TextureCoordinates.SetNum(NUM_VOXEL_TEXTURE_COORDINATES + bAdditionalUV0 + bAdditionalUV1 + bAdditionalUV2);
		for (uint32 Tex = 0; Tex < NUM_VOXEL_TEXTURE_COORDINATES; Tex++)
		{
			Buffer.TextureCoordinates[Tex].Reserve(Num);
		}
		if (bAdditionalUV0) Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 0].Reserve(Num);
		if (bAdditionalUV1) Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 1].Reserve(Num);
		if (bAdditionalUV2) Buffer.TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES + 2].Reserve(Num);
	}
}

inline FVoxelMaterialIndices GetMaterialIndices_DoubleIndex(
	const FVoxelMaterial& MaterialA,
	const FVoxelMaterial& MaterialB,
	const FVoxelMaterial& MaterialC,
	int32 MaxIndices)
{
	TStackArray<uint8, 256> MaxStrengths;
	TArray<uint8, TFixedAllocator<6>> UsedIds;

	const auto AddMaterial = [&](const FVoxelMaterial& Material)
	{
		const auto AddIndex = [&](uint8 Index, uint8 Strength)
		{
			// Do not consider indices that are totally hidden
			if (Strength == 0) return;

			if (!UsedIds.Contains(Index))
			{
				UsedIds.Add(Index);
				MaxStrengths[Index] = 0;
			}
			
			MaxStrengths[Index] = FMath::Max(MaxStrengths[Index], Strength);
		};
		
		AddIndex(Material.GetDoubleIndex_IndexA(), 255 - Material.GetDoubleIndex_Blend());
		AddIndex(Material.GetDoubleIndex_IndexB(), Material.GetDoubleIndex_Blend());
	};

	AddMaterial(MaterialA);
	AddMaterial(MaterialB);
	AddMaterial(MaterialC);
	
	check(UsedIds.Num() > 0);

	if (UsedIds.Num() > MaxIndices)
	{
		// Sort reverse by strength
		UsedIds.Sort([&](uint8 A, uint8 B) { return MaxStrengths[A] > MaxStrengths[B]; });
		// Pop lower ones
		UsedIds.SetNum(MaxIndices);
	}
	UsedIds.Sort();

	FVoxelMaterialIndices MaterialIndices;
	MaterialIndices.NumIndices = UsedIds.Num();
	for (int32 Index = 0; Index < UsedIds.Num(); Index++)
	{
		MaterialIndices.SortedIndices[Index] = UsedIds[Index];
	}
	
	check(MaterialIndices.NumIndices > 0);

	return MaterialIndices;
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelMesherUtilities::CreateChunkFromVertices(
	const FVoxelRendererSettings& Settings, 
	TArray<uint32>&& Indices, 
	TArray<FVoxelMesherVertex>&& Vertices)
{
	VOXEL_FUNCTION_COUNTER();

	auto Chunk = MakeVoxelShared<FVoxelChunkMesh>();
	
	const int32 MaxMaterialIndices = Settings.DynamicSettings->MaxMaterialIndices.GetValue();
	check(MaxMaterialIndices > 0);
	
	if (Settings.MaterialConfig == EVoxelMaterialConfig::RGB)
	{
		Chunk->SetIsSingle(true);
		FVoxelChunkMeshBuffers& Buffers = Chunk->CreateSingleBuffers();
		
		Buffers.Indices = MoveTemp(Indices);

		ReserveBuffer(Buffers, Vertices.Num(), Settings.bRenderWorld);
		for (auto& Vertex : Vertices)
		{
			AddVertexToBuffer(Vertex, Buffers, Settings.bRenderWorld);
		}
	}
	else if (Settings.MaterialConfig == EVoxelMaterialConfig::SingleIndex)
	{
		Chunk->SetIsSingle(false);

		TStackArray<TMap<int32, int32>, 256> IndicesMaps{ ForceInit };
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

			const uint8 MaterialIndexToUse = FMath::Min3(MaterialIndexA, MaterialIndexB, MaterialIndexC);
			
			FVoxelMaterialIndices MaterialIndices;
			MaterialIndices.NumIndices = 1;
			MaterialIndices.SortedIndices[0] = MaterialIndexToUse;

			bool bAdded;
			FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(MaterialIndices, bAdded);
			if (bAdded)
			{
				ReserveBuffer(Buffer, Vertices.Num(), Settings.bRenderWorld);
			}
			
			TMap<int32, int32>& IndicesMap = IndicesMaps[MaterialIndexToUse];

			const auto AddVertex = [&](int32 Index, const FVoxelMesherVertex& Vertex)
			{
				int32 FinalIndex;
				if (int32* FinalIndexPtr = IndicesMap.Find(Index))
				{
					FinalIndex = *FinalIndexPtr;
				}
				else
				{
					FinalIndex = AddVertexToBuffer(Vertex, Buffer, Settings.bRenderWorld);
					IndicesMap.Add(Index, FinalIndex);
				}
				Buffer.Indices.Emplace(FinalIndex);
			};
			
			AddVertex(IndexA, VertexA);
			AddVertex(IndexB, VertexB);
			AddVertex(IndexC, VertexC);
		}
	}
	else
	{
		check(Settings.MaterialConfig == EVoxelMaterialConfig::DoubleIndex);
		Chunk->SetIsSingle(false);
		
		TMap<FVoxelMaterialIndices, TMap<int32, int32>> IndicesMaps;
		for (int32 I = 0; I < Indices.Num(); I += 3)
		{
			const int32 IndexA = Indices[I + 0];
			const int32 IndexB = Indices[I + 1];
			const int32 IndexC = Indices[I + 2];
			
			const FVoxelMesherVertex& VertexA = Vertices[IndexA];
			const FVoxelMesherVertex& VertexB = Vertices[IndexB];
			const FVoxelMesherVertex& VertexC = Vertices[IndexC];
			
			const FVoxelMaterial& MaterialA = VertexA.Material;
			const FVoxelMaterial& MaterialB = VertexB.Material;
			const FVoxelMaterial& MaterialC = VertexC.Material;
			
			const FVoxelMaterialIndices MaterialIndices = GetMaterialIndices_DoubleIndex(
				MaterialA,
				MaterialB,
				MaterialC,
				MaxMaterialIndices);

			// ForceInit to send valid values through UVs
			TStackArray<uint8, 6> StrengthsA{ ForceInit };
			TStackArray<uint8, 6> StrengthsB{ ForceInit };
			TStackArray<uint8, 6> StrengthsC{ ForceInit };

			for (int32 Index = 0; Index < MaterialIndices.NumIndices; Index++)
			{
				const uint8 MaterialIndex = MaterialIndices.SortedIndices[Index];

				const auto ComputeStrength = [&](auto& Strengths, auto& Material)
				{
					Strengths[Index] =
						Material.GetDoubleIndex_IndexA() == MaterialIndex
						? 255 - Material.GetDoubleIndex_Blend()
						: Material.GetDoubleIndex_IndexB() == MaterialIndex
						? Material.GetDoubleIndex_Blend()
						: 0;
				};
				ComputeStrength(StrengthsA, MaterialA);
				ComputeStrength(StrengthsB, MaterialB);
				ComputeStrength(StrengthsC, MaterialC);
			}

			const bool bNeedAdditionalUV0 = MaterialIndices.NumIndices > 1;
			const bool bNeedAdditionalUV1 = MaterialIndices.NumIndices > 2;
			const bool bNeedAdditionalUV2 = MaterialIndices.NumIndices > 4;
			
			bool bAdded;
			FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(MaterialIndices, bAdded);
			if (bAdded)
			{
				ReserveBuffer(Buffer, Vertices.Num(), Settings.bRenderWorld,
					bNeedAdditionalUV0,
					bNeedAdditionalUV1,
					bNeedAdditionalUV2);
			}
			
			TMap<int32, int32>& IndicesMap = IndicesMaps.FindOrAdd(MaterialIndices);
			
			const auto AddVertex = [&](const int32 VertexIndex, const auto& Strengths, const FVoxelMesherVertex& Vertex)
			{
				int32 FinalIndex;
				int32* FinalIndexPtr = IndicesMap.Find(VertexIndex);
				if (FinalIndexPtr)
				{
					FinalIndex = *FinalIndexPtr;
				}
				else
				{
					const FVector2D UV0 = { FVoxelUtilities::UINT8ToFloat(Strengths[0]),FVoxelUtilities::UINT8ToFloat(Strengths[1]) };
					const FVector2D UV1 = { FVoxelUtilities::UINT8ToFloat(Strengths[2]),FVoxelUtilities::UINT8ToFloat(Strengths[3]) };
					const FVector2D UV2 = { FVoxelUtilities::UINT8ToFloat(Strengths[4]),FVoxelUtilities::UINT8ToFloat(Strengths[5]) };

					FinalIndex = AddVertexToBuffer(
						Vertex,
						Buffer,
						Settings.bRenderWorld,
						bNeedAdditionalUV0 ? &UV0 : nullptr,
						bNeedAdditionalUV1 ? &UV1 : nullptr,
						bNeedAdditionalUV2 ? &UV2 : nullptr);
					IndicesMap.Add(VertexIndex, FinalIndex);
				}
				Buffer.Indices.Emplace(FinalIndex);
			};
			
			AddVertex(IndexA, StrengthsA, VertexA);
			AddVertex(IndexB, StrengthsB, VertexB);
			AddVertex(IndexC, StrengthsC, VertexC);
		}
	}

	return Chunk;
}