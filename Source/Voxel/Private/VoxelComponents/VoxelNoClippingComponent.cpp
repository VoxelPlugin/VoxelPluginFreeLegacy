// Copyright 2020 Phyronnaz

#include "VoxelComponents/VoxelNoClippingComponent.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "Async/Async.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UVoxelNoClippingComponent::UVoxelNoClippingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const double Time = FPlatformTime::Seconds();

	if (!AsyncResult.IsValid() && LastTickTime + TickRate < Time)
	{
		// Time to start a new search
		LastTickTime = Time;
		StartAsyncTask();
	}

	if (!AsyncResult.IsValid() || !AsyncResult.IsReady())
	{
		// Search is not started/not complete
		if (bIsInsideSurface)
		{
			// We should have a search in progress already
			ensure(AsyncResult.IsValid());
			// Always fire the event on tick
			BroadcastMoveTowardsSurface();
		}
		return;
	}

	FAsyncResult Result = AsyncResult.Get();
	AVoxelWorld* VoxelWorld = PendingVoxelWorld.Get();

	AsyncResult = {};
	PendingVoxelWorld = {};
	
	if (!VoxelWorld || !VoxelWorld->IsCreated())
	{
		LOG_VOXEL(Warning, TEXT("NoClippingComponent: Clearing task result as voxel world is now invalid"));
		VoxelWorld = nullptr;
		Result.ClosestSafeLocation.Reset();
	}

	if (!Result.bInsideSurface)
	{
		// We're safe
		if (bIsInsideSurface)
		{
			bIsInsideSurface = false;
			BroadcastStopMovingTowardsSurface();
		}
		return;
	}

	// We're not safe!
	if (Result.ClosestSafeLocation)
	{
		// We found a safe location: teleport there
		
		const FVector NewPosition = VoxelWorld->LocalToGlobal(Result.ClosestSafeLocation.GetValue());
		const FVector Delta = NewPosition - GetComponentLocation();

		AActor& Owner = *GetOwner();
		
		auto* Root = Owner.GetRootComponent();
		if (!ensure(Root))
		{
			return;
		}

		LOG_VOXEL(Log, TEXT("NoClippingComponent: teleporting %s to %s (delta: %s)"), *Owner.GetName(), *(Root->GetComponentLocation() + Delta).ToString(), *Delta.ToString());
		Root->MoveComponent(Delta, Owner.GetActorQuat(), false, nullptr, MOVECOMP_NoFlags, ETeleportType::ResetPhysics);

		BroadcastOnTeleported();
		
		// We're safe
		if (bIsInsideSurface)
		{
			bIsInsideSurface = false;
			BroadcastStopMovingTowardsSurface();
		}
	}
	else
	{
		bIsInsideSurface = true;

		// Start a task now, so we can read the result as soon as possible
		StartAsyncTask();
		
		LOG_VOXEL(Log, TEXT("NoClippingComponent: could not find a safe location, firing MoveTowardsSurface. SearchRange: %d"), SearchRange);
		BroadcastMoveTowardsSurface();
	}

	ensure(AsyncResult.IsValid() || !bIsInsideSurface);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::StartAsyncTask()
{
	if (!ensure(!AsyncResult.IsValid()))
	{
		return;
	}
	ensure(!PendingVoxelWorld.IsValid());

	AVoxelWorld* VoxelWorld = nullptr;
	FVoxelVector Location{ ForceInit };

	for (auto* It : TActorRange<AVoxelWorld>(GetWorld()))
	{
		if (!It->IsCreated())
		{
			continue;
		}

		Location = It->GlobalToLocalFloat(GetComponentLocation());
		if (It->GetWorldBounds().ContainsFloat(Location))
		{
			if (VoxelWorld)
			{
				FVoxelMessages::Warning("Multiple voxel worlds are overlapping NoClippingComponent!", GetOwner());
			}
			VoxelWorld = It;
		}
	}

	if (VoxelWorld)
	{
		PendingVoxelWorld = VoxelWorld;

		AsyncResult = Async(EAsyncExecution::TaskGraph, [Data = VoxelWorld->GetDataSharedPtr(), Location, SearchRange = SearchRange]()
		{
			return AsyncTask(*Data, Location, SearchRange);
		});
	}
	else
	{
		// No voxel world found, stop moving towards surface if needed
		if (bIsInsideSurface)
		{
			LOG_VOXEL(Log, TEXT("NoClippingComponent: went outside of the voxel world bounds, stopping moving towards the surface"));
			bIsInsideSurface = false;
			BroadcastStopMovingTowardsSurface();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::BroadcastMoveTowardsSurface() const
{
	if (bEnableDefaultBehavior)
	{
		auto* CharacterMovement = GetCharacterMovement();
		if (CharacterMovement)
		{
			ResetVelocity(*CharacterMovement);

			FVector Direction;
			if (bIsPlanet)
			{
				Direction = (CharacterMovement->GetActorLocation() - PlanetCenter).GetSafeNormal();
			}
			else
			{
				Direction = FVector::UpVector;
			}
			CharacterMovement->AddImpulse(Direction * Speed, true);
		}
	}
	
	MoveTowardsSurface.Broadcast();
}

void UVoxelNoClippingComponent::BroadcastStopMovingTowardsSurface() const
{
	if (bEnableDefaultBehavior)
	{
		auto* CharacterMovement = GetCharacterMovement();
		if (CharacterMovement)
		{
			ResetVelocity(*CharacterMovement);
		}
	}

	StopMovingTowardsSurface.Broadcast();
}

void UVoxelNoClippingComponent::BroadcastOnTeleported() const
{
	if (bEnableDefaultBehavior)
	{
		auto* CharacterMovement = GetCharacterMovement();
		if (CharacterMovement)
		{
			ResetVelocity(*CharacterMovement);
		}
	}

	OnTeleported.Broadcast();
}

UCharacterMovementComponent* UVoxelNoClippingComponent::GetCharacterMovement() const
{
	auto* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		auto* CharacterMovement = Character->GetCharacterMovement();
		ensure(CharacterMovement);
		return CharacterMovement;
	}
	else
	{
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelNoClippingComponent::ResetVelocity(UCharacterMovementComponent& CharacterMovement)
{
	CharacterMovement.Velocity = FVector::ZeroVector;
	CharacterMovement.ClearAccumulatedForces();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNoClippingComponent::FAsyncResult UVoxelNoClippingComponent::AsyncTask(const FVoxelData& Data, const FVoxelVector& ComponentLocation, int32 SearchRange)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	const FVoxelIntBox Bounds = FVoxelIntBox(ComponentLocation).Extend(FMath::Max(1, SearchRange));
	FVoxelReadScopeLock Lock(Data, Bounds, FUNCTION_FNAME);
	
	const FVoxelConstDataAccelerator Accelerator(Data);
	for (auto& Neighbor : FVoxelUtilities::GetNeighbors(ComponentLocation))
	{
		if (Accelerator.GetValue(Neighbor, 0).IsEmpty())
		{
			// At least one of the voxel near us is not empty: consider ourselves safe
			return {};
		}
	}

	const double StartTime = FPlatformTime::Seconds();

	FAsyncResult Result;
	Result.bInsideSurface = true;

	float BestDistance = 1e9;

	// Somewhat suboptimal: we could query the voxels in "circles" instead
	// It's so fast that it's probably not worth the hassle
	const auto Values = Data.GetValues(Bounds);
	const FIntVector Size = Bounds.Size();
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				if (!Values[X + Size.X * Y + Size.X * Size.Y * Z].IsEmpty())
				{
					continue;
				}

				const FIntVector Position = Bounds.Min + FIntVector(X, Y, Z);
				const float Distance = FVoxelVector::Distance(FVoxelVector(Position), ComponentLocation);
				if (Distance < BestDistance)
				{
					BestDistance = Distance;
					Result.ClosestSafeLocation = Position;
				}
			}
		}
	}
	
	const double EndTime = FPlatformTime::Seconds();

	LOG_VOXEL(Verbose, TEXT("NoClippingComponent search took %.3fms. Success: %s"), (EndTime - StartTime) * 1000, *LexToString(Result.ClosestSafeLocation.IsSet()));
	
	return Result;
}