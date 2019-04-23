// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"

namespace FVoxelBlueprintErrorsEditor
{
	void LogError(const TSharedRef<FTokenizedMessage>& Message);
}