// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelTools.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ValueOctree.h"
#include "ChunkOctree.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

void UVoxelTools::SetValueSphere(AVoxelWorld* World, FVector Position, float Radius, bool bAdd, bool bAsync, float ValueMultiplier)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;

	/**
	 * Shrink octree to speed up value access
	 */
	 // Corners of the box where modified values are
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

	// Get smallest octree
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

	for (int X = -IntRadius; X <= IntRadius; X++)
	{
		for (int Y = -IntRadius; Y <= IntRadius; Y++)
		{
			for (int Z = -IntRadius; Z <= IntRadius; Z++)
			{
				FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
				float Distance = FVector(X, Y, Z).Size();

				if (Distance <= Radius + 3)
				{
					float Value = FMath::Clamp(Radius - Distance, -2.f, 2.f) / 2;

					Value *= ValueMultiplier * (bAdd ? -1 : 1);

					if (SmallValueOctree->IsInOctree(CurrentPosition)) // Prevent crash
					{
						if ((bAdd && (Value <= 0 || SmallValueOctree->GetValue(CurrentPosition) * Value >= 0)) || (!bAdd && (Value > 0 || SmallValueOctree->GetValue(CurrentPosition) * Value > 0)))
						{
							//DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), 5, FColor::Red, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::UpVector), FColor::Green, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::RightVector), FColor::Green, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::ForwardVector), FColor::Green, false, 1);
							SmallValueOctree->SetValue(CurrentPosition, Value);
							World->QueueUpdate(CurrentPosition, bAsync);
						}
					}
				}
			}
		}
	}
}

void UVoxelTools::SetMaterialSphere(AVoxelWorld* World, FVector Position, float Radius, uint8 MaterialIndex, bool bUseLayer1, float FadeDistance, bool bAsync)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FIntVector LocalPosition = World->GlobalToLocal(Position);

	const float VoxelDiagonalLength = 1.73205080757f;
	int Size = FMath::CeilToInt(Radius + FadeDistance + VoxelDiagonalLength);

	for (int X = -Size; X <= Size; X++)
	{
		for (int Y = -Size; Y <= Size; Y++)
		{
			for (int Z = -Size; Z <= Size; Z++)
			{
				FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
				float Distance = FVector(X, Y, Z).Size();

				FVoxelMaterial Material = World->GetMaterial(CurrentPosition);
				if (Distance < Radius + FadeDistance + VoxelDiagonalLength)
				{
					// Set alpha
					float Alpha = FMath::Clamp((Radius + FadeDistance - Distance) / FadeDistance, 0.f, 1.f);
					if (bUseLayer1)
					{
						Alpha = 1 - Alpha;
					}
					if ((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
					{
						// Same color
						Alpha = bUseLayer1 ? FMath::Min(Alpha, Material.Alpha) : FMath::Max(Alpha, Material.Alpha);
					}
					Material.Alpha = Alpha;

					// Set index
					if (bUseLayer1)
					{
						Material.Index1 = MaterialIndex;
					}
					else
					{
						Material.Index2 = MaterialIndex;
					}

					// Apply changes
					World->SetMaterial(CurrentPosition, Material);
					World->QueueUpdate(CurrentPosition, bAsync);
				}
				else if (Distance < Radius + FadeDistance + 2 * VoxelDiagonalLength && !((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex))
				{
					Material.Alpha = bUseLayer1 ? 1 : 0;
					World->SetMaterial(CurrentPosition, Material);
					World->QueueUpdate(CurrentPosition, bAsync);
				}
			}
		}
	}
}

void UVoxelTools::SetValueCone(AVoxelWorld * World, FVector Position, float Radius, float Height, bool bAdd, bool bAsync, float ValueMultiplier)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);

	// Position in voxel space
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
					float Value = FMath::Clamp(CurrentRadius - Distance, -2.f, 2.f) / 2;

					Value *= ValueMultiplier * (bAdd ? -1 : 1);

					if ((Value < 0 && bAdd) || (Value >= 0 && !bAdd) || (World->GetValue(CurrentPosition) * Value > 0))
					{
						World->SetValue(CurrentPosition, Value);
						World->QueueUpdate(CurrentPosition, bAsync);
					}
				}
			}
		}
	}
}

