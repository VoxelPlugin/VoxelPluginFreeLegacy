// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManager.h"
#include "VoxelTools/Tools/VoxelTool.h"

#include "Engine/Blueprint.h"
#include "UObject/UObjectHash.h"
#include "AssetRegistryModule.h"

UVoxelToolManager::UVoxelToolManager()
{
	SharedConfig = CreateDefaultSubobject<UVoxelToolSharedConfig>(STATIC_FNAME("SharedConfig"));
}

void UVoxelToolManager::CreateDefaultTools(bool bLoadBlueprints)
{
	VOXEL_FUNCTION_COUNTER();

	ActiveTool = nullptr;
	Tools.Empty();

	if (bLoadBlueprints)
	{
		TArray<UClass*> ToolClasses;
		GetDerivedClasses(UVoxelTool::StaticClass(), ToolClasses);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		
		FARFilter Filter;
		Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());

		for (auto* Class : ToolClasses)
		{
			if (Class->HasAnyClassFlags(CLASS_Native))
			{
				Filter.TagsAndValues.Add(FBlueprintTags::NativeParentClassPath, FString::Printf(TEXT("%s'%s'"), *UClass::StaticClass()->GetName(), *Class->GetPathName()));
			}
		}

		TArray<FAssetData> Assets;
		AssetRegistryModule.Get().GetAssets(Filter, Assets);

		for (auto& Asset : Assets)
		{
			Asset.GetAsset();
		}
	}
	
	TArray<UClass*> ToolClasses;
	GetDerivedClasses(UVoxelTool::StaticClass(), ToolClasses);

	ToolClasses.RemoveAllSwap([](UClass* Class)
	{
		return Class->HasAnyClassFlags(CLASS_Abstract) || !Class->GetDefaultObject<UVoxelTool>()->bShowInDropdown;
	});

	for (auto* Class : ToolClasses)
	{
		// Skip SKEL and REINST classes.
		if (Class->GetName().StartsWith(TEXT("SKEL_")) || 
			Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}
		
		auto* Tool = NewObject<UVoxelTool>(this, Class, NAME_None, GetMaskedFlags(RF_Transient | RF_Transactional));
		Tool->SharedConfig = SharedConfig;
		Tools.Add(Tool);
	}
}

void UVoxelToolManager::SetActiveTool(UVoxelTool* NewActiveTool)
{
	if (ActiveTool == NewActiveTool)
	{
		return;
	}
	
	if (ActiveTool)
	{
		ActiveTool->DisableTool();
	}

	ActiveTool = NewActiveTool;

	if (ActiveTool)
	{
		ActiveTool->EnableTool();
	}
}

void UVoxelToolManager::SetActiveToolByClass(TSubclassOf<UVoxelTool> NewActiveTool)
{
	if (!NewActiveTool)
	{
		SetActiveTool(nullptr);
		return;
	}
	
	for (auto* Tool : Tools)
	{
		if (Tool->GetClass() == NewActiveTool)
		{
			SetActiveTool(Tool);
			return;
		}
	}
}

void UVoxelToolManager::SetActiveToolByName(FName NewActiveTool)
{
	for (auto* Tool : Tools)
	{
		if (Tool->ToolName == NewActiveTool)
		{
			SetActiveTool(Tool);
			return;
		}
	}
}
