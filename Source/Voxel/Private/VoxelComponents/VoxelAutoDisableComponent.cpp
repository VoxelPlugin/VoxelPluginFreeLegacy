// Copyright 2020 Phyronnaz

#include "VoxelComponents/VoxelAutoDisableComponent.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelWorld.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "Components/PrimitiveComponent.h"

UVoxelAutoDisableComponent::UVoxelAutoDisableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxelAutoDisableComponent::BeginPlay()
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::BeginPlay();

	for (UActorComponent* Component : GetOwner()->GetComponents())
	{
		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
		{
			if (PrimitiveComponent->BodyInstance.bSimulatePhysics)
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
	VOXEL_FUNCTION_COUNTER();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PrimitiveComponentsWithPhysicsEnabled.Num() == 0)
	{
		return;
	}

	bool bShouldSimulatePhysics = false;
	const FVector Position = GetOwner()->GetActorLocation();
	for (TActorIterator<AVoxelWorld> It(GetWorld()); It; ++It)
	{
		auto* World = *It;
		if (World->IsCreated())
		{
			auto LocalPosition = World->GlobalToLocal(Position);
			auto& LODManager = World->GetLODManager();
			if (LODManager.Settings.WorldBounds.Contains(LocalPosition))
			{
				uint8 LOD;
				if (LODManager.AreCollisionsEnabled(LocalPosition, LOD) && LOD <= MaxVoxelChunksLODForPhysics)
				{
					bShouldSimulatePhysics = true;
					break;
				}
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
	VOXEL_FUNCTION_COUNTER();
	
	bWaitingToBeActivated = false;
	bArePhysicsEnabled = true;
	for (auto& Component : PrimitiveComponentsWithPhysicsEnabled)
	{
		Component->SetSimulatePhysics(true);
	}
}