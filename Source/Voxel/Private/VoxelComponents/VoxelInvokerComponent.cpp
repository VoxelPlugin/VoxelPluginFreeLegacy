// Copyright 2020 Phyronnaz

#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorldInterface.h"
#include "VoxelMessages.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BrushComponent.h"

FIntVector UVoxelInvokerComponentBase::GetInvokerVoxelPosition(const AVoxelWorldInterface* VoxelWorld) const
{
	return GetInvokerVoxelPosition(const_cast<AVoxelWorldInterface*>(VoxelWorld));
}

FVoxelInvokerSettings UVoxelInvokerComponentBase::GetInvokerSettings(const AVoxelWorldInterface* VoxelWorld) const
{
	return GetInvokerSettings(const_cast<AVoxelWorldInterface*>(VoxelWorld));
}

bool UVoxelInvokerComponentBase::IsLocalInvoker_Implementation() const
{
	auto* Owner = Cast<APawn>(GetOwner());
	return !Owner || Owner->IsLocallyControlled();
}

FIntVector UVoxelInvokerComponentBase::GetInvokerVoxelPosition_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	return FIntVector(0);
}

FVoxelInvokerSettings UVoxelInvokerComponentBase::GetInvokerSettings_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInvokerComponentBase::EnableInvoker()
{
	if (bIsInvokerEnabled)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invoker already enabled"), this);
		return;
	}
	bIsInvokerEnabled = true;
	
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.AddUnique(MakeWeakObjectPtr(this));
	Array.RemoveAllSwap([](auto& X) { return !X.IsValid(); });

	LOG_VOXEL(Log, TEXT("Voxel Invoker enabled; Name: %s; Owner: %s"), *GetName(), *GetOwner()->GetName());
}

void UVoxelInvokerComponentBase::DisableInvoker()
{
	if (!bIsInvokerEnabled)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invoker already disabled"), this);
		return;
	}
	bIsInvokerEnabled = false;
	
	auto& Array = Components.FindOrAdd(GetWorld());
	Array.RemoveAllSwap([this](auto& X) { return !X.IsValid() || X == this; });

	LOG_VOXEL(Log, TEXT("Voxel Invoker disabled; Name: %s"), *GetName());
}

bool UVoxelInvokerComponentBase::IsInvokerEnabled() const
{
	return bIsInvokerEnabled;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInvokerComponentBase::OnRegister()
{
	Super::OnRegister();
	if (bStartsEnabled && ensure(!bIsInvokerEnabled))
	{
		EnableInvoker();
	}
}

void UVoxelInvokerComponentBase::OnUnregister()
{
	Super::OnUnregister();
	if (bIsInvokerEnabled)
	{
		DisableInvoker();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelInvokerComponentBase::RefreshAllVoxelInvokers()
{
	VOXEL_FUNCTION_COUNTER();
	OnForceRefreshInvokers.Broadcast();
}

const TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>>& UVoxelInvokerComponentBase::GetInvokers(UWorld* World)
{
	auto& Result = Components.FindOrAdd(World);
	Result.RemoveAllSwap([](auto& X) { return !X.IsValid(); });
	return Result;
}

FSimpleMulticastDelegate UVoxelInvokerComponentBase::OnForceRefreshInvokers;
TMap<TWeakObjectPtr<UWorld>, TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>>> UVoxelInvokerComponentBase::Components;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector UVoxelSimpleInvokerComponent::GetInvokerVoxelPosition_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	return VoxelWorld->GlobalToLocal(GetInvokerGlobalPosition());
}

FVoxelInvokerSettings UVoxelSimpleInvokerComponent::GetInvokerSettings_Implementation(AVoxelWorldInterface* VoxelWorld) const
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

UVoxelLODVolumeInvokerComponent::UVoxelLODVolumeInvokerComponent()
{
	bUseForEvents = false;
	bUseForPriorities = false;
}

bool UVoxelLODVolumeInvokerComponent::IsLocalInvoker_Implementation() const
{
	// Always true for a volume
	return true;
}

FIntVector UVoxelLODVolumeInvokerComponent::GetInvokerVoxelPosition_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	return VoxelWorld->GlobalToLocal(GetComponentLocation());
}

FVoxelInvokerSettings UVoxelLODVolumeInvokerComponent::GetInvokerSettings_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	auto* Volume = CastChecked<AVoxelLODVolume>(GetOwner());
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

	InvokerComponent = CreateDefaultSubobject<UVoxelLODVolumeInvokerComponent>("Invoker Component");
	InvokerComponent->SetupAttachment(RootComponent);
}

#if WITH_EDITOR
void AVoxelLODVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UVoxelInvokerComponentBase::RefreshAllVoxelInvokers();
}
#endif