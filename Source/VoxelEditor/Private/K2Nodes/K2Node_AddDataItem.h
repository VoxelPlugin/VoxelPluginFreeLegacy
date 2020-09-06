// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "K2Node.h"
#include "VoxelGraphDataItemConfig.h"
#include "K2Node_AddDataItem.generated.h"

UCLASS()
class VOXELEDITOR_API UK2Node_AddDataItem : public UK2Node
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	const UVoxelGraphDataItemConfig* DataItemConfig;

public:
	//~ Begin UEdGraphNode Interface
    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetTooltipText() const override;
    //~ End UEdGraphNode Interface

    //~ Begin K2Node Interface
    virtual FText GetMenuCategory() const override;
    virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    //~ End K2Node Interface

private:
	static FText GetTitle(const FString& AssetName);
	static FText GetTooltip(const FString& AssetName);

	static const FName PC_Generator;
	static const FName PC_Bounds;
	static const FName PC_Mask;
};