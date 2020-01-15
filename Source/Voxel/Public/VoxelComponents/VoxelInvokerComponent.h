// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VoxelInvokerComponent.generated.h"

class AVoxelWorld;
class APlayerCameraManager;

/**
 * Component to set the voxels LOD and collisions
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Will use the speed of the owner to determine the position to use
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bEnablePrediction = false;

	// Will multiply the velocity by this to get the new position
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (EditCondition = bEnablePrediction, ClampMin = 0))
	float PredictionTime = 1;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bUseForLODs = true;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bUseForCollisions = true;
	// In cm
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (EditCondition = bUseForCollisions, ClampMin = 0))
	float CollisionsRange = 1000;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bUseForNavmesh = true;
	// In cm
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (EditCondition = bUseForNavmesh, ClampMin = 0))
	float NavmeshRange = 1000;

public:
	bool IsLocalInvoker() const;

	virtual FVector GetPosition() const;

public:
	static const TArray<TWeakObjectPtr<UVoxelInvokerComponent>>& GetInvokers(UWorld* World);

protected:
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	//~ End UActorComponent Interface

private:
	static TMap<UWorld*, TArray<TWeakObjectPtr<UVoxelInvokerComponent>>> Components;

	void RegisterInvoker();
	void UnregisterInvoker();
};

// Will find the camera and use it to set its position
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerAutoCameraComponent : public UVoxelInvokerComponent
{
	GENERATED_BODY()

public:
	UVoxelInvokerAutoCameraComponent();
	
	virtual FVector GetPosition() const override;
};