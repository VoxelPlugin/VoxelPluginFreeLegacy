// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPart.generated.h"


class UProceduralMeshComponent;
class VoxelData;
class AVoxelWorld;



UCLASS()
class AVoxelPart : public AActor
{
	GENERATED_BODY()

public:
	AVoxelPart();

	void Init(VoxelData* Data, AVoxelWorld* World);

private:
	UPROPERTY(EditAnywhere)
		UProceduralMeshComponent* PrimaryMesh;

};
