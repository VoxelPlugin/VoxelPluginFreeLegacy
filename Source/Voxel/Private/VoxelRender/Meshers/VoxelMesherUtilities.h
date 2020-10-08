// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelMaterial.h"
#include "VoxelDirection.h"
#include "VoxelRender/VoxelProcMeshTangent.h"

struct FVoxelRendererSettings;
struct FVoxelChunkMesh;

struct FVoxelMesherVertex
{
	FVector Position;
	FVector Normal;
	FVoxelProcMeshTangent Tangent;
	FVector2D TextureCoordinate;
	FVoxelMaterial Material;
};

namespace FVoxelMesherUtilities
{
	TVoxelSharedPtr<FVoxelChunkMesh> CreateChunkFromVertices(
		const FVoxelRendererSettings& Settings,
		int32 LOD,
		TArray<uint32>&& Indices,
		TArray<FVoxelMesherVertex>&& Vertices);

	inline FVector GetTranslatedTransvoxel(const FVector& Vertex, const FVector& Normal, uint8 TransitionsMask, uint8 LOD)
	{
		const int32 Step = 1 << LOD;
		const int32 Size = RENDER_CHUNK_SIZE << LOD;
		
		const float LowerBound = Step;
		const float UpperBound = (RENDER_CHUNK_SIZE - 1) * Step;
		
		if ((LowerBound <= Vertex.X && Vertex.X <= UpperBound) &&
			(LowerBound <= Vertex.Y && Vertex.Y <= UpperBound) &&
			(LowerBound <= Vertex.Z && Vertex.Z <= UpperBound))
		{
			// Fast exit
			return Vertex;
		}
		
		if ((Vertex.X == 0.f && !(TransitionsMask & EVoxelDirectionFlag::XMin)) || (Vertex.X == Size && !(TransitionsMask & EVoxelDirectionFlag::XMax)) ||
	  	    (Vertex.Y == 0.f && !(TransitionsMask & EVoxelDirectionFlag::YMin)) || (Vertex.Y == Size && !(TransitionsMask & EVoxelDirectionFlag::YMax)) ||
	  	    (Vertex.Z == 0.f && !(TransitionsMask & EVoxelDirectionFlag::ZMin)) || (Vertex.Z == Size && !(TransitionsMask & EVoxelDirectionFlag::ZMax)))
		{
			// Can't translate when on a corner
			return Vertex;
		}

		FVector Delta(0.f);

		if ((TransitionsMask & EVoxelDirectionFlag::XMin) && Vertex.X < LowerBound)
		{
			Delta.X = LowerBound - Vertex.X;
		}
		if ((TransitionsMask & EVoxelDirectionFlag::XMax) && Vertex.X > UpperBound)
		{
			Delta.X = UpperBound - Vertex.X;
		}
		if ((TransitionsMask & EVoxelDirectionFlag::YMin) && Vertex.Y < LowerBound)
		{
			Delta.Y = LowerBound - Vertex.Y;
		}
		if ((TransitionsMask & EVoxelDirectionFlag::YMax) && Vertex.Y > UpperBound)
		{
			Delta.Y = UpperBound - Vertex.Y;
		}
		if ((TransitionsMask & EVoxelDirectionFlag::ZMin) && Vertex.Z < LowerBound)
		{
			Delta.Z = LowerBound - Vertex.Z;
		}
		if ((TransitionsMask & EVoxelDirectionFlag::ZMax) && Vertex.Z > UpperBound)
		{
			Delta.Z = UpperBound - Vertex.Z;
		}

		Delta /= 4;

		const FVector Q = FVector(
			(1 - Normal.X * Normal.X) * Delta.X -      Normal.Y * Normal.X  * Delta.Y -      Normal.Z * Normal.X  * Delta.Z,
			   - Normal.X * Normal.Y  * Delta.X + (1 - Normal.Y * Normal.Y) * Delta.Y -      Normal.Z * Normal.Y  * Delta.Z,
			   - Normal.X * Normal.Z  * Delta.X -      Normal.Y * Normal.Z  * Delta.Y + (1 - Normal.Z * Normal.Z) * Delta.Z);

		return Vertex + Q;
	}
	
