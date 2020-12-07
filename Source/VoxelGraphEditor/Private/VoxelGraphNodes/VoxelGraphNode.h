// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelGraphNode.generated.h"

class UVoxelNode;

UCLASS()
class UVoxelGraphNode : public UVoxelGraphNode_Base
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UVoxelNode* VoxelNode;

	/** Set the VoxelNode this represents (also assigns this to the VoxelNode in Editor)*/
	void SetVoxelNode(UVoxelNode* InVoxelNode);
	/** Fix up the node's owner after being copied */
	void PostCopyNode();
	/** Create a new input pin for this node */
	void CreateInputPin();
	/** Create a new output pin for this node */
	void CreateOutputPin();
	/** Remove a specific input pin from this node and recompile the generator */
	void RemoveInputPin(UEdGraphPin* InGraphPin);

public:
	bool CanSplitPin_Voxel(const UEdGraphPin& Pin) const;
	bool CanCombinePin(const UEdGraphPin& Pin) const;

	bool TrySplitPin(UEdGraphPin& Pin, bool bOnlyCheck);
	bool TryCombinePin(UEdGraphPin& Pin, bool bOnlyCheck);

	void CombineAll();

	static bool HasVectorPin(UVoxelNode& Node, EEdGraphPinDirection Direction);

public:
	// UVoxelGraphNodeInterface interface
	virtual UVoxelNode* GetVoxelNode() const override { return VoxelNode; }
	virtual bool IsOutdated() const override;
	// End of UVoxelGraphNodeInterface interface
	
	// UVoxelGraphNode_Base interface
	virtual void CreateInputPins() override;
	virtual void CreateOutputPins() override;
	virtual void RestoreVectorPins(const TArray<UEdGraphPin*>& OldInputPins, const TArray<UEdGraphPin*>& OldOutputPins) override;
	virtual bool IsCompact() const override;
	virtual FLinearColor GetNodeBodyColor() const override;
	virtual void AddInputPin() override;
	virtual bool CanAddInputPin() const override;
	// End of UVoxelGraphNode_Base interface

	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual void PrepareForCopying() override;
	virtual FText GetTooltipText() const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual bool CanUserDeleteNode() const override;
	virtual bool CanDuplicateNode() const override;
	virtual bool CanJumpToDefinition() const override;
	virtual void JumpToDefinition() const override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	// End of UEdGraphNode interface

	// UObject interface
	virtual void PostLoad() override;
	virtual void PostEditImport() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	// End of UObject interface

private:
	/** Make sure the voxel node is owned by the generator */
	void ResetVoxelNodeOwner();
};