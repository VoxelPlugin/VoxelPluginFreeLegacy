// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorldEditor.generated.h"

class AVoxelWorld;
class UVoxelInvokerEditorComponent;

UCLASS(notplaceable)
class VOXELEDITOR_API AVoxelWorldEditor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor();

	//~ Begin AActor interface
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const final { return true; }
	virtual bool IsEditorOnly() const final { return true; }
#endif
	//~ End AActor interface

private:
	UPROPERTY()
	UVoxelInvokerEditorComponent* Invoker;
};

UCLASS(notplaceable)
class VOXELEDITOR_API UVoxelInvokerEditorComponent : public UVoxelInvokerComponent
{
	GENERATED_BODY()

public:
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	//~ End UActorComponent Interface
};