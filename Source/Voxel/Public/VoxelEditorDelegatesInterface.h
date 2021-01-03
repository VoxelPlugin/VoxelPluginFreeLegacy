// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class VOXEL_API IVoxelEditorDelegatesInterface
{
public:
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_TwoParams(FBindEditorDelegates, IVoxelEditorDelegatesInterface*, UObject*);
	static FBindEditorDelegates BindEditorDelegatesDelegate;

	void BindEditorDelegates(UObject* Self)
	{
		BindEditorDelegatesDelegate.Broadcast(this, Self);
	}
	
	virtual ~IVoxelEditorDelegatesInterface() = default;
	virtual void OnPreSaveWorld(uint32 SaveFlags, UWorld* World) {}
	virtual void OnPreBeginPIE(bool bIsSimulating) {}
	virtual void OnEndPIE(bool bIsSimulating) {}
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) {}
	virtual void OnPreExit() {}
	virtual void OnApplyObjectToActor(UObject* Object, AActor* Actor) {}
#endif
};