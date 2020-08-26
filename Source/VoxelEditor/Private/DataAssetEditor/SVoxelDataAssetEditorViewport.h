// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "SEditorViewport.h"

class FVoxelDataAssetEditorViewportClient;
class IVoxelDataAssetEditor;
class SDockTab;
class FAdvancedPreviewScene;

class SVoxelDataAssetEditorViewportToolBar : public SCommonEditorViewportToolbarBase
{
public:
	SLATE_BEGIN_ARGS(SVoxelDataAssetEditorViewportToolBar) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<class SVoxelDataAssetEditorViewport> InViewport);

	//~ Begin SCommonEditorViewportToolbarBase interface
	virtual TSharedRef<SWidget> GenerateShowMenu() const override;
	//~ End SCommonEditorViewportToolbarBase interface
};

class SVoxelDataAssetEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SVoxelDataAssetEditorViewport){}
		SLATE_ARGUMENT(IVoxelDataAssetEditor*, Editor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SVoxelDataAssetEditorViewport();
	
	void RefreshViewport();

	/** Event handlers */
	void TogglePreviewBackground();
	bool IsPreviewBackgroundToggled() const;

	//~ Begin ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override {}
	//~ End ICommonEditorViewportToolbarInfoProvider interface

protected:
	//~ Begin SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
	virtual void BindCommands() override;
	virtual bool IsVisible() const override;
	//~ End SEditorViewport interface

private:
	/** The parent tab where this viewport resides */
	TWeakPtr<SDockTab> ParentTab;

	IVoxelDataAssetEditor* DataAssetEditor = nullptr;
	FAdvancedPreviewScene* AdvancedPreviewScene = nullptr;

	/** Level viewport client */
	TSharedPtr<FVoxelDataAssetEditorViewportClient> EditorViewportClient;
};