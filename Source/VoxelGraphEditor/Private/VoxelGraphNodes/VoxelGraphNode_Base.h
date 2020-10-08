// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "VoxelGraphNode_Base.generated.h"

class UEdGraphPin;
class UEdGraphSchema;

UCLASS()
class UVoxelGraphNode_Base : public UVoxelGraphNodeInterface
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelGraphNode_Base Interface
	virtual void CreateInputPins() {}
	virtual void CreateOutputPins() {}
	virtual void RestoreVectorPins(const TArray<UEdGraphPin*>& OldInputPins, const TArray<UEdGraphPin*>& OldOutputPins) {}

	virtual bool IsCompact() const { return false; }
	virtual FLinearColor GetNodeBodyColor() const { return FLinearColor::White; }

	/** Add an input pin to this node and recompile the generator */
	virtual void AddInputPin() {}
	/** Checks whether an input can be added to this node */
	virtual bool CanAddInputPin() const { return false; }
	//~ End UVoxelGraphNode_Base Interface

	TArray<UEdGraphPin*> GetInputPins() const;
	TArray<UEdGraphPin*> GetOutputPins() const;

	UEdGraphPin* GetInputPin(int32 InputIndex);
	UEdGraphPin* GetOutputPin(int32 OutputIndex);

	int32 GetInputPinIndex(const UEdGraphPin* Pin) const;
	int32 GetOutputPinIndex(const UEdGraphPin* Pin) const;

	int32 GetInputCount() const;
	int32 GetOutputCount() const;

	/**
	 * Handles inserting the node between the FromPin and what the FromPin was original connected to
	 *
	 * @param FromPin			The pin this node is being spawned from
	 * @param NewLinkPin		The new pin the FromPin will connect to
	 * @param OutNodeList		Any nodes that are modified will get added to this list for notification purposes
	 */
	void InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList);

	//~ Begin UEdGraphNode Interface.
	virtual void AllocateDefaultPins() final override;
	virtual void ReconstructNode() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override;
	//~ End UEdGraphNode Interface.
};