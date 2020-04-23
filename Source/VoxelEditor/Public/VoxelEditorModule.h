// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/IToolkit.h"

class IToolkitHost;
class UVoxelDataAsset;

class IVoxelEditorModule : public IModuleInterface
{
public:

	virtual void RefreshVoxelWorlds(UObject* MatchingGenerator = nullptr) = 0;
};