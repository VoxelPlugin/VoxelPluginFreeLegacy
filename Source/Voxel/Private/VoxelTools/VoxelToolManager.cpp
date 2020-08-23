// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManager.h"
#include "VoxelTools/Tools/VoxelTool.h"

#include "UObject/UObjectHash.h"

UVoxelToolManager::UVoxelToolManager()
{
	SharedConfig = CreateDefaultSubobject<UVoxelToolSharedConfig>(STATIC_FNAME("SharedConfig"));
}

void UVoxelToolManager::CreateDefaultTools()
{
	VOXEL_FUNCTION_COUNTER();

	ActiveTool = nullptr;
	Tools.Empty();
	
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
