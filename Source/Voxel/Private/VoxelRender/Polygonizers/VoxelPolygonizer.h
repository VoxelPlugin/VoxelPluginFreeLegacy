// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelId.h"
#include "IntBox.h"
#include "VoxelConfigEnums.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelData/VoxelDataOctree.h"

class FVoxelData;
class AVoxelWorld;
struct FVoxelStatsElement;
class FVoxelDebugManager;
class FVoxelPolygonizerAsyncWorkBase;
class FVoxelTransitionsPolygonizerAsyncWork;

struct FVoxelPolygonizerBaseSettings
{
	int32 LOD;
	FVoxelData* Data;
	TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe> DebugManager;
	FIntVector ChunkPosition;
	EVoxelNormalConfig NormalConfig;
	EVoxelMaterialConfig MaterialConfig;
	EVoxelUVConfig UVConfig;
	float UVScale;
	FVoxelMeshProcessingParameters MeshParameters;

	FVoxelChunk* Chunk;
	FVoxelStatsElement* Stats;

	FVoxelPolygonizerBaseSettings() = default;
	FVoxelPolygonizerBaseSettings(FVoxelPolygonizerAsyncWorkBase* Work);
};

class FVoxelPolygonizerBase
{
public:
	const int32 LOD;
	// Step between cubes
	const int32 Step;
	const int32 Size;
	FVoxelData* const Data;
	TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe> const DebugManager;
	const FIntVector ChunkPosition;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const float UVScale;
	const FVoxelMeshProcessingParameters MeshParameters;

	FVoxelChunk& Chunk;
	FVoxelStatsElement& Stats;

	FVoxelPolygonizerBase(const FVoxelPolygonizerBaseSettings& Settings)
		: LOD(Settings.LOD)
		, Step(1 << Settings.LOD)
		, Size(CHUNK_SIZE << Settings.LOD)
		, Data(Settings.Data)
		, DebugManager(Settings.DebugManager)
		, ChunkPosition(Settings.ChunkPosition)
		, NormalConfig(Settings.NormalConfig)
		, MaterialConfig(Settings.MaterialConfig)
		, UVConfig(Settings.UVConfig)
		, UVScale(Settings.UVScale)
		, MeshParameters(Settings.MeshParameters)
		, Chunk(*Settings.Chunk)
		, Stats(*Settings.Stats)
	{
	}
	virtual ~FVoxelPolygonizerBase() = default;

	virtual bool Create() = 0;
};

using FVoxelPolygonizerSettings = FVoxelPolygonizerBaseSettings;

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

struct FVoxelTransitionsPolygonizerSettings : public FVoxelPolygonizerBaseSettings
{
	uint8 TransitionsMask;

	FVoxelTransitionsPolygonizerSettings() = default;
	FVoxelTransitionsPolygonizerSettings(FVoxelTransitionsPolygonizerAsyncWork* Work);
};

class FVoxelTransitionsPolygonizer : public FVoxelPolygonizerBase
{
public:
	const uint8 TransitionsMask;
	const int32 HalfLOD;
	const int32 HalfStep;

	FVoxelTransitionsPolygonizer(const FVoxelTransitionsPolygonizerSettings& Settings)
		: FVoxelPolygonizerBase(Settings)
		, TransitionsMask(Settings.TransitionsMask)
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