// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/Actors/VoxelDataItemActor.h"
#include "VoxelMinimal.h"

#include "Engine/World.h"
#include "TimerManager.h"

AVoxelDataItemActor::AVoxelDataItemActor()
{
#if WITH_EDITOR
	static bool bAddedDelegate = false;
	if (!bAddedDelegate)
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
		{
			if (!PropertyChangedEvent.Property ||
				PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
			{
				return;
			}
			auto* Component = Cast<UActorComponent>(Object);
			if (!Component)
			{
				return;
			}
			auto* DataItemActor = Cast<AVoxelDataItemActor>(Component->GetOwner());
			if (!DataItemActor)
			{
				return;
			}

			DataItemActor->ScheduleRefresh();
		});
	}
#endif
}

void AVoxelDataItemActor::ScheduleRefresh()
{
	if (RefreshDelay <= 0)
	{
		OnRefresh.Broadcast();
	}
	else
	{
		if (auto* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(RefreshTimerHandle, MakeWeakObjectPtrDelegate(this, [=]()
			{
				OnRefresh.Broadcast();
			}), RefreshDelay, false);
		}	
	}
}

#if WITH_EDITOR
void AVoxelDataItemActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Note: PropertyChangedEvent.Property will be null when undoing
	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		ScheduleRefresh();
	}
}

void AVoxelDataItemActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	ScheduleRefresh();
}
#endif

void AVoxelDataItemActor::Destroyed()
{
	OnRefresh.Broadcast();
	
	Super::Destroyed();
}