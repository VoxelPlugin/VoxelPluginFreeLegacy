// Copyright 2020 Phyronnaz

#include "VoxelGraphPreviewSettings.h"
#include "IVoxelGraphEditor.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UVoxelGraphPreviewSettings::UVoxelGraphPreviewSettings()
{
	static ConstructorHelpers::FObjectFinderOptional<UStaticMesh> MeshObject(TEXT("/Voxel/Preview/SM_Plane"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> HeightmapMaterialObject(TEXT("/Voxel/Preview/M_PreviewMaterial"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> SliceMaterialObject(TEXT("/Voxel/Preview/M_2DPreviewMaterial"));

	Mesh = MeshObject.Get();
	HeightmapMaterial = HeightmapMaterialObject.Get();
	SliceMaterial = SliceMaterialObject.Get();

	IndexColors.Add(FColorList::Red);
	IndexColors.Add(FColorList::Green);
	IndexColors.Add(FColorList::Blue);
	IndexColors.Add(FColorList::Yellow);
	IndexColors.Add(FColorList::Magenta);
	IndexColors.Add(FColorList::Cyan);
	IndexColors.Add(FColorList::Orange);
	IndexColors.Add(FColorList::Aquamarine);
	IndexColors.Add(FColorList::BakerChocolate);
	IndexColors.Add(FColorList::Brass);
	IndexColors.Add(FColorList::Gold);
}

#if WITH_EDITOR
void UVoxelGraphPreviewSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (LeftToRight == BottomToTop)
	{
		// Else crash
		if (PropertyChangedEvent.MemberProperty && 
			PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphPreviewSettings, BottomToTop))
		{
			LeftToRight = EVoxelGraphPreviewAxes((int32(BottomToTop) + 1) % 3);
		}
		else
		{
			BottomToTop = EVoxelGraphPreviewAxes((int32(LeftToRight) + 1) % 3);
		}
	}

	NumRangeAnalysisChunksPerAxis = FMath::Clamp(NumRangeAnalysisChunksPerAxis, 1, Resolution);

	const FVoxelGraphPreviewSettingsWrapper Wrapper(*this);
	ResolutionMultiplierLog = Wrapper.LOD;
	PreviewedBounds = Wrapper.Bounds;

	// Don't let the previewed voxel go out of the bounds
	PreviewedVoxel = Wrapper.Bounds.Clamp(PreviewedVoxel);

	if (Graph && PropertyChangedEvent.MemberProperty)
	{
		const bool bAutomatic = PropertyChangedEvent.MemberProperty->HasMetaData(STATIC_FNAME("Automatic"));
		const bool bUpdateItems = PropertyChangedEvent.MemberProperty->HasMetaData(STATIC_FNAME("UpdateItems"));
		const bool bMeshOnly = PropertyChangedEvent.MemberProperty->HasMetaData(STATIC_FNAME("MeshOnly"));

		if (!bAutomatic || PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			EVoxelGraphPreviewFlags Flags = EVoxelGraphPreviewFlags::None;
			if (!bAutomatic)
			{
				Flags |= EVoxelGraphPreviewFlags::ManualPreview;
			}
			Flags |= EVoxelGraphPreviewFlags::UpdateMeshSettings;
			if (!bMeshOnly)
			{
				Flags |= EVoxelGraphPreviewFlags::UpdateTextures;
			}
			if (bUpdateItems)
			{
				Flags |= EVoxelGraphPreviewFlags::UpdatePlaceableItems;
			}

			IVoxelGraphEditor::GetVoxelGraphEditor()->UpdatePreview(Graph, Flags);
		}
	}
}
#endif

FVoxelGraphPreviewSettingsWrapper::FVoxelGraphPreviewSettingsWrapper(const UVoxelGraphPreviewSettings& Settings)
{
	LOD = FMath::Clamp(Settings.ResolutionMultiplierLog, 0, 20);
	Step = 1 << LOD;
	Resolution = Settings.Resolution;
	
	LeftToRight = Settings.LeftToRight;
	BottomToTop = Settings.BottomToTop;
	
	Center = FVoxelUtilities::DivideRound(Settings.Center, Step) * Step;

	{
		Start = Center;
		const int32 Offset = Resolution / 2 * Step;
		GetAxis(Start, LeftToRight) -= Offset;
		GetAxis(Start, BottomToTop) -= Offset;
	}

	{
		Size = FIntVector(1, 1, 1);
		GetAxis(Size, LeftToRight) = Resolution;
		GetAxis(Size, BottomToTop) = Resolution;
	}

	Bounds = FVoxelIntBox(Start, Start + Size * Step);
}