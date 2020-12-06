// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSubsystem.h"
#include "VoxelDataSubsystem.generated.h"

class FVoxelData;

UCLASS()
class VOXEL_API UVoxelDataSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelDataSubsystem);
};

class FVoxelDataSubsystem : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelDataSubsystemProxy);

	TVoxelSharedPtr<FVoxelData> GetData() const { return Data; }

	//~ Begin IVoxelSubsystem Interface
	virtual void Create() override;
	virtual void PreDestructor() override;
	//~ End IVoxelSubsystem Interface

private:
	TVoxelSharedPtr<FVoxelData> Data;
};