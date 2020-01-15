// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGeneratorSamplerNodes.h"
#include "VoxelWorldGeneratorMergeNode.generated.h"

class UVoxelGraphOutputsConfig;

UCLASS(DisplayName = "World Generator Merge", Category = "World Generator")
class VOXELGRAPH_API UVoxelNode_WorldGeneratorMerge : public UVoxelNode_WorldGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:	
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelGraphOutputsConfig* Outputs;

	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelMaterialConfig MaterialConfig;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	TArray<FVoxelWorldGeneratorPicker> WorldGenerators;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Config")
	float Tolerance = 0.00001;

	UVoxelNode_WorldGeneratorMerge();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	virtual int32 GetOutputPinsCount() const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};