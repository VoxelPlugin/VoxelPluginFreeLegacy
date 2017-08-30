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
#include "LandscapeDataAccess.h"
#include "Landscape.h"
#include "LandscapeComponent.h"

DEFINE_LOG_CATEGORY(VoxelEditorLog)

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
	if (VoxelAsset->Landscape)
	{
		int MipLevel = 0;
		int ComponentSize = 0;
		int Count = 0;

		for (auto Component : VoxelAsset->Landscape->GetLandscapeActor()->LandscapeComponents)
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

		for (auto Component : VoxelAsset->Landscape->GetLandscapeActor()->LandscapeComponents)
		{
			FLandscapeComponentDataInterface DataInterface(Component, MipLevel);
			int Size = (Component->ComponentSizeQuads + 1) >> MipLevel;

			TArray<uint8> Weightmap1;
			TArray<uint8> Weightmap2;
			TArray<uint8> Weightmap3;
			DataInterface.GetWeightmapTextureData(VoxelAsset->LayerInfo1, Weightmap1);
			DataInterface.GetWeightmapTextureData(VoxelAsset->LayerInfo2, Weightmap2);
			DataInterface.GetWeightmapTextureData(VoxelAsset->LayerInfo3, Weightmap3);

			int32 WeightmapSize = ((Component->SubsectionSizeQuads + 1) * Component->NumSubsections) >> MipLevel;

			for (int X = 0; X < Size; X++)
			{
				for (int Y = 0; Y < Size; Y++)
				{
					FVector Vertex = DataInterface.GetWorldVertex(X, Y);
					FVector LocalVertex = (Vertex - VoxelAsset->Landscape->GetActorLocation()) / Component->GetComponentTransform().GetScale3D();
					Values[LocalVertex.X + TotalSize * LocalVertex.Y] = Vertex.Z;

					uint8 Weight1 = 0;
					uint8 Weight2 = 0;
					uint8 Weight3 = 0;
					if (Weightmap1.Num())
					{
						Weight1 = Weightmap1[X + WeightmapSize * Y];
					}
					if (Weightmap2.Num())
					{
						Weight2 = Weightmap2[X + WeightmapSize * Y];
					}
					if (Weightmap3.Num())
					{
						Weight3 = Weightmap3[X + WeightmapSize * Y];
					}
					Colors[LocalVertex.X + TotalSize * LocalVertex.Y] = FColor(Weight1, Weight2, Weight3);
				}
			}
		}

		int Depth = FMath::CeilToInt(FMath::Log2(TotalSize)) - 4;
		VoxelAsset->Width = 16 << Depth;

		VoxelAsset->Heights.SetNum(VoxelAsset->Width * VoxelAsset->Width);
		VoxelAsset->Weights.SetNum(VoxelAsset->Width * VoxelAsset->Width);

		for (int X = 0; X < TotalSize; X++)
		{
			for (int Y = 0; Y < TotalSize; Y++)
			{
				VoxelAsset->Heights[X + VoxelAsset->Width * Y] = Values[X + TotalSize * Y];
				VoxelAsset->Weights[X + VoxelAsset->Width * Y] = Colors[X + TotalSize * Y];
			}
		}
	}
	else
	{
		UE_LOG(VoxelEditorLog, Error, TEXT("Invalid landscape"));
	}

	return FReply::Handled();
}
