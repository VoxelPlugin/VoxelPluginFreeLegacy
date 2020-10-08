// Copyright 2020 Phyronnaz

#include "AssetTypeActions_VoxelGraphGenerator.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphEditorModule.h"

UClass* FAssetTypeActions_VoxelGraphGenerator::GetSupportedClass() const
{
	return UVoxelGraphGenerator::StaticClass();
}

void FAssetTypeActions_VoxelGraphGenerator::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto* VoxelGraphGenerator = Cast<UVoxelGraphGenerator>(*ObjIt);
		if (VoxelGraphGenerator)
		{
			IVoxelGraphEditorModule& VoxelGraphEditorModule = FModuleManager::LoadModuleChecked<IVoxelGraphEditorModule>("VoxelGraphEditor");
			VoxelGraphEditorModule.CreateVoxelGraphEditor(Mode, EditWithinLevelEditor, VoxelGraphGenerator);
		}
	}
}