	template<typename T>
	FORCEINLINE FVector2D GetUVs(const T& Mesher, const FVector& IntersectionPoint)
	{
		const auto& UVConfig = Mesher.Settings.UVConfig;
		const auto& UVScale = Mesher.Settings.UVScale;
		const auto& Data = Mesher.Data;
		const auto& ChunkPosition = Mesher.ChunkPosition;

		if (UVConfig == EVoxelUVConfig::PackWorldUpInUVs)
		{
			const FVector WorldUp = Data.Generator->GetUpVector(
				v_flt(ChunkPosition.X) + IntersectionPoint.X,
				v_flt(ChunkPosition.Y) + IntersectionPoint.Y,
				v_flt(ChunkPosition.Z) + IntersectionPoint.Z).GetSafeNormal();
			return FVector2D(WorldUp.X, WorldUp.Y);
		}
		else if (UVConfig == EVoxelUVConfig::GlobalUVs)
		{
			return FVector2D(ChunkPosition.X + IntersectionPoint.X, ChunkPosition.Y + IntersectionPoint.Y) / UVScale;
		}
		else
		{
			check(UVConfig == EVoxelUVConfig::PerVoxelUVs);
			return FVector2D(IntersectionPoint.X, IntersectionPoint.Y);
		}
	}
	
	template<typename T>
	inline static void SanitizeMesh(TArray<uint32>& Indices, TArray<T>& Vertices)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		TArray<uint32> NewIndices;
		NewIndices.Reserve(Indices.Num());
		check(Indices.Num() % 3 == 0);
		for (int32 Index = 0; Index < Indices.Num(); Index += 3)
		{
			const uint32 IndexA = FVoxelUtilities::Get(Indices, Index + 0);
			const uint32 IndexB = FVoxelUtilities::Get(Indices, Index + 1);
			const uint32 IndexC = FVoxelUtilities::Get(Indices, Index + 2);

			const auto& A = FVoxelUtilities::Get(Vertices, IndexA);
			const auto& B = FVoxelUtilities::Get(Vertices, IndexB);
			const auto& C = FVoxelUtilities::Get(Vertices, IndexC);
			
			const FVector BA = B.Position - A.Position;
			const FVector CA = C.Position - A.Position;
			const FVector Cross = FVector::CrossProduct(BA, CA);
			if (Cross.Size() > 1e-4) // See Chaos::FConvexBuilder::IsValidTriangle
			{
				NewIndices.Emplace(IndexA);
				NewIndices.Emplace(IndexB);
				NewIndices.Emplace(IndexC);
			}
		}
		Indices = MoveTemp(NewIndices);
	}
	
	template<typename T>
	inline static void RemoveUnusedVertices(TArray<uint32>& Indices, TArray<T>& Vertices)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();

		TBitArray<> UsedVertices(false, Vertices.Num());
		for (uint32 Index : Indices)
		{
			UsedVertices[Index] = true;
		}
		
		TArray<uint32> NewIndices;
		NewIndices.Empty(Vertices.Num());
		NewIndices.SetNumUninitialized(Vertices.Num());
		
		int32 WriteIndex = 0;
		for (int32 ReadIndex = 0; ReadIndex < Vertices.Num(); ReadIndex++)
		{
			if (UsedVertices[ReadIndex])
			{
				NewIndices[ReadIndex] = WriteIndex;
				Vertices[WriteIndex] = Vertices[ReadIndex];
				WriteIndex++;
			}
			else
			{
				NewIndices[ReadIndex] = -1;
			}
		}
		
		check(WriteIndex <= Vertices.Num());
		Vertices.SetNum(WriteIndex, false);

		for (uint32& Index : Indices)
		{
			Index = NewIndices[Index];
			checkVoxelSlow(Index != -1);
		}
	}
}