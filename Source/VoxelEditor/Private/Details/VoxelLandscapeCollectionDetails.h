// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FVoxelLandscapeCollectionDetails : public IDetailCustomization
{
public:
	FVoxelLandscapeCollectionDetails() = default;

private:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};
