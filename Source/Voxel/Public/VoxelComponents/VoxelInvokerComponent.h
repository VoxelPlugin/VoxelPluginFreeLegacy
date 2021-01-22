// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Volume.h"
#include "VoxelInvokerSettings.h"
#include "VoxelInvokerComponent.generated.h"

class AVoxelWorld;

// Voxel Invokers are used to configure the voxel world LOD, collisions and navmesh
UCLASS(Abstract, Blueprintable, ClassGroup = Voxel)
class VOXEL_API UVoxelInvokerComponentBase : public USceneComponent
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker")
	bool bIsInvokerEnabled = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker", AdvancedDisplay)
	bool bEditorOnlyInvoker = false;
	
	// Whether or not this invoker will be used to trigger voxel events
	// Example of voxel events include:
	// - foliage spawning
	// - foliage collisions
	// - manually bound events
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Events")
	bool bUseForEvents = true;

	// Whether to use to compute the tasks priorities
	// If true, the task priorities will be higher if they are closer to this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Priority")
	bool bUseForPriorities = true;

public:
	// Is this invoker local? If false, bUseForLOD will always be considered as false
	// Useful for multiplayer, to only compute the LOD for the local player
	// Defaults to Cast<APawn>(GetOwner())->IsLocallyControlled()
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel|Invoker")
	bool IsLocalInvoker() const;
	
	// Lets you specify on which voxel worlds to use this invoker
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel|Invoker")
	bool ShouldUseInvoker(AVoxelWorld* VoxelWorld) const;
	bool ShouldUseInvoker(const AVoxelWorld* VoxelWorld) const;

	// Used to detect if the invoker has moved
	// Also used for events
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel|Invoker")
	FIntVector GetInvokerVoxelPosition(AVoxelWorld* VoxelWorld) const;
	FIntVector GetInvokerVoxelPosition(const AVoxelWorld* VoxelWorld) const;

	// Get the invoker settings
	// All the bounds are in voxel space
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel|Invoker")
	FVoxelInvokerSettings GetInvokerSettings(AVoxelWorld* VoxelWorld) const;
	FVoxelInvokerSettings GetInvokerSettings(const AVoxelWorld* VoxelWorld) const;

public:
	//~ Begin UVoxelInvokerComponentBase Interface
	virtual bool IsLocalInvoker_Implementation() const;
	virtual bool ShouldUseInvoker_Implementation(AVoxelWorld* VoxelWorld) const;
	virtual FIntVector GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const;
	virtual FVoxelInvokerSettings GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const;
	//~ End UVoxelInvokerComponentBase Interface

protected:
	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//~ End UActorComponent Interface
	//
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Invoker")
	static void RefreshAllVoxelInvokers();
	
	static TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> GetInvokers(const AVoxelWorld* VoxelWorld);
	static FSimpleMulticastDelegate OnForceRefreshInvokers;

private:
	static TMap<TWeakObjectPtr<UWorld>, TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>>> Components;
};

// Voxel Invokers are used to configure the voxel world LOD, collisions and navmesh
// Simple position based invoker
UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelSimpleInvokerComponent : public UVoxelInvokerComponentBase
{
	GENERATED_BODY()
		
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|LOD")
	bool bUseForLOD = true;

	// You should leave this to 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|LOD", meta = (DisplayName = "LOD to Set", EditCondition = bUseForLOD, ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26))
	int32 LODToSet = 0;

	// In cm. Will set LODToSet around the invoker on this distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|LOD", meta = (DisplayName = "LOD Range", EditCondition = bUseForLOD, ClampMin = 0))
	float LODRange = 1000;

	// Will enable high res collisions around the invoker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Collisions")
	bool bUseForCollisions = true;
	
	// In cm. Will enable high res collisions on chunks under this distance from this invoker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Collisions", meta = (EditCondition = bUseForCollisions, ClampMin = 0))
	float CollisionsRange = 1000;

	// Will enable high res navmesh around the invoker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Navmesh")
	bool bUseForNavmesh = true;
	
	// In cm. Will enable high res navmesh on chunks under this distance from this invoker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Navmesh", meta = (EditCondition = bUseForNavmesh, ClampMin = 0))
	float NavmeshRange = 1000;

public:
	// VoxelSimpleInvokerComponent's GetInvokerVoxelPosition and GetInvokerSettings functions are calling GetInvokerGlobalPosition to find the global position of the invoker
	// Defaults to GetComponentPosition
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel|Invoker")
	FVector GetInvokerGlobalPosition() const;

public:
	//~ Begin UVoxelInvokerComponentBase Interface
	virtual FIntVector GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const override;
	virtual FVoxelInvokerSettings GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const override;
	//~ End UVoxelInvokerComponentBase Interface

protected:
	//~ Begin UVoxelSimpleInvokerComponent Interface
	virtual FVector GetInvokerGlobalPosition_Implementation() const;
	//~ End UVoxelSimpleInvokerComponent Interface
};

