// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelMeshAsset.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"


AVoxelMeshAsset::AVoxelMeshAsset() : ValueMultiplier(1), MaxResolution(10000)
{
	DebugLineBatch = CreateDefaultSubobject<ULineBatchComponent>(FName("LineBatch"));
};

void AVoxelMeshAsset::LineTrace(UPrimitiveComponent* Component, TArray<std::forward_list<float>>& ValuesLists, FIntVector IMin, int X, int Y, int Z, int DeltaX, int DeltaY, int DeltaZ)
{
	FVector StartPosition = FVector(X + IMin.X, Y + IMin.Y, Z + IMin.Z) * VoxelSizeInUnit;
	FHitResult HitResult;
	FVector EndPosition = FVector(X + DeltaX + IMin.X, Y + DeltaY + IMin.Y, Z + DeltaZ + IMin.Z) * VoxelSizeInUnit;
	FCollisionQueryParams Params;

	FVector F;

	if (Component->GetDistanceToCollision(StartPosition, F) < 0 || Component->GetDistanceToCollision(EndPosition, F) < 0)
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid mesh"));
	}

	float StartSign;
	float EndSign;
	bool StartIsOutside = Component->GetDistanceToCollision(StartPosition, F) > 0;
	bool EndIsOutside = Component->GetDistanceToCollision(EndPosition, F) > 0;

	IsInside[X + Size.X * Y + Size.X * Size.Y * Z] = !StartIsOutside;

	if (bAdd)
	{
		StartSign = StartIsOutside ? 1 : -1;
		EndSign = EndIsOutside ? 1 : -1;
	}
	else
	{
		StartSign = StartIsOutside ? -1 : 1;
		EndSign = EndIsOutside ? -1 : 1;
	}

	if (Component->LineTraceComponent(HitResult, StartPosition, EndPosition, Params))
	{
		/*
		UE_LOG(VoxelLog, Warning, TEXT("Start: (%f, %f, %f"), StartPosition.X, StartPosition.Y, StartPosition.Z);
		UE_LOG(VoxelLog, Warning, TEXT("Impact: (%f, %f, %f"), HitResult.ImpactPoint.X, HitResult.ImpactPoint.Y, HitResult.ImpactPoint.Z);
		UE_LOG(VoxelLog, Warning, TEXT("End: (%f, %f, %f"), EndPosition.X, EndPosition.Y, EndPosition.Z);
		DrawDebugDirectionalArrow(Component->GetWorld(), StartPosition, HitResult.ImpactPoint, 2, FColor::Red, true, 10, 0, 1);
		DrawDebugString(Component->GetWorld(), (StartPosition + HitResult.ImpactPoint) / 2, FString::Printf(TEXT("%f"), Distance), 0, FColor::Black, 10, false);
		*/

		DebugLineBatch->DrawPoint(HitResult.ImpactPoint, FColor::Black, 5, 2);

		float Distance = (HitResult.ImpactPoint - StartPosition).Size() / VoxelSizeInUnit;
		ValuesLists[X + Size.X * Y + Size.X * Size.Y * Z].push_front(StartSign * Distance);
		int Index = X + DeltaX + Size.X * (Y + DeltaY) + Size.X * Size.Y * (Z + DeltaZ);
		if (0 <= Index && Index < Size.X * Size.Y * Size.Z)
		{
			ValuesLists[Index].push_front(EndSign * (1 - Distance));
		}
	}
	else
	{
		if (((StartSign < 0 && bAdd) || (StartSign > 0 && !bAdd)) && ValuesLists[X + Size.X * Y + Size.X * Size.Y * Z].empty())
		{
			ValuesLists[X + Size.X * Y + Size.X * Size.Y * Z].push_front(StartSign);
		}
	}
}