// TODO: Rewrite
void UVoxelTools::SetValueProjection(AVoxelWorld* World, FVector Position, FVector Direction, float Radius, float Stength, bool bAdd,
									 float MaxDistance, bool bAsync, bool bDebugLines, bool bDebugPoints, float MinValue, float MaxValue)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Direction * MaxDistance / 2;

	/**
	 * Create a 2D basis from (Tangent, Bitangent)
	 */
	 // Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Direction.Z * (Direction.X * Tangent.X + Direction.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();

	TSet<FIntVector> ModifiedPositions;
	// Scale to make sure we don't miss any point when rounding
	float Scale = World->GetTransform().GetScale3D().Size() / 4;

	int IntRadius = FMath::CeilToInt(Radius);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			if (x*x + y*y < Radius*Radius)
			{
				FHitResult Hit;
				// Use precedent basis
				FVector Start = ToolPosition + (Tangent * x + Bitangent * y) * Scale;
				FVector End = Start - Direction * MaxDistance;
				if (bDebugLines)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic) && Hit.Actor->IsA(AVoxelChunk::StaticClass()))
				{
					if (bDebugPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}
					ModifiedPositions.Add(World->GlobalToLocal(Hit.ImpactPoint - Hit.ImpactNormal / 2 * (bAdd ? 1 : -1)));
				}
			}
		}
	}
	for (FIntVector Point : ModifiedPositions)
	{
		if (bAdd)
		{
			World->SetValue(Point, World->GetValue(Point) - Stength);
		}
		else
		{
			World->SetValue(Point, FMath::Clamp(World->GetValue(Point) + Stength, MinValue, MaxValue));
		}
		World->QueueUpdate(Point, bAsync);
	}
}

// TODO: Rewrite
void UVoxelTools::SetMaterialProjection(AVoxelWorld * World, FVector Position, FVector Direction, float Radius, uint8 MaterialIndex, bool bUseLayer1,
										float FadeDistance, float MaxDistance, bool bAsync, bool bDebugLines, bool bDebugPoints)
{
	if (World == NULL)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Direction * MaxDistance / 2;

	/**
	* Create a 2D basis from (Tangent, Bitangent)
	*/
	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Direction.Z * (Direction.X * Tangent.X + Direction.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();

	TArray<FIntVector> Positions;
	TArray<FVoxelMaterial> Materials;

	float Scale = World->GetTransform().GetScale3D().Size() / 4;
	const float VoxelDiagonalLength = World->GetTransform().GetScale3D().Size();

	int IntRadius = FMath::CeilToInt(Radius + FadeDistance);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			FHitResult Hit;
			// Use precedent basis
			FVector Start = ToolPosition + (Tangent * x + Bitangent * y) * Scale;
			FVector End = Start - Direction * MaxDistance;
			if (bDebugLines)
			{
				DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
			}

			if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic) && Hit.Actor->IsA(AVoxelChunk::StaticClass()))
			{
				if (bDebugPoints)
				{
					DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
				}
				TArray<FVector> CurrentPositions;
				CurrentPositions.Add(Hit.ImpactPoint);

				CurrentPositions.Add(Hit.ImpactPoint - Hit.ImpactNormal / 2);
				CurrentPositions.Add(Hit.ImpactPoint + Hit.ImpactNormal / 2);

				for (FVector CurrentPosition : CurrentPositions)
				{
					FIntVector CurrentLocalPosition = World->GlobalToLocal(CurrentPosition);
					float Distance = FVector2D(FVector::DotProduct(CurrentPosition - Position, Tangent), FVector::DotProduct(CurrentPosition - Position, Bitangent)).Size() / Scale;


					if (!Positions.Contains(CurrentLocalPosition))
					{
						Positions.Add(CurrentLocalPosition);
						FVoxelMaterial Material = World->GetMaterial(CurrentLocalPosition);
						if (Distance < Radius + FadeDistance + VoxelDiagonalLength)
						{
							// Set alpha
							float Alpha = FMath::Clamp((Radius + FadeDistance - Distance) / FadeDistance, 0.f, 1.f);
							if (bUseLayer1)
							{
								Alpha = 1 - Alpha;
							}
							if ((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
							{
								// Same color
								Alpha = bUseLayer1 ? FMath::Min(Alpha, Material.Alpha) : FMath::Max(Alpha, Material.Alpha);
							}
							Material.Alpha = Alpha;

							// Set index
							if (bUseLayer1)
							{
								Material.Index1 = MaterialIndex;
							}
							else
							{
								Material.Index2 = MaterialIndex;
							}

							// Apply changes
							World->SetMaterial(CurrentLocalPosition, Material);
							World->QueueUpdate(CurrentLocalPosition, bAsync);
						}
						else if (Distance < Radius + FadeDistance + 3 * VoxelDiagonalLength && !((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex))
						{
							Material.Alpha = bUseLayer1 ? 1 : 0;
							World->SetMaterial(CurrentLocalPosition, Material);
							World->QueueUpdate(CurrentLocalPosition, bAsync);
						}
					}
				}
			}
		}
	}
}

