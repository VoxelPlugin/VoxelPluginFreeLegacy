// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "FastNoise.h"
#include "Misc/IQueuedWork.h"
#include "LatentActions.h"
#include "VoxelWorld.h"
#include "VoxelLatentActionHelpers.h"
#include "VoxelCraterTools.generated.h"

class AVoxelWorld;
class FVoxelData;

/**
 * Async task to add a crater
 */
class FVoxelAddCraterAsyncWork : public FVoxelLatentActionAsyncWork
{
public:
	// In
	FIntVector const Position;
	float const Radius;
	FastNoise const Noise;
	float const SampleRate;
	// Out
	FIntBox Bounds;
	TArray<FIntVector> Positions;

	FVoxelAddCraterAsyncWork(TWeakObjectPtr<AVoxelWorld> World, const FIntVector& Position, float Radius, float SampleRate)
		: FVoxelLatentActionAsyncWork(World)
		, Position(Position)
		, Radius(Radius)
		, SampleRate(SampleRate)
	{
	}

	virtual void DoWork() override;
};
/**
 * Crater blueprint tools for voxels
 */
UCLASS()
class VOXEL_API UVoxelCraterTools : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Add a crater
	 * @param	World				Voxel World
	 * @param	Position			Center of the crater
	 * @param	Radius				Radius of the crater in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void AddCrater(AVoxelWorld* World, FIntVector Position, float Radius, float PositionsSampleRate, TArray<FIntVector>& Positions);
	/**
	 * Add a crater in an async thread
	 * @param	World				Voxel World
	 * @param	Position			Center of the crater
	 * @param	Radius				Radius of the crater in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject"))
	static void AddCraterAsync(UObject* WorldContextObject, AVoxelWorld* World, FIntVector Position, float Radius, float PositionsSampleRate, TArray<FIntVector>& Positions, FLatentActionInfo LatentInfo);
};