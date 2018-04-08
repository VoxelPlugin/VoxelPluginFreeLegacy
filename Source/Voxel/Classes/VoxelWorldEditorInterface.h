// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldEditorInterface.generated.h"

class AVoxelWorld;
class UVoxelInvokerComponent;

/**
 * Interface to use the VoxelWorldEditor in the VoxelEditor module
 */
UCLASS(notplaceable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API AVoxelWorldEditorInterface : public AActor
{
	GENERATED_BODY()

public:
	virtual TWeakObjectPtr<UVoxelInvokerComponent> GetInvoker()
	{
		return TWeakObjectPtr<UVoxelInvokerComponent>();
	}

	virtual void Init(TWeakObjectPtr<AVoxelWorld> NewWorld)
	{
		
	}
};