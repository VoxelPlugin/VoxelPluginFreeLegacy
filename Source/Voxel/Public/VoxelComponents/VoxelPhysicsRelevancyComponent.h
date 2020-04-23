// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "VoxelPhysicsRelevancyComponent.generated.h"

class UPrimitiveComponent;

/**
 * Disable physics on actors that are out of the Voxel World collision range
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent, Keywords = "voxel auto disable component"))
class VOXEL_API UVoxelPhysicsRelevancyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelPhysicsRelevancyComponent();

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = "0", ClampMax = "24", UIMin = "0", UIMax = "24"), DisplayName = "Max LOD For Physics")
	uint8 MaxVoxelChunksLODForPhysics = 2;

	// Delay to allow the voxel chunks collisions to be updated. In seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float TimeToWaitBeforeActivating = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float TickInterval = 0.1;

protected:
	//~ Begin UActorComponent Interface
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:
	TArray<UPrimitiveComponent*> PrimitiveComponentsWithPhysicsEnabled;
	bool bArePhysicsEnabled = true;
	FTimerHandle Handle;
	bool bWaitingToBeActivated = false;

	void ActivatePhysics();
};
