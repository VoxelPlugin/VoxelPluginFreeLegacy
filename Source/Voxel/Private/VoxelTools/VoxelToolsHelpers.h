// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelRender/IVoxelLODManager.h"

#include "Logging/MessageLog.h"
#include "Misc/CoreMisc.h"
#include "UObject/Stack.h"

// TODO: Proper thing instead of AccessViolation
inline void VoxelLogBlueprintError(const FText& Error)
{
#if WITH_EDITOR
	FBlueprintExceptionTracker& BlueprintExceptionTracker = FBlueprintExceptionTracker::Get();
	if (BlueprintExceptionTracker.ScriptStack.Num() > 0)
	{
		FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, Error);
		FBlueprintCoreDelegates::ThrowScriptException(nullptr, *BlueprintExceptionTracker.ScriptStack.Last(), ExceptionInfo);
	}
	else
#endif
	{
		FMessageLog("PIE").Error(Error);
	}
}

#define CHECK_VOXELWORLD_IS_CREATED_IMPL(Name, ReturnValue) \
if (!World) \
{ \
	VoxelLogBlueprintError(FText::Format(NSLOCTEXT("Voxel", "VoxelWorldIsInvalid", "{0}: Voxel World is invalid!"), FText::FromString(FString(Name)))); \
	return ReturnValue; \
} \
if (!World->IsCreated()) \
{ \
	VoxelLogBlueprintError(FText::Format(NSLOCTEXT("Voxel", "VoxelWorldIsntCreated", "{0}: Voxel World isn't created!"), FText::FromString(FString(Name)))); \
	return ReturnValue; \
}
#define CHECK_VOXELWORLD_IS_CREATED() CHECK_VOXELWORLD_IS_CREATED_IMPL(__FUNCTION__, {});
#define CHECK_VOXELWORLD_IS_CREATED_VOID() CHECK_VOXELWORLD_IS_CREATED_IMPL(__FUNCTION__,);

namespace FVoxelToolsHelpers
{
	template<EVoxelLockType LockType, typename TLambda>
	inline bool EditToolsHelper(const FString& Name, AVoxelWorld* World, const FIntBox& Bounds, bool bUpdateRender, bool bAllowFailure, TLambda Lambda)
	{
		FVoxelData& Data = World->GetData();
		{
			FVoxelScopeTryLock<LockType> Lock(Data, Bounds, bAllowFailure ? EDIT_TOOLS_LOCK_TIMEOUT : 1e9, Name);
			if (!Lock.Success())
			{
				const FString LockerName = Lock.GetLockerName();
				World->GetDebugManager().ReportToolFailure(Name, LockerName == "Timeout!" ? "Timeout!" : ("Data locked by " + LockerName));
				return false;
			}
			Lambda(Data);
		}
		if (bUpdateRender)
		{
			World->GetLODManager().UpdateBounds(Bounds, true);
		}
		return true;
	}
}