#include "VoxelMeshAssetDetails.h"
#include "VoxelMeshAsset.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Editor.h"

TSharedRef<IDetailCustomization> FVoxelMeshAssetDetails::MakeInstance()
{
	return MakeShareable(new FVoxelMeshAssetDetails());
}

void FVoxelMeshAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			AVoxelMeshAsset* CurrentCaptureActor = Cast<AVoxelMeshAsset>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				MeshAsset = CurrentCaptureActor;
				break;
			}
		}
	}

	//DetailLayout.HideCategory("Hide");
	DetailLayout.EditCategory("VoxelMeshAsset")
		.AddCustomRow(FText::FromString(TEXT("Import")))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Import from mesh")))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelMeshAssetDetails::OnImport)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Import")))
		]
		];
	/*
		DetailLayout.EditCategory("VoxelMeshAsset")
			.AddCustomRow(FText::FromString(TEXT("Update")))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(FText::FromString(TEXT("Update lines")))
			]
		.ValueContent()
			.MaxDesiredWidth(125.f)
			.MinDesiredWidth(125.f)
			[
				SNew(SButton)
				.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FVoxelMeshAssetDetails::OnUpdateLines)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(FText::FromString(TEXT("Update")))
			]
			];*/
}

FReply FVoxelMeshAssetDetails::OnImport()
{
	if (MeshAsset.IsValid())
	{
		MeshAsset->Import();
	}
	return FReply::Handled();
}

//FReply FVoxelMeshAssetDetails::OnUpdateLines()
//{
//	if (MeshAsset.IsValid())
//	{
//		MeshAsset->UpdateLines();
//	}
//	return FReply::Handled();
//}
