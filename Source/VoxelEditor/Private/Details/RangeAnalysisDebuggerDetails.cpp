// Copyright 2020 Phyronnaz

#include "RangeAnalysisDebuggerDetails.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelNodes/VoxelOptimizationNodes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

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
			.Text(VOXEL_LOCTEXT("Reset"))
		];

	SAssignNew(UpdateButton, SButton)
		.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([=]() { if (Node.IsValid()) { Node->UpdateGraph(); } return FReply::Handled(); })
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(VOXEL_LOCTEXT("Update"))
		];
	
	DetailLayout.EditCategory("Bounds")
	.AddCustomRow(VOXEL_LOCTEXT("Reset"))
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(VOXEL_LOCTEXT("Reset bounds"))
	]
	.ValueContent()
	[
		ResetButton.ToSharedRef()
	];

	DetailLayout.EditCategory("Graph")
	.AddCustomRow(VOXEL_LOCTEXT("Update"))
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(VOXEL_LOCTEXT("Update Graph"))
	]
	.ValueContent()
	[
		UpdateButton.ToSharedRef()
	];
}