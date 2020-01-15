// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class SVoxelGraphNode : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SVoxelGraphNode){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode);
	
	/** Called when GraphNode changes its error information, may be called when no change has actually occurred: */
	void RefreshErrorInfo();

protected:
	//~ Begin SGraphNode Interface
	virtual void UpdateGraphNode() override;
	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual EVisibility IsAddPinButtonVisible() const override;
	virtual FReply OnAddPin() override;
	virtual TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	//~ End SGraphNode Interface

private:
	/** Set up node in 'standard' mode */
	void UpdateStandardNode();
	/** Set up node in 'compact' mode */
	void UpdateCompactNode();
	/** Get title in compact mode */
	FText GetNodeCompactTitle() const;
	/** @return the tint for the node's main body */
	FSlateColor GetNodeBodyColor() const;
	/** Set-up the error reporting widget for the node */
	void SetupErrorReporting();
	/** Called to set error text on the node */
	void UpdateErrorInfo();
	/* Called when text is committed on the node */
	void OnNameTextCommited ( const FText& InText, ETextCommit::Type CommitInfo ) ;
	
	FSlateColor GetInfoColor() const { return InfoColor; }
	FText GetInfoMsgToolTip() const { return FText::FromString(InfoMsg); }

	FSlateColor GetWarningColor() const { return WarningColor; }
	FText GetWarningMsgToolTip() const { return FText::FromString(WarningMsg); }

	TSharedPtr<IErrorReportingWidget> InfoReporting;
	TSharedPtr<IErrorReportingWidget> WarningReporting;
	FSlateColor InfoColor;
	FSlateColor WarningColor;
	FString InfoMsg;
	FString WarningMsg;

	UVoxelGraphNode_Base* VoxelNode = nullptr;
};

class SVoxelColorGraphNode : public SVoxelGraphNode
{
public:
	SLATE_BEGIN_ARGS(SVoxelColorGraphNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode, class UVoxelNode_ColorParameter* InColorNode);
	
	//~ Begin SGraphNode Interface
	virtual void CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox) override;
	//~ End SGraphNode Interface

private:
	FLinearColor SelectedColor;
	UVoxelNode_ColorParameter* ColorNode = nullptr;
	TSharedPtr<SWidget> DefaultValueWidget;

	FLinearColor GetParameterColor() const;
	void SetParameterColor(FLinearColor Color);

	FReply OnColorBoxClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
};

class SVoxelAssetPickerGraphNode : public SVoxelGraphNode
{
public:
	SLATE_BEGIN_ARGS(SVoxelAssetPickerGraphNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class UVoxelGraphNode_Base* InNode, class UVoxelAssetPickerNode* InAssetPickerNode);
	
	//~ Begin SGraphNode Interface
	virtual void CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox) override;
	//~ End SGraphNode Interface

private:
	UVoxelAssetPickerNode* AssetPickerNode = nullptr;
	TSharedPtr<SWidget> DefaultValueWidget;

	void SetAsset(const FAssetData& Asset);
	bool OnShouldFilterAsset(const FAssetData& Asset);
	FString GetObjectPath() const;
};