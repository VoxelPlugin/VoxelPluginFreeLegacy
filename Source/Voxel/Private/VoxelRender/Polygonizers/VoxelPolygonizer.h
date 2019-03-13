// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelId.h"
#include "IntBox.h"
#include "VoxelConfigEnums.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelData/VoxelDataOctree.h"

class FVoxelData;
class AVoxelWorld;
struct FVoxelStatsElement;

class FVoxelPolygonizerBase
{
public:
	const int LOD;
	// Step between cubes
	const int Step;
	const int Size;
	FVoxelData* const Data;
	TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe> const DebugManager;
	const FIntVector ChunkPosition;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const FVoxelMeshProcessingParameters MeshParameters;

	FVoxelChunk& Chunk;
	FVoxelStatsElement& Stats;

	FVoxelPolygonizerBase(FVoxelPolygonizerAsyncWorkBase* Work)
		: LOD(Work->LOD)
		, Step(1 << Work->LOD)
		, Size(CHUNK_SIZE << Work->LOD)
		, Data(&Work->Data.Get())
		, DebugManager(Work->DebugManager)
		, ChunkPosition(Work->ChunkPosition)
		, NormalConfig(Work->NormalConfig)
		, MaterialConfig(Work->MaterialConfig)
		, UVConfig(Work->UVConfig)
		, MeshParameters(Work->MeshParameters)
		, Chunk(*Work->Chunk)
		, Stats(Work->Stats)
	{
	}
	virtual ~FVoxelPolygonizerBase() = default;

	virtual bool Create() = 0;
};

class FVoxelPolygonizer : public FVoxelPolygonizerBase
{
public:
	using FVoxelPolygonizerBase::FVoxelPolygonizerBase;
	virtual ~FVoxelPolygonizer() = default;

	bool Create() final;

protected:
	FVoxelLockedOctrees Octrees;

	virtual FIntBox GetValuesBounds() const = 0;
	virtual FIntBox GetLockedBounds() const = 0;
	virtual bool CreateChunk() = 0;
};

class FVoxelTransitionsPolygonizer : public FVoxelPolygonizerBase
{
public:
	const uint8 TransitionsMask;
	const int HalfLOD;
	const int HalfStep;

	FVoxelTransitionsPolygonizer(FVoxelTransitionsPolygonizerAsyncWork* Work)
		: FVoxelPolygonizerBase(Work)
		, TransitionsMask(Work->TransitionsMask)
		, HalfLOD(LOD - 1)
		, HalfStep(Step / 2)
	{
	}
	virtual ~FVoxelTransitionsPolygonizer() = default;

	bool Create() final;

protected:
	FVoxelLockedOctrees Octrees;

	virtual FIntBox GetBounds() const = 0;
	virtual bool CreateTransitions() = 0;
};