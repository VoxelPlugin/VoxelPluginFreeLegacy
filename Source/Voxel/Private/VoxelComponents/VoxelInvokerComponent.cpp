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

bool UVoxelInvokerComponentBase::IsLocalInvoker_Implementation() const
{
	auto* Owner = Cast<APawn>(GetOwner());
	return !Owner || Owner->IsLocallyControlled();
}

FIntVector UVoxelInvokerComponentBase::GetInvokerVoxelPosition_Implementation(AVoxelWorldInterface* VoxelWorld) const
{
	return FIntVector(0);
}

void UVoxelInvokerComponentBase::GetInvokerSettings_Implementation(
	AVoxelWorldInterface* VoxelWorld, 
	bool& bUseForLOD, 
	int32& LODToSet, 
	FIntBox& LODBounds, 
	bool& bUseForCollisions, 
	FIntBox& CollisionsBounds, 
	bool& bUseForNavmesh,
	FIntBox& NavmeshBounds,
	bool& bUseForTessellation,
	FIntBox& TessellationBounds) const
{
	bUseForLOD = false;
	bUseForCollisions = false;
	bUseForNavmesh = false;
	bUseForTessellation = false;
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

void UVoxelSimpleInvokerComponent::GetInvokerSettings_Implementation(
	AVoxelWorldInterface* VoxelWorld,
	bool& OutbUseForLOD,
	int32& OutLODToSet,
	FIntBox& OutLODBounds,
	bool& OutbUseForCollisions,
	FIntBox& OutCollisionsBounds,
	bool& OutbUseForNavmesh,
	FIntBox& OutNavmeshBounds,
	bool& OutbUseForTessellation,
	FIntBox& OutTessellationBounds) const
{
	const FVector InvokerGlobalPosition = GetInvokerGlobalPosition();
	const auto GetVoxelBounds = [&](float Distance)
	{
		return FIntBox::SafeConstruct(
			VoxelWorld->GlobalToLocal(InvokerGlobalPosition - Distance, EVoxelWorldCoordinatesRounding::RoundDown),
			VoxelWorld->GlobalToLocal(InvokerGlobalPosition + Distance, EVoxelWorldCoordinatesRounding::RoundUp)
		);
	};
	
	OutbUseForLOD = bUseForLOD;
	OutLODToSet = LODToSet;
	OutLODBounds = GetVoxelBounds(LODRange);

	OutbUseForCollisions = bUseForCollisions;
	OutCollisionsBounds = GetVoxelBounds(CollisionsRange);

	OutbUseForNavmesh = bUseForNavmesh;
	OutNavmeshBounds = GetVoxelBounds(NavmeshRange);

	OutbUseForTessellation = bUseForTessellation;
	OutTessellationBounds = GetVoxelBounds(TessellationRange);
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

void UVoxelLODVolumeInvokerComponent::GetInvokerSettings_Implementation(
	AVoxelWorldInterface* VoxelWorld,
	bool& OutbUseForLOD,
	int32& OutLODToSet,
	FIntBox& OutLODBounds,
	bool& OutbUseForCollisions,
	FIntBox& OutCollisionsBounds,
	bool& OutbUseForNavmesh,
	FIntBox& OutNavmeshBounds,
	bool& OutbUseForTessellation,
	FIntBox& OutTessellationBounds) const
{
	auto* Volume = CastChecked<AVoxelLODVolume>(GetOwner());
	const FBox VolumeWorldBounds = Volume->GetBounds().GetBox();
	const FIntBox VolumeBounds = FIntBox::SafeConstruct(
		VoxelWorld->GlobalToLocal(VolumeWorldBounds.Min, EVoxelWorldCoordinatesRounding::RoundDown),
		VoxelWorld->GlobalToLocal(VolumeWorldBounds.Max, EVoxelWorldCoordinatesRounding::RoundUp)
	);

	OutbUseForLOD = bUseForLOD;
	OutLODToSet = LODToSet;
	OutLODBounds = VolumeBounds;

	OutbUseForCollisions = bUseForCollisions;
	OutCollisionsBounds = VolumeBounds;

	OutbUseForNavmesh = bUseForNavmesh;
	OutNavmeshBounds = VolumeBounds;

	OutbUseForTessellation = bUseForTessellation;
	OutTessellationBounds = VolumeBounds;
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