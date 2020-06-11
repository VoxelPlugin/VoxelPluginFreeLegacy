// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkMesh.h"

FORCEINLINE int32 AddVertexToBuffer(
	const FVoxelMesherVertex& Vertex,
	FVoxelChunkMeshBuffers& Buffer, 
	bool bRenderWorld,
	EVoxelMaterialConfig MaterialConfig,
	const FColor* Color = nullptr,
	const FVector2D* UV = nullptr)
{
	const int32 Index = Buffer.Positions.Emplace(Vertex.Position);
	if (bRenderWorld)
	{
		Buffer.Normals.Emplace(Vertex.Normal);
		Buffer.Tangents.Emplace(Vertex.Tangent);
		Buffer.TextureCoordinates[0].Emplace(Vertex.TextureCoordinate);

		if (MaterialConfig == EVoxelMaterialConfig::MultiIndex)
		{
			check(Color && UV);
			Buffer.Colors.Emplace(*Color);
			Buffer.TextureCoordinates[1].Emplace(*UV);
			if (VOXEL_MATERIAL_ENABLE_UV2) Buffer.TextureCoordinates[2].Emplace(Vertex.Material.GetUV_AsFloat(2));
			if (VOXEL_MATERIAL_ENABLE_UV3) Buffer.TextureCoordinates[3].Emplace(Vertex.Material.GetUV_AsFloat(3));
		}
		else
		{
			Buffer.Colors.Emplace(Vertex.Material.GetColor());
			if (VOXEL_MATERIAL_ENABLE_UV0) Buffer.TextureCoordinates[1].Emplace(Vertex.Material.GetUV_AsFloat(0));
			if (VOXEL_MATERIAL_ENABLE_UV1) Buffer.TextureCoordinates[2].Emplace(Vertex.Material.GetUV_AsFloat(1));
			if (VOXEL_MATERIAL_ENABLE_UV2) Buffer.TextureCoordinates[3].Emplace(Vertex.Material.GetUV_AsFloat(2));
			if (VOXEL_MATERIAL_ENABLE_UV3) Buffer.TextureCoordinates[4].Emplace(Vertex.Material.GetUV_AsFloat(3));
		}
	}
	return Index;
}

