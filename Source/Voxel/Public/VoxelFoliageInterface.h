// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelFoliageInterface.generated.h"

UCLASS(Abstract)
class VOXEL_API UVoxelFoliageBiomeBase : public UObject
{
	GENERATED_BODY()
};

UCLASS(Abstract)
class VOXEL_API UVoxelFoliageCollectionBase : public UObject
{
	GENERATED_BODY()
};

UCLASS(Abstract)
class VOXEL_API UVoxelFoliageInterfaceSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_ABSTRACT_VOXEL_SUBSYSTEM_PROXY_BODY(IVoxelFoliageInterface);
};

class IVoxelFoliageInterface : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelFoliageInterfaceSubsystemProxy);

	//~ Begin IVoxelSubsystem Interface
	virtual EVoxelSubsystemFlags GetFlags() const override { return EVoxelSubsystemFlags::RecreateFoliage; }
	//~ End IVoxelSubsystem Interface

	// Returns the biome index to output in the biome generator output
	virtual int32 RegisterBiome(UVoxelFoliageBiomeBase* Biome) = 0;

	virtual void Regenerate(const FVoxelIntBox& Bounds) = 0;
	virtual void MarkDirty(const FVoxelIntBox& Bounds) = 0;
	
#if WITH_EDITOR
	virtual bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const = 0;
#endif
};