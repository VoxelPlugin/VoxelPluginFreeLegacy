// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class AVoxelWorld;
class AVoxelWorldEditor;

DECLARE_LOG_CATEGORY_EXTERN(VoxelEditorLog, Log, All);
class FVoxelWorldDetails : public IDetailCustomization
{
public:
	FVoxelWorldDetails();

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

	FReply OnWorldToggle();

	FReply OnUpdateVoxelModifiers();

private:
	TWeakObjectPtr<AVoxelWorld> World;
	TWeakObjectPtr<AVoxelWorldEditor> EditorWorld;
};
