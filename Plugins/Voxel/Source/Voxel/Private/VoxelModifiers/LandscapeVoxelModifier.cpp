#include "VoxelPrivatePCH.h"
#include "VoxelModifiers/LandscapeVoxelModifier.h"
#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "EmptyWorldGenerator.h"
#include "VoxelPart.h"
#include "Engine/World.h"

ALandscapeVoxelModifier::ALandscapeVoxelModifier()
	: Part(nullptr)
{
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;
}

void ALandscapeVoxelModifier::ApplyToWorld(AVoxelWorld* World)
{
	if (Landscape)
	{
		ALandscapeVoxelAsset* InstancedLandscape = Landscape.GetDefaultObject();

		check(InstancedLandscape);

		InstancedLandscape->Init(World->GetVoxelSize());

		FIntVector Position = World->GlobalToLocal(GetActorLocation());

		FVoxelData* Data = World->Data;

		FValueOctree* LastOctree = nullptr;

		for (int X = 0; X < InstancedLandscape->Size; X++)
		{
			for (int Y = 0; Y < InstancedLandscape->Size; Y++)
			{
				for (int Z = 0; Z < InstancedLandscape->Size; Z++)
				{
					Data->SetValueAndMaterialNotThreadSafe(X + Position.X, Y + Position.Y, Z + Position.Z, InstancedLandscape->GetDefaultValue(X, Y, Z), InstancedLandscape->GetDefaultMaterial(X, Y, Z), LastOctree);
				}
			}
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("LandscapeVoxelModifier: no landscape selected"));
	}
}

void ALandscapeVoxelModifier::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (GetWorld() && bEnablePreview)
	{
		UpdateRender();
	}
}

void ALandscapeVoxelModifier::UpdateRender()
{
	return;

	if (Landscape && PreviewWorld)
	{
		ALandscapeVoxelAsset* InstancedLandscape = Landscape.GetDefaultObject();

		check(InstancedLandscape);

		InstancedLandscape->Init(PreviewWorld->GetVoxelSize());

		const uint8 Depth = FMath::CeilToInt(FMath::Log2(InstancedLandscape->Size / 16.f));
		AVoxelWorldGenerator* Generator = GetWorld()->SpawnActor<AEmptyWorldGenerator>(FVector::ZeroVector, FRotator::ZeroRotator);

		FVoxelData Data(Depth, Generator);

		const int S = Data.Size() / 2;
		for (int X = 0; X < InstancedLandscape->Size; X++)
		{
			for (int Y = 0; Y < InstancedLandscape->Size; Y++)
			{
				for (int Z = 0; Z < InstancedLandscape->Size; Z++)
				{
					Data.SetValue(X - S, Y - S, Z - S, InstancedLandscape->GetDefaultValue(X, Y, Z));
					Data.SetMaterial(X - S, Y - S, Z - S, InstancedLandscape->GetDefaultMaterial(X, Y, Z));
				}
			}
		}

		if (Part)
		{
			Part->Destroy();
		}

		Part = GetWorld()->SpawnActor<AVoxelPart>(FVector::ZeroVector, FRotator::ZeroRotator);
		Part->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Part->SetActorRelativeLocation(FVector::OneVector * S * PreviewWorld->GetVoxelSize());
		Part->SetActorRotation(FRotator::ZeroRotator);
		Part->SetActorScale3D(PreviewWorld->GetActorScale());

		Part->Init(&Data, PreviewWorld->VoxelMaterial);
		Generator->Destroy();
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("LandscapeVoxelModifier: no landscape or no world selected"));
	}
}
