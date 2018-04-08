// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IntBox.h"
#include "VoxelInvokerComponent.generated.h"

class AVoxelWorld;

/**
 * Component to set the voxels LOD and collisions
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Should this invoker generate collisions?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseForCollisions;

	// Should this invoker be used for voxels LOD?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseForLOD;

	// In world space. Chunks under this distance will have the highest resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseForLOD"))
	float DistanceOffset;

	// Will show a red point if local player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AdvancedDisplay))
	bool bDebugMultiplayer;


	UVoxelInvokerComponent();

	// Getters
	bool UseForCollisions();
	bool UseForRender();

	FVector GetPosition() const;
	FIntBox GetCameraBounds(const AVoxelWorld* World) const;

protected:
	//~ Begin UActorComponent Interface
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:
	TSet<class AVoxelWorld*> AlreadyProcessedWorlds;
};