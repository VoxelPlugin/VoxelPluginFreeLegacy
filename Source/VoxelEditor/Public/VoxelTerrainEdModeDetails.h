// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEd/Public/Toolkits/BaseToolkit.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDetailCustomization.h"

class FVoxelTerrainEdModeDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TSharedPtr<FUICommandList> CommandList;

	FVoxelTerrainEdMode* GetEditorMode();
	bool IsToolActive(FName ToolName);
	bool IsToolModeActive(FName ToolModeName);
	void UpdateSculptInfo();
};