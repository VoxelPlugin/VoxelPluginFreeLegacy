// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphNode_Base.h"
#include "VoxelPinCategory.h"
#include "VoxelGraphNode_Root.generated.h"

UCLASS()
class UVoxelGraphNode_Root : public UVoxelGraphNode_Base
{
	GENERATED_BODY()

public:
	// UEdGraphNode interface
	virtual bool CanUserDeleteNode() const override { return false; };
	virtual bool CanDuplicateNode() const override { return false; }
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override { return FText::FromString(TEXT("Start")); };
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor::Red; };
	// End of UEdGraphNode interface

	// UVoxelGraphNode_Base interface
	virtual void CreateOutputPins() override
	{
		CreatePin(EGPD_Output, FVoxelPinCategory::GetName(EVoxelPinCategory::Exec), FName(), nullptr, FName(" "));
	}
	// End of UVoxelGraphNode_Base interface
};