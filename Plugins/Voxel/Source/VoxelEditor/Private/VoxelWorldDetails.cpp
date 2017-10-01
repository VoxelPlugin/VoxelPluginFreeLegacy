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
#include "VoxelWorldEditor.h"

DEFINE_LOG_CATEGORY(VoxelWorldEditorLog)

FVoxelWorldDetails::FVoxelWorldDetails()
	: EditorWorld(nullptr)
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
		.OnClicked(this, &FVoxelWorldDetails::OnWorldUpdate)
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
		.Text(FText::FromString(TEXT("Update camera position")))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FVoxelWorldDetails::OnUpdateCameraPosition)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Update")))
		]
		];
}

FReply FVoxelWorldDetails::OnWorldUpdate()
{
	if (World.IsValid())
	{
		if (!EditorWorld.IsValid())
		{
			EditorWorld = World->GetWorld()->SpawnActor<AVoxelWorldEditor>();
			EditorWorld->World = World;
		}

		World->CreateInEditor(EditorWorld->GetInvoker());
	}

	return FReply::Handled();
}

FReply FVoxelWorldDetails::OnUpdateCameraPosition()
{
	if (World.IsValid())
	{
		if (World->IsCreated())
		{
			if (World->GetWorld()->WorldType == EWorldType::Editor)
			{
				auto Client = static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
				if (Client)
				{
					FVector CameraPosition = Client->GetViewLocation();
					//World->UpdateLOD(CameraPosition);
					UE_LOG(VoxelWorldEditorLog, Error, TEXT("Not implemented"));
				}
				else
				{
					UE_LOG(VoxelWorldEditorLog, Error, TEXT("Cannot find editor camera"));
				}
			}
		}
	}

	return FReply::Handled();
}
