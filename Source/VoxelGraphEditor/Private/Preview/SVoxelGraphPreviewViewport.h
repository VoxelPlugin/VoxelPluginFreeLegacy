// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FVoxelGraphPreviewViewportClient;
class IVoxelGraphEditorToolkit;
class FEditorViewportClient;
class FAdvancedPreviewScene;
class SDockTab;

/**
 * Material Editor Preview viewport widget
 */
class SVoxelGraphPreviewViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS( SVoxelGraphPreviewViewport ){}
		SLATE_ARGUMENT(TWeakPtr<IVoxelGraphEditorToolkit>, VoxelGraphEditorToolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SVoxelGraphPreviewViewport();
	
	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

	void RefreshViewport();

protected:
	/** SEditorViewport interface */
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
	/** The parent tab where this viewport resides */
	TWeakPtr<SDockTab> ParentTab;

	/** Pointer back to the material editor tool that owns us */
	TWeakPtr<IVoxelGraphEditorToolkit> VoxelGraphEditorToolkit;

	/** Level viewport client */
	TSharedPtr<FEditorViewportClient> EditorViewportClient;
	FAdvancedPreviewScene* AdvancedPreviewScene = nullptr;
};