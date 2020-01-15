// Copyright 2020 Phyronnaz

#include "RangeAnalysisDebuggerDetails.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelNodes/VoxelOptimizationNodes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "Voxel"

TSharedRef<IDetailCustomization> FRangeAnalysisDebuggerDetails::MakeInstance()
{
	return MakeShareable(new FRangeAnalysisDebuggerDetails());
}

FRangeAnalysisDebuggerDetails::FRangeAnalysisDebuggerDetails()
{

}

void FRangeAnalysisDebuggerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}
	Node = CastChecked<UVoxelNode_RangeAnalysisDebuggerFloat>(Objects[0].Get());

	Node->UpdateFromBin();

	SAssignNew(ResetButton, SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([=]() { if (Node.IsValid()) { Node->Reset(); } return FReply::Handled(); })
		.IsEnabled_Lambda([=]() { return Node.IsValid() && Node->Bins->bMinMaxInit; })
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("Reset", "Reset"))
		];

	SAssignNew(UpdateButton, SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([=]() { if (Node.IsValid()) { Node->UpdateGraph(); } return FReply::Handled(); })
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("Update", "Update"))
		];
	
	DetailLayout.EditCategory("Bounds")
	.AddCustomRow(LOCTEXT("Reset", "Reset"))
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("ResetBounds", "Reset bounds"))
	]
	.ValueContent()
	[
		ResetButton.ToSharedRef()
	];

	DetailLayout.EditCategory("Graph")
	.AddCustomRow(LOCTEXT("Update", "Update"))
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("UpdateGraph", "Update Graph"))
	]
	.ValueContent()
	[
		UpdateButton.ToSharedRef()
	];
}

#undef LOCTEXT_NAMESPACE