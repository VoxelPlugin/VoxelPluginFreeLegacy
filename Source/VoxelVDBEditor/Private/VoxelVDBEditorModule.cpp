// Copyright 2020 Phyronnaz

#include "VoxelVDBEditorModule.h"
#include "VoxelMinimal.h"
#include "VoxelVDBAsset.h"
#include "VoxelEditorModule.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_Base.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "HAL/FileManager.h"

#include "EditorStyleSet.h"
#include "EditorReimportHandler.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"

class FAssetTypeActions_VoxelVDBAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel VDB Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 0, 64); }
	virtual UClass* GetSupportedClass() const override { return UVoxelVDBAsset::StaticClass(); }
	virtual uint32 GetCategories() override
	{
		return FModuleManager::LoadModuleChecked<IVoxelEditorModule>("VoxelEditor").GetVoxelAssetTypeCategory();
	}
	
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
	{
		const auto Assets = GetTypedWeakObjectPtrs<UVoxelVDBAsset>(InObjects);

		MenuBuilder.AddMenuEntry(
			VOXEL_LOCTEXT("Reimport"),
			VOXEL_LOCTEXT("Reimport the selected asset(s)."),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.ReimportAsset"),
			FUIAction(
				FExecuteAction::CreateSP(this, &FAssetTypeActions_VoxelVDBAsset::ExecuteReimport, Assets),
				FCanExecuteAction::CreateSP(this, &FAssetTypeActions_VoxelVDBAsset::CanExecuteReimport, Assets)
			)
		);
		MenuBuilder.AddMenuEntry(
			VOXEL_LOCTEXT("Export"),
			VOXEL_LOCTEXT("Export the selected asset(s)."),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ContentBrowser.AssetActions.OpenInExternalEditor"),
			FUIAction(
				FExecuteAction::CreateSP(this, &FAssetTypeActions_VoxelVDBAsset::ExecuteExport, Assets)
			)
		);
	}

private:
	bool CanExecuteReimport(const TArray<TWeakObjectPtr<UVoxelVDBAsset>> Objects) const
	{
		for (auto& Object : Objects)
		{
			if (Object.IsValid() && !Object->ImportPath.IsEmpty())
			{
				return true;
			}
		}
		return false;
	}

	void ExecuteReimport(const TArray<TWeakObjectPtr<UVoxelVDBAsset>> Objects) const
	{
		for (auto& Object : Objects)
		{
			if (Object.IsValid() && !Object->ImportPath.IsEmpty())
			{
				FReimportManager::Instance()->Reimport(Object.Get(), /*bAskForNewFileIfMissing=*/true);
			}
		}
	}
	void ExecuteExport(const TArray<TWeakObjectPtr<UVoxelVDBAsset>> Objects) const
	{
		for (auto& Object : Objects)
		{
			if (Object.IsValid())
			{
				const FString DefaultPath = FPaths::ProjectSavedDir() / "VDB";
				IFileManager::Get().MakeDirectory(*DefaultPath);
				
				IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
				check(DesktopPlatform);
				
				TArray<FString> OutFiles;
				if (DesktopPlatform->SaveFileDialog(
					nullptr,
					"Destination",
					DefaultPath,
					Object->GetName() + ".vdb",
					TEXT("VDB file (*.vdb)|*.vdb"),
					EFileDialogFlags::None,
					OutFiles))
				{
					check(OutFiles.Num() == 1);
					const FString Path = OutFiles[0];

					FString Error;
					const bool bSuccess = Object->GetData()->SaveVDB(Path, Error);

					const FString Text = bSuccess
					? "Successfully exported " + Path
					: "Failed to export " + Path + ": " + Error;
					
					FNotificationInfo Info(FText::FromString(Text));
					Info.ExpireDuration = 10.f;
					Info.CheckBoxState = bSuccess ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					FSlateNotificationManager::Get().AddNotification(Info);
				}
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelVDBEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeActions_VoxelVDBAsset>());
}

IMPLEMENT_MODULE(FVoxelVDBEditorModule, VoxelVDBEditor);