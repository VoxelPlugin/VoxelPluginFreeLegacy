// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPart.generated.h"

class UProceduralMeshComponent;
class UVoxelAutoDisableComponent;
class FVoxelData;
class AVoxelWorld;


UCLASS()
class AVoxelPart : public AActor
{
	GENERATED_BODY()

public:
	AVoxelPart();

	void Init(FVoxelData* Data, AVoxelWorld* World);

private:
	UPROPERTY(EditAnywhere)
		UProceduralMeshComponent* PrimaryMesh;

	UPROPERTY(EditAnywhere)
		UVoxelAutoDisableComponent* AutoDisableComponent;

};
