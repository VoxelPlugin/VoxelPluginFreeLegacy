// Copyright 2019 Phyronnaz

#include "VoxelComponents/VoxelAutoDisableComponent.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelWorld.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

DECLARE_CYCLE_STAT(TEXT("VoxelAutoDisableComponent::Tick"), STAT_VoxelAutoDisableComponent_Tick, STATGROUP_Voxel);

UVoxelAutoDisableComponent::UVoxelAutoDisableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UVoxelAutoDisableComponent::BeginPlay()
{
	Super::BeginPlay();

	for (UActorComponent* Component : GetOwner()->GetComponents())
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
		{
			if (PrimitiveComponent->IsSimulatingPhysics())
			{
				PrimitiveComponentsWithPhysicsEnabled.Add(PrimitiveComponent);
			}
		}
	}

	if (PrimitiveComponentsWithPhysicsEnabled.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}

	SetComponentTickInterval(TickInterval);
}


void UVoxelAutoDisableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelAutoDisableComponent_Tick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PrimitiveComponentsWithPhysicsEnabled.Num() == 0)
	{
		return;
	}

	bool bShouldSimulatePhysics = false;
	FVector Position = GetOwner()->GetActorLocation();
	for (TActorIterator<AVoxelWorld> It(GetWorld()); It; ++It)
	{
		auto* World = *It;
		auto LocalPosition = World->GlobalToLocal(Position);
		auto& LODManager = World->GetLODManager();
		if (LODManager.Settings.WorldBounds.IsInside(LocalPosition))
		{
			uint8 LOD;
			if (LODManager.AreCollisionsEnabled(LocalPosition, LOD) && LOD <= MaxVoxelChunksLODForPhysics)
			{
				bShouldSimulatePhysics = true;
				break;
			}
		}
	}
	if (bArePhysicsEnabled != bShouldSimulatePhysics)
	{
		if (bShouldSimulatePhysics)
		{
			if (!bWaitingToBeActivated)
			{
				auto& TimerManager = GetWorld()->GetTimerManager();
				TimerManager.SetTimer(Handle, this, &UVoxelAutoDisableComponent::ActivatePhysics, TimeToWaitBeforeActivating);
				bWaitingToBeActivated = true;
			}
		}
		else
		{
			if (bWaitingToBeActivated)
			{
				auto& TimerManager = GetWorld()->GetTimerManager();
				TimerManager.ClearTimer(Handle);
				bWaitingToBeActivated = false;
			}
			bArePhysicsEnabled = false;
			for (auto& Component : PrimitiveComponentsWithPhysicsEnabled)
			{
				Component->SetSimulatePhysics(false);
			}
		}
	}
}

void UVoxelAutoDisableComponent::ActivatePhysics()
{
	bWaitingToBeActivated = false;
	bArePhysicsEnabled = true;
	for (auto& Component : PrimitiveComponentsWithPhysicsEnabled)
	{
		Component->SetSimulatePhysics(true);
	}
}