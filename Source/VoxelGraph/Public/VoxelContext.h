// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelRange.h"
#include "VoxelIntBox.h"
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
		bool bHasCustomTransform)
		: LOD(LOD)
		, Items(Items)
		, LocalToWorld(LocalToWorld)
		, bHasCustomTransform(bHasCustomTransform)
	{
	}

	FORCEINLINE v_flt GetWorldX() const { return WorldX; }
	FORCEINLINE v_flt GetWorldY() const { return WorldY; }
	FORCEINLINE v_flt GetWorldZ() const { return WorldZ; }

	FORCEINLINE v_flt GetLocalX() const { return LocalX; }
	FORCEINLINE v_flt GetLocalY() const { return LocalY; }
	FORCEINLINE v_flt GetLocalZ() const { return LocalZ; }

private:
	v_flt WorldX = 0;
	v_flt WorldY = 0;
	v_flt WorldZ = 0;
	
	v_flt LocalX = 0;
	v_flt LocalY = 0;
	v_flt LocalZ = 0;
	
	template<bool bCustomTransform>
	FORCEINLINE void UpdateCoordinates(v_flt NewWorldX, v_flt NewWorldY, v_flt NewWorldZ)
	{
		checkVoxelSlow(bCustomTransform == bHasCustomTransform);

		WorldX = NewWorldX;
		WorldY = NewWorldY;
		WorldZ = NewWorldZ;

		if (bCustomTransform)
		{
			const FVector Local = LocalToWorld.InverseTransformPosition(FVector(WorldX, WorldY, WorldZ));
			LocalX = Local.X;
			LocalY = Local.Y;
			LocalZ = Local.Z;
		}
		else
		{
			LocalX = NewWorldX;
			LocalY = NewWorldY;
			LocalZ = NewWorldZ;
		}
	}

	template<typename, typename>
	friend class TVoxelGraphGeneratorInstanceHelper;
	friend class FVoxelGraphPreview;
};

struct VOXELGRAPH_API FVoxelContextRange
{
	const int32 LOD;
	const FVoxelItemStack Items;
	const FTransform LocalToWorld;
	const bool bHasCustomTransform;
	const FVoxelIntBox WorldBounds;
	const FVoxelIntBox LocalBounds;

	static const FVoxelContextRange EmptyContext;

	FVoxelContextRange(
		int32 LOD,
		const FVoxelItemStack& Items,
		const FTransform& LocalToWorld,
		bool bHasCustomTransform,
		const FVoxelIntBox& WorldBounds)
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