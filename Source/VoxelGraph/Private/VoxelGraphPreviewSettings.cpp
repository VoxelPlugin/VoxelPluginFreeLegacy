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
}

#if WITH_EDITOR
void UVoxelGraphPreviewSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

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