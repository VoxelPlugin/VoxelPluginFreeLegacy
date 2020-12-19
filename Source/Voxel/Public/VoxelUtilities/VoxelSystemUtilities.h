// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPluginManager.h"

class FVoxelLogTable;

namespace FVoxelSystemUtilities
{
	// Delay until next fire; 0 means "next frame"
	VOXEL_API void DelayedCall(TFunction<void()> Call, float Delay = 0);

	struct FTable
	{
		FString Title;
		TSharedPtr<FVoxelLogTable> Table;
	};
	VOXEL_API void WriteTables(const FString& BaseName, const TArray<FTable>& Tables);

	VOXEL_API IPlugin& GetPlugin();
}