// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorldEditorControls.generated.h"

class AVoxelWorld;
class UVoxelInvokerEditorComponent;

UCLASS(NotPlaceable)
class VOXELEDITOR_API AVoxelWorldEditorControls : public AActor
{
	GENERATED_BODY()

public:
	AVoxelWorldEditorControls();

	//~ Begin AActor interface
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override final { return true; }
	virtual bool IsEditorOnly() const override final { return true; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End AActor interface

	UPROPERTY(Transient)
	bool bOverrideLocation = false;
	UPROPERTY(Transient)
	FVector LocationOverride;

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	UVoxelInvokerEditorComponent* Invoker;
};

UCLASS(NotPlaceable)
class VOXELEDITOR_API UVoxelInvokerEditorComponent : public UVoxelSimpleInvokerComponent
{
	GENERATED_BODY()

public:
	UVoxelInvokerEditorComponent();
};