inline void ReserveBuffer(
	FVoxelChunkMeshBuffers& Buffer,
	int32 Num,
	bool bRenderWorld,
	EVoxelMaterialConfig MaterialConfig)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	Buffer.Positions.Reserve(Num);
	if (bRenderWorld)
	{
		Buffer.Normals.Reserve(Num);
		Buffer.Tangents.Reserve(Num);
		Buffer.Colors.Reserve(Num);

		if (MaterialConfig == EVoxelMaterialConfig::MultiIndex)
		{
			Buffer.TextureCoordinates.SetNum(2);
			Buffer.TextureCoordinates[0].Reserve(Num);
			// Note: we always create the additional UV channel, else it creates issues when merging chunks
			Buffer.TextureCoordinates[1].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV2) Buffer.TextureCoordinates[2].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV3) Buffer.TextureCoordinates[3].Reserve(Num);
		}
		else
		{
			Buffer.TextureCoordinates.SetNum(1 + VOXEL_MATERIAL_ENABLE_UV0 + VOXEL_MATERIAL_ENABLE_UV1 + VOXEL_MATERIAL_ENABLE_UV2 + VOXEL_MATERIAL_ENABLE_UV3);
			Buffer.TextureCoordinates[0].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV0) Buffer.TextureCoordinates[1].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV1) Buffer.TextureCoordinates[2].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV2) Buffer.TextureCoordinates[3].Reserve(Num);
			if (VOXEL_MATERIAL_ENABLE_UV3) Buffer.TextureCoordinates[4].Reserve(Num);
		}
	}
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelMesherUtilities::CreateChunkFromVertices(
	const FVoxelRendererSettings& Settings, 
	int32 LOD,
	TArray<uint32>&& Indices, 
	TArray<FVoxelMesherVertex>&& Vertices)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	auto Chunk = MakeVoxelShared<FVoxelChunkMesh>();
	
	if (Settings.MaterialConfig == EVoxelMaterialConfig::RGB)
	{
		Chunk->SetIsSingle(true);
		FVoxelChunkMeshBuffers& Buffers = Chunk->CreateSingleBuffers();
		
		Buffers.Indices = MoveTemp(Indices);

		ReserveBuffer(Buffers, Vertices.Num(), Settings.bRenderWorld, EVoxelMaterialConfig::RGB);
		for (auto& Vertex : Vertices)
		{
			AddVertexToBuffer(Vertex, Buffers, Settings.bRenderWorld, EVoxelMaterialConfig::RGB);
		}
	}
	else if (Settings.MaterialConfig == EVoxelMaterialConfig::SingleIndex)
	{
		Chunk->SetIsSingle(false);

		TVoxelStaticArray<TMap<int32, int32>, 256> IndicesMaps{ ForceInit };
		for (int32 I = 0; I < Indices.Num(); I += 3)
		{
			const int32 IndexA = Indices[I + 0];
			const int32 IndexB = Indices[I + 1];
			const int32 IndexC = Indices[I + 2];
			
			const FVoxelMesherVertex& VertexA = Vertices[IndexA];
			const FVoxelMesherVertex& VertexB = Vertices[IndexB];
			const FVoxelMesherVertex& VertexC = Vertices[IndexC];
			
			const uint8 MaterialIndexA = VertexA.Material.GetSingleIndex();
			const uint8 MaterialIndexB = VertexB.Material.GetSingleIndex();
			const uint8 MaterialIndexC = VertexC.Material.GetSingleIndex();

			const uint8 MaterialIndexToUse = FMath::Min3(MaterialIndexA, MaterialIndexB, MaterialIndexC);
			
			FVoxelMaterialIndices MaterialIndices;
			MaterialIndices.NumIndices = 1;
			MaterialIndices.SortedIndices[0] = MaterialIndexToUse;

			bool bAdded;
			FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(MaterialIndices, bAdded);
			if (bAdded)
			{
				ReserveBuffer(Buffer, Vertices.Num(), Settings.bRenderWorld, EVoxelMaterialConfig::SingleIndex);
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
					FinalIndex = AddVertexToBuffer(Vertex, Buffer, Settings.bRenderWorld, EVoxelMaterialConfig::SingleIndex);
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
		check(Settings.MaterialConfig == EVoxelMaterialConfig::MultiIndex);
		Chunk->SetIsSingle(false);

		const int32 MaxMaterialIndices = FMath::Clamp(Settings.DynamicSettings->LODData[LOD].MaxMaterialIndices.GetValue(), 1, 6);

		constexpr int32 StaticMaxMaterialIndices = 6;
		
		if (Indices.Num() > 0)
		{
			TArray<uint8, TFixedAllocator<256>> MaterialIndices;
			{
				TVoxelStaticArray<float, 256> Strengths{ ForceInit };
				for (const auto& Vertex : Vertices)
				{
					const FVoxelMaterial& Material = Vertex.Material;
					
					const auto ClampStrength = [](float Strength)
					{
						// TODO
						// Consider that under threshold, material won't be displayed anyways
						return Strength < 0.f ? 0.f : Strength;
					};
					
					const float Blend0 = Material.GetMultiIndex_Blend0_AsFloat();
					const float Blend1 = Material.GetMultiIndex_Blend1_AsFloat();
					const float Blend2 = Material.GetMultiIndex_Blend2_AsFloat();

					const TVoxelStaticArray<float, 4> MaterialStrengths = FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>({ Blend0, Blend1, Blend2 });
					
					Strengths[Vertex.Material.GetMultiIndex_Index0()] += ClampStrength(MaterialStrengths[0]);
					Strengths[Vertex.Material.GetMultiIndex_Index1()] += ClampStrength(MaterialStrengths[1]);
					Strengths[Vertex.Material.GetMultiIndex_Index2()] += ClampStrength(MaterialStrengths[2]);
					Strengths[Vertex.Material.GetMultiIndex_Index3()] += ClampStrength(MaterialStrengths[3]);
				}

				struct FIndexStrength
				{
					uint8 Index = 0;
					float Strength = 0;
				};

				TArray<FIndexStrength, TFixedAllocator<256>> SortedStrengths;
				for (int32 Index = 0; Index < 256; Index++)
				{
					if (Strengths[Index] > 0.f)
					{
						SortedStrengths.Add({ uint8(Index), Strengths[Index] });
					}
				}
				SortedStrengths.Sort([](FIndexStrength A, FIndexStrength B) { return A.Strength > B.Strength; });

				for (auto& IndexStrength : SortedStrengths)
				{
					if (IndexStrength.Strength == 0.f)
					{
						break;
					}
					MaterialIndices.Add(IndexStrength.Index);
					if (MaterialIndices.Num() == MaxMaterialIndices)
					{
						break;
					}
				}

				if (MaterialIndices.Num() == 0)
				{
					MaterialIndices.Add(0);
				}
				MaterialIndices.Sort();

				check(1 <= MaterialIndices.Num() && MaterialIndices.Num() <= MaxMaterialIndices);
			}

			const auto MakeBuffer = [&]() -> auto&
			{
				FVoxelMaterialIndices VoxelMaterialIndices;
				VoxelMaterialIndices.NumIndices = MaterialIndices.Num();
				VoxelMaterialIndices.SortedIndices.CopyFromArray(MaterialIndices);

				bool bAdded;
				FVoxelChunkMeshBuffers& Buffer = Chunk->FindOrAddBuffer(VoxelMaterialIndices, bAdded);
				check(bAdded);
				ReserveBuffer(Buffer, Vertices.Num(), Settings.bRenderWorld, EVoxelMaterialConfig::MultiIndex);

				return Buffer;
			};
			FVoxelChunkMeshBuffers& Buffer = MakeBuffer();

			Buffer.Indices = MoveTemp(Indices);

			for (const FVoxelMesherVertex& Vertex : Vertices)
			{
				FColor Color{ ForceInit };
				FVector2D UV{ ForceInit };

				if (MaterialIndices.Num() > 1)
				{
					TArray<float, TFixedAllocator<StaticMaxMaterialIndices>> Strengths;
					{
						const FVoxelMaterial& Material = Vertex.Material;
						const float Blend0 = Material.GetMultiIndex_Blend0_AsFloat();
						const float Blend1 = Material.GetMultiIndex_Blend1_AsFloat();
						const float Blend2 = Material.GetMultiIndex_Blend2_AsFloat();

						const TVoxelStaticArray<float, 4> MaterialStrengths = FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>({ Blend0, Blend1, Blend2 });

						for (uint8 MaterialIndex : MaterialIndices)
						{
							// Consider all channels, as eg we could have 0 0 0 0 as indices but 0 1 1 as blends
							Strengths.Add(
								(MaterialIndex == Material.GetMultiIndex_Index0() ? MaterialStrengths[0] : 0.f) +
								(MaterialIndex == Material.GetMultiIndex_Index1() ? MaterialStrengths[1] : 0.f) +
								(MaterialIndex == Material.GetMultiIndex_Index2() ? MaterialStrengths[2] : 0.f) +
								(MaterialIndex == Material.GetMultiIndex_Index3() ? MaterialStrengths[3] : 0.f));
						}
					}

					const auto Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Dynamic<TFixedAllocator<StaticMaxMaterialIndices - 1>>(Strengths);

					if (Alphas.Num() > 0) Color.R = FVoxelUtilities::FloatToUINT8(Alphas[0]);
					if (Alphas.Num() > 1) Color.G = FVoxelUtilities::FloatToUINT8(Alphas[1]);
					if (Alphas.Num() > 2) Color.B = FVoxelUtilities::FloatToUINT8(Alphas[2]);
					if (Alphas.Num() > 3) UV.X = Alphas[3];
					if (Alphas.Num() > 4) UV.Y = Alphas[4];
				}

				Color.A = Vertex.Material.GetMultiIndex_Wetness();
				
				AddVertexToBuffer(Vertex, Buffer, Settings.bRenderWorld, EVoxelMaterialConfig::MultiIndex, &Color, &UV);
			}
		}
	}

	return Chunk;
}