void AVoxelMeshAsset::Import()
{
	if (!ActorToImport)
	{
		UE_LOG(VoxelLog, Error, TEXT("ActorToImport is NULL"));
		return;
	}

	FVector Min = MinPosition->GetActorLocation();
	FVector Max = MaxPosition->GetActorLocation();

	Min = FVector(FMath::FloorToInt(Min.X), FMath::FloorToInt(Min.Y), FMath::FloorToInt(Min.Z)) / VoxelSizeInUnit;
	Max = FVector(FMath::CeilToInt(Max.X), FMath::CeilToInt(Max.Y), FMath::CeilToInt(Max.Z)) / VoxelSizeInUnit;

	FIntVector IMin = FIntVector(FMath::FloorToInt(Min.X), FMath::FloorToInt(Min.Y), FMath::FloorToInt(Min.Z));
	FIntVector IMax = FIntVector(FMath::CeilToInt(Max.X), FMath::CeilToInt(Max.Y), FMath::CeilToInt(Max.Z));

	if ((double)(IMax.X - IMin.X) * (double)(IMax.Y - IMin.Y) * (double)(IMax.Z - IMin.Z) > MaxResolution)
	{
		UE_LOG(VoxelLog, Error, TEXT("Resolution too high"));
		return;
	}
	if (IMax.X - IMin.X < 0 || IMax.Y - IMin.Y < 0 || IMax.Z - IMin.Z < 0)
	{
		UE_LOG(VoxelLog, Error, TEXT("Min > Max"));
		return;
	}

	UpdateLines();

	Size = FIntVector(IMax.X - IMin.X, IMax.Y - IMin.Y, IMax.Z - IMin.Z);

	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);

	// Values[i] = ValuesList[i].Mean()
	TArray<std::forward_list<float>> ValuesLists;
	ValuesLists.SetNum(Size.X * Size.Y * Size.Z);
	Values.SetNum(Size.X * Size.Y * Size.Z);
	IsInside.SetNum(Size.X * Size.Y * Size.Z);

	TArray<int> Delta;
	Delta.SetNum(3);
	UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(ActorToImport->FindComponentByClass(ComponentClass));
	if (Component == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("Component not found"));
		return;
	}
	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				for (int Sign = -1; Sign != 3; Sign += 2)
				{
					for (int i = 0; i < 3; i++)
					{
						Delta[0] = 0;
						Delta[1] = 0;
						Delta[2] = 0;

						Delta[i] = Sign;
						LineTrace(Component, ValuesLists, IMin, X, Y, Z, Delta[0], Delta[1], Delta[2]);
					}
				}
			}
		}
	}

	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				auto List = ValuesLists[Index];
				if (!List.empty())
				{
					int Count = 0;
					float Total = 0;
					for (float Value : List)
					{
						Total += Value;
						Count++;
					}
					Values[Index] = Total / Count;
				}
			}
		}
	}
	UE_LOG(VoxelLog, Log, TEXT("Import successful"));
}

