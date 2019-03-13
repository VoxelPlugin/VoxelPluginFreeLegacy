// Copyright 2019 Phyronnaz

#include "VoxelComponents/VoxelAutoDisableComponent.h"
#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/MessageLog.h"
#include "EngineUtils.h"

DECLARE_CYCLE_STAT(TEXT("VoxelAutoDisableComponent::Tick"), STAT_VoxelAutoDisable_Tick, STATGROUP_Voxel);

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
				PrimitiveComponents.Add(PrimitiveComponent);
			}
		}
	}

	if (!bAutoFindWorld)
	{
		ensure(InGameWorlds.Num() == 0);
		InGameWorlds.Add(DefaultWorld);
	}
}


void UVoxelAutoDisableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelAutoDisable_Tick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (PrimitiveComponents.Num())
	{
		InGameWorlds.RemoveAll([](auto& X) { return !X.IsValid(); });

		if (bAutoFindWorld && InGameWorlds.Num() == 0)
		{
			for (TActorIterator<AVoxelWorld> It(GetWorld()); It; ++It)
			{
				if (IsValid(*It))
				{
					InGameWorlds.Add(*It);
				}
			}

			if (InGameWorlds.Num() == 0)
			{
				FMessageLog("PIE").Error(FText::FromString("VoxelAutoDisableComponent: No world found (actor: " + GetOwner()->GetName() + ")"));
			}
		}

		if (InGameWorlds.Num() > 0)
		{
			FVector Position = GetOwner()->GetActorLocation();
			int MinLOD = 255;
			for (auto& GameWorld : InGameWorlds)
			{
				FIntVector LocalPosition = GameWorld->GlobalToLocal(Position);
				/*if (GameWorld->GetData().IsInWorld(LocalPosition)) TODO
				{
					MinLOD = FMath::Min(MinLOD, GameWorld->GetLODAt(LocalPosition));
				}*/
			}
			bool bShouldSimulatePhysics = MinLOD <= MaxLODForPhysics;
			if (bPhysicsEnabled != bShouldSimulatePhysics)
			{
				if (bShouldSimulatePhysics && TimeUntilActivation > 0)
				{
					TimeUntilActivation -= DeltaTime;
				}
				else
				{
					if (!bShouldSimulatePhysics)
					{
						TimeUntilActivation = TimeToWaitBeforeActivating;
					}
					bPhysicsEnabled = bShouldSimulatePhysics;
					for (auto& Component : PrimitiveComponents)
					{
						Component->SetSimulatePhysics(bPhysicsEnabled);
					}
				}
			}
		}
	}
}