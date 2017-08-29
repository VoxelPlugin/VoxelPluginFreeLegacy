// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VoxelFactoryNew.h"

#include "VoxelAsset.h"


/* UVoxelFactoryNew structors
 *****************************************************************************/

UVoxelFactoryNew::UVoxelFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = AVoxelAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* UVoxelFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<AVoxelAsset>(InParent, InClass, InName, Flags);
}


bool UVoxelFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
