// Copyright 2019 Phyronnaz

#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorld.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

TMap<UWorld*, TArray<TWeakObjectPtr<UVoxelInvokerComponent>>> UVoxelInvokerComponent::Components;

bool UVoxelInvokerComponent::IsLocalInvoker()
{
	auto* Owner = Cast<APawn>(GetOwner());
	return !Owner || Owner->IsLocallyControlled() || Owner->GetWorld()->IsServer();
}

FVector UVoxelInvokerComponent::GetPosition() const
{
	FVector Position = GetComponentLocation();
	if (bEnablePrediction)
	{
		Position += GetOwner()->GetVelocity() * PredictionTime;
	}
	return Position;
}

const TArray<TWeakObjectPtr<UVoxelInvokerComponent>>& UVoxelInvokerComponent::GetInvokers(UWorld* World)
{
	auto* Result = Components.Find(World);
	if (Result)
	{
		Result->RemoveAll([](auto& X) { return !X.IsValid(); });
		return *Result;
	}
	else
	{
		static const TArray<TWeakObjectPtr<UVoxelInvokerComponent>> EmptyArray;
		return EmptyArray;
	}
}

void UVoxelInvokerComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInvoker();
}

void UVoxelInvokerComponent::OnUnregister()
{
	Super::OnUnregister();
	UnregisterInvoker();
}

void UVoxelInvokerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	RegisterInvoker();
}

void UVoxelInvokerComponent::Deactivate()
{
	Super::Deactivate();
	UnregisterInvoker();
}

void UVoxelInvokerComponent::RegisterInvoker()
{
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.AddUnique(TWeakObjectPtr<UVoxelInvokerComponent>(this));
	Array.RemoveAll([](auto& X) { return !X.IsValid(); });

	UE_LOG(LogVoxel, Log, TEXT("Voxel Invoker registered. Owner: %s"), *GetOwner()->GetName());
}

void UVoxelInvokerComponent::UnregisterInvoker()
{
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.RemoveAll([this](auto& X) { return !X.IsValid() || X == this; });

	UE_LOG(LogVoxel, Log, TEXT("Voxel Invoker unregistered"));
}

///////////////////////////////////////////////////////////////////////////////

UVoxelInvokerAutoCameraComponent::UVoxelInvokerAutoCameraComponent()
{
	bUseForLODs = true;
	bUseForCollisions = false;
	bUseForNavmesh = false;
}

FVector UVoxelInvokerAutoCameraComponent::GetPosition() const
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (ensure(CameraManager))
	{
		return CameraManager->GetCameraLocation();
	}
	else
	{
		return FVector::ZeroVector;
	}
}