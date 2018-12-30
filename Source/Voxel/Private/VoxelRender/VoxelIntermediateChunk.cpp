// Copyright 2018 Phyronnaz

#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelGlobals.h"
#if ENABLE_TESSELLATION
#include "ThirdParty/nvtesslib/inc/nvtess.h"
#endif
#if ENABLE_OPTIMIZE_INDICES
#include "ThirdParty/ForsythTriOO/Src/forsythtriangleorderoptimizer.h"
#endif

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
	virtual nv::Vertex getVertex(unsigned int Index) const
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
	FVoxelStaticMeshNvRenderBuffer(const FVoxelStaticMeshNvRenderBuffer&);
	FVoxelStaticMeshNvRenderBuffer& operator=(const FVoxelStaticMeshNvRenderBuffer&);
};
#endif

void FVoxelChunkBuffers::BuildAdjacency()
{
#if ENABLE_TESSELLATION
	if (Indices.Num())
	{
		FVoxelStaticMeshNvRenderBuffer StaticMeshRenderBuffer(Positions, Indices);
		nv::IndexBuffer* PnAENIndexBuffer = nv::tess::buildTessellationBuffer(&StaticMeshRenderBuffer, nv::DBM_PnAenDominantCorner, true);
		check(PnAENIndexBuffer);
		const int32 IndexCount = (int32)PnAENIndexBuffer->getLength();
		AdjacencyIndices.Empty(IndexCount);
		AdjacencyIndices.AddUninitialized(IndexCount);
		for (int32 Index = 0; Index < IndexCount; ++Index)
		{
			AdjacencyIndices[Index] = (*PnAENIndexBuffer)[Index];
		}
		delete PnAENIndexBuffer;
	}
	else
	{
		AdjacencyIndices.Empty();
	}
#endif
}

void FVoxelChunkBuffers::OptimizeIndices()
{
#if ENABLE_OPTIMIZE_INDICES
	TArray<uint32> OptimizedIndices;
	OptimizedIndices.AddUninitialized(Indices.Num());
	uint16 CacheSize = 32;
	Forsyth::OptimizeFaces(Indices.GetData(), Indices.Num(), GetNumVertices(), OptimizedIndices.GetData(), CacheSize);
	Indices = MoveTemp(OptimizedIndices);
#endif
}
