// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTools.h"
#include "VoxelWorld.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void UVoxelTools::SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
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

				float Alpha = FMath::Clamp(Radius - Distance, -1.f, 1.f);

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
	if (bApplyUpdates)
	{
		World->ApplyQueuedUpdates(bAsync);
	}
}

void UVoxelTools::SetColorSphere(AVoxelWorld* World, FVector Position, float Radius, FLinearColor Color, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
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
				float Alpha = FMath::Clamp((Radius - Distance) / Radius, 0.f, 1.f);
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

void UVoxelTools::SetValueProjection(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, int Stength, bool bAdd, float Height, bool bQueueUpdate, bool bApplyUpdates, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	check(World);
	FVector ToolPosition = Position + Normal * Height;

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
				FVector End = Start - Normal * 2 * Height;
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

void UVoxelTools::SetColorProjection(AVoxelWorld * World, FVector Position, FVector Normal, float Radius, FLinearColor Color, float Height, bool bQueueUpdate, bool bApplyUpdates, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	check(World);
	FVector ToolPosition = Position + Normal * Height;

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
				FVector End = Start - Normal * 2 * Height;
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
		World->SetColor(Point, Color.ToFColor(true));

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
