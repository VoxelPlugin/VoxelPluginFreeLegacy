// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

// See sky light details in the engine code

class AVoxelLandscapeImporter;

class FVoxelLandscapeImporterDetails : public IDetailCustomization
{
public:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnCreateFromLandscape();

private:
	/** The selected landscape modifier */
	TWeakObjectPtr<AVoxelLandscapeImporter> Importer;
};