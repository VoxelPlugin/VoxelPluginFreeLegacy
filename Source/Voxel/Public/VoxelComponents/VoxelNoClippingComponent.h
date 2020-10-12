// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Async/Future.h"
#include "Components/SceneComponent.h"
#include "VoxelNoClippingComponent.generated.h"

class FVoxelData;
class AVoxelWorld;
class UCharacterMovementComponent;
struct FVoxelVector;

// Add this to your player to prevent it from falling through the voxel world when digging under its feet
//
// On tick, will check if the player is inside the voxel world surface
// If it is, it will search for the nearest safe position
// - if found: teleport there, resetting its velocity. OnTeleported will be fired.
// - if not: the MoveTowardsSurface event will be fired every tick, ignoring TickRate. Bind this and eg move the player upwards.
//   When the player will be safe again, StopMovingTowardsSurface will be fired, eg to reset the velocity.
//
// If you set EnableDefaultBehavior to true, you don't need to bind any of the event, Characters will be automatically handled
UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelNoClippingComponent : public USceneComponent
{
	GENERATED_BODY()
		
public:
	UVoxelNoClippingComponent();

public:
	// Delay in seconds between checks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Config")
	float TickRate = 0.1f;

	// How far in voxels to search for an empty voxel
	// Keep low!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Config", meta = (UIMin = 1, UIMax = 32))
	int32 SearchRange = 5;

public:
	// If true, you don't need to implement any of the events, just set the settings below
	// If the owner is a character, it will be automatically detected & the correct behavior will be applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Default Behavior")
	bool bEnableDefaultBehavior = true;

	// Speed in unreal units per second at which to move towards the surface
	// If you set this too high, we might overshot!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Default Behavior", meta = (EditCondition = "bEnableDefaultBehavior"))
	float Speed = 6000;

	// If true, will move away from a point instead of upwards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Default Behavior", meta = (EditCondition = "bEnableDefaultBehavior"))
	bool bIsPlanet = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Default Behavior", meta = (EditCondition = "bEnableDefaultBehavior && bIsPlanet"))
	FVector PlanetCenter = FVector::ZeroVector;
	
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMoveTowardsSurface);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopMovingTowardsSurface);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeleported);
	
	// Will be called when we're clipping, but no safe position can be found
	// You can bind this to eg move the player upward
	// When the player will be safe again, StopMovingTowardsSurface will be fired
	UPROPERTY(BlueprintAssignable)
	FOnMoveTowardsSurface MoveTowardsSurface;

	// Will be fired once we're safe again
	UPROPERTY(BlueprintAssignable)
	FOnStopMovingTowardsSurface StopMovingTowardsSurface;

	// Called when we teleported to a safe location. You usually want to clear the velocity in there to avoid issues
	UPROPERTY(BlueprintAssignable)
	FOnTeleported OnTeleported;

public:
	// Implement this to select which voxel worlds to consider
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel")
	bool ShouldUseVoxelWorld(AVoxelWorld* VoxelWorld);

	virtual bool ShouldUseVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
	{
		// Use all voxel worlds by default
		return true;
	}

public:
	// True if we are currently inside the voxel world surface
	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	bool bIsInsideSurface = false;

protected:
	//~ Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

private:
	double LastTickTime = 0;

	struct FAsyncResult
	{
		bool bInsideSurface = false;
		TOptional<FIntVector> ClosestSafeLocation;
	};
	TFuture<TArray<FAsyncResult>> AsyncResult;
	TArray<TWeakObjectPtr<AVoxelWorld>> PendingVoxelWorlds;

	void StartAsyncTask();

	void BroadcastMoveTowardsSurface() const;
	void BroadcastStopMovingTowardsSurface() const;
	void BroadcastOnTeleported() const;

	UCharacterMovementComponent* GetCharacterMovement() const;

	static void ResetVelocity(UCharacterMovementComponent& CharacterMovement);
	static FAsyncResult AsyncTask(const FVoxelData& Data, const FVoxelVector& ComponentLocation, int32 SearchRange);
};