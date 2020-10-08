// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class AVoxelMeshImporter;

// See sky light details in the engine code

class FVoxelMeshImporterDetails : public IDetailCustomization
{
public:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnCreateFromMesh();

private:
	TWeakObjectPtr<AVoxelMeshImporter> MeshImporter;
};