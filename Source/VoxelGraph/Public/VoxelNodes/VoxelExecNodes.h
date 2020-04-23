// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelNodeStructs.h"
#include "VoxelGraphOutputs.h"
#include "VoxelGraphConstants.h"
#include "VoxelExecNodes.generated.h"

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_MaterialSetter : public UVoxelSetterNode
{
	GENERATED_BODY()

public:
	virtual int32 GetOutputIndex() const override;
};

// Set the color at that position. Inputs between 0 and 1
UCLASS(DisplayName = "Set Color")
class VOXELGRAPH_API UVoxelNode_SetColor : public UVoxelNode_MaterialSetter
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SetColor();
};

// Set the material index at that position. Input clamped between 0 and 255.
// DataA/B/C can be used in the material using the GetSingleIndexData material function
// DataA/B/C are between 0 and 1
UCLASS(DisplayName = "Set Single Index")
class VOXELGRAPH_API UVoxelNode_SetSingleIndex : public UVoxelNode_MaterialSetter
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SetSingleIndex();
};

// Set double index
// Data can be used in the material using the GetDoubleIndexData material function
// Data is between 0 and 1
UCLASS(DisplayName = "Set Double Index")
class VOXELGRAPH_API UVoxelNode_SetDoubleIndex : public UVoxelNode_MaterialSetter
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SetDoubleIndex();
};

// Set the material additional UVs channels
// By default the plugin has 2 UV channels that can be queried using TexCoord[1] and TexCoord[2] in the material
// Values should be between 0.f and 1.f
// Index should be 0 or 1 (or 2/3 if you enabled them in VoxelUserGlobals.h) 
UCLASS(DisplayName = "Set UV Channel")
class VOXELGRAPH_API UVoxelNode_SetUVs : public UVoxelNode_MaterialSetter
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SetUVs();
};

UCLASS(DisplayName = "Set Node", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_SetNode : public UVoxelSetterNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UVoxelNode_SetNode();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;

	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FName GetInputPinName(int32 PinIndex) const override;

	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface
	
	//~ Begin UVoxelSetterNode Interface
	virtual int32 GetOutputIndex() const override;
	//~ End UVoxelSetterNode Interface
	
#if WITH_EDITOR
public:
	// Returns: if valid
	bool UpdateSetterNode();
	void SetIndex(uint32 NewIndex);

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
#endif

protected:
	UPROPERTY()
	uint32 Index;

	UPROPERTY()
	FVoxelGraphOutput CachedOutput;
};

UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelNode_SetValueNode : public UVoxelNode_SetNode
{
	GENERATED_BODY()

public:
	UVoxelNode_SetValueNode()
	{
		Index = FVoxelGraphOutputsIndices::ValueIndex;
	}
};

// Break the graph into multiple functions
UCLASS(DisplayName = "Function Separator", Category = "Flow Control")
class VOXELGRAPH_API UVoxelNode_FunctionSeparator : public UVoxelNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FVoxelNamedDataPin> ArgTypes = { {EVoxelDataPinCategory::Float, "Value"} };

	UVoxelNode_FunctionSeparator() = default;

	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetColor() const override;

	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;

	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;

	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;

	virtual int32 GetOutputPinsCount() const override;

	//~ End UVoxelNode Interface

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// Merges exec flow
UCLASS(DisplayName = "Flow Merge", Category = "Flow Control")
class VOXELGRAPH_API UVoxelNode_FlowMerge : public UVoxelNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FVoxelNamedDataPin> Types = { {EVoxelDataPinCategory::Float, "Value"} };

	//~ Begin UVoxelNode Interface
	virtual FLinearColor GetColor() const override;

	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;

	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;

	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;

	virtual int32 GetOutputPinsCount() const override;

	//~ End UVoxelNode Interface

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};