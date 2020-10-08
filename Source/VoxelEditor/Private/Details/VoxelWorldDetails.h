// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class AVoxelWorld;
class SButton;

class FVoxelWorldDetails : public IDetailCustomization
{
public:
	const bool bIsDataAssetEditor;
	
	explicit FVoxelWorldDetails(bool bIsDataAssetEditor = false)
		: bIsDataAssetEditor(bIsDataAssetEditor)
	{
	}

private:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	static void BakeWorld(AVoxelWorld& World);
};