void UVoxelTools::SmoothValue(AVoxelWorld * World, FVector Position, FVector Direction, float Radius, float Speed, float MaxDistance,
							  bool bAsync, bool bDebugLines, bool bDebugPoints, float MinValue, float MaxValue)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);
	FVector ToolPosition = Position + Direction * MaxDistance / 2;

	/**
	* Create a 2D basis from (Tangent, Bitangent)
	*/
	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Direction.Z * (Direction.X * Tangent.X + Direction.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();

	TArray<FIntVector> ModifiedPositions;
	TArray<float> DistancesToTool;

	// Scale to make sure we don't miss any point when rounding
	float Scale = World->GetTransform().GetScale3D().Size() / 4;

	int IntRadius = FMath::CeilToInt(Radius);
	for (int x = -IntRadius; x <= IntRadius; x++)
	{
		for (int y = -IntRadius; y <= IntRadius; y++)
		{
			if (x*x + y*y < Radius*Radius)
			{
				FHitResult Hit;
				// Use precedent basis
				FVector Start = ToolPosition + (Tangent * x + Bitangent * y) * Scale;
				FVector End = Start - Direction * MaxDistance;
				if (bDebugLines)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic) && Hit.Actor->IsA(AVoxelChunk::StaticClass()))
				{
					if (bDebugPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}
					FIntVector ModifiedPosition = World->GlobalToLocal(Hit.ImpactPoint);
					if (!ModifiedPositions.Contains(ModifiedPosition))
					{
						ModifiedPositions.Add(ModifiedPosition);
						DistancesToTool.Add((Hit.ImpactPoint - Start).Size());
					}
				}
			}
		}
	}

	// Compute mean distance
	float MeanDistance(0);
	for (float Distance : DistancesToTool)
	{
		MeanDistance += Distance;
	}
	MeanDistance /= DistancesToTool.Num();

	// Debug
	if (bDebugPoints)
	{
		for (int x = -IntRadius; x <= IntRadius; x++)
		{
			for (int y = -IntRadius; y <= IntRadius; y++)
			{
				if (x*x + y*y < Radius*Radius)
				{
					FHitResult Hit;
					// Use precedent basis
					FVector Start = ToolPosition + (Tangent * x + Bitangent * y) * Scale;
					FVector End = Start - Direction * MaxDistance;

					DrawDebugPoint(World->GetWorld(), Start + (End - Start).GetSafeNormal() * MeanDistance, 2, FColor::Blue, false, 1);
				}
			}
		}
	}

	// Update values
	for (int i = 0; i < ModifiedPositions.Num(); i++)
	{
		FIntVector Point = ModifiedPositions[i];
		float Distance = DistancesToTool[i];
		float Delta = Speed * (MeanDistance - Distance);
		World->SetValue(Point, FMath::Clamp(Delta + World->GetValue(Point), MinValue, MaxValue));
		World->QueueUpdate(Point, bAsync);
	}
}

