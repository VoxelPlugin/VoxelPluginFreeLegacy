// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRenderFactory.generated.h"

class AVoxelWorld;
class AActor;
class IVoxelRender;

UENUM()
enum class EVoxelRenderType : uint8
{
	WithLOD,
	Cubic
};


class VOXEL_API FVoxelRenderFactory
{
public:
	static TSharedPtr<IVoxelRender> GetVoxelRender(EVoxelRenderType VoxelRender, AVoxelWorld* World, AActor* ChunksOwner);
};