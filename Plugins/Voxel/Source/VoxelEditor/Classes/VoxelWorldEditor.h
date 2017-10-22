// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldEditorInterface.h"
#include "VoxelWorldEditor.generated.h"

class AVoxelWorld;
class UVoxelInvokerComponent;

UCLASS(notplaceable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXELEDITOR_API AVoxelWorldEditor : public AVoxelWorldEditorInterface
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor();

	TWeakObjectPtr<UVoxelInvokerComponent> GetInvoker() override;
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
