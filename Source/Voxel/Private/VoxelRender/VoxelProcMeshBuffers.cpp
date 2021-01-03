// Copyright 2021 Phyronnaz

#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelRenderUtilities.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Indices);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Positions);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Colors);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_Adjacency);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelProcMeshMemory_UVsAndTangents);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Voxel Proc Mesh Buffers"), STAT_NumVoxelProcMeshBuffers, STATGROUP_VoxelCounters);

FVoxelProcMeshBuffers::FVoxelProcMeshBuffers()
{
	INC_DWORD_STAT(STAT_NumVoxelProcMeshBuffers);
}

FVoxelProcMeshBuffers::~FVoxelProcMeshBuffers()
{
	DEC_DWORD_STAT(STAT_NumVoxelProcMeshBuffers);
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);

#define UPDATE(Name) \
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_ ##  Name, LastAllocatedSize_ ## Name);
	
	UPDATE(Indices);
	UPDATE(Positions);
	UPDATE(Colors);
	UPDATE(Adjacency);
	UPDATE(UVsAndTangents);

#undef UPDATE
}

uint32 FVoxelProcMeshBuffers::GetAllocatedSize() const
{
	return
		Guids.GetAllocatedSize() +
		VertexBuffers.StaticMeshVertexBuffer.GetResourceSize() +
		VertexBuffers.PositionVertexBuffer.GetNumVertices() * VertexBuffers.PositionVertexBuffer.GetStride() +
		VertexBuffers.ColorVertexBuffer.GetNumVertices() * VertexBuffers.ColorVertexBuffer.GetStride() +
		IndexBuffer.GetAllocatedSize() +
		AdjacencyIndexBuffer.GetAllocatedSize() +
		CollisionCubes.GetAllocatedSize();
}

void FVoxelProcMeshBuffers::UpdateStats()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);
	LastAllocatedSize = GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory, LastAllocatedSize);

#define UPDATE(Name, Size) \
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_ ## Name, LastAllocatedSize_ ## Name); \
	LastAllocatedSize_ ## Name = Size; \
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelProcMeshMemory_ ## Name, LastAllocatedSize_ ## Name); \

	UPDATE(Indices, IndexBuffer.GetAllocatedSize());
	UPDATE(Positions, VertexBuffers.PositionVertexBuffer.GetNumVertices() * VertexBuffers.PositionVertexBuffer.GetStride());
	UPDATE(Colors, VertexBuffers.ColorVertexBuffer.GetNumVertices() * VertexBuffers.ColorVertexBuffer.GetStride());
	UPDATE(Adjacency, AdjacencyIndexBuffer.GetAllocatedSize());
	UPDATE(UVsAndTangents, VertexBuffers.StaticMeshVertexBuffer.GetResourceSize());

#undef UPDATE
}