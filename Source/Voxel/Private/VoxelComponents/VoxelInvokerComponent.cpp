// Copyright 2021 Phyronnaz

#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"

bool UVoxelInvokerComponentBase::ShouldUseInvoker(const AVoxelWorld* VoxelWorld) const
{
	return ShouldUseInvoker(const_cast<AVoxelWorld*>(VoxelWorld));
}

FIntVector UVoxelInvokerComponentBase::GetInvokerVoxelPosition(const AVoxelWorld* VoxelWorld) const
{
	return GetInvokerVoxelPosition(const_cast<AVoxelWorld*>(VoxelWorld));
}

FVoxelInvokerSettings UVoxelInvokerComponentBase::GetInvokerSettings(const AVoxelWorld* VoxelWorld) const
{
	return GetInvokerSettings(const_cast<AVoxelWorld*>(VoxelWorld));
}

bool UVoxelInvokerComponentBase::IsLocalInvoker_Implementation() const
{
	auto* Owner = Cast<APawn>(GetOwner());
	return !Owner || Owner->IsLocallyControlled();
}

bool UVoxelInvokerComponentBase::ShouldUseInvoker_Implementation(AVoxelWorld* VoxelWorld) const
{
	return true;
}

FIntVector UVoxelInvokerComponentBase::GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const
{
	return FIntVector(0);
}

FVoxelInvokerSettings UVoxelInvokerComponentBase::GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const
{
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInvokerComponentBase::OnRegister()
{
	Super::OnRegister();
	
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.AddUnique(MakeWeakObjectPtr(this));

	LOG_VOXEL(Log, TEXT("Voxel Invoker registered; Name: %s; Owner: %s"), *GetName(), *GetOwner()->GetName());
}

void UVoxelInvokerComponentBase::OnUnregister()
{
	Super::OnUnregister();
	
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.RemoveAllSwap([this](TWeakObjectPtr<UVoxelInvokerComponentBase> X) { return !X.IsValid() || X == this; });
}

#if WITH_EDITOR
void UVoxelInvokerComponentBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	RefreshAllVoxelInvokers();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInvokerComponentBase::RefreshAllVoxelInvokers()
{
	VOXEL_FUNCTION_COUNTER();
	OnForceRefreshInvokers.Broadcast();
}

TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> UVoxelInvokerComponentBase::GetInvokers(const AVoxelWorld* VoxelWorld)
{
	if (!ensure(VoxelWorld))
	{
		return {};
	}

	auto* World = VoxelWorld->GetWorld();
	if (!ensure(World))
	{
		return {};
	}
	
	const bool bIsEditorWorld =
		World->WorldType == EWorldType::Editor ||
		World->WorldType == EWorldType::EditorPreview;;
	
	auto Result = Components.FindOrAdd(World);
	Result.RemoveAllSwap([&](TWeakObjectPtr<UVoxelInvokerComponentBase> Invoker)
	{
		return
			!Invoker.IsValid() ||
			!Invoker->bIsInvokerEnabled ||
			(Invoker->bEditorOnlyInvoker && !bIsEditorWorld) ||
			!Invoker->ShouldUseInvoker(VoxelWorld);
	});
	return Result;
}

FSimpleMulticastDelegate UVoxelInvokerComponentBase::OnForceRefreshInvokers;
TMap<TWeakObjectPtr<UWorld>, TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>>> UVoxelInvokerComponentBase::Components;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector UVoxelSimpleInvokerComponent::GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const
{
	return VoxelWorld->GlobalToLocal(GetInvokerGlobalPosition());
}

FVoxelInvokerSettings UVoxelSimpleInvokerComponent::GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const
{
	const FVector InvokerGlobalPosition = GetInvokerGlobalPosition();
	const auto GetVoxelBounds = [&](float Distance)
	{
		return FVoxelIntBox::SafeConstruct(
			VoxelWorld->GlobalToLocal(InvokerGlobalPosition - Distance, EVoxelWorldCoordinatesRounding::RoundDown),
			VoxelWorld->GlobalToLocal(InvokerGlobalPosition + Distance, EVoxelWorldCoordinatesRounding::RoundUp)
		);
	};

	FVoxelInvokerSettings Settings;
	
	Settings.bUseForLOD = bUseForLOD;
	Settings.LODToSet = LODToSet;
	Settings.LODBounds = GetVoxelBounds(LODRange);

	Settings.bUseForCollisions = bUseForCollisions;
	Settings.CollisionsBounds = GetVoxelBounds(CollisionsRange);

	Settings.bUseForNavmesh = bUseForNavmesh;
	Settings.NavmeshBounds = GetVoxelBounds(NavmeshRange);

	return Settings;
}

FVector UVoxelSimpleInvokerComponent::GetInvokerGlobalPosition_Implementation() const
{
	return GetComponentLocation();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVector UVoxelInvokerWithPredictionComponent::GetInvokerGlobalPosition_Implementation() const
{
	FVector Position = GetComponentLocation();
	if (bEnablePrediction)
	{
		Position += GetOwner()->GetVelocity() * PredictionTime;
	}
	return Position;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVector UVoxelInvokerAutoCameraComponent::GetInvokerGlobalPosition_Implementation() const
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelVolumeInvokerComponent::UVoxelVolumeInvokerComponent()
{
	bUseForEvents = false;
	bUseForPriorities = false;
}

bool UVoxelVolumeInvokerComponent::IsLocalInvoker_Implementation() const
{
	// Always true for a volume
	return true;
}

FIntVector UVoxelVolumeInvokerComponent::GetInvokerVoxelPosition_Implementation(AVoxelWorld* VoxelWorld) const
{
	return VoxelWorld->GlobalToLocal(GetComponentLocation());
}

FVoxelInvokerSettings UVoxelVolumeInvokerComponent::GetInvokerSettings_Implementation(AVoxelWorld* VoxelWorld) const
{
	auto* Volume = Cast<AVolume>(GetOwner());
	if (!Volume)
	{
		return {};
	}
	
	const FBox VolumeWorldBounds = Volume->GetBounds().GetBox();
	const FVoxelIntBox VolumeBounds = FVoxelIntBox::SafeConstruct(
		VoxelWorld->GlobalToLocal(VolumeWorldBounds.Min, EVoxelWorldCoordinatesRounding::RoundDown),
		VoxelWorld->GlobalToLocal(VolumeWorldBounds.Max, EVoxelWorldCoordinatesRounding::RoundUp)
	);

	FVoxelInvokerSettings Settings;

	Settings.bUseForLOD = bUseForLOD;
	Settings.LODToSet = LODToSet;
	Settings.LODBounds = VolumeBounds;

	Settings.bUseForCollisions = bUseForCollisions;
	Settings.CollisionsBounds = VolumeBounds;

	Settings.bUseForNavmesh = bUseForNavmesh;
	Settings.NavmeshBounds = VolumeBounds;

	return Settings;
}

///////////////////////////////////////////////////////////////////////////////

AVoxelLODVolume::AVoxelLODVolume()
{
	GetBrushComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	GetBrushComponent()->bAlwaysCreatePhysicsState = true;
	GetBrushComponent()->SetMobility(EComponentMobility::Movable);

	InvokerComponent = CreateDefaultSubobject<UVoxelVolumeInvokerComponent>("Invoker Component");
	InvokerComponent->SetupAttachment(RootComponent);
}

#if WITH_EDITOR
void AVoxelLODVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UVoxelInvokerComponentBase::RefreshAllVoxelInvokers();
}
#endif