// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"

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
			if (World->GetSaveObject())
			{
				World->GetSaveObject()->Modify();
			}
		}
	}
};