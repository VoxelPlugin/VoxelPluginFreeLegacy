// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelDataItemActor.generated.h"

class AVoxelWorld;

UCLASS(Abstract)
class VOXEL_API AVoxelDataItemActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelDataItemActor();

public:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "AddItemToWorld")
	void K2_AddItemToWorld(AVoxelWorld* World);
	
	void K2_AddItemToWorld_Implementation(AVoxelWorld* World)
	{
		AddItemToWorld(World);
	}

	virtual void AddItemToWorld(AVoxelWorld* World) {}

	void CallAddItemToWorld(AVoxelWorld* World)
	{
		TGuardValue<bool> AllowScriptsInEditor(GAllowActorScriptExecutionInEditor, true);
		K2_AddItemToWorld(World);
	}

public:
	// If true, will automatically remove & add back the item to the voxel world when edited
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutomaticUpdates = true;

	// Delay in second to queue the refresh, to merge eventual duplicate queries together
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0))
	float RefreshDelay = 0.1f;

	DECLARE_EVENT(AVoxelDataAssetActor, FOnRefresh);
	FOnRefresh OnRefresh;

	UFUNCTION(BlueprintCallable, Category = "Voxel Data Item Actor")
	void ScheduleRefresh();

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif
	virtual void Destroyed() override;
	//~ End UObject Interface

private:
	FTimerHandle RefreshTimerHandle;
};