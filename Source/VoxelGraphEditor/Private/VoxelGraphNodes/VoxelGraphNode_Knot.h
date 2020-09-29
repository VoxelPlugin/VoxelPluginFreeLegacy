// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphNode.h"
#include "VoxelGraphNode_Knot.generated.h"

UCLASS()
class UVoxelGraphNode_Knot : public UVoxelGraphNode
{
	GENERATED_BODY()

public:
	// UEdGraphNode interface

	virtual void CreateInputPins() override;
	virtual void CreateOutputPins() override;
	
	virtual void ReconstructNode() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldOverridePinNames() const override;
	virtual FText GetPinNameOverride(const UEdGraphPin& Pin) const override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual bool CanSplitPin(const UEdGraphPin* Pin) const override;
	virtual bool IsCompilerRelevant() const override { return false; }
	virtual UEdGraphPin* GetPassThroughPin(const UEdGraphPin* FromPin) const override;	
	virtual bool ShouldDrawNodeAsControlPointOnly(int32& OutInputPinIndex, int32& OutOutputPinIndex) const override { OutInputPinIndex = 0;  OutOutputPinIndex = 1; return true; }
	// End of UEdGraphNode interface

	virtual bool IsCompact() const override { return true; }

	UEdGraphPin* GetInputPin() const
	{
		return Pins[0];
	}

	UEdGraphPin* GetOutputPin() const
	{
		return Pins[1];
	}

	void PropagatePinType();

	TArray<UEdGraphPin*> GetAllInputPins();
	TArray<UEdGraphPin*> GetAllOutputPins();

private:
	void PropagatePinTypeFromInput();
	void PropagatePinTypeFromOutput();

	/** Recursion guard boolean to prevent PropagatePinType from infinitely recursing if you manage to create a loop of knots */
	bool bRecursionGuard;
};