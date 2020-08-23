// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelStaticWorld.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class VOXEL_API AVoxelStaticWorld : public AActor
{
	GENERATED_BODY()

public:
	AVoxelStaticWorld();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Voxel")
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<UStaticMeshComponent*> Meshes;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};