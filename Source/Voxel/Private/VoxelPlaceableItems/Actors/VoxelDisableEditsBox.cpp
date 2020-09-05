// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/Actors/VoxelDisableEditsBox.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "Components/BoxComponent.h"

AVoxelDisableEditsBox::AVoxelDisableEditsBox()
{
	SetActorEnableCollision(false);

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetBoxExtent(FVector::OneVector * 100);
	SetRootComponent(Box);
	
#if WITH_EDITOR
	PrimaryActorTick.bCanEverTick = true;
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelDisableEditsBox::AddItemToWorld(AVoxelWorld* World)
{
	check(World);

	FVoxelDisableEditsBoxItemReference Reference;
	UVoxelAssetTools::AddDisableEditsBox(
		Reference,
		World,
		GetBox(World));
}

FVoxelIntBox AVoxelDisableEditsBox::GetBox(AVoxelWorld* World) const
{
	check(World);

	const FIntVector Position = World->GlobalToLocal(GetActorLocation() + FVector::OneVector * World->VoxelSize / 2.f);

	const FVector Scale = GetActorScale3D();
	
	const float Ratio = World->VoxelSize / 100.f;
	const FIntVector S = FVoxelUtilities::RoundToInt(Scale / Ratio);

	return FVoxelIntBox(Position - S, Position + S);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void AVoxelDisableEditsBox::BeginPlay()
{
    Super::BeginPlay();
	PrimaryActorTick.SetTickFunctionEnable(false);
}

void AVoxelDisableEditsBox::Tick(float DeltaTime)
{
	if (PreviewWorld)
	{
		if (VoxelSize != PreviewWorld->VoxelSize || WorldLocation != PreviewWorld->GetActorLocation())
		{
			ClampTransform();
		}
	}
}

void AVoxelDisableEditsBox::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PreviewWorld)
	{
		ClampTransform();
	}
}

void AVoxelDisableEditsBox::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished && PreviewWorld)
	{
		ClampTransform();
	}
}

///////////////////////////////////////////////////////////////////////////////

void AVoxelDisableEditsBox::ClampTransform()
{
	VoxelSize = PreviewWorld->VoxelSize;
	WorldLocation = PreviewWorld->GetActorLocation();

	{
		FVector Position = GetActorLocation();

		const FVector HalfSize = FVector(VoxelSize) / 2;

		Position -= HalfSize;
		Position -= WorldLocation;
		Position /= VoxelSize;

		Position.X = FMath::RoundToInt(Position.X);
		Position.Y = FMath::RoundToInt(Position.Y);
		Position.Z = FMath::RoundToInt(Position.Z);

		Position *= VoxelSize;
		Position += WorldLocation;
		Position += HalfSize;

		SetActorLocation(Position);
	}
	{
		FVector Scale = GetActorScale3D();

		const float Ratio = VoxelSize / 100.f;
		Scale.X = FMath::RoundToInt(FMath::Max(Scale.X, 0.f) / Ratio) * Ratio;
		Scale.Y = FMath::RoundToInt(FMath::Max(Scale.Y, 0.f) / Ratio) * Ratio;
		Scale.Z = FMath::RoundToInt(FMath::Max(Scale.Z, 0.f) / Ratio) * Ratio;

		SetActorScale3D(Scale);
	}
	SetActorRotation(FRotator::ZeroRotator);
}
#endif