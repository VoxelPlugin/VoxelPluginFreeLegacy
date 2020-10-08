// Copyright 2020 Phyronnaz

#include "Details/VoxelLandscapeImporterDetails.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "Factories/VoxelHeightmapAssetFactory.h"
#include "VoxelImporters/VoxelLandscapeImporter.h"

#include "Landscape.h"

#include "DetailLayoutBuilder.h"
#include "Misc/MessageDialog.h"

void FVoxelLandscapeImporterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}
	Importer = CastChecked<AVoxelLandscapeImporter>(Objects[0].Get());

	auto LayerInfos = DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelLandscapeImporter, LayerInfos))->AsArray();;
	uint32 Num = 0;
	LayerInfos->GetNumElements(Num);
	for (uint32 Index = 0; Index < Num; Index++)
	{
		auto Handle = LayerInfos->GetElement(Index);
		switch (Importer->MaterialConfig)
		{
		case EVoxelHeightmapImporterMaterialConfig::RGB:
			DetailLayout.HideProperty(Handle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelLandscapeImporterLayerInfo, Index)));
			break;
		case EVoxelHeightmapImporterMaterialConfig::FourWayBlend:
		case EVoxelHeightmapImporterMaterialConfig::FiveWayBlend:
		case EVoxelHeightmapImporterMaterialConfig::SingleIndex:
		case EVoxelHeightmapImporterMaterialConfig::MultiIndex:
			DetailLayout.HideProperty(Handle->GetChildHandle(GET_MEMBER_NAME_STATIC(FVoxelLandscapeImporterLayerInfo, Layer)));
			break;
		default:
			check(false);
			break;
		}
	}

	FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateLambda([&DetailLayout]()
	{
		DetailLayout.ForceRefreshDetails();
	});
	DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelLandscapeImporter, MaterialConfig))->SetOnPropertyValueChanged(RefreshDelegate);
	DetailLayout.GetProperty(GET_MEMBER_NAME_STATIC(AVoxelLandscapeImporter, LayerInfos))->SetOnPropertyValueChanged(RefreshDelegate);

	FVoxelEditorUtilities::AddButtonToCategory(DetailLayout,
		"Create VoxelLandscapeAsset from Landscape",
		VOXEL_LOCTEXT("Create"),
		VOXEL_LOCTEXT("Create From Landscape"),
		VOXEL_LOCTEXT("Create"),
		false,
		FOnClicked::CreateSP(this, &FVoxelLandscapeImporterDetails::OnCreateFromLandscape),
		TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateLambda([Importer = Importer]() { return Importer.IsValid() && Importer->Landscape; })));
}

FReply FVoxelLandscapeImporterDetails::OnCreateFromLandscape()
{
	auto* Factory = NewObject<UVoxelHeightmapAssetFloatFactory>();
	Factory->MaterialConfig = Importer->MaterialConfig;
	Factory->LayerInfos = Importer->LayerInfos;
	Factory->Components = Importer->Landscape->LandscapeComponents;
	Factory->ActorLocation = Importer->Landscape->GetActorLocation();
	Factory->AssetName = Importer->Landscape->GetName();

	FVoxelEditorUtilities::CreateAssetWithDialog(UVoxelHeightmapAssetFloat::StaticClass(), Factory);

	return FReply::Handled();
}