// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Containers/Queue.h"

class UVoxelProceduralMeshComponent;

// We don't want to have every component ticking
// Will be deleted on the game thread when pinned
class IVoxelProceduralMeshComponent_PhysicsCallbackHandler : public TVoxelSharedFromThis<IVoxelProceduralMeshComponent_PhysicsCallbackHandler>
{
public:
	void TickHandler();

private:
	struct FCallback
	{
		uint64 CookerId;
		TWeakObjectPtr<UVoxelProceduralMeshComponent> Component;
	};
	TQueue<FCallback, EQueueMode::Mpsc> Queue;

public:
	void CookerCallback(uint64 CookerId, TWeakObjectPtr<UVoxelProceduralMeshComponent> Component);
};