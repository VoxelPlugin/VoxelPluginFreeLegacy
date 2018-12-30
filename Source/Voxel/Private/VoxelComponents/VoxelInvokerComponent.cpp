// Copyright 2018 Phyronnaz

#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelInvokerComponent::Tick"), STAT_FVoxelInvokerComponent_Tick, STATGROUP_Voxel);

bool UVoxelInvokerComponent::IsLocalInvoker()
{
	auto* Owner = Cast<APawn>(GetOwner());
	return !Owner || Owner->IsLocallyControlled() || Owner->GetWorld()->IsServer();
}

FVector UVoxelInvokerComponent::GetPosition() const
{
	return GetOwner()->GetActorLocation();
}

void UVoxelInvokerComponent::OnRegister()
{
	Super::OnRegister();

	auto& Array = Components.FindOrAdd(GetWorld());
	Array.AddUnique(TWeakObjectPtr<UVoxelInvokerComponent>(this));
	Array.RemoveAll([](auto& X) { return !X.IsValid(); });

	ComponentChangeVersion++;

	UE_LOG(LogVoxel, Log, TEXT("Voxel Invoker registered. Owner: %s"), *GetOwner()->GetName());
}

TMap<UWorld*, TArray<TWeakObjectPtr<UVoxelInvokerComponent>>> UVoxelInvokerComponent::Components;
int UVoxelInvokerComponent::ComponentChangeVersion = 0;
