// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelInvokerComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine.h"


UVoxelInvokerComponent::UVoxelInvokerComponent() : bNeedUpdate(true), DistanceOffset(0)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVoxelInvokerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bNeedUpdate)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorld::StaticClass(), FoundActors);

		if (FoundActors.Num() == 0)
		{
			UE_LOG(VoxelLog, Warning, TEXT("No world found"));
		}
		else
		{
			for (auto Actor : FoundActors)
			{
				((AVoxelWorld*)Actor)->AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent>(this));
			}
		}

		bNeedUpdate = false;
	}
}