void UVoxelTools::ImportMesh(AVoxelWorld* World, TSubclassOf<AVoxelMeshAsset> MeshToImport, FVector Position, bool bAsync, bool bDebugPoints)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);

	auto MeshAsset = MeshToImport.GetDefaultObject();
	if (MeshAsset == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("MeshToImport is NULL"));
		return;
	}
	MeshAsset->ImportIntoWorld(World, World->GlobalToLocal(Position), bAsync, bDebugPoints);
}

void UVoxelTools::GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController, AVoxelWorld*& World, FVector& Position, FVector& Normal, FVector& CameraDirection, EBlueprintSuccess& Branches)
{
	FHitResult HitResult;
	if (PlayerController->GetWorld()->LineTraceSingleByChannel(HitResult, WorldPosition, WorldPosition + WorldDirection * MaxDistance, ECC_WorldDynamic))
	{
		if (HitResult.Actor->IsA(AVoxelChunk::StaticClass()))
		{
			World = Cast<AVoxelWorld>(HitResult.Actor->GetAttachParentActor());
			if (World)
			{
				Position = HitResult.ImpactPoint;
				Normal = HitResult.ImpactNormal;
				CameraDirection = (HitResult.TraceEnd - HitResult.TraceStart).GetSafeNormal();
				Branches = EBlueprintSuccess::Sucess;
			}
			else
			{
				Branches = EBlueprintSuccess::Failed;
			}
		}
		else
		{
			Branches = EBlueprintSuccess::Failed;
		}
	}
	else
	{
		Branches = EBlueprintSuccess::Failed;
	}
}

void UVoxelTools::GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches)
{
	UE_LOG(VoxelLog, Log, TEXT("Client adress: %s"), *PlayerController->GetPlayerNetworkAddress());
	UE_LOG(VoxelLog, Log, TEXT("Server adress: %s"), *PlayerController->GetServerNetworkAddress());

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player);

	if (PlayerController->GetLocalPlayer() && PlayerController->GetLocalPlayer()->ViewportClient)
	{
		FVector2D MousePosition;
		if (PlayerController->GetLocalPlayer()->ViewportClient->GetMousePosition(MousePosition))
		{
			// Early out if we clicked on a HUD hitbox
			if (PlayerController->GetHUD() != NULL && PlayerController->GetHUD()->GetHitBoxAtCoordinates(MousePosition, true))
			{
				Branches = EBlueprintSuccess::Failed;
			}
			else
			{
				if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, MousePosition, WorldPosition, WorldDirection) == true)
				{
					Branches = EBlueprintSuccess::Sucess;
				}
				else
				{
					Branches = EBlueprintSuccess::Failed;
				}
			}
		}
		else
		{
			Branches = EBlueprintSuccess::Failed;
		}
	}
	else
	{
		Branches = EBlueprintSuccess::Failed;
	}
}

