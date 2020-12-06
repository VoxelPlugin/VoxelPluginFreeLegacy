// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FVoxelMeshSpawnerDetails : public IDetailCustomization
{
public:
	FVoxelMeshSpawnerDetails() = default;

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};