// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphOutputs.h"
#include "VoxelSpawners/VoxelSpawnerOutputsConfig.h"
#include "VoxelGraphOutputsConfig.generated.h"

UCLASS(CollapseCategories)
class VOXELGRAPH_API UVoxelGraphOutputsConfig : public UVoxelSpawnerOutputsConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelGraphOutput> Outputs;
	
	//~ Begin UVoxelSpawnerOutputConfig Interface
	virtual TArray<FName> GetFloatOutputs() const override;
	//~ End UVoxelSpawnerOutputConfig Interface

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