void UVoxelTools::ApplyWaterEffect(AVoxelWorld* World, FVector Position, float Radius, float DownSpeed, float LateralSpeed, bool bAsync, float ValueMultiplier)
{
	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;

	/**
	* Shrink octree to speed up value access
	*/
	// Corners of the box where modified values are
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

	// Get smallest octree
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

	auto SmallValueOctreeCopy = SmallValueOctree;

	for (int Z = -IntRadius + 1; Z < IntRadius; Z++)
	{
		for (int Y = -IntRadius + 1; Y < IntRadius; Y++)
		{
			for (int X = -IntRadius + 1; X < IntRadius; X++)
			{
				const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
				float CurrentValue = SmallValueOctreeCopy->GetValue(CurrentPosition);

				if (CurrentValue < ValueMultiplier)
				{
					const FIntVector BelowPosition = CurrentPosition + FIntVector(0, 0, -1);
					const float BelowValue = SmallValueOctreeCopy->GetValue(BelowPosition);

					float DownDeltaValue = 0;

					if (Z != 1 - IntRadius && CurrentValue < BelowValue && BelowValue > -ValueMultiplier)
					{
						DownDeltaValue = FMath::Clamp(BelowValue - CurrentValue, 0.f, DownSpeed);

						SmallValueOctree->SetValue(CurrentPosition, CurrentValue + DownDeltaValue);
						SmallValueOctree->SetValue(BelowPosition, BelowValue - DownDeltaValue);

						World->QueueUpdate(CurrentPosition, bAsync);
						World->QueueUpdate(BelowPosition, bAsync);

						CurrentValue = CurrentValue + DownDeltaValue;
					}

					if (DownDeltaValue < DownSpeed / 2)
					{
						const FIntVector ForwardPosition = CurrentPosition + FIntVector(1, 0, 0);
						const float ForwardValue = SmallValueOctreeCopy->GetValue(ForwardPosition);

						if (X != IntRadius - 1 && CurrentValue < ForwardValue && ForwardValue > -ValueMultiplier)
						{
							float DeltaValue = FMath::Clamp(ForwardValue - CurrentValue, 0.f, LateralSpeed) / 4;

							SmallValueOctree->SetValue(CurrentPosition, CurrentValue + DeltaValue);
							SmallValueOctree->SetValue(ForwardPosition, ForwardValue - DeltaValue);

							World->QueueUpdate(CurrentPosition, bAsync);
							World->QueueUpdate(ForwardPosition, bAsync);

							CurrentValue = CurrentValue + DeltaValue;
						}

						const FIntVector BackPosition = CurrentPosition + FIntVector(-1, 0, 0);
						const float BackValue = SmallValueOctreeCopy->GetValue(BackPosition);

						if (X != 1 - IntRadius && CurrentValue < BackValue && BackValue > -ValueMultiplier)
						{
							float DeltaValue = FMath::Clamp(BackValue - CurrentValue, 0.f, LateralSpeed) / 3;

							SmallValueOctree->SetValue(CurrentPosition, CurrentValue + DeltaValue);
							SmallValueOctree->SetValue(BackPosition, BackValue - DeltaValue);

							World->QueueUpdate(CurrentPosition, bAsync);
							World->QueueUpdate(BackPosition, bAsync);

							CurrentValue = CurrentValue + DeltaValue;
						}

						const FIntVector RightPosition = CurrentPosition + FIntVector(0, 1, 0);
						const float RightValue = SmallValueOctreeCopy->GetValue(RightPosition);

						if (Y != IntRadius - 1 && CurrentValue < RightValue && RightValue > -ValueMultiplier)
						{
							float DeltaValue = FMath::Clamp(RightValue - CurrentValue, 0.f, LateralSpeed) / 2;

							SmallValueOctree->SetValue(CurrentPosition, CurrentValue + DeltaValue);
							SmallValueOctree->SetValue(RightPosition, RightValue - DeltaValue);

							World->QueueUpdate(CurrentPosition, bAsync);
							World->QueueUpdate(RightPosition, bAsync);

							CurrentValue = CurrentValue + DeltaValue;
						}

						const FIntVector LeftPosition = CurrentPosition + FIntVector(0, -1, 0);
						const float LeftValue = SmallValueOctreeCopy->GetValue(LeftPosition);

						if (Y != 1 - IntRadius && CurrentValue < LeftValue && LeftValue > -ValueMultiplier)
						{
							float DeltaValue = FMath::Clamp(LeftValue - CurrentValue, 0.f, LateralSpeed) / 1;

							SmallValueOctree->SetValue(CurrentPosition, CurrentValue + DeltaValue);
							SmallValueOctree->SetValue(LeftPosition, LeftValue - DeltaValue);

							World->QueueUpdate(CurrentPosition, bAsync);
							World->QueueUpdate(LeftPosition, bAsync);

							CurrentValue = CurrentValue + DeltaValue;
						}
					}
				}
			}
		}
	}
}