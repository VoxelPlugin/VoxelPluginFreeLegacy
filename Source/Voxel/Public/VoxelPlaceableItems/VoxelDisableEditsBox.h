// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemActor.h"
#include "VoxelDisableEditsBox.generated.h"

class UBoxComponent;
class AVoxelWorld;

UCLASS()
class VOXEL_API AVoxelDisableEditsBox : public AVoxelPlaceableItemActor
{
	GENERATED_BODY()

public:
	AVoxelDisableEditsBox();


private:
	FIntBox GetBox(AVoxelWorld* World) const;

	UPROPERTY()
	UBoxComponent* Box;

#if WITH_EDITOR
protected:
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	bool ShouldTickIfViewportsOnly() const override { return true; }
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void PostEditMove(bool bFinished) override;

private:
	float VoxelSize = 0;
	FVector WorldLocation;

	void ClampTransform();
#endif
};