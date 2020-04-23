// Copyright 2020 Phyronnaz

#include "AssetTypeActions_VoxelGraphMacro.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelGraphEditorModule.h"

UClass* FAssetTypeActions_VoxelGraphMacro::GetSupportedClass() const
{
	return UVoxelGraphMacro::StaticClass();
}

void FAssetTypeActions_VoxelGraphMacro::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto* VoxelGraphMacro = Cast<UVoxelGraphMacro>(*ObjIt);
		if (VoxelGraphMacro)
		{
			IVoxelGraphEditorModule& VoxelGraphEditorModule = FModuleManager::LoadModuleChecked<IVoxelGraphEditorModule>("VoxelGraphEditor");
			VoxelGraphEditorModule.CreateVoxelGraphEditor(Mode, EditWithinLevelEditor, VoxelGraphMacro);
		}
	}
}