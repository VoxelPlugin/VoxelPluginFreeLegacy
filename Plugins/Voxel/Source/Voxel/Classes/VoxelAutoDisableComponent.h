// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelAutoDisableComponent.generated.h"

class AVoxelWorld;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VOXEL_API UVoxelAutoDisableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoxelAutoDisableComponent();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UPrimitiveComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AVoxelWorld* World;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAutoFindWorld;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20"))
		int CullDepth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bSimulatePhysics;

	bool bAutoFindComponent;

	UPrimitiveComponent* Component;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
