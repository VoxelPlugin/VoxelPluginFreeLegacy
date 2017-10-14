#include "VoxelPrivatePCH.h"
#include "VoxelModifiers/SplineVoxelModifier.h"
#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "EmptyWorldGenerator.h"
#include "Engine/World.h"
#include "VoxelRender.h"
#include "VoxelData.h"
#include "VoxelWorldEditor.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Volume.h"
#include "Engine/TriggerVolume.h"
#include "DrawDebugHelpers.h"

ASplineVoxelModifier::ASplineVoxelModifier()
	: Size(100)
	, Data(nullptr)
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

void ASplineVoxelModifier::ApplyToWorld(AVoxelWorld* World)
{
	// Destroy render for bounds
	for (auto Component : GetComponentsByClass(UActorComponent::StaticClass()))
	{
		if (!Cast<UCapsuleComponent>(Component) && !Cast<USplineComponent>(Component))
		{
			Component->DestroyComponent();
		}
	}

	FVector Origin, BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);
	BoxExtent += FVector::OneVector * (Size + World->GetVoxelSize());

	DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);

	FVoxelData* Data = World->GetData();

	{
		Data->BeginSet();

		FValueOctree* LastOctree = nullptr;

		FVector Start = (Origin - BoxExtent) / World->GetVoxelSize();
		FVector End = (Origin + BoxExtent) / World->GetVoxelSize();

		for (int X = Start.X; X <= End.X; X++)
		{
			for (int Y = Start.Y; Y <= End.Y; Y++)
			{
				for (int Z = Start.Z; Z <= End.Z; Z++)
				{
					float NewValue = bSubstrative ? -1 : 1;

					for (auto Spline : Splines)
					{
						FVector Position = FVector(X, Y, Z) * World->GetVoxelSize();
						float Distance = (Spline->FindLocationClosestToWorldLocation(Position, ESplineCoordinateSpace::World) - Position).Size();

						if (bSubstrative)
						{
							NewValue = FMath::Max(NewValue, FMath::Clamp((Size - Distance) / World->GetVoxelSize(), -2.f, 2.f) / 2.f);
						}
						else
						{
							NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - Size) / World->GetVoxelSize(), -2.f, 2.f) / 2.f);
						}
					}

					if ((bSubstrative && -1 + KINDA_SMALL_NUMBER < NewValue) || (!bSubstrative && NewValue < 1 - KINDA_SMALL_NUMBER))
					{
						float OldValue;
						FVoxelMaterial Tmp;
						Data->GetValueAndMaterial(X, Y, Z, OldValue, Tmp, LastOctree);
						if ((bSubstrative && OldValue <= 0) || (!bSubstrative && OldValue >= 0))
						{
							if (bSetMaterial)
							{
								Data->SetValueAndMaterial(X, Y, Z, NewValue, Material, LastOctree);
							}
							else
							{
								Data->SetValue(X, Y, Z, NewValue, LastOctree);
							}
						}
					}
				}
			}
		}

		Data->EndSet();
	}
}

void ASplineVoxelModifier::BeginPlay()
{
	for (auto Component : GetComponentsByClass(UActorComponent::StaticClass()))
	{
		if (!Cast<UCapsuleComponent>(Component))
		{
			Component->DestroyComponent();
		}
	}
}

#if WITH_EDITOR

void ASplineVoxelModifier::Tick(float DeltaTime)
{
	if (GetWorld()->WorldType == EWorldType::Editor && Render)
	{
		Render->Tick(DeltaTime);
	}
}

bool ASplineVoxelModifier::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void ASplineVoxelModifier::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (USplineComponent*& Spline : Splines)
	{
		if (!Spline)
		{
			Spline = NewObject<USplineComponent>(this);
			Spline->OnComponentCreated();
			Spline->RegisterComponent();
			Spline->SetWorldLocation(GetActorLocation());
		}
	}

	for (auto Component : GetComponentsByClass(USplineComponent::StaticClass()))
	{
		USplineComponent* Spline = (USplineComponent*)Component;
		if (Spline && !Splines.Contains(Spline))
		{
			Spline->DestroyComponent();
		}
	}

	for (auto Component : GetComponentsByClass(UActorComponent::StaticClass()))
	{
		if (!Cast<UCapsuleComponent>(Component) && !Cast<USplineComponent>(Component))
		{
			Component->DestroyComponent();
		}
	}


	if (bEnablePreview)
	{
		UpdateRender();
	}
	else
	{
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

void ASplineVoxelModifier::UpdateRender()
{
	if (PreviewWorld)
	{
		FVector Origin, BoxExtent;
		GetActorBounds(false, Origin, BoxExtent);
		BoxExtent += FVector::OneVector * (Size + PreviewWorld->GetVoxelSize());

		DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);

		const float Max = FMath::Max(
			(Origin + BoxExtent - GetActorLocation()).GetAbs().GetMax(),
			(Origin - BoxExtent - GetActorLocation()).GetAbs().GetMax()
		);

		const uint8 Depth = FMath::CeilToInt(FMath::Log2(Max / 16.f)) + 1;

		if (!Generator)
		{
			Generator = GetWorld()->SpawnActor<AEmptyWorldGenerator>(FVector::ZeroVector, FRotator::ZeroRotator);
		}

		if (Data)
		{
			delete Data;
		}
		Data = new FVoxelData(Depth, Generator);

		{
			Data->BeginSet();

			FValueOctree* LastOctree = nullptr;

			FVector Start = (Origin - BoxExtent - GetActorLocation()) / PreviewWorld->GetVoxelSize();
			FVector End = (Origin + BoxExtent - GetActorLocation()) / PreviewWorld->GetVoxelSize();

			for (int X = Start.X; X <= End.X; X++)
			{
				for (int Y = Start.Y; Y <= End.Y; Y++)
				{
					for (int Z = Start.Z; Z <= End.Z; Z++)
					{
						float NewValue = 1;

						for (auto Spline : Splines)
						{
							FVector Position = GetActorLocation() + FVector(X, Y, Z) * PreviewWorld->GetVoxelSize();
							float Distance = (Spline->FindLocationClosestToWorldLocation(Position, ESplineCoordinateSpace::World) - Position).Size();
							NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - Size) / PreviewWorld->GetVoxelSize(), -2.f, 2.f) / 2.f);
						}

						if (NewValue < 1 - KINDA_SMALL_NUMBER)
						{
							if (bSetMaterial)
							{
								Data->SetValueAndMaterial(X, Y, Z, NewValue, Material, LastOctree);
							}
							else
							{
								Data->SetValue(X, Y, Z, NewValue, LastOctree);
							}
						}
					}
				}
			}

			Data->EndSet();
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
