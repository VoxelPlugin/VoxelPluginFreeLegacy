// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDirection.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelData/VoxelData.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelConfigEnums.h"
#include "VoxelDebug/VoxelStats.h"

// +1: for end edge, +2: for normals
#define MC_EXTENDED_CHUNK_SIZE (CHUNK_SIZE + 3)

#define EDGE_INDEX_COUNT 4

class FVoxelMCPolygonizer
{
public:
	FVoxelMCPolygonizer(
		int LOD, 
		FVoxelData* Data, 
		const FIntVector& ChunkPosition, 
		EVoxelNormalConfig NormalConfig, 
		EVoxelMaterialConfig MaterialConfig, 
		EVoxelUVConfig UVConfig,
		bool bCacheLOD0Chunks,
		FVoxelMeshProcessingParameters MeshParameters);

	bool CreateChunk(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats);
	
	// For NormalImpl
	inline FVoxelValue GetValue(int X, int Y, int Z, int QueryLOD) const;

private:
	const int LOD;
	// Step between cubes
	const int Step;
	const int Size;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const bool bCacheLOD0Chunks;
	const FVoxelMeshProcessingParameters MeshParameters;

	TUniquePtr<FVoxelData::MapAccelerator> MapAccelerator;

	FVoxelValue CachedValues[MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE];

	// Cache to get index of already created vertices
	int Cache0[CHUNK_SIZE * CHUNK_SIZE * EDGE_INDEX_COUNT];
	int Cache1[CHUNK_SIZE * CHUNK_SIZE * EDGE_INDEX_COUNT];
	bool bCurrentCacheIs0 = false;

	inline int* GetCurrentCache() { return bCurrentCacheIs0 ? Cache0 : Cache1; }
	inline int* GetOldCache()     { return bCurrentCacheIs0 ? Cache1 : Cache0; }
	inline int GetCacheIndex(int EdgeIndex, int LX, int LY) const
	{
		checkVoxelSlow(0 <= LX && LX < CHUNK_SIZE);
		checkVoxelSlow(0 <= LY && LY < CHUNK_SIZE);
		checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < EDGE_INDEX_COUNT);
		return EdgeIndex + LX * EDGE_INDEX_COUNT + LY * EDGE_INDEX_COUNT * CHUNK_SIZE;
	}

	FVector GetNormal(const FVector& Position) const;

	inline FVoxelValue GetValueNoCache(int X, int Y, int Z) const
	{
		checkVoxelSlow(LOD > 0);
		return MapAccelerator->GetValue(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}
	inline FVoxelMaterial GetMaterialNoCache(int X, int Y, int Z) const
	{
		return MapAccelerator->GetMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}	
	inline FVoxelMaterial GetMaterialNoCache(const FIntVector& P) const { return GetMaterialNoCache(P.X, P.Y, P.Z); }

private:	
	struct FLocalVoxelVertex
	{
		FVector Position;
		FVector GradientNormal;
		FVector NormalSum = FVector::ZeroVector;
		FVoxelMaterial Material = FVoxelMaterial(NoInit);
		FVector2D UVs;
		EVoxelNormalConfig VertexNormalConfig;
		bool bForceUseGradient = false;
		
		FLocalVoxelVertex(EVoxelNormalConfig VertexNormalConfig) : VertexNormalConfig(VertexNormalConfig) {}

		inline FVoxelProcMeshVertex GetProcMeshVertex() const
		{
			FVector Normal;
			if (VertexNormalConfig == EVoxelNormalConfig::GradientNormal || bForceUseGradient)
			{
				Normal = GradientNormal;
			}
			else if (VertexNormalConfig == EVoxelNormalConfig::MeshNormal)
			{
				Normal = NormalSum.GetSafeNormal();
			}
			else
			{
				check(VertexNormalConfig == EVoxelNormalConfig::NoNormal);
				Normal = FVector::ZeroVector;
			}

			return FVoxelProcMeshVertex(Position, Normal, FVoxelProcMeshTangent(), Material.GetColor(), UVs, Material.GetVoxelGrassId(), Material.GetVoxelActorId());
		}
	};
};

#define TRANSITION_EDGE_INDEX_COUNT 10

class FVoxelMCTransitionsPolygonizer
{
public:
	FVoxelMCTransitionsPolygonizer(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, uint8 TransitionsMask, EVoxelNormalConfig NormalConfig, EVoxelMaterialConfig MaterialConfig, EVoxelUVConfig UVConfig, FVoxelMeshProcessingParameters MeshParameters);

	bool CreateTransitions(FVoxelChunk& OutChunk, FVoxelStatsElement& Stats);

private:
	const int LOD; // LOD of the low res chunk (ie highest LOD value)
	const int HalfLOD;
	const int Step;
	const int HalfStep;
	const int Size;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const uint8 TransitionsMask;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const FVoxelMeshProcessingParameters MeshParameters;
	
	TUniquePtr<FVoxelData::MapAccelerator> MapAccelerator;
	
	int Cache2D[CHUNK_SIZE * CHUNK_SIZE * TRANSITION_EDGE_INDEX_COUNT];
	inline int GetCacheIndex(int EdgeIndex, int LX, int LY) const
	{
		checkVoxelSlow(0 <= LX && LX < CHUNK_SIZE);
		checkVoxelSlow(0 <= LY && LY < CHUNK_SIZE);
		checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < TRANSITION_EDGE_INDEX_COUNT);
		return EdgeIndex + LX * TRANSITION_EDGE_INDEX_COUNT + LY * TRANSITION_EDGE_INDEX_COUNT * CHUNK_SIZE;
	}

	FVector GetNormal(const FVector& Position) const;

	inline FVoxelValue GetValue(EVoxelDirection Direction, int X, int Y, int QueryLOD) const
	{
		int GX, GY, GZ;
		Local2DToGlobal(Direction, X, Y, 0, GX, GY, GZ);

		return MapAccelerator->GetValue(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, QueryLOD);
	}
	inline FVoxelMaterial GetMaterial(int X, int Y, int Z) const
	{
		return MapAccelerator->GetMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}
	
	inline void Local2DToGlobal(EVoxelDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ) const
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

private:
	struct FLocalVoxelVertex
	{
		FVector Position;
		FVector GradientNormal;
		FVoxelMaterial Material = FVoxelMaterial(NoInit);
		FVector2D UVs;
		EVoxelNormalConfig VertexNormalConfig;

		FLocalVoxelVertex(EVoxelNormalConfig VertexNormalConfig) : VertexNormalConfig(VertexNormalConfig) {}

		inline FVoxelProcMeshVertex GetProcMeshVertex() const
		{
			FVector Normal;
			if (VertexNormalConfig == EVoxelNormalConfig::GradientNormal || VertexNormalConfig == EVoxelNormalConfig::MeshNormal)
			{
				Normal = GradientNormal;
			}
			else
			{
				check(VertexNormalConfig == EVoxelNormalConfig::NoNormal);
				Normal = FVector::ZeroVector;
			}

			return FVoxelProcMeshVertex(Position, Normal, FVoxelProcMeshTangent(), Material.GetColor(), UVs, Material.GetVoxelGrassId(), Material.GetVoxelActorId());
		}
	};
};
