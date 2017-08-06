// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelTools.h"
#include "VoxelWorld.h"

void UVoxelTools::SetValueSphere(AVoxelWorld* world, FVector position, float radius, bool add, bool bQueueUpdate, bool bApplyUpdates)
{
	FIntVector Position = world->GlobalToLocal(position);
	int Radius = FMath::Ceil(radius);
	for (int x = -Radius; x <= Radius; x++)
	{
		for (int y = -Radius; y <= Radius; y++)
		{
			for (int z = -Radius; z <= Radius; z++)
			{
				FIntVector CurrentPosition = Position + FIntVector(x, y, z);
				float Distance = FVector(x, y, z).Size();

				int Value = (add ? -1 : 1) * (int)(1000 * (radius - Distance) / radius);


				if ((Value < 0 && add) || (Value >= 0 && !add) || (world->GetValue(CurrentPosition) * Value > 0))
				{
					world->SetValue(CurrentPosition, Value);
					if (bQueueUpdate)
					{
						world->ScheduleUpdate(CurrentPosition);
					}
				}
			}
		}
	}
	if (bApplyUpdates)
	{
		world->ApplyQueuedUpdates();
	}
}

void UVoxelTools::SetColorSphere(AVoxelWorld* world, FVector position, float radius, FLinearColor color, bool bQueueUpdate, bool bApplyUpdates)
{
	FIntVector Position = world->GlobalToLocal(position);
	int Radius = FMath::Ceil(radius);
	for (int x = -Radius; x <= Radius; x++)
	{
		for (int y = -Radius; y <= Radius; y++)
		{
			for (int z = -Radius; z <= Radius; z++)
			{
				FIntVector CurrentPosition = Position + FIntVector(x, y, z);
				float Distance = FVector(x, y, z).Size();
				float Alpha = (radius - Distance) / radius;
				FColor CurrentColor = FLinearColor::LerpUsingHSV(world->GetColor(CurrentPosition), color, FMath::Clamp(Alpha, 0.f, 1.f)).ToFColor(true);

				world->SetColor(CurrentPosition, CurrentColor);
				if (bQueueUpdate)
				{
					world->ScheduleUpdate(CurrentPosition);
				}
			}
		}
	}
	if (bApplyUpdates)
	{
		world->ApplyQueuedUpdates();
	}
}
