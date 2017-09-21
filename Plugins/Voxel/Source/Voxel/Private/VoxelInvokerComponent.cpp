// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelInvokerComponent.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelChunk.h"
#include "Engine.h"


UVoxelInvokerComponent::UVoxelInvokerComponent() : bAutoFindWorld(true), World(nullptr), DistanceOffset(0)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxelInvokerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
			World->AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent>(this));
		}
	}
}