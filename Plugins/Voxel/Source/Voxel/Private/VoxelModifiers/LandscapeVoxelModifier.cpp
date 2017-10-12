#include "VoxelPrivatePCH.h"
#include "VoxelModifiers/LandscapeVoxelModifier.h"
#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "EmptyWorldGenerator.h"
#include "Engine/World.h"
#include "VoxelRender.h"
#include "VoxelData.h"
#include "VoxelWorldEditor.h"

ALandscapeVoxelModifier::ALandscapeVoxelModifier()
	: Data(nullptr)
	, Render(nullptr)
	, Generator(nullptr)
{
#if WITH_EDITOR
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	PrimaryActorTick.bCanEverTick = true;
#endif
}

void ALandscapeVoxelModifier::ApplyToWorld(AVoxelWorld* World)
{
	if (Landscape)
	{
		ALandscapeVoxelAsset* InstancedLandscape = Landscape.GetDefaultObject();

		check(InstancedLandscape);

		InstancedLandscape->Init(World->GetVoxelSize());

		FIntVector Position = World->GlobalToLocal(GetActorLocation());

		FVoxelData* Data = World->GetData();

		FValueOctree* LastOctree = nullptr;

		for (int X = 0; X < InstancedLandscape->Size; X++)
		{
			for (int Y = 0; Y < InstancedLandscape->Size; Y++)
			{
				const int Min = 0 * InstancedLandscape->GetMinBound(X, Y);
				const int Max = InstancedLandscape->GetMaxBound(X, Y);
				for (int Z = Min; Z <= Max; Z++)
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

#if WITH_EDITOR
void ALandscapeVoxelModifier::Tick(float DeltaTime)
{
	if (Render)
	{
		Render->Tick(DeltaTime);
	}
}

bool ALandscapeVoxelModifier::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void ALandscapeVoxelModifier::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bEnablePreview)
	{
		UpdateRender();
	}
	else
	{
		for (auto Component : GetComponentsByClass(UActorComponent::StaticClass()))
		{
			if (!Cast<UCapsuleComponent>(Component))
			{
				Component->DestroyComponent();
			}
		}

		if (Render)
		{
			delete Render;
			Render = nullptr;
		}
		if (Data)
		{
			delete Data;
			Data = nullptr;
		}
	}
}

void ALandscapeVoxelModifier::UpdateRender()
{
	if (Landscape && PreviewWorld)
	{
		ALandscapeVoxelAsset* InstancedLandscape = Landscape.GetDefaultObject();

		check(InstancedLandscape);

		InstancedLandscape->Init(PreviewWorld->GetVoxelSize());

		const uint8 Depth = FMath::CeilToInt(FMath::Log2(InstancedLandscape->Size / 16.f)) + 1;
		if (!Generator)
		{
			Generator = GetWorld()->SpawnActor<AEmptyWorldGenerator>(FVector::ZeroVector, FRotator::ZeroRotator);
		}

		if (Data)
		{
			delete Data;
		}
		Data = new FVoxelData(Depth, Generator);

		FValueOctree* LastOctree = nullptr;

		for (int X = 0; X < InstancedLandscape->Size; X++)
		{
			for (int Y = 0; Y < InstancedLandscape->Size; Y++)
			{
				const int Min = 0 * InstancedLandscape->GetMinBound(X, Y);
				const int Max = InstancedLandscape->GetMaxBound(X, Y);
				for (int Z = Min; Z <= Max; Z++)
				{
					Data->SetValueAndMaterialNotThreadSafe(X, Y, Z, InstancedLandscape->GetDefaultValue(X, Y, Z), InstancedLandscape->GetDefaultMaterial(X, Y, Z), LastOctree);
				}
			}
		}

		if (Render)
		{
			delete Render;
		}
		Render = new FVoxelRender(PreviewWorld, this, Data, 4, 4);

		if (PreviewWorld->GetVoxelWorldEditor())
		{
			Render->AddInvoker(PreviewWorld->GetVoxelWorldEditor()->GetInvoker());
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("LandscapeVoxelModifier: no landscape or no world selected"));
	}
}
