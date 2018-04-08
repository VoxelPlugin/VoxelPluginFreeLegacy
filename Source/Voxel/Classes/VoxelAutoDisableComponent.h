// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelAutoDisableComponent.generated.h"

class AVoxelWorld;

/**
 * Disable physics on actors that are out of the Voxel World collision range
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelAutoDisableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelAutoDisableComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoFindWorld;

	// Set automatically if bAutoFindWorld
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AVoxelWorld* World;

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "19", UIMin = "0", UIMax = "19"), DisplayName = "Max LOD For Collisions")
	uint8 MaxLODForCollisions;

	// Delay to allow physics cooking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AdvancedDisplay))
	float TimeToWaitBeforeActivating;

protected:
	//~ Begin UActorComponent Interface
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:
	TArray<UPrimitiveComponent*> Components;
	float TimeUntilActivation;
};
