// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "VoxelMinimal.h"
#include "VoxelPinCategory.h"
#include "VoxelGraphSchema.generated.h"

class UVoxelGraphMacro;
class UVoxelLocalVariableDeclaration;

USTRUCT()
struct FVoxelGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	/** Class of node we want to create */
	UPROPERTY()
	TObjectPtr<UClass> VoxelNodeClass = nullptr;
	
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

USTRUCT()
struct FVoxelGraphSchemaAction_NewMacroNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	UPROPERTY()
	TObjectPtr<UVoxelGraphMacro> Macro = nullptr;

	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

USTRUCT()
struct FVoxelGraphSchemaAction_NewLocalVariableDeclaration : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	UPROPERTY()
	FName DefaultName;
	
	UPROPERTY()
	EVoxelPinCategory PinCategory;
	
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

USTRUCT()
struct FVoxelGraphSchemaAction_NewLocalVariableUsage : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	UPROPERTY()
	TObjectPtr<UVoxelLocalVariableDeclaration> Declaration = nullptr;
	
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

USTRUCT()
struct FVoxelGraphSchemaAction_NewSetterNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	UPROPERTY()
	uint32 Index = 0;
	
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

USTRUCT()
struct FVoxelGraphSchemaAction_NewKnotNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

/** Action to create new comment */
USTRUCT()
struct FVoxelGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};

/** Action to paste clipboard contents into the graph */
USTRUCT()
struct FVoxelGraphSchemaAction_Paste : public FEdGraphSchemaAction
{
	GENERATED_BODY();

public:
	using FEdGraphSchemaAction::FEdGraphSchemaAction;
	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode = true) override;
};


UCLASS()
class VOXELGRAPHEDITOR_API UVoxelGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
	
public:
	bool ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const;

	/** Helper method to add items valid to the palette list */
	void GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const;

	//~ Begin UEdGraphSchema Interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual void TrySetDefaultValue(
		UEdGraphPin& Pin,
		const FString& NewDefaultValue, 
		bool bMarkAsModified
	) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const UE_506_SWITCH(FVector2D, FVector2f)& GraphPosition) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	virtual int32 GetNodeSelectionCount(const UEdGraph* Graph) const override;
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual void DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const UE_506_SWITCH(FVector2D, FVector2f)& GraphPosition, UEdGraph* Graph) const override;
	virtual void GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const override;
	//~ End UEdGraphSchema Interface

private:
	/** Adds actions for creating every type of VoxelNode */
	void GetAllVoxelNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = nullptr) const;
	/** Adds action for creating a comment */
	void GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph = nullptr) const;

private:
	static void InitVoxelNodeClasses();
	
	static TArray<UClass*> VoxelNodeClasses;
	static bool bVoxelNodeClassesInitialized;
};