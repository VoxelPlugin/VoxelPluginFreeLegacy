// Copyright 2020 Phyronnaz

#include "VoxelGraphPreviewSettings.h"
#include "IVoxelGraphEditor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UVoxelGraphPreviewSettings::UVoxelGraphPreviewSettings()
{
	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> MeshObject(TEXT("/Voxel/Preview/SM_Plane"));
	ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> MaterialObject(TEXT("/Voxel/Preview/M_PreviewMaterial"));

	Mesh = MeshObject.Get();
	Material = MaterialObject.Get();

	IndexColors.Add(FColorList::Red);
	IndexColors.Add(FColorList::Green);
	IndexColors.Add(FColorList::Blue);
	IndexColors.Add(FColorList::Yellow);
	IndexColors.Add(FColorList::Magenta);
	IndexColors.Add(FColorList::Cyan);
	IndexColors.Add(FColorList::Orange);
	IndexColors.Add(FColorList::Aquamarine);
}

#if WITH_EDITOR
void UVoxelGraphPreviewSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
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

	Resolution = FMath::Clamp(1 << FMath::FloorLog2(Resolution), 32, 4096);

	PreviewedBounds = GetBounds();

	if (PropertyChangedEvent.MemberProperty && Graph)
	{
		static FName NoRebuild(TEXT("NoRebuild"));

		if (PropertyChangedEvent.MemberProperty->HasMetaData(NoRebuild))
		{
			IVoxelGraphEditor::GetVoxelGraphEditor()->UpdatePreview(Graph, true, false);
		}
		else if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			IVoxelGraphEditor::GetVoxelGraphEditor()->UpdatePreview(Graph, true, true);
		}
	}
}
#endif