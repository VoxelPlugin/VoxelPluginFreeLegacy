// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelProcMeshBuffers.h"

DEFINE_STAT(STAT_VoxelProcMeshMemory);
DEFINE_STAT(STAT_VoxelProcMeshMemory_Indices);
DEFINE_STAT(STAT_VoxelProcMeshMemory_Positions);
DEFINE_STAT(STAT_VoxelProcMeshMemory_Colors);
DEFINE_STAT(STAT_VoxelProcMeshMemory_Adjacency);
DEFINE_STAT(STAT_VoxelProcMeshMemory_UVs_Tangents);

FVoxelProcMeshBuffers::~FVoxelProcMeshBuffers()
{
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);
}

uint32 FVoxelProcMeshBuffers::GetAllocatedSize() const
{
	return
			VertexBuffers.StaticMeshVertexBuffer.GetResourceSize() +
			VertexBuffers.PositionVertexBuffer.GetNumVertices() * VertexBuffers.PositionVertexBuffer.GetStride() +
			VertexBuffers.ColorVertexBuffer.GetNumVertices() * VertexBuffers.ColorVertexBuffer.GetStride() +
			IndexBuffer.GetAllocatedSize() +
			AdjacencyIndexBuffer.GetAllocatedSize();
}

void FVoxelProcMeshBuffers::UpdateStats()
{
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);
	LastAllocatedSize = GetAllocatedSize();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);

	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);
	LastAllocatedSize_Indices = IndexBuffer.GetAllocatedSize();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);

	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);
	LastAllocatedSize_Positions = VertexBuffers.PositionVertexBuffer.GetNumVertices() * VertexBuffers.PositionVertexBuffer.GetStride();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);

	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);
	LastAllocatedSize_Colors = VertexBuffers.ColorVertexBuffer.GetNumVertices() * VertexBuffers.ColorVertexBuffer.GetStride();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);

	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);
	LastAllocatedSize_Adjacency = AdjacencyIndexBuffer.GetAllocatedSize();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);

	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);
	LastAllocatedSize_UVs_Tangents = VertexBuffers.StaticMeshVertexBuffer.GetResourceSize();
	INC_DWORD_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);
}