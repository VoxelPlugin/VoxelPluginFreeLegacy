// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPart.generated.h"

class UMaterialInterface;
class FVoxelData;
class UVoxelProceduralMeshComponent;
class UVoxelAutoDisableComponent;
class AVoxelWorld;
class UHierarchicalInstancedStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoxelPartSetVoxelWorld, AVoxelWorld*, World);

/**
 * Abstract class, does nothing
 */
UCLASS(Abstract)
class AVoxelPart : public AActor
{
	GENERATED_BODY()

public:
	virtual void Init(FVoxelData* Data, AVoxelWorld* World)
	{
		SetVoxelWorld.Broadcast(World);
	}
	
	UPROPERTY(BlueprintAssignable)
	FVoxelPartSetVoxelWorld SetVoxelWorld;
};

/**
 * Spawn a static procedural mesh
 */
UCLASS()
class AVoxelPartSimpleMesh : public AVoxelPart
{
	GENERATED_BODY()

public:
	AVoxelPartSimpleMesh();
	
	void Init(FVoxelData* Data, AVoxelWorld* World) override;

private:
	UPROPERTY(EditAnywhere)
	UVoxelAutoDisableComponent* AutoDisableComponent;
	UPROPERTY(EditAnywhere)
	UVoxelProceduralMeshComponent* Mesh;
};
