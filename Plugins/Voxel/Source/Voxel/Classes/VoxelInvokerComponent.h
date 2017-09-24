// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelInvokerComponent.generated.h"

class AVoxelWorld;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelInvokerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelInvokerComponent();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bNeedUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DistanceOffset;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
