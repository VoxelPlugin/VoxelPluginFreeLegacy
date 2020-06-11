// Copyright 2020 Phyronnaz

#include "VoxelImporters/VoxelLandscapeImporter.h"
#include "Landscape.h"
#include "VoxelMinimal.h"

#if WITH_EDITOR
void AVoxelLandscapeImporter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(AVoxelLandscapeImporter, Landscape) &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && 
		Landscape &&
		LayerInfos.Num() == 0)
	{
		for (auto& Layer : Landscape->EditorLayerSettings)
		{
			FVoxelLandscapeImporterLayerInfo LayerInfo;
			LayerInfo.LayerInfo = Layer.LayerInfoObj;
			LayerInfo.Layer = EVoxelRGBA(LayerInfos.Num() % 4);
			LayerInfo.Index = LayerInfos.Num();
			LayerInfos.Add(LayerInfo);
		}
	}
}
#endif