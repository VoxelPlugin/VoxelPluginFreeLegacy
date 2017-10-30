#include "VoxelPrivatePCH.h"
#include "VoxelSplineImporter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"


AVoxelSplineImporter::AVoxelSplineImporter()
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
	BoxExtent += FVector::OneVector * (VoxelSize + MaxScale.Size());

	DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Blue, false, 10, 0, 10);


	FVector Start = (Origin - BoxExtent) / VoxelSize;
	FVector End = (Origin + BoxExtent) / VoxelSize;

	FIntVector HalfSize =
		FIntVector(
			FMath::CeilToInt((End.X - Start.X) / 2),
			FMath::CeilToInt((End.Y - Start.Y) / 2),
			FMath::CeilToInt((End.Z - Start.Z) / 2)
		);

	for (int X = -HalfSize.X; X <= HalfSize.X; X++)
	{
		for (int Y = -HalfSize.Y; Y <= HalfSize.Y; Y++)
		{
			for (int Z = -HalfSize.Z; Z <= HalfSize.Z; Z++)
			{
				float NewValue = bSubstrative ? -1 : 1;

				for (auto Spline : Splines)
				{
					FVector Position = FVector(X, Y, Z) * VoxelSize;
					const float Key = Spline->FindInputKeyClosestToWorldLocation(Position);
					const float Distance = (Spline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World) - Position).Size();
					const float ScaleSize = Spline->GetScaleAtSplineInputKey(Key).Size();

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
					Asset.SetValue(X, Y, Z, 0);
					Asset.SetMaterial(X, Y, Z, FVoxelMaterial(0, 0, 0));
					Asset.SetVoxelType(X, Y, Z, FVoxelType(IgnoreValue, IgnoreMaterial));
				}
				else
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
			}
		}
	}
}