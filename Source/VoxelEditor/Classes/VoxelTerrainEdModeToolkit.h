// Copyright 2018 Phyronnaz

#pragma once

#include "Editor/UnrealEd/Public/Toolkits/BaseToolkit.h"
#include "IDetailsView.h"

class FVoxelTerrainEdModeDetails;

class FVoxelTerrainEdModeToolkit : public FModeToolkit
{
public:
	~FVoxelTerrainEdModeToolkit();

	/** Initializes the geometry mode toolkit */
	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return ToolkitWidget; }
	
	void OnChangeMode(FName ModeName);
	bool IsModeEnabled(FName ModeName) const;
	bool IsModeActive(FName ModeName) const;

	void OnChangeTool(FName ToolName);
	bool IsToolEnabled(FName ToolName) const;
	bool IsToolActive(FName ToolName) const;

	FReply Undo();
	FReply Redo();
	FReply Undo10();
	FReply Redo10();
	FReply Undo100();
	FReply Redo100();

	bool GetIsEnabled() const;

private:
	TSharedPtr<IDetailsView> DetailsPanel;
	TSharedPtr<FVoxelTerrainEdModeDetails> Customization;

	FSlateBrush ToolIcon;

	TSharedPtr<SWidget> ToolkitWidget;
	FButtonStyle ButtonStyle;

	bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;
	bool GetIsPropertyEditingEnabled(const FPropertyAndParent& PropertyAndParent) const;
};
