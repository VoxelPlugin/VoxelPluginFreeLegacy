// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorSamplerNodes.h"
#include "VoxelGeneratorMergeNode.generated.h"

class UVoxelGraphOutputsConfig;

UCLASS(DisplayName = "Generator Merge", Category = "Generator")
class VOXELGRAPH_API UVoxelNode_GeneratorMerge : public UVoxelNode_GeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:	
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelGraphOutputsConfig* Outputs;

	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelMaterialConfig MaterialConfig;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelGeneratorPicker> Generators;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Config")
	float Tolerance = 0.00001;

	UVoxelNode_GeneratorMerge();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	virtual int32 GetOutputPinsCount() const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface

	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Generators); }
};