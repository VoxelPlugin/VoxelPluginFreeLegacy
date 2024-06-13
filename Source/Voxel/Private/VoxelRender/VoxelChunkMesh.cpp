// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "DistanceFieldAtlas.h"

#if ENABLE_TESSELLATION
#include "ThirdParty/nvtesslib/inc/nvtess.h"
#endif

#if ENABLE_OPTIMIZE_INDICES
#include "ThirdParty/ForsythTriOO/Src/forsythtriangleorderoptimizer.h"
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelChunkMeshMemory);

#if ENABLE_TESSELLATION
/**
* Provides static mesh render data to the NVIDIA tessellation library.
*/
class FVoxelStaticMeshNvRenderBuffer : public nv::RenderBuffer
{
public:

	/** Construct from static mesh render buffers. */
	FVoxelStaticMeshNvRenderBuffer(
		const TArray<FVector>& InPositionVertexBuffer,
		const TArray<uint32>& Indices)
		: PositionVertexBuffer(InPositionVertexBuffer)
	{
		mIb = new nv::IndexBuffer((void*)Indices.GetData(), nv::IBT_U32, Indices.Num(), false);
	}

	/** Retrieve the position and first texture coordinate of the specified index. */
	virtual nv::Vertex getVertex(const unsigned int Index) const override
	{
		nv::Vertex Vertex;

		const FVector& Position = PositionVertexBuffer[Index];
		Vertex.pos.x = Position.X;
		Vertex.pos.y = Position.Y;
		Vertex.pos.z = Position.Z;

		Vertex.uv.x = 0.0f;
		Vertex.uv.y = 0.0f;

		return Vertex;
	}

private:
	/** The position vertex buffer for the static mesh. */
	const TArray<FVector>& PositionVertexBuffer;

	/** Copying is forbidden. */
	FVoxelStaticMeshNvRenderBuffer(const FVoxelStaticMeshNvRenderBuffer&) = delete;
	FVoxelStaticMeshNvRenderBuffer& operator=(const FVoxelStaticMeshNvRenderBuffer&) = delete;
};
#endif

void FVoxelChunkMeshBuffers::BuildAdjacency(TArray<uint32>& OutAdjacencyIndices) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
#if ENABLE_TESSELLATION
	if (Indices.Num())
	{
		FVoxelStaticMeshNvRenderBuffer StaticMeshRenderBuffer(Positions, Indices);
		nv::IndexBuffer* PnAENIndexBuffer = nv::tess::buildTessellationBuffer(&StaticMeshRenderBuffer, nv::DBM_PnAenDominantCorner, true);
		check(PnAENIndexBuffer);
		const int32 IndexCount = int32(PnAENIndexBuffer->getLength());
		OutAdjacencyIndices.Empty(IndexCount);
		OutAdjacencyIndices.AddUninitialized(IndexCount);
		for (int32 Index = 0; Index < IndexCount; ++Index)
		{
			OutAdjacencyIndices[Index] = (*PnAENIndexBuffer)[Index];
		}
		delete PnAENIndexBuffer;
	}
	else
	{
		OutAdjacencyIndices.Empty();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelChunkMeshBuffers::OptimizeIndices()
{
#if ENABLE_OPTIMIZE_INDICES
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	TArray<uint32> OptimizedIndices;
	OptimizedIndices.AddUninitialized(Indices.Num());
	const uint16 CacheSize = 32;
	Forsyth::OptimizeFaces(Indices.GetData(), Indices.Num(), GetNumVertices(), OptimizedIndices.GetData(), CacheSize);
	Indices = MoveTemp(OptimizedIndices);
#endif
}

void FVoxelChunkMeshBuffers::Shrink()
{
	Positions.Shrink();
	Normals.Shrink();
	Tangents.Shrink();
	Colors.Shrink();
	for (auto& T : TextureCoordinates) T.Shrink();

	UpdateStats();
}

void FVoxelChunkMeshBuffers::ComputeBounds()
{
	Bounds = FBox(ForceInit);
	for (auto& Vertex : Positions)
	{
		Bounds += Vertex;
	}
}

void FVoxelChunkMeshBuffers::UpdateStats()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
	LastAllocatedSize = Indices.GetAllocatedSize();
	LastAllocatedSize += Positions.GetAllocatedSize();
	LastAllocatedSize += Normals.GetAllocatedSize();
	LastAllocatedSize += Tangents.GetAllocatedSize();
	LastAllocatedSize += Colors.GetAllocatedSize();
	for (auto& T : TextureCoordinates) LastAllocatedSize += T.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelChunkMeshMemory, LastAllocatedSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelChunkMesh::BuildDistanceField(int32 LOD, const FIntVector& Position, const FVoxelData& Data, const FVoxelRendererSettingsBase& Settings)
{
}
