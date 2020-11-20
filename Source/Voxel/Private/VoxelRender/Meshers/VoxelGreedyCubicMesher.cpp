// Copyright 2020 Phyronnaz

#include "VoxelRender/Meshers/VoxelGreedyCubicMesher.h"
#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelData/VoxelDataIncludes.h"

FVoxelIntBox FVoxelGreedyCubicMesher::GetBoundsToCheckIsEmptyOn() const
{
	return FVoxelIntBox(ChunkPosition - FIntVector(Step), ChunkPosition - FIntVector(Step) + CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * Step);
}

FVoxelIntBox FVoxelGreedyCubicMesher::GetBoundsToLock() const
{
	return GetBoundsToCheckIsEmptyOn();
}

TVoxelSharedPtr<FVoxelChunkMesh> FVoxelGreedyCubicMesher::CreateFullChunkImpl(FVoxelMesherTimes& Times)
{
	TArray<FVoxelMesherVertex> Vertices;
	TArray<uint32> Indices;

	TArray<FColor> TextureData;
	CreateGeometryTemplate(Times, Indices, Vertices, &TextureData);

	UnlockData();
	
	return MESHER_TIME_INLINE(CreateChunk, FVoxelMesherUtilities::CreateChunkFromVertices(
		Settings,
		LOD,
		MoveTemp(Indices),
		MoveTemp(Vertices),
		&TextureData));
}


void FVoxelGreedyCubicMesher::CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices)
{
	struct FVertex : FVector
	{
		FVertex() = default;
		FVertex(const FVoxelMesherVertex& Vertex)
			: FVector(Vertex.Position)
		{
		}
	};
	
	CreateGeometryTemplate(Times, Indices, reinterpret_cast<TArray<FVertex>&>(Vertices), nullptr);

	UnlockData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void FVoxelGreedyCubicMesher::CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices, TArray<FColor>* TextureData)
{
	if (TextureData)
	{
		Accelerator = MakeUnique<FVoxelConstDataAccelerator>(Data, GetBoundsToLock());
	}
	
	constexpr int32 NumVoxels = RENDER_CHUNK_SIZE * RENDER_CHUNK_SIZE * RENDER_CHUNK_SIZE;
	constexpr int32 NumVoxelsWithNeighbors = CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS * CUBIC_CHUNK_SIZE_WITH_NEIGHBORS;

	TVoxelStaticBitArray<NumVoxelsWithNeighbors> ValuesBitArray;
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Query Data");
		
		TVoxelStaticArray<FVoxelValue, NumVoxelsWithNeighbors> Values;

		TVoxelQueryZone<FVoxelValue> QueryZone(GetBoundsToCheckIsEmptyOn(), FIntVector(CUBIC_CHUNK_SIZE_WITH_NEIGHBORS), LOD, Values);
		MESHER_TIME_INLINE_VALUES(NumVoxelsWithNeighbors, Data.Get<FVoxelValue>(QueryZone, LOD));

		for (int32 Index = 0; Index < NumVoxelsWithNeighbors; Index++)
		{
			ValuesBitArray.Set(Index, !Values[Index].IsEmpty());
		}
	}

	TVoxelStaticArray<TVoxelStaticBitArray<NumVoxels>, 6> FacesBitArrays;
	FacesBitArrays.Memzero();
	
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Find faces");

		const auto GetValue = [&](int32 X, int32 Y, int32 Z)
		{
			return ValuesBitArray.Test((X + 1) + (Y + 1) * (RENDER_CHUNK_SIZE + 2) + (Z + 1) * (RENDER_CHUNK_SIZE + 2) * (RENDER_CHUNK_SIZE + 2));
		};
		const auto GetFaceIndex = [&](int32 X, int32 Y, int32 Z)
		{
			return X + Y * RENDER_CHUNK_SIZE + Z * RENDER_CHUNK_SIZE * RENDER_CHUNK_SIZE;
		};
		
		for (int32 Z = 0; Z < RENDER_CHUNK_SIZE; Z++)
		{
			for (int32 Y = 0; Y < RENDER_CHUNK_SIZE; Y++)
			{
				for (int32 X = 0; X < RENDER_CHUNK_SIZE; X++)
				{
					if (!GetValue(X, Y, Z))
					{
						continue;
					}

					if (!GetValue(X - 1, Y, Z)) FacesBitArrays[0].Set(GetFaceIndex(Y, Z, X), true);
					if (!GetValue(X + 1, Y, Z)) FacesBitArrays[1].Set(GetFaceIndex(Y, Z, X), true);
					if (!GetValue(X, Y - 1, Z)) FacesBitArrays[2].Set(GetFaceIndex(Z, X, Y), true);
					if (!GetValue(X, Y + 1, Z)) FacesBitArrays[3].Set(GetFaceIndex(Z, X, Y), true);
					if (!GetValue(X, Y, Z - 1)) FacesBitArrays[4].Set(GetFaceIndex(X, Y, Z), true);
					if (!GetValue(X, Y, Z + 1)) FacesBitArrays[5].Set(GetFaceIndex(X, Y, Z), true);
				}
			}
		}
	}

	for (int32 Direction = 0; Direction < 6; Direction++)
	{
		TArray<FCubicQuad, TFixedAllocator<NumVoxels>> Quads;
		GreedyMeshing2D<RENDER_CHUNK_SIZE>(FacesBitArrays[Direction], Quads);
		
		VOXEL_ASYNC_SCOPE_COUNTER("Add faces");
		for (auto& Quad : Quads)
		{
			AddFace(Times, Direction, Quad, Step, Indices, Vertices, TextureData);
		}
	}
}

