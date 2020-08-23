// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelMinimal.h"
#include "Toolkits/BaseToolkit.h"

class FToolBarBuilder;
class FVoxelEditorToolsPanel;

class FVoxelEdModeToolkit : public FModeToolkit
{
public:
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;

	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

	/** Mode Toolbar Palettes **/
	virtual void GetToolPaletteNames(TArray<FName>& InPaletteName) const UE_24_SWITCH(,override);
	virtual void BuildToolPalette(FName PaletteName, FToolBarBuilder& ToolbarBuilder) UE_24_SWITCH(,override);
	virtual void OnToolPaletteChanged(FName PaletteName) UE_24_SWITCH(,override);

	/** Modes Panel Header Information **/
	virtual FText GetActiveToolDisplayName() const UE_25_SWITCH(,override);
	virtual FText GetActiveToolMessage() const UE_25_SWITCH(,override);

private:
	FVoxelEditorToolsPanel& GetPanel() const;
};