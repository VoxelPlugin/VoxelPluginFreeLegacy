// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ClassViewerFilter.h"
#include "Kismet2/KismetEditorUtilities.h"

class FVoxelAssetClassParentFilter : public IClassViewerFilter
{
public:
	FVoxelAssetClassParentFilter() = default;

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet<const UClass*> AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags = CLASS_None;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase = false;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		const bool bAllowed =
			!InClass->HasAnyClassFlags(DisallowedClassFlags) &&
			InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return
			!InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) &&
			InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};
