// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelFoliageInterface.generated.h"

class UVoxelFoliageBiome;
struct FVoxelFoliageSaveImpl;
struct FVoxelFoliageTransforms;

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

	// Returns the biome index to output in the biome generator output
	virtual int32 RegisterBiome(UVoxelFoliageBiome* Biome) = 0;
	
	virtual void Regenerate(const FVoxelIntBox& Bounds) = 0;
	virtual void MarkDirty(const FVoxelIntBox& Bounds) = 0;
	
	virtual void SaveTo(FVoxelFoliageSaveImpl& Save) = 0;
	virtual void LoadFrom(const FVoxelFoliageSaveImpl& Save) = 0;

	virtual bool GetTransforms(const FGuid& FoliageGuid, TArray<TVoxelSharedPtr<FVoxelFoliageTransforms>>& OutTransforms) const = 0;
	
#if WITH_EDITOR
	virtual bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const = 0;
#endif
};