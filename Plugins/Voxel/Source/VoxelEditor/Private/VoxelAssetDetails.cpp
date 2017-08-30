#include "VoxelAssetDetails.h"
#include "VoxelAsset.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

TSharedRef<IDetailCustomization> FVoxelAssetDetails::MakeInstance()
{
	return MakeShareable(new FVoxelAssetDetails());
}

void FVoxelAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			AVoxelAsset* CurrentCaptureActor = Cast<AVoxelAsset>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				VoxelAsset = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.HideCategory("Hide");
	DetailLayout.EditCategory("Voxel Asset")
		.AddCustomRow(FText::FromString(TEXT("Create")))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create from landscape")))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelAssetDetails::OnCreateFromLandscape)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create")))
		]
		];
}

FReply FVoxelAssetDetails::OnCreateFromLandscape()
{
	VoxelAsset->CreateFromLandcape();
	return FReply::Handled();
}
