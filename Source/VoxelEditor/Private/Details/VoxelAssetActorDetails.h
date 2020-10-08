// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"
#include "Widgets/Input/SButton.h"

class AVoxelAssetActor;;

class FVoxelAssetActorDetails : public IDetailCustomization
{
public:
	FVoxelAssetActorDetails() = default;

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
	TWeakObjectPtr<AVoxelAssetActor> AssetActor;
};