void AVoxelMeshAsset::UpdateLines()
{
	if (!MinPosition)
	{
		UE_LOG(VoxelLog, Error, TEXT("MinPosition is NULL"));
		return;
	}
	if (!MaxPosition)
	{
		UE_LOG(VoxelLog, Error, TEXT("MaxPosition is NULL"));
		return;
	}

	DebugLineBatch->Flush();

	FVector Min = MinPosition->GetActorLocation();
	FVector Max = MaxPosition->GetActorLocation();

	Min = FVector(FMath::FloorToInt(Min.X), FMath::FloorToInt(Min.Y), FMath::FloorToInt(Min.Z)) / VoxelSizeInUnit;
	Max = FVector(FMath::CeilToInt(Max.X), FMath::CeilToInt(Max.Y), FMath::CeilToInt(Max.Z)) / VoxelSizeInUnit;

	FIntVector IMin = FIntVector(FMath::FloorToInt(Min.X), FMath::FloorToInt(Min.Y), FMath::FloorToInt(Min.Z));
	FIntVector IMax = FIntVector(FMath::CeilToInt(Max.X), FMath::CeilToInt(Max.Y), FMath::CeilToInt(Max.Z));

	if ((double)(IMax.X - IMin.X) * (double)(IMax.Y - IMin.Y) * (double)(IMax.Z - IMin.Z) > MaxResolution)
	{
		UE_LOG(VoxelLog, Error, TEXT("Resolution too high"));
		return;
	}
	if (IMax.X - IMin.X < 0 || IMax.Y - IMin.Y < 0 || IMax.Z - IMin.Z < 0)
	{
		UE_LOG(VoxelLog, Error, TEXT("Min > Max"));
		return;
	}

	TArray<FBatchedLine> Lines;

	TArray<int> Delta;

	Delta.SetNum(3);

	if ((double)(IMax.X - IMin.X) * (double)(IMax.Y - IMin.Y) * (double)(IMax.Z - IMin.Z) < 25000)
	{
		for (int X = IMin.X; X < IMax.X; X++)
		{
			for (int Y = IMin.Y; Y < IMax.Y; Y++)
			{
				for (int Z = IMin.Z; Z < IMax.Z; Z++)
				{
					for (int Sign = -1; Sign != 3; Sign += 2)
					{
						for (int i = 0; i < 3; i++)
						{
							Delta[0] = 0;
							Delta[1] = 0;
							Delta[2] = 0;

							Delta[i] = Sign;
							FVector Start = FVector(X, Y, Z) * VoxelSizeInUnit;
							FVector End = FVector(X + Delta[0], Y + Delta[1], Z + Delta[2]) * VoxelSizeInUnit;
							Lines.Add(FBatchedLine(Start, End, FColor::Red, -1, VoxelSizeInUnit / 100, 0));
						}
					}
				}
			}
		}
	}
	else
	{
		Min = MinPosition->GetActorLocation();
		Max = MaxPosition->GetActorLocation();

		Lines.Add(FBatchedLine(Min, Min + FVector::RightVector * (Max.X - Min.X), FColor::Red, -1, 10, 0));
		Lines.Add(FBatchedLine(Min, Min + FVector::ForwardVector * (Max.Y - Min.Y), FColor::Red, -1, 10, 0));
		Lines.Add(FBatchedLine(Min, Min + FVector::UpVector * (Max.Z - Min.Z), FColor::Red, -1, 10, 0));

		Lines.Add(FBatchedLine(Max, Max - FVector::RightVector * (Max.X - Min.X), FColor::Red, -1, 10, 0));
		Lines.Add(FBatchedLine(Max, Max - FVector::ForwardVector * (Max.Y - Min.Y), FColor::Red, -1, 10, 0));
		Lines.Add(FBatchedLine(Max, Max - FVector::UpVector * (Max.Z - Min.Z), FColor::Red, -1, 10, 0));
	}
	DebugLineBatch->DrawLines(Lines);
}

void AVoxelMeshAsset::ImportIntoWorld(AVoxelWorld* World, FIntVector Position, bool bAsync, bool bDebugPoints)
{
	check(World);
	for (int X = 0; X < Size.X; X++)
	{
		for (int Y = 0; Y < Size.Y; Y++)
		{
			for (int Z = 0; Z < Size.Z; Z++)
			{
				int Index = X + Size.X * Y + Size.X * Size.Y * Z;
				FIntVector CurrentPosition = Position + FIntVector(X, Y, Z);

				if (bDebugPoints)
				{
					DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), 5, FColor::Red, false, 5);
				}

				if (IsInside[Index] || Values[Index] * World->GetValue(CurrentPosition) > 0)
				{
					World->SetValue(CurrentPosition, Values[Index] * ValueMultiplier);
					World->QueueUpdate(CurrentPosition);
				}
			}
		}
	}
	World->ApplyQueuedUpdates(bAsync);
}
