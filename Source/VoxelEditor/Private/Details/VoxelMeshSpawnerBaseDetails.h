// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"
#include "Widgets/Input/SButton.h"

class FVoxelMeshSpawnerBaseDetails : public IDetailCustomization
{
public:
	FVoxelMeshSpawnerBaseDetails() = default;

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};