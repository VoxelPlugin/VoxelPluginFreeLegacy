// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDirection.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelPolygonizer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelDebug/VoxelStats.h"

class AVoxelWorld;

// +1: for end edge
#define MC_EXTENDED_CHUNK_SIZE (CHUNK_SIZE + 1)

#define EDGE_INDEX_COUNT 4

class FVoxelMCPolygonizer : public FVoxelPolygonizer
{
public:
	using FVoxelPolygonizer::FVoxelPolygonizer;

protected:
	FIntBox GetValuesBounds() const final;
	FIntBox GetLockedBounds() const final;
	bool CreateChunk() final;

private:
	TUniquePtr<FVoxelDataUtilities::MapAccelerator> MapAccelerator;

	FVoxelValue CachedValues[MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE * MC_EXTENDED_CHUNK_SIZE];

	// Cache to get index of already created vertices
	int32 Cache0[CHUNK_SIZE * CHUNK_SIZE * EDGE_INDEX_COUNT];
	int32 Cache1[CHUNK_SIZE * CHUNK_SIZE * EDGE_INDEX_COUNT];
	bool bCurrentCacheIs0 = false;

	inline int32* GetCurrentCache() { return bCurrentCacheIs0 ? Cache0 : Cache1; }
	inline int32* GetOldCache()     { return bCurrentCacheIs0 ? Cache1 : Cache0; }
	inline int32 GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY) const
	{
		checkVoxelSlow(0 <= LX && LX < CHUNK_SIZE);
		checkVoxelSlow(0 <= LY && LY < CHUNK_SIZE);
		checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < EDGE_INDEX_COUNT);
		return EdgeIndex + LX * EDGE_INDEX_COUNT + LY * EDGE_INDEX_COUNT * CHUNK_SIZE;
	}

	FVector GetNormal(const FVector& Position) const;

	inline FVoxelValue GetValueNoCache(int32 X, int32 Y, int32 Z)
	{
		FVoxelScopeValueAccessCounter Counter(Stats);
		checkVoxelSlow(LOD > 0);
		return MapAccelerator->GetValue(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}
	inline FVoxelMaterial GetMaterialNoCache(int32 X, int32 Y, int32 Z)
	{
		FVoxelScopeMaterialAccessCounter Counter(Stats);
		return MapAccelerator->GetMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}	
	inline FVoxelMaterial GetMaterialNoCache(const FIntVector& P) { return GetMaterialNoCache(P.X, P.Y, P.Z); }

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

			return FVoxelProcMeshVertex(Position, Normal, FVoxelProcMeshTangent(), Material.GetColor(), UVs);
		}
	};
};

#define TRANSITION_EDGE_INDEX_COUNT 10

class FVoxelMCTransitionsPolygonizer : public FVoxelTransitionsPolygonizer
{
public:
	using FVoxelTransitionsPolygonizer::FVoxelTransitionsPolygonizer;

protected:
	FIntBox GetBounds() const final;
	bool CreateTransitions() final;

private:	
	TUniquePtr<FVoxelDataUtilities::MapAccelerator> MapAccelerator;
	
	int32 Cache2D[CHUNK_SIZE * CHUNK_SIZE * TRANSITION_EDGE_INDEX_COUNT];
	inline int32 GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY) const
	{
		checkVoxelSlow(0 <= LX && LX < CHUNK_SIZE);
		checkVoxelSlow(0 <= LY && LY < CHUNK_SIZE);
		checkVoxelSlow(0 <= EdgeIndex && EdgeIndex < TRANSITION_EDGE_INDEX_COUNT);
		return EdgeIndex + LX * TRANSITION_EDGE_INDEX_COUNT + LY * TRANSITION_EDGE_INDEX_COUNT * CHUNK_SIZE;
	}

	FVector GetNormal(const FVector& Position) const;

	inline FVoxelValue GetValue(EVoxelDirection Direction, int32 X, int32 Y, int32 QueryLOD) const
	{
		int32 GX, GY, GZ;
		Local2DToGlobal(Direction, X, Y, 0, GX, GY, GZ);

		return MapAccelerator->GetValue(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, QueryLOD);
	}
	inline FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z) const
	{
		return MapAccelerator->GetMaterial(X + ChunkPosition.X, Y + ChunkPosition.Y, Z + ChunkPosition.Z, LOD);
	}
	
	inline void Local2DToGlobal(EVoxelDirection Direction, int32 LX, int32 LY, int32 LZ, int32& OutGX, int32& OutGY, int32& OutGZ) const
	{
		const int32& S = Size;
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

			return FVoxelProcMeshVertex(Position, Normal, FVoxelProcMeshTangent(), Material.GetColor(), UVs);
		}
	};
};