template<uint32 GridSize, typename Allocator>
FORCEINLINE void FVoxelGreedyCubicMesher::GreedyMeshing2D(TVoxelStaticBitArray<GridSize * GridSize * GridSize>& InFaces, TArray<FCubicQuad, Allocator>& OutQuads)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	for (uint32 Layer = 0; Layer < GridSize; Layer++)
	{
		static_assert(((GridSize * GridSize) % TVoxelStaticBitArray<GridSize * GridSize * GridSize>::NumBitsPerWord) == 0, "");
		auto& Faces = reinterpret_cast<TVoxelStaticBitArray<GridSize * GridSize>&>(*(InFaces.GetData() + Layer * GridSize * GridSize / InFaces.NumBitsPerWord));
		
		const auto TestAndClear = [&](uint32 X, uint32 Y)
		{
			checkVoxelSlow(X < GridSize);
			checkVoxelSlow(Y < GridSize);
			
			return Faces.TestAndClear(X + Y * GridSize);
		};
		const auto TestAndClearLine = [&](uint32 X, uint32 Width, uint32 Y)
		{
			checkVoxelSlow(X + Width <= GridSize);
			checkVoxelSlow(Y < GridSize);
			
			return Faces.TestAndClearRange(X + Y * GridSize, Width);
		};
		
		for (uint32 X = 0; X < GridSize; X++)
		{
			const uint32 Mask = 1 << X;
			for (uint32 Y = 0; Y < GridSize;)
			{
				if (GridSize == 32)
				{
					// Simpler logic, as Y is the word index
					if (!(Faces.GetInternal(Y) & Mask))
					{
						Y++;
						continue;
					}
				}
				else
				{
					if (!Faces.Test(X + Y * GridSize))
					{
						Y++;
						continue;
					}
				}

				uint32 Width = 1;
				while (X + Width < GridSize && TestAndClear(X + Width, Y))
				{
					Width++;
				}

				uint32 Height = 1;
				while (Y + Height < GridSize && TestAndClearLine(X, Width, Y + Height))
				{
					Height++;
				}

				OutQuads.Add(FCubicQuad{ Layer, X, Y, Width, Height });

				Y += Height;
			}
		}
	}
}

