// Copyright 2020 Phyronnaz

#include "AssetTypeActions_VoxelGraphWorldGenerator.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphEditorModule.h"

UClass* FAssetTypeActions_VoxelGraphWorldGenerator::GetSupportedClass() const
{
	return UVoxelGraphGenerator::StaticClass();
}

void FAssetTypeActions_VoxelGraphWorldGenerator::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto* VoxelGraphWorldGenerator = Cast<UVoxelGraphGenerator>(*ObjIt);
		if (VoxelGraphWorldGenerator)
		{
			IVoxelGraphEditorModule& VoxelGraphEditorModule = FModuleManager::LoadModuleChecked<IVoxelGraphEditorModule>("VoxelGraphEditor");
			VoxelGraphEditorModule.CreateVoxelGraphEditor(Mode, EditWithinLevelEditor, VoxelGraphWorldGenerator);
		}
	}
}