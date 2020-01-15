// Copyright 2020 Phyronnaz

#pragma once

#include "Toolkits/BaseToolkit.h"
#include "IDetailsView.h"

class FVoxelEdModeToolkit : public FModeToolkit
{
public:
	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;
	
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }
	
private:
	FSlateBrush ToolIcon;
	TSharedPtr<SWidget> ToolkitWidget;
};