// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/IToolkit.h"

class IVoxelGraphEditorToolkit;
class UVoxelGraphGenerator;
class IToolkitHost;
class UEdGraphNode;

class IVoxelGraphEditorModule : public IModuleInterface
{
public:
	virtual TSharedRef<IVoxelGraphEditorToolkit> CreateVoxelGraphEditor(
		const EToolkitMode::Type Mode, 
		const TSharedPtr<IToolkitHost>& InitToolkitHost, 
		UVoxelGraphGenerator* Generator) = 0;
};