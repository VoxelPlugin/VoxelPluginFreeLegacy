// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "IVoxelPool.h"
#include "VoxelRender/IVoxelLODManager.h"

#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "Engine/Engine.h"
#include "Misc/QueuedThreadPool.h"
#include "Misc/IQueuedWork.h"

class FVoxelData;

class FVoxelLatentActionAsyncWork : public IQueuedWork
{
public:
	// In
	TWeakObjectPtr<AVoxelWorld> const World;
	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> const Data;

	FVoxelLatentActionAsyncWork(TWeakObjectPtr<AVoxelWorld> World)
		: World(World)
		, Data(World->GetDataSharedPtr())
	{
		DoneEvent = FPlatformProcess::GetSynchEventFromPool(true);
		DoneEvent->Reset();
	}
	virtual ~FVoxelLatentActionAsyncWork() override
	{
		if (DoneEvent)
		{
			FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
			DoneEvent = nullptr;
		}
	}

	//~ Begin IQueuedWork Interface
	void DoThreadedWork() final
	{
		DoWork();
		FScopeLock Lock(&DoneSection);
		IsDoneCounter.Increment();
		DoneEvent->Trigger();
	}
	void Abandon() final
	{
		FScopeLock Lock(&DoneSection);
		IsDoneCounter.Increment();
		DoneEvent->Trigger();
	}
	//~ End IQueuedWork Interface

	inline bool IsDone() const { return IsDoneCounter.GetValue() > 0; }
	inline void WaitForCompletion()
	{
		DoneEvent->Wait();
		FScopeLock Lock(&DoneSection);
	}

	virtual void DoWork() = 0;

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;
};

class FVoxelLatentActionAsyncWorkLambda : public FVoxelLatentActionAsyncWork
{
public:
	TFunction<void(FVoxelData&)> Function;
	FVoxelLatentActionAsyncWorkLambda(TWeakObjectPtr<AVoxelWorld> World, TFunction<void(FVoxelData&)> Function)
		: FVoxelLatentActionAsyncWork(World)
		, Function(Function)
	{
	}

	virtual void DoWork() override
	{
		Function(*Data);
	}
};

template<typename TValue>
class FVoxelLatentActionAsyncWorkLambdaWithValue : public FVoxelLatentActionAsyncWork
{
public:
	TValue Value;
	TFunction<void(FVoxelData&, TValue&)> Function;
	FVoxelLatentActionAsyncWorkLambdaWithValue(TWeakObjectPtr<AVoxelWorld> World, TFunction<void(FVoxelData&, TValue&)> Function)
		: FVoxelLatentActionAsyncWork(World)
		, Function(Function)
	{
	}

	virtual void DoWork() override
	{
		Function(*Data, Value);
	}
};

template<class TWork>
class FVoxelLatentAction : public FPendingLatentAction
{
public:
	TSharedPtr<TWork> Work;
	TFunction<void(TWork&)> Callback;
	FString const Description;
	FName const ExecutionFunction;
	int32 const OutputLink;
	FWeakObjectPtr const CallbackTarget;

	FVoxelLatentAction(TSharedPtr<TWork> Work, const FLatentActionInfo& LatentInfo, const FString& Description, TFunction<void(TWork&)> Callback)
		: Work(Work)
		, Callback(Callback)
		, Description(Description)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}
	virtual ~FVoxelLatentAction() override
	{
		Work->WaitForCompletion();
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		bool bFinished = Work->IsDone();
		if (bFinished && Work->World.IsValid())
		{
			Callback(*Work);
		}
		Response.FinishAndTriggerIf(bFinished, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return Description;
	}
#endif
};

namespace FVoxelLatentActionHelpers
{
	template<typename TWork, typename T1, typename T2>
	inline bool StartAction(UObject* WorldContextObject, FLatentActionInfo LatentInfo, AVoxelWorld* World, const FString& Description, T1 CreateWork, T2 FinishWork)
	{
		if (UWorld* WorldContext = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
		{
			FLatentActionManager& LatentActionManager = WorldContext->GetLatentActionManager();
			if (!LatentActionManager.FindExistingAction<FVoxelLatentAction<TWork>>(LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				TSharedPtr<TWork> Task = CreateWork();

				World->GetPool().QueueAsyncEditTask(Task.Get());

				LatentActionManager.AddNewAction(
					LatentInfo.CallbackTarget,
					LatentInfo.UUID,
					new FVoxelLatentAction<TWork>(Task, LatentInfo, Description, FinishWork));
				return true;
			}
		}
		return false;
	}
	
	template<typename TLambda, typename TBounds>
	inline bool AsyncHelper(
		UObject* WorldContextObject, 
		FLatentActionInfo LatentInfo,
		const FString& Name, 
		AVoxelWorld* World, 
		const TBounds& Bounds, 
		bool bUpdateRender, 
		TLambda Lambda)
	{
		return FVoxelLatentActionHelpers::StartAction<FVoxelLatentActionAsyncWorkLambda>(
			WorldContextObject, 
			LatentInfo, 
			World, 
			Name + ": Waiting for completion", 
			[&]()
		{
			return MakeShared<FVoxelLatentActionAsyncWorkLambda>(World, Lambda);
		},
			[bUpdateRender, Bounds](FVoxelLatentActionAsyncWorkLambda& Work)
		{
			if (bUpdateRender)
			{
				Work.World->GetLODManager().UpdateBounds(Bounds, true);
			}
		});
	}

	template<typename TLambda, typename TValue, typename TBounds = FIntBox>
	inline bool AsyncHelperWithValue(
		UObject* WorldContextObject, 
		FLatentActionInfo LatentInfo,
		const FString& Name, 
		AVoxelWorld* World, 
		TValue& Value,
		TLambda Lambda,
		bool bUpdateRender = false,
		const TBounds& Bounds = TBounds())
	{
		return FVoxelLatentActionHelpers::StartAction<FVoxelLatentActionAsyncWorkLambdaWithValue<TValue>>(
			WorldContextObject, 
			LatentInfo, 
			World, 
			Name + ": Waiting for completion", 
			[&]()
		{
			return MakeShared<FVoxelLatentActionAsyncWorkLambdaWithValue<TValue>>(World, Lambda);
		},
			[&Value, bUpdateRender, Bounds, WeakPtr = TWeakObjectPtr<UObject>(WorldContextObject)](FVoxelLatentActionAsyncWorkLambdaWithValue<TValue>& Work)
		{
			if (WeakPtr.IsValid())
			{
				Value = MoveTemp(Work.Value);
			}
			if (bUpdateRender)
			{
				Work.World->GetLODManager().UpdateBounds(Bounds, true);
			}
		});
	}
}

