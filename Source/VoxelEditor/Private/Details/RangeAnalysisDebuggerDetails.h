// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class UVoxelNode_RangeAnalysisDebuggerFloat;
class SButton;

class FRangeAnalysisDebuggerDetails : public IDetailCustomization
{
public:
	FRangeAnalysisDebuggerDetails() = default;

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
	TWeakObjectPtr<UVoxelNode_RangeAnalysisDebuggerFloat> Node;

	TSharedPtr<SButton> ResetButton;
	TSharedPtr<SButton> UpdateButton;
};