// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "VoxelWorldEditor.generated.h"

class AVoxelWorld;
class UVoxelInvokerComponent;

UCLASS(notplaceable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXELEDITOR_API AVoxelWorldEditor : public AVoxelWorldEditorInterface
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor();

	void Init(TWeakObjectPtr<AVoxelWorld> NewWorld) override;

	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const override;
#endif

private:
	UPROPERTY()
	UVoxelInvokerComponent* Invoker;

	TWeakObjectPtr<AVoxelWorld> World;
};
