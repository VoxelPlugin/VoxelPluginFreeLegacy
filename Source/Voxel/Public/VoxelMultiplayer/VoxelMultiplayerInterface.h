// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMinimal.h"
#include "VoxelDiff.h"
#include "UObject/Object.h"
#include "VoxelMultiplayerInterface.generated.h"

struct FVoxelMaterial;
struct FVoxelCompressedWorldSaveImpl;
class IVoxelMultiplayerClient;
class IVoxelMultiplayerServer;

UCLASS(Abstract, BlueprintType)
class VOXEL_API UVoxelMultiplayerInterface : public UObject
{
	GENERATED_BODY()

public:
};

