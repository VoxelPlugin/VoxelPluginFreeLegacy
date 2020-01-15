// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorldEditor.generated.h"

class AVoxelWorld;
class UVoxelInvokerEditorComponent;

UCLASS(NotPlaceable)
class VOXELEDITOR_API AVoxelWorldEditor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldEditor();

	//~ Begin AActor interface
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override final { return true; }
	virtual bool IsEditorOnly() const override final { return true; }
#endif
	//~ End AActor interface

	UPROPERTY(Transient)
	bool bOverrideLocation = false;
	UPROPERTY(Transient)
	FVector LocationOverride;

private:
	UPROPERTY()
	UVoxelInvokerEditorComponent* Invoker;
};

UCLASS(NotPlaceable)
class VOXELEDITOR_API UVoxelInvokerEditorComponent : public UVoxelInvokerComponent
{
	GENERATED_BODY()

public:
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	//~ End UActorComponent Interface
};