// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "IVoxelPool.h"

#include "Engine/Engine.h"
#include "Async/Async.h"

FVoxelLatentActionAsyncWork::FVoxelLatentActionAsyncWork(FName Name)
	: FVoxelAsyncWorkWithWait(Name, 1e9)
{
}

uint32 FVoxelLatentActionAsyncWork::GetPriority() const
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLatentActionAsyncWork_WithWorld::FVoxelLatentActionAsyncWork_WithWorld(
	FName Name,
	TWeakObjectPtr<AVoxelWorld> World,
	TFunction<void(FVoxelData&)> Function)
	: FVoxelLatentActionAsyncWork(Name)
	, World(World)
	, Data(World->GetDataSharedPtr())
	, Function(MoveTemp(Function))
{
}

void FVoxelLatentActionAsyncWork_WithWorld::DoWork()
{
	const auto PinnedData = Data.Pin();
	if (PinnedData.IsValid())
	{
		Function(*PinnedData);
	}
}

bool FVoxelLatentActionAsyncWork_WithWorld::IsValid() const
{
	return World.IsValid() && Data.IsValid();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLatentActionAsyncWork_WithoutWorld::FVoxelLatentActionAsyncWork_WithoutWorld(FName Name, TFunction<void()> Function, TFunction<bool()> IsValidLambda)
	: FVoxelLatentActionAsyncWork(Name)
	, Function(MoveTemp(Function))
	, IsValidLambda(MoveTemp(IsValidLambda))
{
}

void FVoxelLatentActionAsyncWork_WithoutWorld::DoWork()
{
	Function();
}

bool FVoxelLatentActionAsyncWork_WithoutWorld::IsValid() const
{
	return IsValidLambda();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelToolHelpers::UpdateWorld(AVoxelWorld* World, const FVoxelIntBox& Bounds)
{
	check(World);
	World->GetLODManager().UpdateBounds(Bounds);
}

void FVoxelToolHelpers::StartAsyncEditTask(AVoxelWorld* World, IVoxelQueuedWork* Work)
{
	if (World)
	{
		World->GetPool().QueueTask(EVoxelTaskType::AsyncEditFunctions, Work);
	}
	else
	{
		// Should be safe as async tasks should be flushed on close
		AsyncTask(ENamedThreads::AnyThread, [Work]() { Work->DoThreadedWork(); });
	}
}

float FVoxelToolHelpers::GetRealDistance(AVoxelWorld* World, float Distance, bool bConvertToVoxelSpace)
{
	if (bConvertToVoxelSpace)
	{
		return Distance / World->VoxelSize;
	}
	else
	{
		return Distance;
	}
}

FVoxelVector FVoxelToolHelpers::GetRealPosition(AVoxelWorld* World, const FVector& Position, bool bConvertToVoxelSpace)
{
	if (bConvertToVoxelSpace)
	{
		return World->GlobalToLocalFloat(Position);
	}
	else
	{
		return Position;
	}
}

FTransform FVoxelToolHelpers::GetRealTransform(AVoxelWorld* World, FTransform Transform, bool bConvertToVoxelSpace)
{
	if (bConvertToVoxelSpace)
	{
		Transform *= World->GetActorTransform().Inverse();
		Transform.ScaleTranslation(1.f / World->VoxelSize);
	}
	return Transform;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelToolHelpers::StartLatentAction(UObject* WorldContextObject, FLatentActionInfo LatentInfo, FName Name, bool bHideLatentWarnings, TFunction<FPendingLatentAction*()> CreateLatentAction)
{
	if (UWorld* WorldContext = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = WorldContext->GetLatentActionManager();
		if (!LatentActionManager.FindExistingAction<FPendingLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			auto* LatentAction = CreateLatentAction();
			LatentActionManager.AddNewAction(
				LatentInfo.CallbackTarget,
				LatentInfo.UUID,
				LatentAction);
			return true;
		}
		else
		{
			if (!bHideLatentWarnings)
			{
				FVoxelMessages::Info(
					FString::Printf(
						TEXT("%s: task already pending for this node (tick HideLatentWarnings on the node to hide this message)."),
						*Name.ToString()));
			}
			return false;
		}
	}
	else
	{
		FVoxelMessages::Info(
			FString::Printf(
				TEXT("%s: invalid world context object."),
				*Name.ToString()));
		return false;
	}
}

bool FVoxelToolHelpers::StartAsyncLatentAction_WithWorld(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FName Name, 
	bool bHideLatentWarnings, 
	TFunction<void(FVoxelData&)> DoWork, 
	EVoxelUpdateRender UpdateRender, 
	const FVoxelIntBox& BoundsToUpdate)
{
	return StartAsyncLatentActionImpl<FVoxelLatentActionAsyncWork_WithWorld>(
		WorldContextObject,
		LatentInfo,
		World,
		Name,
		bHideLatentWarnings,
		[&]() { return new FVoxelLatentActionAsyncWork_WithWorld(Name, World, DoWork); },
		[=](FVoxelLatentActionAsyncWork_WithWorld& Work)
		{
			if (UpdateRender == EVoxelUpdateRender::UpdateRender && Work.World.IsValid())
			{
				UpdateWorld(Work.World.Get(), BoundsToUpdate);
			}
		});
}

bool FVoxelToolHelpers::StartAsyncLatentAction_WithoutWorld(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	FName Name, 
	bool bHideLatentWarnings, 
	TFunction<void()> DoWork,
	TFunction<bool()> IsValid)
{
	return StartAsyncLatentActionImpl<FVoxelLatentActionAsyncWork_WithoutWorld>(
		WorldContextObject,
		LatentInfo,
		nullptr,
		Name,
		bHideLatentWarnings,
		[&]() { return new FVoxelLatentActionAsyncWork_WithoutWorld(Name, DoWork, MoveTemp(IsValid)); },
		[=](auto&) {});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////