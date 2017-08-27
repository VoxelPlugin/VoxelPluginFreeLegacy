// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelTools.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ValueOctree.h"
#include "ChunkOctree.h"
#include "MeshImporter.h"

void UVoxelTools::SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius);

	TArray<FIntVector> Corners = {
		LocalPosition + FIntVector(-IntRadius, -IntRadius, -IntRadius),
		LocalPosition + FIntVector(+IntRadius, -IntRadius, -IntRadius),
		LocalPosition + FIntVector(-IntRadius, +IntRadius, -IntRadius),
		LocalPosition + FIntVector(+IntRadius, +IntRadius, -IntRadius),
		LocalPosition + FIntVector(-IntRadius, -IntRadius, +IntRadius),
		LocalPosition + FIntVector(+IntRadius, -IntRadius, +IntRadius),
		LocalPosition + FIntVector(-IntRadius, +IntRadius, +IntRadius),
		LocalPosition + FIntVector(+IntRadius, +IntRadius, +IntRadius)
	};

	// Get smallest octrees
	ValueOctree* SmallValueOctree = World->GetValueOctree().Get();
	ValueOctree* OldSmallValueOctree = SmallValueOctree;
	check(SmallValueOctree);

	bool ContinueShrinking = true;
	while (ContinueShrinking)
	{
		ContinueShrinking = !SmallValueOctree->IsLeaf();
		for (auto Corner : Corners)
		{
			ContinueShrinking = ContinueShrinking && SmallValueOctree->IsInOctree(Corner);
		}

		if (ContinueShrinking)
		{
			OldSmallValueOctree = SmallValueOctree;
			SmallValueOctree = SmallValueOctree->GetChild(Corners[0]);
		}
	}
	SmallValueOctree = OldSmallValueOctree;


	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			for (int z = -IntRadius; z <= IntRadius; z++)
			{
				FIntVector CurrentPosition = LocalPosition + FIntVector(x, y, z);
				float Distance = FVector(x, y, z).Size();

				if (Distance <= Radius + 1)
				{
					float Value = (bAdd ? -1 : 1) * (Radius - Distance);

					if ((Value < 0 && bAdd) || (Value >= 0 && !bAdd) || (SmallValueOctree->GetValue(CurrentPosition) * Value > 0))
					{
						SmallValueOctree->SetValue(CurrentPosition, Value);
						if (bQueueUpdate)
						{
							World->QueueUpdate(CurrentPosition);
						}
					}
				}
			}
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, float FadeDistance, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius);

	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			for (int z = -IntRadius; z <= IntRadius; z++)
			{
				FIntVector CurrentPosition = LocalPosition + FIntVector(x, y, z);
				float Distance = FVector(x, y, z).Size();
				float Alpha = (FMath::Clamp(Radius - Distance, -1.f, FadeDistance) + 1) / (1 + FadeDistance);
				FColor CurrentColor = FLinearColor::LerpUsingHSV(World->GetColor(CurrentPosition), Color, Alpha).ToFColor(true);

				World->SetColor(CurrentPosition, CurrentColor);
				if (bQueueUpdate)
				{
					World->QueueUpdate(CurrentPosition);
				}
			}
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SetValueCone(AVoxelWorld * World, FVector Position, float Radius, float Height, bool bAdd, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius);
	int IntHeight = FMath::CeilToInt(Height);

	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			for (int z = 0; z <= IntHeight; z++)
			{
				FIntVector CurrentPosition = LocalPosition + FIntVector(x, y, z);
				float CurrentRadius = z * (Radius / Height);
				float Distance = FVector2D(x, y).Size();

				if (Distance <= CurrentRadius + 1)
				{
					float Alpha = FMath::Clamp(CurrentRadius - Distance, -1.f, 1.f);

					int Value = (bAdd ? -1 : 1) * (int)(127 * Alpha);


					if ((Value < 0 && bAdd) || (Value >= 0 && !bAdd) || (World->GetValue(CurrentPosition) * Value > 0))
					{
						World->SetValue(CurrentPosition, Value);
						if (bQueueUpdate)
						{
							World->QueueUpdate(CurrentPosition);
						}
					}
				}
			}
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SetValueProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, int Stength, bool bAdd, float MaxDistance, bool bQueueUpdate, bool bApplyUpdates, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Normal * MaxDistance / 2;

	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();

	TArray<FIntVector> Points;
	float Scale = World->GetTransform().GetScale3D().Size() / 4;

	int IntRadius = FMath::CeilToInt(Radius);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			if (x*x + y*y < Radius*Radius)
			{
				FHitResult Hit;
				FVector Start = ToolPosition + Tangent * x * Scale + Bitangent * y * Scale;
				FVector End = Start - Normal * MaxDistance;
				if (bDebugLines)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic))
				{
					if (bDebugPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}
					Points.AddUnique(World->GlobalToLocal(Hit.ImpactPoint));
				}
			}
		}
	}
	for (FIntVector Point : Points)
	{
		if (bAdd)
		{
			World->Add(Point, Stength);
		}
		else
		{
			World->Remove(Point, Stength);
		}
		if (bQueueUpdate)
		{
			World->QueueUpdate(Point);
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SetColorProjection(AVoxelWorld * World, FVector Position, FVector Normal, float Radius, FLinearColor Color, float FadeDistance, float MaxDistance, bool bQueueUpdate, bool bApplyUpdates, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	if (World == NULL)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Normal * MaxDistance / 2;

	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();

	TArray<FIntVector> Positions;
	TArray<FColor> Colors;

	float Scale = World->GetTransform().GetScale3D().Size() / 4;

	int IntRadius = FMath::CeilToInt(Radius);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			FHitResult Hit;
			FVector Start = ToolPosition + Tangent * x * Scale + Bitangent * y * Scale;
			FVector End = Start - Normal * MaxDistance;
			if (bDebugLines)
			{
				DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
			}

			if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic))
			{
				if (bDebugPoints)
				{
					DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
				}

				FIntVector CurrentPosition = World->GlobalToLocal(Hit.ImpactPoint);
				float Distance = FVector2D(x, y).Size();
				float Alpha = (FMath::Clamp(Radius - Distance, -1.f, FadeDistance) + 1) / (1 + FadeDistance);
				FColor CurrentColor = FLinearColor::LerpUsingHSV(World->GetColor(CurrentPosition), Color, Alpha).ToFColor(true);

				if (!Positions.Contains(CurrentPosition))
				{
					Positions.Add(CurrentPosition);
					Colors.Add(CurrentColor);
				}
			}
		}
	}
	for (int i = 0; i < Positions.Num(); i++)
	{
		World->SetColor(Positions[i], Colors[i]);

		if (bQueueUpdate)
		{
			World->QueueUpdate(Positions[i]);
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SmoothValue(AVoxelWorld * World, FVector Position, FVector Normal, float Radius, float Speed, float MaxDistance, bool bQueueUpdate, bool bApplyUpdates, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Normal * MaxDistance / 2;

	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();

	TArray<FIntVector> Positions;
	float MeanValue = 0;

	float Scale = World->GetTransform().GetScale3D().Size() / 4;

	int IntRadius = FMath::CeilToInt(Radius);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			if (x*x + y*y < Radius*Radius)
			{
				FHitResult Hit;
				FVector Start = ToolPosition + Tangent * x * Scale + Bitangent * y * Scale;
				FVector End = Start - Normal * MaxDistance;
				if (bDebugLines)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}

				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic))
				{
					if (bDebugPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}

					FIntVector CurrentPosition = World->GlobalToLocal(Hit.ImpactPoint);

					if (!Positions.Contains(CurrentPosition))
					{
						Positions.Add(CurrentPosition);
						MeanValue += World->GetValue(CurrentPosition);
					}
				}
			}
		}
	}

	if (Positions.Num() != 0)
	{
		MeanValue /= Positions.Num();
	}

	for (FIntVector CurrentPosition : Positions)
	{
		World->SetValue(CurrentPosition, FMath::Lerp(World->GetValue(CurrentPosition), MeanValue, Speed));

		if (bQueueUpdate)
		{
			World->QueueUpdate(CurrentPosition);
		}
	}
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::ImportMesh(bool bAdd, AVoxelWorld* World, UPrimitiveComponent* Component, AActor* Actor)
{
	if (Actor == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("ImportMesh: Actor is Invalid"));
	}
	else
	{
		FVector Origin;
		FVector BoxExtent;
		Actor->GetActorBounds(false, Origin, BoxExtent);
		MeshImporter::ImportMesh(bAdd, Component, World, Origin - BoxExtent, Origin + BoxExtent);
	}
}
