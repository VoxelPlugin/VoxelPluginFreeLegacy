// Copyright 2018 Phyronnaz

#include "VoxelInvokerComponent.h"
#include "VoxelPrivate.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelInvokerComponent::Tick"), STAT_FVoxelInvokerComponent_Tick, STATGROUP_Voxel);

UVoxelInvokerComponent::UVoxelInvokerComponent()
	: DistanceOffset(1000)
	, bDebugMultiplayer(false)
	, bUseForCollisions(true)
	, bUseForLOD(true)
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UVoxelInvokerComponent::UseForCollisions()
{
	return bUseForCollisions;
}

bool UVoxelInvokerComponent::UseForRender()
{
	return bUseForLOD && (!Cast<APawn>(GetOwner()) || Cast<APawn>(GetOwner())->IsLocallyControlled());
}

FVector UVoxelInvokerComponent::GetPosition() const
{
	return GetOwner()->GetActorLocation();
}

FIntBox UVoxelInvokerComponent::GetCameraBounds(const AVoxelWorld* World) const
{
	const FIntVector LocalPosition = World->GlobalToLocal(GetPosition());
	float D = DistanceOffset / World->GetVoxelSize();
	FIntVector FD(D, D, D);
	return FIntBox(LocalPosition - FD, LocalPosition + FD);
}

void UVoxelInvokerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelInvokerComponent_Tick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDebugMultiplayer && Cast<APawn>(GetOwner()) && Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		DrawDebugPoint(GetWorld(), GetOwner()->GetActorLocation(), 100, FColor::Red, false, DeltaTime * 1.1f, 0);
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass((UObject*)GetOwner()->GetWorld(), AVoxelWorld::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		UE_LOG(LogVoxel, Warning, TEXT("No world found"));
	}
	else
	{
		for (auto Actor : FoundActors)
		{
			auto World = CastChecked<AVoxelWorld>(Actor);
			if (!AlreadyProcessedWorlds.Contains(World) && World->IsCreated())
			{
				World->AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent>(this));
				AlreadyProcessedWorlds.Add(World);
			}
		}
	}
}