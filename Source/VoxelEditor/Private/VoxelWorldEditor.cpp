// Copyright 2019 Phyronnaz

#include "VoxelWorldEditor.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelSave.h"

#include "Factories/VoxelWorldSaveObjectFactory.h"

#include "LevelEditorViewport.h"
#include "Editor.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "EditorDirectories.h"
#include "EditorSupportDelegates.h"

inline UObject* CreateAssetWithDialog(const FString& AssetName, const FString& PackagePath, UClass* AssetClass, UFactory* Factory, FName CallingContext)
{
	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = NSLOCTEXT("Voxel", "SaveAssetDialogTitle", "Save Voxel World Data As");
	SaveAssetDialogConfig.DefaultPath = PackagePath;
	SaveAssetDialogConfig.DefaultAssetName = AssetName;
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
	if (!SaveObjectPath.IsEmpty())
	{
		FEditorDelegates::OnConfigureNewAssetProperties.Broadcast(Factory);
		if (Factory->ConfigureProperties())
		{
			const FString SavePackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
			const FString SavePackagePath = FPaths::GetPath(SavePackageName);
			const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::NEW_ASSET, PackagePath);

			return FAssetToolsModule::GetModule().Get().CreateAsset(SaveAssetName, SavePackagePath, AssetClass, Factory, CallingContext);
		}
	}

	return nullptr;
}

inline UObject* CreateAssetWithDialog(UClass* AssetClass, UFactory* Factory, FName CallingContext = NAME_None)
{
	if (Factory != nullptr)
	{
		// Determine the starting path. Try to use the most recently used directory
		FString AssetPath;

		const FString DefaultFilesystemDirectory = FEditorDirectories::Get().GetLastDirectory(ELastDirectory::NEW_ASSET);
		if (DefaultFilesystemDirectory.IsEmpty() || !FPackageName::TryConvertFilenameToLongPackageName(DefaultFilesystemDirectory, AssetPath))
		{
			// No saved path, just use the game content root
			AssetPath = TEXT("/Game");
		}

		FString PackageName;
		FString AssetName;
		FAssetToolsModule::GetModule().Get().CreateUniqueAssetName(AssetPath / Factory->GetDefaultNewAssetName(), TEXT(""), PackageName, AssetName);

		return CreateAssetWithDialog(AssetName, AssetPath, AssetClass, Factory, CallingContext);
	}

	return nullptr;
}

class FVoxelWorldEditor : public IVoxelWorldEditor
{
public:
	FVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() override
	{
		return Cast<UVoxelWorldSaveObject>(CreateAssetWithDialog(UVoxelWorldSaveObject::StaticClass(), NewObject<UVoxelWorldSaveObjectFactory>()));
	}

	virtual void BindEditorDelegates(AVoxelWorld* World) override
	{
		if (!FEditorDelegates::PreSaveWorld.IsBoundToObject(World))
		{
			FEditorDelegates::PreSaveWorld.AddUObject(World, &AVoxelWorld::OnPreSaveWorld);
		}
		if (!FEditorDelegates::PreBeginPIE.IsBoundToObject(World))
		{
			FEditorDelegates::PreBeginPIE.AddUObject(World, &AVoxelWorld::OnPreBeginPIE);
		}
		if (!FEditorDelegates::EndPIE.IsBoundToObject(World))
		{
			FEditorDelegates::EndPIE.AddUObject(World, &AVoxelWorld::OnEndPIE);
		}
		if (!FEditorSupportDelegates::PrepareToCleanseEditorObject.IsBoundToObject(World))
		{
			FEditorSupportDelegates::PrepareToCleanseEditorObject.AddUObject(World, &AVoxelWorld::OnPrepareToCleanseEditorObject);
		}
		if (!FCoreDelegates::OnPreExit.IsBoundToObject(World))
		{
			FCoreDelegates::OnPreExit.AddUObject(World, &AVoxelWorld::OnPreExit);
		}
	}
};

AVoxelWorldEditor::AVoxelWorldEditor()
{
	PrimaryActorTick.bCanEverTick = true;

	Invoker = CreateDefaultSubobject<UVoxelInvokerEditorComponent>(FName("Editor Invoker"));
	Invoker->bUseForCollisions = false;
	Invoker->bUseForNavmesh = false;
	RootComponent = Invoker;

	if (!IVoxelWorldEditor::GetVoxelWorldEditor())
	{
		IVoxelWorldEditor::SetVoxelWorldEditor(MakeShared<FVoxelWorldEditor>());
	}
}

void AVoxelWorldEditor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		auto* Viewport = GEditor->GetActiveViewport();
		if (Viewport)
		{
			auto* Client = static_cast<FLevelEditorViewportClient*>(Viewport->GetClient());
			if (Client)
			{
				FVector CameraPosition = Client->GetViewLocation();
				SetActorLocation(CameraPosition);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Cannot find editor camera"));
			}
		}
	}
	else
	{
		Destroy();
	}
}


void UVoxelInvokerEditorComponent::OnRegister()
{
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		Super::OnRegister();
	}
	else
	{
		USceneComponent::OnRegister();
	}
}