// Voxel Invokers are used to configure the voxel world LOD, collisions and navmesh
// Same as simple invoker, but optionally use the velocity to predict the position
UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerWithPredictionComponent : public UVoxelSimpleInvokerComponent
{
	GENERATED_BODY()
		
public:
	// Will use the speed of the owner to determine the position to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Prediction")
	bool bEnablePrediction = false;

	// Will multiply the velocity by this to get the new position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Prediction", meta = (EditCondition = bEnablePrediction, ClampMin = 0))
	float PredictionTime = 1;

protected:
	//~ Begin UVoxelSimpleInvokerComponent Interface
	virtual FVector GetInvokerGlobalPosition_Implementation() const override;
	//~ End UVoxelSimpleInvokerComponent Interface
};

// Voxel Invokers are used to configure the voxel world LOD, collisions and navmesh
// Will find the camera and use it to set its position
// This is probably NOT what you want: use a UVoxelSimpleInvoker instead!
UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerAutoCameraComponent : public UVoxelSimpleInvokerComponent
{
	GENERATED_BODY()

protected:
	//~ Begin UVoxelSimpleInvokerComponent Interface
	virtual FVector GetInvokerGlobalPosition_Implementation() const override;
	//~ End UVoxelSimpleInvokerComponent Interface
};

// Add this to a volume to set the LOD of the voxels in a volume, or always enable collision/navmesh in a volume
UCLASS(ClassGroup = Voxel, meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelVolumeInvokerComponent : public UVoxelInvokerComponentBase
{
	GENERATED_BODY()

public:
	UVoxelVolumeInvokerComponent();

	// Will set the LOD in the volume to a fixed value
	// Note that the displayed LOD might have a higher resolution than this if another invoker is close
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Volume", DisplayName = "Use for LOD")
	bool bUseForLOD = true;
	
	// Will set the LOD in the volume to a fixed value
	// Note that the displayed LOD might have a higher resolution than this if another invoker is close
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Volume", DisplayName = "LOD to Set", meta = (EditCondition = bUseForLOD))
	int32 LODToSet = 0;

	// Will compute high res collision in the volume
	// Note that collisions might still be computed by the voxel world even if this is false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Volume")
	bool bUseForCollisions = false;
	
	// Will compute high res navmesh in the volume
	// Note that navmesh might still be computed by the voxel world even if this is false
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Invoker|Volume")
	bool bUseForNavmesh = false;

protected:
	//~ Begin UVoxelInvokerComponentBase Interface
	virtual bool IsLocalInvoker_Implementation() const override;
	virtual FIntVector GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const override;
	virtual FVoxelInvokerSettings GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const override;
	//~ End UVoxelInvokerComponentBase Interface
};

// Volume with a voxel invoker
// Sets the LOD of the voxels in a volume, or always enable collision/navmesh in a volume
UCLASS(Blueprintable, hidecategories=(Advanced, Attachment, Collision, Volume), DisplayName = "Voxel LOD Volume")
class VOXEL_API AVoxelLODVolume : public AVolume
{
	GENERATED_BODY()

public:
	AVoxelLODVolume();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UVoxelVolumeInvokerComponent* InvokerComponent;

protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface
};