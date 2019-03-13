// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelAutoDisableComponent.generated.h"

class AVoxelWorld;

/**
 * Disable physics on actors that are out of the Voxel World collision range
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelAutoDisableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelAutoDisableComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bAutoFindWorld = true;

	// Set automatically if bAutoFindWorld
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "!bAutoFindWorld", DisplayName = "World"))
	AVoxelWorld* DefaultWorld;

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (AdvancedDisplay, ClampMin = "0", ClampMax = "25", UIMin = "0", UIMax = "25"), DisplayName = "Max LOD For Collisions")
	uint8 MaxLODForPhysics = 0;

	// Delay to allow physics cooking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (AdvancedDisplay))
	float TimeToWaitBeforeActivating = 10;

protected:
	//~ Begin UActorComponent Interface
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	float TimeUntilActivation = 0;
	TArray<TWeakObjectPtr<AVoxelWorld>> InGameWorlds;
	bool bPhysicsEnabled = true;
};
