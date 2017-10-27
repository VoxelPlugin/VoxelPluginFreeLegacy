#include "VoxelLandscapeImporterDetails.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "IDetailsView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#include "LandscapeDataAccess.h"
#include "Landscape.h"
#include "LandscapeComponent.h"

#include "VoxelLandscapeImporter.h"
#include "VoxelMaterial.h"
#include "VoxelLandscapeAsset.h"

#include "MessageDialog.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "PackageTools.h"
#include "AssetRegistryModule.h"

#include "Factories/VoxelLandscapeAssetFactory.h"


TSharedRef<IDetailCustomization> UVoxelLandscapeImporterDetails::MakeInstance()
{
	return MakeShareable(new UVoxelLandscapeImporterDetails());
}

void UVoxelLandscapeImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			AVoxelLandscapeImporter* CurrentCaptureActor = Cast<AVoxelLandscapeImporter>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				LandscapeImporter = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory("Create VoxelLandscapeAsset from Landscape")
		.AddCustomRow(FText::FromString(TEXT("Create")))
		.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.OnClicked(this, &UVoxelLandscapeImporterDetails::OnCreateFromLandscape)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::FromString(TEXT("Create")))
			]
		];
}

FReply UVoxelLandscapeImporterDetails::OnCreateFromLandscape()
{
	if (LandscapeImporter->Landscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : LandscapeImporter->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;
			Count++;
			if (ComponentSize == 0)
			{
				ComponentSize = Size;
			}
			else
			{
				check(ComponentSize == Size);
			}

		}

		check(FMath::RoundToInt(FMath::Sqrt(Count)) * FMath::RoundToInt(FMath::Sqrt(Count)) == Count);
		int TotalSize = FMath::RoundToInt(FMath::Sqrt(Count)) * ComponentSize;

		TArray<float> Heights;
		TArray<FVoxelMaterial> Materials;
		Heights.SetNum(TotalSize * TotalSize);
		Materials.SetNum(TotalSize * TotalSize);

		for (auto Component : LandscapeImporter->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<TArray<uint8>> Weightmaps;
			Weightmaps.SetNum(LandscapeImporter->LayerInfos.Num());

			for (int i = 0; i < Weightmaps.Num(); i++)
			{
				DataInterface.GetWeightmapTextureData(LandscapeImporter->LayerInfos[i], Weightmaps[i]);
			}

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - LandscapeImporter->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
					Heights[LocalVertex.X + TotalSize * LocalVertex.Y] = Vertex.Z;

					uint8 MaxIndex = 0;
					uint8 MaxValue = 0;
					uint8 SecondMaxIndex = 0;
					uint8 SecondMaxValue = 0;

					for (int i = 0; i < Weightmaps.Num(); i++)
					{
						if (Weightmaps[i].Num())
						{
							uint8 Weight = Weightmaps[i][X + WeightmapSize * Y];
							if (Weight > MaxValue)
							{
								SecondMaxValue = MaxValue;
								SecondMaxIndex = MaxIndex;
								MaxValue = Weight;
								MaxIndex = i;
							}
							else if (Weight > SecondMaxValue)
							{
								SecondMaxValue = Weight;
								SecondMaxIndex = i;
							}
						}
					}

					Materials[LocalVertex.X + TotalSize * LocalVertex.Y] = FVoxelMaterial(MaxIndex, SecondMaxIndex, FMath::Clamp<uint8>(((255 - MaxValue) + SecondMaxValue) / 2, 0, 255));
				}
			}
		}

		// See \Engine\Source\Editor\UnrealEd\PrivateLevelEditorViewport.cpp:409

		FString NewPackageName = PackageTools::SanitizePackageName(TEXT("/Game/") + LandscapeImporter->SavePath.Path + TEXT("/") + LandscapeImporter->FileName);
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
		UVoxelLandscapeAssetFactory* LandscapeFactory = NewObject<UVoxelLandscapeAssetFactory>();

		UVoxelLandscapeAsset* LandscapeAsset = (UVoxelLandscapeAsset*)LandscapeFactory->FactoryCreateNew(UVoxelLandscapeAsset::StaticClass(), Package, LandscapeImporter->FileName, RF_Standalone | RF_Public, NULL, GWarn);

		check(LandscapeAsset);

		LandscapeAsset->Init(Heights, Materials, TotalSize);

		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(LandscapeAsset);

		// Set the dirty flag so this package will get saved later
		Package->SetDirtyFlag(true);



		FString Text = NewPackageName + TEXT(" was successfully created");
		FNotificationInfo Info(FText::FromString(Text));
		Info.ExpireDuration = 4.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		if (!LandscapeImporter->Landscape)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid Landscape")));
		}
		else
		{
			check(false);
		}
	}

	return FReply::Handled();
}
