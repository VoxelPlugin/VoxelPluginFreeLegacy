#include "VoxelSplineImporterDetails.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#include "VoxelSplineImporter.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.h"

#include "MessageDialog.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "PackageTools.h"
#include "AssetRegistryModule.h"

#include "Factories/VoxelDataAssetFactory.h"


TSharedRef<IDetailCustomization> UVoxelSplineImporterDetails::MakeInstance()
{
	return MakeShareable(new UVoxelSplineImporterDetails());
}

void UVoxelSplineImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			AVoxelSplineImporter* CurrentCaptureActor = Cast<AVoxelSplineImporter>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				SplineImporter = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory("Create VoxelDataAsset from Splines")
		.AddCustomRow(FText::FromString(TEXT("Create")))
		.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &UVoxelSplineImporterDetails::OnCreateFromSplines)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create")))
		]
		];
}

FReply UVoxelSplineImporterDetails::OnCreateFromSplines()
{
	if (SplineImporter->Splines.Num() != 0)
	{
		// See \Engine\Source\Editor\UnrealEd\PrivateLevelEditorViewport.cpp:409

		FString NewPackageName = PackageTools::SanitizePackageName(TEXT("/Game/") + SplineImporter->SavePath.Path + TEXT("/") + SplineImporter->FileName);
		UPackage* Package = CreatePackage(NULL, *NewPackageName);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// See if the material asset already exists with the expected name, if it does, ask what to do
		TArray<FAssetData> OutAssetData;
		if (AssetRegistryModule.Get().GetAssetsByPackageName(*NewPackageName, OutAssetData) && OutAssetData.Num() > 0)
		{
			auto DialogReturn = FMessageDialog::Open(EAppMsgType::YesNoCancel, FText::Format(FText::FromString(TEXT("{0} already exists. Replace it?")), FText::FromString(NewPackageName)));
			switch (DialogReturn)
			{
			case EAppReturnType::No:
				return FReply::Handled();
			case EAppReturnType::Yes:
				break;
			case EAppReturnType::Cancel:
				return FReply::Handled();
			default:
				check(false);
			}
		}

		// Create a VoxelLandscapeAsset
		UVoxelDataAssetFactory* DataAssetFactory = NewObject<UVoxelDataAssetFactory>();

		UVoxelDataAsset* DataAsset = (UVoxelDataAsset*)DataAssetFactory->FactoryCreateNew(UVoxelDataAsset::StaticClass(), Package, FName(*(SplineImporter->FileName)), RF_Standalone | RF_Public, NULL, GWarn);

		check(DataAsset);

		FDecompressedVoxelDataAsset Asset;
		SplineImporter->ImportToAsset(Asset);
		DataAsset->InitFromAsset(&Asset);

		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(DataAsset);

		// Set the dirty flag so this package will get saved later
		Package->SetDirtyFlag(true);



		FString Text = NewPackageName + TEXT(" was successfully created");
		FNotificationInfo Info(FText::FromString(Text));
		Info.ExpireDuration = 4.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		if (SplineImporter->Splines.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No splines")));
		}
		else
		{
			check(false);
		}
	}

	return FReply::Handled();
}
