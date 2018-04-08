// Copyright 2018 Phyronnaz

#include "VoxelAutoDisableComponent.h"
#include "VoxelPrivate.h"
#include "VoxelWorld.h"
#include "Kismet/GameplayStatics.h"

DECLARE_CYCLE_STAT(TEXT("VoxelAutoDisableComponent ~ Tick"), STAT_VoxelAutoDisable_Tick, STATGROUP_Voxel);

UVoxelAutoDisableComponent::UVoxelAutoDisableComponent()
	: bAutoFindWorld(true)
	, World(nullptr)
	, MaxLODForCollisions(0)
	, TimeToWaitBeforeActivating(10)
	, TimeUntilActivation(-1)
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UVoxelAutoDisableComponent::BeginPlay()
{
	Super::BeginPlay();

	for (UActorComponent* Component : GetOwner()->GetComponents())
	{
		if (Component && Component->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
		{
			UPrimitiveComponent* PrimitiveComponent = CastChecked<UPrimitiveComponent>(Component);
			if (PrimitiveComponent->IsSimulatingPhysics())
			{
				Components.Add(PrimitiveComponent);
			}
		}
	}
}


void UVoxelAutoDisableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelAutoDisable_Tick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Components.Num())
	{
		if (!World && bAutoFindWorld)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass((UObject*)GetOwner()->GetWorld(), AVoxelWorld::StaticClass(), FoundActors);

			if (FoundActors.Num() == 0)
			{
				UE_LOG(LogVoxel, Warning, TEXT("No world found"));
			}
			else
			{
				if (FoundActors.Num() > 1)
				{
					UE_LOG(LogVoxel, Warning, TEXT("More than one world found"));
				}
				World = (AVoxelWorld*)FoundActors[0];
			}
		}

		if (World)
		{
			FIntVector LocalPosition = World->GlobalToLocal(GetOwner()->GetActorLocation());
			if (World->IsInWorld(LocalPosition))
			{
				const int LOD = World->GetLODAt(LocalPosition);				
				bool bShouldSimulatePhysics = LOD <= FMath::Min<int>(MaxLODForCollisions, World->GetMaxCollisionsLOD());
				if (bShouldSimulatePhysics)
				{
					if (TimeUntilActivation > 0)
					{
						TimeUntilActivation -= DeltaTime;
						if (TimeUntilActivation <= 0)
						{
							for (auto Component : Components)
							{
								Component->SetSimulatePhysics(true);
							}
						}
					}
					else
					{
						TimeUntilActivation = TimeToWaitBeforeActivating;
					}
				}
				else
				{
					TimeUntilActivation = -1;
					for (auto Component : Components)
					{
						Component->SetSimulatePhysics(false);
					}
				}
			}
		}
	}
}

