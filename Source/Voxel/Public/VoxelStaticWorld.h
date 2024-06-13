// Copyright Voxel Plugin SAS. All Rights Reserved.

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
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<TObjectPtr<UStaticMeshComponent>> Meshes;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};