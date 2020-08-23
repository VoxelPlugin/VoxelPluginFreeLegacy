// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VoxelEditorDelegatesInterface.generated.h"

UINTERFACE(BlueprintType)
class VOXEL_API UVoxelEditorDelegatesInterface : public UInterface
{
	GENERATED_BODY()
};

class VOXEL_API IVoxelEditorDelegatesInterface : public IInterface
{
	GENERATED_BODY()
	
public:
#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_TwoParams(FBindEditorDelegates, IVoxelEditorDelegatesInterface*, UObject*);
	static FBindEditorDelegates BindEditorDelegatesDelegate;

	void BindEditorDelegates(UObject* Self)
	{
		BindEditorDelegatesDelegate.Broadcast(this, Self);
	}
	
	virtual void OnPreSaveWorld(uint32 SaveFlags, UWorld* World) {}
	virtual void OnPreBeginPIE(bool bIsSimulating) {}
	virtual void OnEndPIE(bool bIsSimulating) {}
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) {}
	virtual void OnPreExit() {}
	virtual void OnApplyObjectToActor(UObject* Object, AActor* Actor) {}
#endif
};