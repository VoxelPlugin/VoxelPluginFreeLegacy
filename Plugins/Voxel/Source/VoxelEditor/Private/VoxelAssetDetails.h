// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class AVoxelAsset;

DECLARE_LOG_CATEGORY_EXTERN(VoxelEditorLog, Log, All);

class FVoxelAssetDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();
private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnCreateFromLandscape();
private:
	/** The selected sky light */
	TWeakObjectPtr<AVoxelAsset> VoxelAsset;
};
