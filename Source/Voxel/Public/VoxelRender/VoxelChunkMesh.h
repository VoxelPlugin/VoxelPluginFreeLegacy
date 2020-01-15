// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelRender/VoxelProcMeshTangent.h"
#include "VoxelRender/VoxelBlendedMaterial.h"

class FDistanceFieldVolumeData;
class FVoxelData;

DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Chunk Mesh Memory"), STAT_VoxelChunkMeshMemory, STATGROUP_VoxelMemory, VOXEL_API);

struct VOXEL_API FVoxelChunkMeshBuffers
{
	TArray<uint32> Indices;
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<FVoxelProcMeshTangent> Tangents;
	TArray<FColor> Colors;
	TArray<FVector2D> TextureCoordinates[NUM_VOXEL_TEXTURE_COORDINATES];
	FBox Bounds;
	FGuid Guid; // Use to avoid rebuilding collisions when the mesh didn't change

	~FVoxelChunkMeshBuffers()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
	}

	template<typename TAllocator>
	inline void SetIndices(TArray<uint32, TAllocator>&& InIndices)
	{
		Indices = MoveTemp(InIndices);
	}

	void Reserve(int32 Num, bool bRenderWorld)
	{
		Positions.Reserve(Num);
		if (bRenderWorld)
		{
			Normals.Reserve(Num);
			Tangents.Reserve(Num);
			Colors.Reserve(Num);
			for (uint32 Tex = 0; Tex < NUM_VOXEL_TEXTURE_COORDINATES; Tex++)
			{
				TextureCoordinates[Tex].Reserve(Num);
			}
		}
	}

	template<typename TVertex>
	FORCEINLINE uint32 AddVertex(const TVertex& Vertex, bool bRenderWorld)
	{
		const int32 Index = Positions.Emplace(Vertex.Position);
		if (bRenderWorld)
		{
			Normals.Emplace(Vertex.Normal);
			Tangents.Emplace(Vertex.Tangent);
			Colors.Emplace(Vertex.Color);
			for (uint32 Tex = 0; Tex < NUM_VOXEL_TEXTURE_COORDINATES; Tex++)
			{
				TextureCoordinates[Tex].Emplace(Vertex.TextureCoordinates[Tex]);
			}
		}
		return Index;
	}
	FORCEINLINE void AddIndex(uint32 Index)
	{
		Indices.Emplace(Index);
	}

	inline int32 GetNumVertices() const
	{
		return Positions.Num();
	}

	inline int32 GetAllocatedSize() const
	{
		return Indices.GetAllocatedSize()
			+ Positions.GetAllocatedSize()
			+ Normals.GetAllocatedSize()
			+ Tangents.GetAllocatedSize()
			+ Colors.GetAllocatedSize()
			+ [&]() { uint32 Count = 0; for (auto& T : TextureCoordinates) Count += T.GetAllocatedSize(); return Count; }();
	}

	void BuildAdjacency(TArray<uint32>& OutAdjacencyIndices) const;
	void OptimizeIndices();
	void Shrink();
	void ComputeBounds();

private:
	int32 LastAllocatedSize = 0;

	void UpdateStat()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
		LastAllocatedSize = GetAllocatedSize();
		INC_DWORD_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
	}
};

struct FVoxelChunkMesh
{
public:
	inline bool IsSingle() const
	{
		return bSingleBuffers;
	}
	inline bool IsEmpty() const
	{
		return bSingleBuffers ? SingleBuffers->Indices.Num() == 0 : Map.Num() == 0;
	}

	inline FBox GetBounds() const
	{
		return Bounds;
	}
	
	inline TVoxelSharedPtr<const FVoxelChunkMeshBuffers> GetSingleBuffers() const
	{
		ensure(IsSingle());
		ensure(SingleBuffers.IsValid());
		return SingleBuffers;
	}
	inline TVoxelSharedPtr<const FDistanceFieldVolumeData> GetDistanceFieldVolumeData() const
	{
		return DistanceFieldVolumeData;
	}
	inline TVoxelSharedPtr<const FVoxelChunkMeshBuffers> FindBuffer(FVoxelBlendedMaterialUnsorted Material) const
	{
		ensure(!IsSingle());
		return Map.FindRef(Material);
	}

public:
	inline void SetIsSingle(bool bIsSingle)
	{
		bSingleBuffers = bIsSingle;
	}
	inline FVoxelChunkMeshBuffers& CreateSingleBuffers()
	{
		ensure(IsSingle());
		ensure(!SingleBuffers.IsValid());
		SingleBuffers = MakeVoxelShared<FVoxelChunkMeshBuffers>();
		return *SingleBuffers;
	}
	inline FVoxelChunkMeshBuffers& FindOrAddBuffer(FVoxelBlendedMaterialUnsorted Material)
	{
		ensure(!IsSingle());
		auto* BufferPtr = Map.Find(Material);
		if (!BufferPtr)
		{
			BufferPtr = &Map.Add(Material, MakeVoxelShared<FVoxelChunkMeshBuffers>());
		}
		return **BufferPtr;
	}
	
public:
	void BuildDistanceField(int32 LOD, const FIntVector& Position, const FVoxelData& Data);
	void ComputeBounds();
	void ComputeGuid();
	
	template<typename T>
	inline void IterateBuffers(T Lambda)
	{
		if (bSingleBuffers)
		{
			Lambda(*SingleBuffers);
		}
		else
		{
			for (auto& It : Map)
			{
				Lambda(*It.Value);
			}
		}
	}
	template<typename T>
	inline void IterateBuffers(T Lambda) const
	{
		if (bSingleBuffers)
		{
			Lambda(static_cast<const FVoxelChunkMeshBuffers&>(*SingleBuffers));
		}
		else
		{
			for (auto& It : Map)
			{
				Lambda(static_cast<const FVoxelChunkMeshBuffers&>(*It.Value));
			}
		}
	}
	
	template<typename T>
	inline void IterateMaterials(T Lambda) const
	{
		ensure(!IsSingle());
		for(auto& It : Map)
		{
			Lambda(It.Key);
		}
	}

private:
	FBox Bounds;
	bool bSingleBuffers = false;
	TMap<FVoxelBlendedMaterialUnsorted, TVoxelSharedPtr<FVoxelChunkMeshBuffers>> Map;
	TVoxelSharedPtr<FVoxelChunkMeshBuffers> SingleBuffers;
	TVoxelSharedPtr<FDistanceFieldVolumeData> DistanceFieldVolumeData;
};