template<typename T>
FORCEINLINE void FVoxelGreedyCubicMesher::AddFace(
	FVoxelMesherTimes& Times,
	int32 Direction,
	const FCubicQuad& Quad, 
	int32 Step,
	TArray<uint32>& Indices,
	TArray<T>& Vertices,
	TArray<FColor>* TextureData)
{
	const int32 ZAxis = Direction / 2;
	const bool bInverted = Direction & 0x1;

	const int32 XAxis = (ZAxis + 1) % 3;
	const int32 YAxis = (ZAxis + 2) % 3;
	
	/**
	 * 1 --- 2
	 * |  /  |
	 * 0 --- 3
	 * 
	 * Triangles: 0 1 2, 0 2 3
	 */

	if (bInverted)
	{
		Indices.Add(Vertices.Num() + 2);
		Indices.Add(Vertices.Num() + 1);
		Indices.Add(Vertices.Num() + 0);

		Indices.Add(Vertices.Num() + 3);
		Indices.Add(Vertices.Num() + 2);
		Indices.Add(Vertices.Num() + 0);
	}
	else
	{
		Indices.Add(Vertices.Num() + 0);
		Indices.Add(Vertices.Num() + 1);
		Indices.Add(Vertices.Num() + 2);

		Indices.Add(Vertices.Num() + 0);
		Indices.Add(Vertices.Num() + 2);
		Indices.Add(Vertices.Num() + 3);
	}

	FVoxelMesherVertex Vertex;
	Vertex.Material = FVoxelMaterial::Default();

	if (TextureData)
	{
		const auto GetMaterial = [&](int32 X, int32 Y)
		{
			FIntVector Position;
			Position[XAxis] = Quad.StartX + X;
			Position[YAxis] = Quad.StartY + Y;
			Position[ZAxis] = Quad.Layer;

			Position += ChunkPosition;

			return MESHER_TIME_INLINE_MATERIALS(1, Accelerator->GetMaterial(Position, LOD));
		};
		
		if (Quad.SizeX == 1 && Quad.SizeY == 1)
		{
			Vertex.Material = GetMaterial(0, 0);
			Vertex.Material.CubicColor_SetUseTextureFalse();
		}
		else
		{
			// TODO don't allocate texture data if all colors are the same
			bool bMaterialSet = false;
			for (uint32 Y = 0; Y < Quad.SizeY; Y++)
			{
				for (uint32 X = 0; X < Quad.SizeX; X++)
				{
					const FVoxelMaterial Material = GetMaterial(X, Y);

					if (!bMaterialSet)
					{
						bMaterialSet = true;
						Vertex.Material = Material;
						Vertex.Material.CubicColor_SetQuadWidth(Quad.SizeX);
						Vertex.Material.CubicColor_SetTextureDataIndex(TextureData->Num());
					}

					TextureData->Add(Material.GetColor());
				}
			}
		}
	}

	Vertex.Normal = FVector{ ForceInit };
	Vertex.Normal[ZAxis] = bInverted ? 1 : -1;
	
	Vertex.Tangent.TangentX = FVector{ ForceInit };
	Vertex.Tangent.TangentX[XAxis] = 1;

	const auto SetPosition = [&](int32 X, int32 Y)
	{
		Vertex.TextureCoordinate.X = Quad.SizeX * X;
		Vertex.TextureCoordinate.Y = Quad.SizeY * Y;
		Vertex.Position[XAxis] = Quad.StartX + Quad.SizeX * X;
		Vertex.Position[YAxis] = Quad.StartY + Quad.SizeY * Y;
		Vertex.Position[ZAxis] = Quad.Layer + bInverted;
		Vertex.Position = Step * Vertex.Position - 0.5f;
	};
	
	SetPosition(0, 0);
	Vertices.Add(T(Vertex));

	SetPosition(1, 0);
	Vertices.Add(T(Vertex));
	
	SetPosition(1, 1);
	Vertices.Add(T(Vertex));

	SetPosition(0, 1);
	Vertices.Add(T(Vertex));
}