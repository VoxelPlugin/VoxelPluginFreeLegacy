// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "VoxelWorldEditor.generated.h"

class UVoxelInvokerComponent;

UCLASS(notplaceable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXELEDITOR_API AVoxelWorldEditor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor();

	TWeakObjectPtr<UVoxelInvokerComponent> GetInvoker();

	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const override;
#endif

	TWeakObjectPtr<AVoxelWorld> World;

private:
	UPROPERTY()
		UVoxelInvokerComponent* Invoker;

};
