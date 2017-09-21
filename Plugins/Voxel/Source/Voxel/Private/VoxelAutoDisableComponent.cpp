// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelAutoDisableComponent.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelChunk.h"
#include "Engine.h"

DECLARE_CYCLE_STAT(TEXT("VoxelAutoDisableComponent ~ Tick"), STAT_VOXELAUTODISABLE_TICK, STATGROUP_Voxel);

UVoxelAutoDisableComponent::UVoxelAutoDisableComponent() : Component(nullptr), bSimulatePhysics(false), bAutoFindWorld(true), World(nullptr), CullDepth(4)
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UVoxelAutoDisableComponent::BeginPlay()
{
	Super::BeginPlay();

	Component = Cast<UPrimitiveComponent>(GetOwner()->GetComponentByClass(TSubclassOf<UActorComponent>(ComponentClass)));

	if (!Component)
	{
		UE_LOG(VoxelLog, Error, TEXT("VoxelAutoDisableComponent: Invalid component class"));
	}
	else
	{
		bSimulatePhysics = Component->IsSimulatingPhysics();
	}
}


void UVoxelAutoDisableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_VOXELAUTODISABLE_TICK)

		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Component)
	{
		if (!World)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorld::StaticClass(), FoundActors);

			if (FoundActors.Num() == 0)
			{
				UE_LOG(VoxelLog, Warning, TEXT("No world found"));
			}
			else
			{
				if (FoundActors.Num() > 1)
				{
					UE_LOG(VoxelLog, Warning, TEXT("More than one world found"));
				}
				World = (AVoxelWorld*)FoundActors[0];
			}
		}

		if (World && World->IsCreated())
		{
			FIntVector LocalPosition = World->GlobalToLocal(GetOwner()->GetActorLocation());
			if (World->IsInWorld(LocalPosition))
			{
				AVoxelChunk* Chunk = World->GetChunkAt(LocalPosition);
				if (bSimulatePhysics)
				{
					Component->SetSimulatePhysics(Chunk->GetDepth() == 0);
				}

				Component->SetVisibility(Chunk->GetDepth() <= CullDepth, true);
			}
		}
	}
}

