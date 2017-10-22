// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldEditorInterface.generated.h"

class AVoxelWorld;
class UVoxelInvokerComponent;

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