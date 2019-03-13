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

class FVoxelCubicPolygonizer : public FVoxelPolygonizer
{
public:
	using FVoxelPolygonizer::FVoxelPolygonizer;
	
protected:
	FIntBox GetValuesBounds() const final;
	FIntBox GetLockedBounds() const final;
	bool CreateChunk() final;

private:
	FVoxelValue CachedValues[CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE];
	FVoxelMaterial CachedMaterials[CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE];

	inline int GetIndex(int X, int Y, int Z) const { return (X + 1) + (Y + 1) * CUBIC_EXTENDED_CHUNK_SIZE + (Z + 1) * CUBIC_EXTENDED_CHUNK_SIZE * CUBIC_EXTENDED_CHUNK_SIZE; }
	inline FVoxelValue GetValue(int X, int Y, int Z) const
	{
		check(IsInBounds(X, Y, Z));
		return CachedValues[GetIndex(X, Y, Z)];
	}
	inline FVoxelMaterial GetMaterial(int X, int Y, int Z) const
	{
		check(IsInBounds(X, Y, Z));
		return CachedMaterials[GetIndex(X, Y, Z)];
	}
	inline bool IsInBounds(int X, int Y, int Z) const
	{
		return -1 <= X && -1 <= Y && -1 <= Z && X <= CHUNK_SIZE && Y <= CHUNK_SIZE && Z <= CHUNK_SIZE;
	}
};

class FVoxelCubicTransitionsPolygonizer : public FVoxelTransitionsPolygonizer
{
public:
	using FVoxelTransitionsPolygonizer::FVoxelTransitionsPolygonizer;

protected:
	FIntBox GetBounds() const final;
	bool CreateTransitions() final;

private:
	TUniquePtr<FVoxelDataUtilities::MapAccelerator> MapAccelerator;
	
	inline int GetIndex(int X, int Y, int Z) const { return (X + 1) + (Y + 1) * (CHUNK_SIZE + 3) + (Z + 1) * (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3); }
	inline FVoxelValue GetValue(int InStep, EVoxelDirection Direction, int X, int Y, int Z)
	{
		int GX, GY, GZ;
		Local2DToGlobal(CHUNK_SIZE * Step - InStep, Direction, X, Y, Z, GX, GY, GZ);

		FVoxelScopeValueAccessCounter Counter(Stats);
		return MapAccelerator->GetValue(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, LOD);
	}	
	inline FVoxelMaterial GetMaterial(int InStep, EVoxelDirection Direction, int X, int Y, int Z)
	{
		int GX, GY, GZ;
		Local2DToGlobal(CHUNK_SIZE * Step - InStep, Direction, X, Y, Z, GX, GY, GZ);
		
		FVoxelScopeMaterialAccessCounter Counter(Stats);
		return MapAccelerator->GetMaterial(GX + ChunkPosition.X, GY + ChunkPosition.Y, GZ + ChunkPosition.Z, LOD);
	}
	
	// LX * HalfStep = GX
	void Add2DFace(int InStep, EVoxelDirection Direction, bool bInvert, const FVoxelMaterial& Material, int LX, int LY, TArray<FCubicLocalVoxelVertex>& Vertices, TArray<uint32>& Indices);

	inline void Local2DToGlobal(int InSize, EVoxelDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ) const
	{
		const int& S = InSize;
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
