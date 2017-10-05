#include "VoxelWorldDetails.h"
#include "VoxelWorld.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "LevelEditorViewport.h"
#include "Editor.h"
#include "IDetailPropertyRow.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(VoxelEditorLog)

FVoxelWorldDetails::FVoxelWorldDetails()
{

}

FVoxelWorldDetails::~FVoxelWorldDetails()
{

}

TSharedRef<IDetailCustomization> FVoxelWorldDetails::MakeInstance()
{
	return MakeShareable(new FVoxelWorldDetails());
}

void FVoxelWorldDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			AVoxelWorld* CurrentCaptureActor = Cast<AVoxelWorld>(CurrentObject.Get());
			if (CurrentCaptureActor)
			{
				World = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory("Voxel")
		.AddCustomRow(FText::FromString(TEXT("Update")))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Toggle world rendering")))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelWorldDetails::OnWorldToggle)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Toggle")))
		]
		];

	DetailLayout.EditCategory("Voxel")
		.AddCustomRow(FText::FromString(TEXT("Update")))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Update voxel modifiers")))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelWorldDetails::OnUpdateVoxelModifiers)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Update")))
		]
		];
}

FReply FVoxelWorldDetails::OnWorldToggle()
{
	/*if (World.IsValid())
	{
		if (World->IsCreated())
		{
			World->DestroyInEditor();
		}
		else
		{
			CreateEditorWorld();

			World->CreateInEditor(EditorWorld->GetInvoker());
		}
	}*/

	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnUpdateVoxelModifiers()
{
	/*if (World.IsValid() && World->GetWorld()->WorldType == EWorldType::Editor)
	{
		bool bRecreate = false;
		if (World->IsCreated())
		{
			bRecreate = true;
			World->DestroyInEditor();
		}

		World->UpdateVoxelModifiers();

		if (bRecreate)
		{
			CreateEditorWorld();

			World->CreateInEditor(EditorWorld->GetInvoker());
		}
	}
	else
	{
		if (!World.IsValid())
		{
			UE_LOG(VoxelEditorLog, Error, TEXT("Invalid World"));
		}
		else
		{
			UE_LOG(VoxelEditorLog, Error, TEXT("Not in editor"));
		}
	}*/

	return FReply::Handled();
}
