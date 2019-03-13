// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "VoxelData/VoxelSave.h"

class FVoxelEditorUtilities
{
public:
	static void ModifyWorld(AVoxelWorld* World)
	{
		if (Cast<AVoxelDataAssetEditorVoxelWorld>(World))
		{
			World->GetWorldGeneratorPicker().WorldGeneratorObject->Modify();
		}
		else
		{
			if (World->SaveObject)
			{
				World->SaveObject->Modify();
			}
		}
	}
};