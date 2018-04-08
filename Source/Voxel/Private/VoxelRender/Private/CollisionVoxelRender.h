// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelRender.h"

class FQueuedThreadPool;
class FCollisionMeshHandler;

class FCollisionVoxelRender : public IVoxelRender
{

public:
	FCollisionVoxelRender(AVoxelWorld* World, AActor* ChunksOwner);
	virtual ~FCollisionVoxelRender() override;

	virtual void Tick(float DeltaTime) override;
	virtual void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker) override;
	virtual void UpdateBoxInternal(const FIntBox& Box) override;
	
private:
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;
	FQueuedThreadPool* const CollisionMeshHandlerThreadPool;
	TArray<TSharedPtr<FCollisionMeshHandler>> CollisionComponents;

	float TimeSinceUpdate;
	bool bNeedToEndCollisionsTasks;
};