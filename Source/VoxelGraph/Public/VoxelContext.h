// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelRange.h"
#include "IntBox.h"
#include "VoxelItemStack.h"

struct VOXELGRAPH_API FVoxelContext
{
	const int32 LOD;
	const FVoxelItemStack Items;
	const FTransform LocalToWorld;
	const bool bHasCustomTransform;

	static const FVoxelContext EmptyContext;

	FVoxelContext(
		int32 LOD,
		const FVoxelItemStack& Items,
		const FTransform& LocalToWorld,
		bool bHasCustomTransform,
		v_flt InWorldX = 0,
		v_flt InWorldY = 0,
		v_flt InWorldZ = 0)
		: LOD(LOD)
		, Items(Items)
		, LocalToWorld(LocalToWorld)
		, bHasCustomTransform(bHasCustomTransform)
	{
		WorldX = InWorldX;
		WorldY = InWorldY;
		WorldZ = InWorldZ;
		UpdateCoordinates();
	}

	FORCEINLINE v_flt GetWorldX() const { return WorldX; }
	FORCEINLINE v_flt GetWorldY() const { return WorldY; }
	FORCEINLINE v_flt GetWorldZ() const { return WorldZ; }

	FORCEINLINE v_flt GetLocalX() const { return LocalX; }
	FORCEINLINE v_flt GetLocalY() const { return LocalY; }
	FORCEINLINE v_flt GetLocalZ() const { return LocalZ; }

	FORCEINLINE void SetWorldX(v_flt NewX)
	{
		WorldX = NewX;
		UpdateCoordinates();
	}
	FORCEINLINE void SetWorldY(v_flt NewY)
	{
		WorldY = NewY;
		UpdateCoordinates();
	}
	FORCEINLINE void SetWorldZ(v_flt NewZ)
	{
		WorldZ = NewZ;
		UpdateCoordinates();
	}
	
private:
	v_flt WorldX;
	v_flt WorldY;
	v_flt WorldZ;
	v_flt LocalX;
	v_flt LocalY;
	v_flt LocalZ;
	
	FORCEINLINE void UpdateCoordinates()
	{
		if (bHasCustomTransform)
		{
			const FVector Local = LocalToWorld.InverseTransformPosition(FVector(WorldX, WorldY, WorldZ));
			LocalX = Local.X;
			LocalY = Local.Y;
			LocalZ = Local.Z;
		}
		else
		{
			LocalX = WorldX;
			LocalY = WorldY;
			LocalZ = WorldZ;
		}
	}
};

struct VOXELGRAPH_API FVoxelContextRange
{
	const int32 LOD;
	const FVoxelItemStack Items;
	const FTransform LocalToWorld;
	const bool bHasCustomTransform;
	const FIntBox WorldBounds;
	const FIntBox LocalBounds;

	static const FVoxelContextRange EmptyContext;

	FVoxelContextRange(
		int32 LOD,
		const FVoxelItemStack& Items,
		const FTransform& LocalToWorld,
		bool bHasCustomTransform,
		const FIntBox& WorldBounds)
		: LOD(LOD)
		, Items(Items)
		, LocalToWorld(LocalToWorld)
		, bHasCustomTransform(bHasCustomTransform)
		, WorldBounds(WorldBounds)
		, LocalBounds(bHasCustomTransform ? WorldBounds.ApplyTransform<EInverseTransform::True>(LocalToWorld, 1 << LOD) : WorldBounds)
	{
	}

	FORCEINLINE TVoxelRange<v_flt> GetWorldX() const { return { v_flt(WorldBounds.Min.X), v_flt(WorldBounds.Max.X) }; }
	FORCEINLINE TVoxelRange<v_flt> GetWorldY() const { return { v_flt(WorldBounds.Min.Y), v_flt(WorldBounds.Max.Y) }; }
	FORCEINLINE TVoxelRange<v_flt> GetWorldZ() const { return { v_flt(WorldBounds.Min.Z), v_flt(WorldBounds.Max.Z) }; }

	FORCEINLINE TVoxelRange<v_flt> GetLocalX() const { return { v_flt(LocalBounds.Min.X), v_flt(LocalBounds.Max.X) }; }
	FORCEINLINE TVoxelRange<v_flt> GetLocalY() const { return { v_flt(LocalBounds.Min.Y), v_flt(LocalBounds.Max.Y) }; }
	FORCEINLINE TVoxelRange<v_flt> GetLocalZ() const { return { v_flt(LocalBounds.Min.Z), v_flt(LocalBounds.Max.Z) }; }
};