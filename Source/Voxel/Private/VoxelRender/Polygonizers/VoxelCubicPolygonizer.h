// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelConfigEnums.h"
#include "VoxelData/VoxelData.h"
#include "VoxelDebug/VoxelStats.h"

class FVoxelData;
struct FVoxelMaterial;

#define CUBIC_EXTENDED_CHUNK_SIZE (CHUNK_SIZE + 2)

struct FCubicLocalVoxelVertex
{
	FVector Position;
	FVector Normal;
	FVoxelProcMeshTangent Tangent;
	FVoxelMaterial Material;
	FVector2D UVs;

	FCubicLocalVoxelVertex() = default;

	inline FVoxelProcMeshVertex GetProcMeshVertex() const
	{
		return FVoxelProcMeshVertex(Position, Normal, Tangent, Material.GetColor(), UVs, Material.GetVoxelGrassId(), Material.GetVoxelActorId());
	}
};

class FVoxelCubicPolygonizer
{
public:
	const int LOD;
	const int Step;
	FVoxelData* const Data;
	FIntVector const ChunkPosition;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const FVoxelMeshProcessingParameters MeshParameters;

	FVoxelCubicPolygonizer(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, EVoxelMaterialConfig MaterialConfig, EVoxelUVConfig UVConfig, FVoxelMeshProcessingParameters MeshParameters);

	bool CreateSection(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats);
	
	inline FVoxelMaterial GetMaterial(int X, int Y, int Z) const
	{
		check(IsInBounds(X, Y, Z));
		return CachedMaterials[GetIndex(X, Y, Z)];
	}

private:
	FVoxelValue CachedValues[CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE];
	FVoxelMaterial CachedMaterials[CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE];

	inline int GetIndex(int X, int Y, int Z) const { return (X + 1) + (Y + 1) * CUBIC_EXTENDED_CHUNK_SIZE + (Z + 1) * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE; }
	inline FVoxelValue GetValue(int X, int Y, int Z) const
	{
		check(IsInBounds(X, Y, Z));
		return CachedValues[GetIndex(X, Y, Z)];
	}
	inline bool IsInBounds(int X, int Y, int Z) const
	{
		return -1 <= X && -1 <= Y && -1 <= Z && X <= CHUNK_SIZE && Y <= CHUNK_SIZE && Z <= CHUNK_SIZE;
	}
};

class FVoxelCubicTransitionsPolygonizer
{
public:
	const int LOD;
	const int Step;
	FVoxelData* const Data;
	FIntVector const ChunkPosition;
	const uint8 TransitionsMask;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const FVoxelMeshProcessingParameters MeshParameters;

	FVoxelCubicTransitionsPolygonizer(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, uint8 TransitionsMask, EVoxelMaterialConfig MaterialConfig, EVoxelUVConfig UVConfig, FVoxelMeshProcessingParameters MeshParameters);

	bool CreateTransitions(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats);

private:
	TUniquePtr<FVoxelData::MapAccelerator> MapAccelerator;
	
	inline int GetIndex(int X, int Y, int Z) const { return (X + 1) + (Y + 1) * (CHUNK_SIZE + 3) + (Z + 1) * (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3); }
	inline FVoxelValue GetValue(int InStep, EVoxelDirection Direction, int X, int Y, int Z) const
	{
		int GX, GY, GZ;
		Local2DToGlobal(CHUNK_SIZE * Step - InStep, Direction, X, Y, Z, GX, GY, GZ);

		return MapAccelerator->GetValue(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, LOD);
	}	
	inline FVoxelMaterial GetMaterial(int InStep, EVoxelDirection Direction, int X, int Y, int Z) const
	{
		int GX, GY, GZ;
		Local2DToGlobal(CHUNK_SIZE * Step - InStep, Direction, X, Y, Z, GX, GY, GZ);

		return MapAccelerator->GetMaterial(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, LOD);
	}
	
	// LX * HalfStep = GX
	void Add2DFace(int InStep, EVoxelDirection Direction, bool bInvert, const FVoxelMaterial& Material, int LX, int LY, TArray<FCubicLocalVoxelVertex>& Vertices, TArray<uint32>& Indices);

	inline void Local2DToGlobal(int Size, EVoxelDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ) const
	{
		const int& S = Size;
		switch (Direction)
		{
		case XMin:
			OutGX = LZ;
			OutGY = LX;
			OutGZ = LY;
			break;
		case XMax:
			OutGX = S - LZ;
			OutGY = LY;
			OutGZ = LX;
			break;
		case YMin:
			OutGX = LY;
			OutGY = LZ;
			OutGZ = LX;
			break;
		case YMax:
			OutGX = LX;
			OutGY = S - LZ;
			OutGZ = LY;
			break;
		case ZMin:
			OutGX = LX;
			OutGY = LY;
			OutGZ = LZ;
			break;
		case ZMax:
			OutGX = LY;
			OutGY = LX;
			OutGZ = S - LZ;
			break;
		default:
			check(false);
			break;
		}
	}
};
