#include "LandscapeVoxelModifierDetails.h"
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
#include "LandscapeVoxelModifier.h"
#include "VoxelMaterial.h"
#include "MessageDialog.h"


TSharedRef<IDetailCustomization> ULandscapeVoxelModifierDetails::MakeInstance()
{
	return MakeShareable(new ULandscapeVoxelModifierDetails());
}

void ULandscapeVoxelModifierDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			ALandscapeVoxelModifier* CurrentCaptureActor = Cast<ALandscapeVoxelModifier>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				LandscapeModifier = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.EditCategory("Import")
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
		.OnClicked(this, &ULandscapeVoxelModifierDetails::OnCreateFromLandscape)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create")))
		]
		];
}

FReply ULandscapeVoxelModifierDetails::OnCreateFromLandscape()
{
	if (LandscapeModifier->Landscape && LandscapeModifier->ExportedLandscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : LandscapeModifier->Landscape->GetLandscapeActor()->LandscapeComponents)
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

		for (auto Component : LandscapeModifier->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<TArray<uint8>> Weightmaps;
			Weightmaps.SetNum(LandscapeModifier->LayerInfos.Num());

			for (int i = 0; i < Weightmaps.Num(); i++)
			{
				DataInterface.GetWeightmapTextureData(LandscapeModifier->LayerInfos[i], Weightmaps[i]);
			}

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - LandscapeModifier->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
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

		LandscapeModifier->InitExportedLandscape(Heights, Materials, TotalSize);
	}
	else
	{
		if (!LandscapeModifier->Landscape)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid Landscape")));
		}
		else if (!LandscapeModifier->ExportedLandscape)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Invalid Exported Landscape")));
		}
		else
		{
			check(false);
		}
	}

	return FReply::Handled();
}
