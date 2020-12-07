// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphOutputs.h"
#include "VoxelGraphOutputsConfig.generated.h"

UCLASS(CollapseCategories)
class VOXELGRAPH_API UVoxelGraphOutputsConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelGraphOutput> Outputs;

public:
#if WITH_EDITOR
	FSimpleMulticastDelegate OnPropertyChanged;
#endif

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	//~ End UObject Interface
};