// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class AVoxelWorld;
class SButton;

class FVoxelWorldDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	FVoxelWorldDetails();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnWorldPreviewToggle();
	FReply OnLoad();
	FReply OnSave();
	FReply OnSaveToFile();
	FReply OnLoadFromFile();

private:
	TWeakObjectPtr<AVoxelWorld> World;

	TSharedPtr<SButton> SaveButton;
	TSharedPtr<SButton> LoadButton;
	TSharedPtr<SButton> SaveFileButton;
	TSharedPtr<SButton> LoadFileButton;

	int LastSaveHistoryPosition;
};
