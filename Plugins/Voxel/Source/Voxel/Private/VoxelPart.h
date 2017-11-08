// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPart.generated.h"

class UVoxelProceduralMeshComponent;
class UVoxelAutoDisableComponent;
class FVoxelData;
class AVoxelWorld;


UCLASS()
class AVoxelPart : public AActor
{
	GENERATED_BODY()

public:
	AVoxelPart();

	void Init(FVoxelData* Data, UMaterialInterface* Material, AVoxelWorld* WorldForAutoDisable = nullptr);

private:
	UPROPERTY(EditAnywhere)
		UVoxelProceduralMeshComponent* PrimaryMesh;

	UPROPERTY(EditAnywhere)
		UVoxelAutoDisableComponent* AutoDisableComponent;

};
