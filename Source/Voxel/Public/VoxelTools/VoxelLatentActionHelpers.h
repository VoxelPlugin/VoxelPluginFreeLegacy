// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "LatentActions.h"
#include "Engine/LatentActionManager.h"
#include "VoxelWorld.h"

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

template<class TWork>
class FVoxelLatentAction : public FPendingLatentAction
{
public:
	TSharedPtr<TWork> Work;
	TFunction<void(TWork&)> Callback;
	FString const Description;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FVoxelLatentAction(TSharedPtr<TWork> Work, const FLatentActionInfo& LatentInfo, FString Description, TFunction<void(TWork&)> Callback)
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

