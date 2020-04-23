// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelProcMeshBuffers.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Indices);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Positions);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Colors);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Adjacency);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_UVs_Tangents);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Voxel Proc Mesh Buffers"), STAT_NumVoxelProcMeshBuffers, STATGROUP_VoxelCounters);

FVoxelProcMeshBuffers::FVoxelProcMeshBuffers()
{
	INC_DWORD_STAT(STAT_NumVoxelProcMeshBuffers);
}

FVoxelProcMeshBuffers::~FVoxelProcMeshBuffers()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);

	DEC_DWORD_STAT(STAT_NumVoxelProcMeshBuffers);
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
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);
	LastAllocatedSize = GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);

	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);
	LastAllocatedSize_Indices = IndexBuffer.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Indices, LastAllocatedSize_Indices);

	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);
	LastAllocatedSize_Positions = VertexBuffers.PositionVertexBuffer.GetNumVertices() * VertexBuffers.PositionVertexBuffer.GetStride();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Positions, LastAllocatedSize_Positions);

	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);
	LastAllocatedSize_Colors = VertexBuffers.ColorVertexBuffer.GetNumVertices() * VertexBuffers.ColorVertexBuffer.GetStride();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Colors, LastAllocatedSize_Colors);

	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);
	LastAllocatedSize_Adjacency = AdjacencyIndexBuffer.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_Adjacency, LastAllocatedSize_Adjacency);

	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);
	LastAllocatedSize_UVs_Tangents = VertexBuffers.StaticMeshVertexBuffer.GetResourceSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_UVs_Tangents, LastAllocatedSize_UVs_Tangents);
}