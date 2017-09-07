#include "LandscapeWorldGeneratorDetails.h"
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
#include "LandscapeWorldGenerator.h"
#include "VoxelMaterial.h"

DEFINE_LOG_CATEGORY(VoxelAssetEditorLog)

TSharedRef<IDetailCustomization> ULandscapeWorldGeneratorDetails::MakeInstance()
{
	return MakeShareable(new ULandscapeWorldGeneratorDetails());
}

void ULandscapeWorldGeneratorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailLayout.GetDetailsView().GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			ALandscapeWorldGenerator* CurrentCaptureActor = Cast<ALandscapeWorldGenerator>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				LandscapeWorldGenerator = CurrentCaptureActor;
				break;
			}
		}
	}

	DetailLayout.HideCategory("Hide");
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
		.OnClicked(this, &ULandscapeWorldGeneratorDetails::OnCreateFromLandscape)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create")))
		]
		];
}

FReply ULandscapeWorldGeneratorDetails::OnCreateFromLandscape()
{
	if (LandscapeWorldGenerator->Landscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : LandscapeWorldGenerator->Landscape->GetLandscapeActor()->LandscapeComponents)
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

		TArray<float> Values;
		TArray<FColor> Colors;
		Values.SetNum(TotalSize * TotalSize);
		Colors.SetNum(TotalSize * TotalSize);

		for (auto Component : LandscapeWorldGenerator->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<TArray<uint8>> Weightmaps;
			Weightmaps.SetNum(LandscapeWorldGenerator->LayerInfos.Num());

			for (int i = 0; i < Weightmaps.Num(); i++)
			{
				DataInterface.GetWeightmapTextureData(LandscapeWorldGenerator->LayerInfos[i], Weightmaps[i]);
			}

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - LandscapeWorldGenerator->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
					Values[LocalVertex.X + TotalSize * LocalVertex.Y] = Vertex.Z;

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

					Colors[LocalVertex.X + TotalSize * LocalVertex.Y] = FVoxelMaterial(MaxIndex, SecondMaxIndex, ((255 - MaxValue) + SecondMaxValue) / 2 / 255.f).ToFColor();
				}
			}
		}

		int Depth = FMath::CeilToInt(FMath::Log2(TotalSize)) - 4;
		LandscapeWorldGenerator->Width = 16 << Depth;

		LandscapeWorldGenerator->Heights.SetNum(LandscapeWorldGenerator->Width * LandscapeWorldGenerator->Width);
		LandscapeWorldGenerator->Weights.SetNum(LandscapeWorldGenerator->Width * LandscapeWorldGenerator->Width);

		for (int X = 0; X < TotalSize; X++)
		{
			for (int Y = 0; Y < TotalSize; Y++)
			{
				LandscapeWorldGenerator->Heights[X + LandscapeWorldGenerator->Width * Y] = Values[X + TotalSize * Y];
				LandscapeWorldGenerator->Weights[X + LandscapeWorldGenerator->Width * Y] = Colors[X + TotalSize * Y];
			}
		}

		LandscapeWorldGenerator->LandscapePosition = LandscapeWorldGenerator->Landscape->GetActorLocation();
	}
	else
	{
		UE_LOG(VoxelAssetEditorLog, Error, TEXT("Invalid landscape"));
	}

	return FReply::Handled();
}
