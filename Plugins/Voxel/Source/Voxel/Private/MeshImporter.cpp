#include "VoxelPrivatePCH.h"
#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "VoxelWorld.h"
#include "MeshImporter.h"

void MeshImporter::LineTrace(bool bAdd, UPrimitiveComponent* Component, TArray<std::forward_list<float>>& Values, TArray<bool>& IsInside, FTransform& WorldTransform, FIntVector IMin, FIntVector Size, int X, int Y, int Z, int DeltaX, int DeltaY, int DeltaZ)
{
	FVector StartPosition = WorldTransform.TransformPosition(static_cast<FVector>(FIntVector(X + IMin.X, Y + IMin.Y, Z + IMin.Z)));
	FHitResult HitResult;
	FVector EndPosition = WorldTransform.TransformPosition(static_cast<FVector>(FIntVector(X + DeltaX + IMin.X, Y + DeltaY + IMin.Y, Z + DeltaZ + IMin.Z)));
	FCollisionQueryParams Params;

	FVector F;
#if WITH_EDITOR
	if (Component->GetDistanceToCollision(StartPosition, F) < 0 || Component->GetDistanceToCollision(EndPosition, F) < 0)
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid mesh"));
	}
#endif // WITH_EDITOR

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

		DrawDebugPoint(Component->GetWorld(), StartPosition, 2, FColor::Red, true, 10);
		DrawDebugPoint(Component->GetWorld(), HitResult.ImpactPoint, 5, FColor::Black, true, 10);
		DrawDebugPoint(Component->GetWorld(), EndPosition, 2, FColor::Blue, true, 10);
		DrawDebugDirectionalArrow(Component->GetWorld(), StartPosition, HitResult.ImpactPoint, 2, FColor::Red, true, 10, 0, 1);
		*/

		float Distance = WorldTransform.InverseTransformVector(HitResult.ImpactPoint - StartPosition).Size();
		Values[X + Size.X * Y + Size.X * Size.Y * Z].push_front(StartSign * Distance);
		int Index = X + DeltaX + Size.X * (Y + DeltaY) + Size.X * Size.Y * (Z + DeltaZ);
		if (0 <= Index && Index < Size.X * Size.Y * Size.Z)
		{
			Values[Index].push_front(EndSign * (1 - Distance));
		}
		//DrawDebugString(Component->GetWorld(), (StartPosition + HitResult.ImpactPoint) / 2, FString::Printf(TEXT("%f"), Distance), 0, FColor::Black, 10, false);
	}
	else
	{
		if (((StartSign < 0 && bAdd) || (StartSign > 0 && !bAdd)) && Values[X + Size.X * Y + Size.X * Size.Y * Z].empty())
		{
			Values[X + Size.X * Y + Size.X * Size.Y * Z].push_front(StartSign);
		}
	}
}

void MeshImporter::ImportMesh(bool bAdd, UPrimitiveComponent* Component, AVoxelWorld* World, FVector MinBound, FVector MaxBound)
{
	FVector Min = World->GetTransform().InverseTransformPosition(MinBound);
	FVector Max = World->GetTransform().InverseTransformPosition(MaxBound);

	FIntVector IMin = FIntVector(FMath::FloorToInt(Min.X) - 2, FMath::FloorToInt(Min.Y) - 2, FMath::FloorToInt(Min.Z) - 2);
	FIntVector IMax = FIntVector(FMath::CeilToInt(Max.X) + 2, FMath::CeilToInt(Max.Y) + 2, FMath::CeilToInt(Max.Z) + 2);
	FIntVector Size = FIntVector(IMax.X - IMin.X, IMax.Y - IMin.Y, IMax.Z - IMin.Z);

	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);

	TArray<std::forward_list<float>> Values;
	TArray<bool> IsInside;
	Values.SetNum(Size.X * Size.Y * Size.Z);
	IsInside.SetNum(Size.X * Size.Y * Size.Z);

	FTransform WorldTransform = World->GetTransform();

	TArray<int> Delta;
	Delta.SetNum(3);
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
						LineTrace(bAdd, Component, Values, IsInside, WorldTransform, IMin, Size, X, Y, Z, Delta[0], Delta[1], Delta[2]);
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
				auto List = Values[X + Size.X * Y + Size.X * Size.Y * Z];
				if (!List.empty())
				{
					int Count = 0;
					float Total = 0;
					for (float Value : List)
					{
						Total += Value;
						Count++;
					}
					FIntVector Position(IMin.X + X, IMin.Y + Y, IMin.Z + Z);
					float NewValue = Total / Count;
					if (IsInside[X + Size.X * Y + Size.X * Size.Y * Z] || NewValue * World->GetValue(Position) > 0)
					{
						World->SetValue(Position, NewValue);
						World->QueueUpdate(Position);
					}
				}
			}
		}
	}
	World->ApplyQueuedUpdates(false);
}
