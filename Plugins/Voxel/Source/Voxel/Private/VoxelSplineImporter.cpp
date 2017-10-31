#include "VoxelPrivatePCH.h"
#include "VoxelSplineImporter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"


AVoxelSplineImporter::AVoxelSplineImporter()
	: VoxelSize(100)
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

void AVoxelSplineImporter::ImportToAsset(FDecompressedVoxelDataAsset& Asset)
{
	// Calculate bounds & max scale
	FBox Bounds(EForceInit::ForceInitToZero);
	float MaxScale = 0;
	for (auto Spline : Splines)
	{
		Bounds += Spline->Bounds.GetBox();
		FVector Min, Max;
		Spline->GetSplinePointsScale().CalcBounds(Min, Max);
		MaxScale = FMath::Max(MaxScale, Max.Y);
	}

	FVector Origin = Bounds.GetCenter();
	FVector BoxExtent = Bounds.GetExtent();
	BoxExtent += FVector::OneVector * (MaxScale + 2 * VoxelSize);

	DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);

	FIntVector HalfSize =
		FIntVector(
			FMath::CeilToInt(BoxExtent.X / VoxelSize),
			FMath::CeilToInt(BoxExtent.Y / VoxelSize),
			FMath::CeilToInt(BoxExtent.Z / VoxelSize)
		);

	Asset.SetHalfSize(HalfSize.X, HalfSize.Y, HalfSize.Z);

	for (int X = -HalfSize.X; X < HalfSize.X; X++)
	{
		for (int Y = -HalfSize.Y; Y < HalfSize.Y; Y++)
		{
			for (int Z = -HalfSize.Z; Z < HalfSize.Z; Z++)
			{
				float NewValue = bSubstrative ? -1 : 1;

				for (auto Spline : Splines)
				{
					FVector Position = Origin + FVector(X, Y, Z) * VoxelSize;
					const float Key = Spline->FindInputKeyClosestToWorldLocation(Position);
					const float Distance = (Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) - Position).Size();
					const float ScaleSize = Spline->GetScaleAtSplineInputKey(Key).Y;

					if (bSubstrative)
					{
						NewValue = FMath::Max(NewValue, FMath::Clamp((ScaleSize - Distance) / VoxelSize, -2.f, 2.f) / 2.f);
					}
					else
					{
						NewValue = FMath::Min(NewValue, FMath::Clamp((Distance - ScaleSize) / VoxelSize, -2.f, 2.f) / 2.f);
					}
				}

				if ((bSubstrative && -1 + KINDA_SMALL_NUMBER < NewValue) || (!bSubstrative && NewValue < 1 - KINDA_SMALL_NUMBER))
				{
					Asset.SetValue(X, Y, Z, NewValue);
					Asset.SetMaterial(X, Y, Z, Material);

					if ((bSubstrative && NewValue <= 0) || (!bSubstrative && NewValue >= 0))
					{
						Asset.SetVoxelType(X, Y, Z, FVoxelType(UseValueIfSameSign, UseMaterial));
					}
					else
					{
						Asset.SetVoxelType(X, Y, Z, FVoxelType(UseValue, UseMaterial));
					}
				}
				else
				{
					Asset.SetValue(X, Y, Z, 0);
					Asset.SetMaterial(X, Y, Z, FVoxelMaterial(0, 0, 0));
					Asset.SetVoxelType(X, Y, Z, FVoxelType(IgnoreValue, IgnoreMaterial));
				}
			}
		}
	}
}

#if WITH_EDITOR
void AVoxelSplineImporter::Tick(float DeltaTime)
{
	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		if (IsSelectedInEditor())
		{
			int UsedSpheres = 0;
			for (auto Spline : Splines)
			{
				auto Positions = Spline->SplineCurves.Position.Points;
				auto Scales = Spline->SplineCurves.Scale.Points;
				for (int i = 0; i < Spline->GetNumberOfSplinePoints(); i++)
				{
					if (i >= Spheres.Num() || !Spheres[i])
					{
						USphereComponent* Component = NewObject<USphereComponent>(this);
						Component->RegisterComponent();
						if (i >= Spheres.Num())
						{
							Spheres.Add(Component);
						}
						else
						{
							Spheres[i] = Component;
						}
					}
					Spheres[i]->SetRelativeLocation(Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local));

					Spheres[i]->SetSphereRadius(Spline->GetScaleAtSplinePoint(i).Y);
					UsedSpheres++;
				}
			}
			for (int i = UsedSpheres; i < Spheres.Num(); i++)
			{
				if (Spheres[i])
				{
					Spheres[i]->DestroyComponent();
				}
			}
			Spheres.SetNum(UsedSpheres);
		}
	}
}

bool AVoxelSplineImporter::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelSplineImporter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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
}
#endif