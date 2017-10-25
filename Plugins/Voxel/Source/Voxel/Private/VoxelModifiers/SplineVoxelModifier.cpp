#include "VoxelPrivatePCH.h"
#include "VoxelModifiers/SplineVoxelModifier.h"
#include "VoxelWorld.h"
#include "Components/CapsuleComponent.h"
#include "EmptyWorldGenerator.h"
#include "Engine/World.h"
#include "VoxelRender.h"
#include "VoxelData.h"
#include "VoxelWorldEditorInterface.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Volume.h"
#include "Engine/TriggerVolume.h"
#include "DrawDebugHelpers.h"
#include "VoxelDataAsset.h"

ASplineVoxelModifier::ASplineVoxelModifier()
	: Size(250)
	, LivePreviewFPS(10)
	, Data(nullptr)
	, Render(nullptr)
	, TimeSinceUpdate(0)
{
#if WITH_EDITOR
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	PrimaryActorTick.bCanEverTick = true;

	Generator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), UEmptyWorldGenerator::StaticClass());
#endif
}

ASplineVoxelModifier::~ASplineVoxelModifier()
{
	if (Render)
	{
		Render->Destroy();
		delete Render;
		Render = nullptr;
	}
	if (Data)
	{
		delete Data;
		Data = nullptr;
	}
}

void ASplineVoxelModifier::ApplyToWorld(AVoxelWorld* World)
{
	FBox Bounds(EForceInit::ForceInitToZero);
	FVector MaxScale = FVector::ZeroVector;
	for (auto Spline : Splines)
	{
		Bounds += Spline->Bounds.GetBox();
		FVector Min, Max;
		Spline->GetSplinePointsScale().CalcBounds(Min, Max);
		MaxScale = FVector(FMath::Max(MaxScale.X, Max.X), FMath::Max(MaxScale.Y, Max.Y), FMath::Max(MaxScale.Z, Max.Z));
	}

	FVector Origin = Bounds.GetCenter();
	FVector BoxExtent = Bounds.GetExtent();
	BoxExtent += FVector::OneVector * (Size + PreviewWorld->GetVoxelSize() + MaxScale.Size());

	DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);

	FVoxelData* WorldData = World->GetData();

	{
		WorldData->BeginSet();

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
						const float Key = Spline->FindInputKeyClosestToWorldLocation(Position);
						const float Distance = (Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) - Position).Size();
						const float ScaleSize = Spline->GetScaleAtSplineInputKey(Key).Size();

						if (bSubstrative)
						{
							NewValue = FMath::Max(NewValue, FMath::Clamp((Size + ScaleSize - Distance) / World->GetVoxelSize(), -2.f, 2.f) / 2.f);
						}
						else
						{
							NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - Size - ScaleSize) / World->GetVoxelSize(), -2.f, 2.f) / 2.f);
						}
					}

					if ((bSubstrative && -1 + KINDA_SMALL_NUMBER < NewValue) || (!bSubstrative && NewValue < 1 - KINDA_SMALL_NUMBER))
					{
						float OldValue;
						FVoxelMaterial Tmp;
						WorldData->GetValueAndMaterial(X, Y, Z, OldValue, Tmp, LastOctree);
						if ((bSubstrative && OldValue <= 0) || (!bSubstrative && OldValue >= 0))
						{
							if (bSetMaterial)
							{
								WorldData->SetValueAndMaterial(X, Y, Z, NewValue, Material, LastOctree);
							}
							else
							{
								WorldData->SetValue(X, Y, Z, NewValue, LastOctree);
							}
						}
					}
				}
			}
		}

		WorldData->EndSet();
	}
}

void ASplineVoxelModifier::UpdateRender()
{
	if (PreviewWorld)
	{
		FBox Bounds(EForceInit::ForceInitToZero);
		FVector MaxScale = FVector::ZeroVector;
		for (auto Spline : Splines)
		{
			Bounds += Spline->Bounds.GetBox();
			FVector Min, Max;
			Spline->GetSplinePointsScale().CalcBounds(Min, Max);
			MaxScale = FVector(FMath::Max(MaxScale.X, Max.X), FMath::Max(MaxScale.Y, Max.Y), FMath::Max(MaxScale.Z, Max.Z));
		}

		FVector Origin = Bounds.GetCenter();
		FVector BoxExtent = Bounds.GetExtent();
		BoxExtent += FVector::OneVector * (Size + PreviewWorld->GetVoxelSize() + MaxScale.Size());

		DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, bLivePreview ? 2 / LivePreviewFPS : 10, 0, 10);

		const float Max = FMath::Max(
			(Origin + BoxExtent - GetActorLocation()).GetAbs().GetMax(),
			(Origin - BoxExtent - GetActorLocation()).GetAbs().GetMax()
		);

		const uint8 Depth = FMath::Max(0, FMath::CeilToInt(FMath::Log2(Max / PreviewWorld->GetVoxelSize() / 16.f))) + 2;

		if (!Data || Data->Depth != Depth)
		{
			Render->Destroy();
			delete Render;
			Render = nullptr;

			if (Depth > 5 && !bNoSizeLimit)
			{
				UE_LOG(VoxelLog, Error, TEXT("Spline is too big: Depth = %d"), Depth);
				return;
			}

			Data = new FVoxelData(Depth, Generator);
		}

		{
			Data->BeginSet();

			Data->Reset();

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
							const float Key = Spline->FindInputKeyClosestToWorldLocation(Position);
							const float Distance = (Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) - Position).Size();
							const float ScaleSize = Spline->GetScaleAtSplineInputKey(Key).Size();

							NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - Size - ScaleSize) / PreviewWorld->GetVoxelSize(), -2.f, 2.f) / 2.f);
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

		if (!Render)
		{
			Render = new FVoxelRender(PreviewWorld, this, Data, 4, 4);

			if (PreviewWorld->GetVoxelWorldEditor())
			{
				Render->AddInvoker(PreviewWorld->GetVoxelWorldEditor()->GetInvoker());
			}
		}
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("LandscapeVoxelModifier: no landscape or no world selected"));
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
	UE_LOG(VoxelLog, Warning, TEXT("VoxelDataAsset"));

	if (VoxelDataAsset)
	{
		VoxelDataAsset->Name = TEXT("VoxelDataAsset");
	}

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		if (IsSelectedInEditor())
		{
			TimeSinceUpdate += DeltaTime;
			if (bLivePreview && TimeSinceUpdate > 1 / LivePreviewFPS)
			{
				UpdateRender();
				if (Render)
				{
					Render->UpdateAll(true);
				}
				TimeSinceUpdate = 0;
			}
		}
		if (Render)
		{
			Render->Tick(DeltaTime);
		}
	}
}

bool ASplineVoxelModifier::ShouldTickIfViewportsOnly() const
{
	return true;
}

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


	if (Render)
	{
		Render->Destroy();
		delete Render;
		Render = nullptr;
	}
	if (Data)
	{
		delete Data;
		Data = nullptr;
	}

	if (bEnablePreview)
	{
		UpdateRender();
	}
}
#endif
