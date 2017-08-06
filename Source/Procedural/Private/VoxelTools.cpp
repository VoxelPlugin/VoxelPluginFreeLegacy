// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTools.h"
#include "VoxelWorld.h"

void UVoxelTools::SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bQueueUpdate, bool bApplyUpdates, bool bAsync)
{
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
