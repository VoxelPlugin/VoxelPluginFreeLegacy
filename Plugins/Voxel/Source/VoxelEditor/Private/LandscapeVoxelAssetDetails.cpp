#include "LandscapeVoxelAssetDetails.h"
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
#include "VoxelAssets/LandscapeVoxelAsset.h"
#include "VoxelMaterial.h"

DEFINE_LOG_CATEGORY(VoxelAssetEditorLog)

TSharedRef<IDetailCustomization> ULandscapeVoxelAssetDetails::MakeInstance()
{
	return MakeShareable(new ULandscapeVoxelAssetDetails());
}

void ULandscapeVoxelAssetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	const TArray<TWeakObjectPtr<AActor>>& SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedActors();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			ALandscapeVoxelAsset* CurrentCaptureActor = Cast<ALandscapeVoxelAsset>(CurrentObject.Get());
			if (CurrentCaptureActor != NULL)
			{
				LandscapeVoxelAsset = CurrentCaptureActor;
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
		.OnClicked(this, &ULandscapeVoxelAssetDetails::OnCreateFromLandscape)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString(TEXT("Create")))
		]
		];
}

FReply ULandscapeVoxelAssetDetails::OnCreateFromLandscape()
{
	if (LandscapeVoxelAsset->Landscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : LandscapeVoxelAsset->Landscape->GetLandscapeActor()->LandscapeComponents)
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

		for (auto Component : LandscapeVoxelAsset->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<TArray<uint8>> Weightmaps;
			Weightmaps.SetNum(LandscapeVoxelAsset->LayerInfos.Num());

			for (int i = 0; i < Weightmaps.Num(); i++)
			{
				DataInterface.GetWeightmapTextureData(LandscapeVoxelAsset->LayerInfos[i], Weightmaps[i]);
			}

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - LandscapeVoxelAsset->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
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
		LandscapeVoxelAsset->Size = 16 << Depth;

		LandscapeVoxelAsset->Heights.SetNum(LandscapeVoxelAsset->Size * LandscapeVoxelAsset->Size);
		LandscapeVoxelAsset->Weights.SetNum(LandscapeVoxelAsset->Size * LandscapeVoxelAsset->Size);

		for (int X = 0; X < TotalSize; X++)
		{
			for (int Y = 0; Y < TotalSize; Y++)
			{
				LandscapeVoxelAsset->Heights[X + LandscapeVoxelAsset->Size * Y] = Values[X + TotalSize * Y];
				LandscapeVoxelAsset->Weights[X + LandscapeVoxelAsset->Size * Y] = Colors[X + TotalSize * Y];
			}
		}
	}
	else
	{
		UE_LOG(VoxelAssetEditorLog, Error, TEXT("Invalid landscape"));
	}

	return FReply::Handled();
}
