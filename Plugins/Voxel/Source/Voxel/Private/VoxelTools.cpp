// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelTools.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "EmptyWorldGenerator.h"
#include "VoxelData.h"
#include "VoxelPart.h"

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ SetValueSphere"), STAT_SetValueSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ SetMaterialSphere"), STAT_SetMaterialSphere, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ SetValueProjection"), STAT_SetValueProjection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ SetMaterialProjection"), STAT_SetMaterialProjection, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ SmoothValue"), STAT_SmoothValue, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ ImportMesh"), STAT_ImportMesh, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ ApplyWaterEffect"), STAT_ApplyWaterEffect, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("VoxelTool ~ RemoveNonConnectedBlocks"), STAT_RemoveNonConnectedBlocks, STATGROUP_Voxel);

void UVoxelTools::SetValueSphere(AVoxelWorld* World, const FVector Position, const float Radius, const bool bAdd, const bool bAsync, const float HardnessMultiplier)
{
	SCOPE_CYCLE_COUNTER(STAT_SetValueSphere);

	if (!World)
	{
		UE_LOG(VoxelLog, Error, TEXT("SetValueSphere: World is NULL"));
		return;
	}

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		Data->BeginSet();
		for (int X = -IntRadius; X <= IntRadius; X++)
		{
			for (int Y = -IntRadius; Y <= IntRadius; Y++)
			{
				for (int Z = -IntRadius; Z <= IntRadius; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
					const float Distance = FVector(X, Y, Z).Size();

					if (Distance <= Radius + 2)
					{
						// We want (Radius - Distance) != 0
						const float Noise = (Radius - Distance == 0) ? 0.0001f : 0;
						float Value = FMath::Clamp(Radius - Distance + Noise, -2.f, 2.f) / 2;

						Value *= HardnessMultiplier;
						Value *= (bAdd ? -1 : 1);

						float OldValue;
						FVoxelMaterial Dummy;
						Data->GetValueAndMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, OldValue, Dummy, LastOctree);

						if ((bAdd && (Value <= 0 || OldValue * Value >= 0)) || (!bAdd && (Value > 0 || OldValue * Value > 0)))
						{
							//DrawDebugPoint(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), 5, FColor::Red, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::UpVector), FColor::Green, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::RightVector), FColor::Green, false, 1);
							//DrawDebugLine(World->GetWorld(), World->GetTransform().TransformPosition((FVector)CurrentPosition), World->GetTransform().TransformPosition((FVector)CurrentPosition + FVector::ForwardVector), FColor::Green, false, 1);
							Data->SetValue(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, Value, LastOctree);
						}
					}
				}
			}
		}
		Data->EndSet();
	}
	World->UpdateChunksOverlappingBox(FVoxelBox(LocalPosition + FIntVector(1, 1, 1) * -IntRadius, LocalPosition + FIntVector(1, 1, 1) * IntRadius), bAsync);
}

//void UVoxelTools::SetValueBox(AVoxelWorld* const World, const FVector Position, const float ExtentXInVoxel, const float ExtentYInVoxel, const float ExtentZInVoxel, const bool bAdd, const bool bAsync, const float HardnessMultiplier)
//{
//	if (World == nullptr)
//	{
//		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
//		return;
//	}
//	check(World);
//
//	FIntVector LocalPosition = World->GlobalToLocal(Position);
//	int IntExtentX = FMath::CeilToInt(ExtentXInVoxel);
//	int IntExtentY = FMath::CeilToInt(ExtentYInVoxel);
//	int IntHeight = FMath::CeilToInt(ExtentZInVoxel * 2);
//
//	float Value = HardnessMultiplier * (bAdd ? -1 : 1);
//
//	for (int X = -IntExtentX; X <= IntExtentX; X++)
//	{
//		for (int Y = -IntExtentY; Y <= IntExtentY; Y++)
//		{
//			for (int Z = 0; Z <= IntHeight; Z++)
//			{
//				FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
//
//				if ((Value < 0 && bAdd) || (Value >= 0 && !bAdd) || (World->GetValue(CurrentPosition) * Value > 0))
//				{
//					World->SetValue(CurrentPosition, Value);
//					World->UpdateChunksAtPosition(CurrentPosition, bAsync);
//				}
//			}
//		}
//	}
//}

void UVoxelTools::SetMaterialSphere(AVoxelWorld* World, const FVector Position, const float Radius, const uint8 MaterialIndex, const bool bUseLayer1, const float FadeDistance, const bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_SetMaterialSphere);

	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("SetMaterialSphere: World is NULL"));
		return;
	}

	FIntVector LocalPosition = World->GlobalToLocal(Position);

	const float VoxelDiagonalLength = 1.73205080757f;
	const int Size = FMath::CeilToInt(Radius + FadeDistance + VoxelDiagonalLength);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		Data->BeginSet();
		for (int X = -Size; X <= Size; X++)
		{
			for (int Y = -Size; Y <= Size; Y++)
			{
				for (int Z = -Size; Z <= Size; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
					const float Distance = FVector(X, Y, Z).Size();


					float Dummy;
					FVoxelMaterial Material;
					Data->GetValueAndMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, Dummy, Material, LastOctree);

					if (Distance < Radius + FadeDistance + VoxelDiagonalLength)
					{
						// Set alpha
						int8 Alpha = 255 * FMath::Clamp((Radius + FadeDistance - Distance) / FadeDistance, 0.f, 1.f);
						if (bUseLayer1)
						{
							Alpha = 256 - Alpha;
						}
						if ((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
						{
							// Same color
							Alpha = bUseLayer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
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
						Data->SetMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, Material, LastOctree);
					}
					else if (Distance < Radius + FadeDistance + 2 * VoxelDiagonalLength && (bUseLayer1 ? Material.Index1 : Material.Index2) != MaterialIndex)
					{
						Material.Alpha = bUseLayer1 ? 255 : 0;
						Data->SetMaterial(CurrentPosition.X, CurrentPosition.Y, CurrentPosition.Z, Material, LastOctree);
					}
				}
			}
		}
		Data->EndSet();
	}
	World->UpdateChunksOverlappingBox(FVoxelBox(LocalPosition + FIntVector(1, 1, 1) * -Size, LocalPosition + FIntVector(1, 1, 1) * Size), bAsync);
}


void FindModifiedPositionsForRaycasts(AVoxelWorld* World, const FVector Position, const FVector Direction, const float Radius, const float MaxDistance, const float Precision,
	const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, std::forward_list<TTuple<FIntVector, float>>& OutModifiedPositionsAndDistances)
{
	const FVector ToolPosition = Position + Direction * MaxDistance / 2;

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
	const FVector Bitangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();

	TSet<FIntVector> AddedPoints;

	// Scale to make sure we don't miss any point when rounding
	const float Scale = Precision * World->GetVoxelSize();

	for (int X = -Radius; X <= Radius; X += Scale)
	{
		for (int Y = -Radius; Y <= Radius; Y += Scale)
		{
			const float Distance = FVector2D(X, Y).Size();
			if (Distance < Radius)
			{
				FHitResult Hit;
				// Use 2D basis
				FVector Start = ToolPosition + (Tangent * X + Bitangent * Y);
				FVector End = Start - Direction * MaxDistance;
				if (bShowRaycasts)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic) && Hit.Actor->IsA(AVoxelWorld::StaticClass()))
				{
					if (bShowHitPoints)
					{
						DrawDebugPoint(World->GetWorld(), Hit.ImpactPoint, 2, FColor::Red, false, 1);
					}
					for (auto Point : World->GetNeighboringPositions(Hit.ImpactPoint))
					{
						if (bShowModifiedVoxels)
						{
							DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(Point), 3, FColor::White, false, 1);
						}
						if (!AddedPoints.Contains(Point))
						{
							AddedPoints.Add(Point);
							OutModifiedPositionsAndDistances.push_front(TTuple<FIntVector, float>(Point, Distance));
						}
					}
				}
			}
		}
	}
}

void UVoxelTools::SetValueProjection(AVoxelWorld* World, const FVector Position, const FVector Direction, const float Radius, const float Strength, const bool bAdd,
	const float MaxDistance, const float Precision, const bool bAsync, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, const float MinValue, const float MaxValue)
{
	SCOPE_CYCLE_COUNTER(STAT_SetValueProjection);

	if (!World)
	{
		UE_LOG(VoxelLog, Error, TEXT("SetValueProjection: World is NULL"));
		return;
	}

	std::forward_list<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, Position, Direction, Radius, MaxDistance, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances);

	for (auto Tuple : ModifiedPositionsAndDistances)
	{
		const FIntVector Point = Tuple.Get<0>();
		const float Distance = Tuple.Get<1>();
		if (bAdd)
		{
			World->SetValue(Point, FMath::Clamp(World->GetValue(Point) - Strength, MinValue, MaxValue));
		}
		else
		{
			World->SetValue(Point, FMath::Clamp(World->GetValue(Point) + Strength, MinValue, MaxValue));
		}
		World->UpdateChunksAtPosition(Point, bAsync);
	}
}

void UVoxelTools::SetMaterialProjection(AVoxelWorld * World, const FVector Position, const FVector Direction, const float Radius, const uint8 MaterialIndex, const bool bUseLayer1,
	const float FadeDistance, const float MaxDistance, const float Precision, const bool bAsync, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_SetMaterialProjection);

	if (!World)
	{
		UE_LOG(VoxelLog, Error, TEXT("SetMaterialProjection: World is NULL"));
		return;
	}

	const float VoxelDiagonalLength = 1.73205080757f * World->GetVoxelSize();

	std::forward_list<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, Position, Direction, Radius + FadeDistance + 2 * VoxelDiagonalLength, MaxDistance, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances);

	for (auto Tuple : ModifiedPositionsAndDistances)
	{
		const FIntVector Point = Tuple.Get<0>();
		const float Distance = Tuple.Get<1>();

		FVoxelMaterial Material = World->GetMaterial(Point);

		if (Distance < Radius + FadeDistance + VoxelDiagonalLength)
		{
			// Set alpha
			int8 Alpha = 255 * FMath::Clamp((Radius + FadeDistance - Distance) / FadeDistance, 0.f, 1.f);
			if (bUseLayer1)
			{
				Alpha = 256 - Alpha;
			}
			if ((bUseLayer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
			{
				// Same color
				Alpha = bUseLayer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
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
			World->SetMaterial(Point, Material);
			World->UpdateChunksAtPosition(Point, bAsync);
		}
		else if ((bUseLayer1 ? Material.Index1 : Material.Index2) != MaterialIndex)
		{
			Material.Alpha = bUseLayer1 ? 255 : 0;
			World->SetMaterial(Point, Material);
			World->UpdateChunksAtPosition(Point, bAsync);
		}
	}
}

void UVoxelTools::SmoothValue(AVoxelWorld * World, FVector Position, FVector Direction, float Radius, float Speed, float MaxDistance,
	bool bAsync, bool bDebugLines, bool bDebugPoints, float MinValue, float MaxValue)
{
	SCOPE_CYCLE_COUNTER(STAT_SmoothValue);

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
	float Scale = World->GetVoxelSize() / 4;

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
				if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldDynamic) && Hit.Actor->IsA(AVoxelWorld::StaticClass()))
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
		World->UpdateChunksAtPosition(Point, bAsync);
	}
}

void UVoxelTools::ImportAsset(AVoxelWorld* World, UVoxelAsset* Asset, FVector Position, const bool bPositionZIsBottom, const bool bForceUseOfAllVoxels, const bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_ImportMesh);

	if (!World)
	{
		UE_LOG(VoxelLog, Error, TEXT("ImportAsset: World is NULL"));
		return;
	}
	check(World);

	if (!Asset)
	{
		UE_LOG(VoxelLog, Error, TEXT("ImportAsset: Asset is NULL"));
		return;
	}

	FIntVector P = World->GlobalToLocal(Position);

	FDecompressedVoxelAsset* DecompressedAsset;
	Asset->GetDecompressedAsset(DecompressedAsset, World->GetVoxelSize());

	FVoxelBox Bounds = DecompressedAsset->GetBounds();
	FVoxelData* Data = World->GetData();
	FValueOctree* LastOctree = nullptr;

	if (bPositionZIsBottom)
	{
		P.Z -= Bounds.Min.Z;
	}

	{
		Data->BeginSet();
		for (int X = Bounds.Min.X; X < Bounds.Max.X; X++)
		{
			for (int Y = Bounds.Min.Y; Y < Bounds.Max.Y; Y++)
			{
				for (int Z = Bounds.Min.Z; Z < Bounds.Max.Z; Z++)
				{
					const float AssetValue = DecompressedAsset->GetValue(X, Y, Z);
					const FVoxelMaterial AssetMaterial = DecompressedAsset->GetMaterial(X, Y, Z);
					const FVoxelType VoxelType = DecompressedAsset->GetVoxelType(X, Y, Z);

					if (bForceUseOfAllVoxels)
					{
						Data->SetValueAndMaterial(P.X + X, P.Y + Y, P.Z + Z, AssetValue, AssetMaterial, LastOctree);
					}
					else if (VoxelType.GetValueType() != IgnoreValue || VoxelType.GetMaterialType() != IgnoreMaterial)
					{
						float OldValue;
						FVoxelMaterial OldMaterial;
						Data->GetValueAndMaterial(P.X + X, P.Y + Y, P.Z + Z, OldValue, OldMaterial, LastOctree);

						const FVoxelMaterial NewMaterial = (VoxelType.GetMaterialType() == UseMaterial) ? AssetMaterial : OldMaterial;
						float NewValue;

						switch (VoxelType.GetValueType())
						{
						case IgnoreValue:
							NewValue = OldValue;
							break;
						case UseValue:
							NewValue = AssetValue;
							break;
						case UseValueIfSameSign:
							NewValue = (AssetValue * OldValue >= 0) ? AssetValue : OldValue;
							break;
						case UseValueIfDifferentSign:
							NewValue = (AssetValue * OldValue <= 0) ? AssetValue : OldValue;
							break;
						default:
							check(false);
						}

						Data->SetValueAndMaterial(P.X + X, P.Y + Y, P.Z + Z, NewValue, NewMaterial, LastOctree);
					}
				}
			}
		}
		Data->EndSet();
	}

	World->UpdateChunksOverlappingBox(FVoxelBox(Bounds.Min + P, Bounds.Max + P), bAsync);

	delete DecompressedAsset;
}

void UVoxelTools::GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController, AVoxelWorld*& World, FVector& Position, FVector& Normal, FVector& CameraDirection, EBlueprintSuccess& Branches)
{
	FHitResult HitResult;
	if (PlayerController->GetWorld()->LineTraceSingleByChannel(HitResult, WorldPosition, WorldPosition + WorldDirection * MaxDistance, ECC_WorldDynamic))
	{
		if (HitResult.Actor->IsA(AVoxelWorld::StaticClass()))
		{
			World = Cast<AVoxelWorld>(HitResult.Actor.Get());
			check(World);

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

void UVoxelTools::GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches)
{
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

void UVoxelTools::ApplyWaterEffect(AVoxelWorld* WaterWorld, AVoxelWorld* SolidWorld, FVector Position, float Radius, float DownSpeed, float LateralSpeed, bool bAsync, float ValueMultiplier)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyWaterEffect);

	if (SolidWorld == nullptr || WaterWorld == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(SolidWorld);
	check(WaterWorld);

	// Position in voxel space
	FIntVector LocalPosition = WaterWorld->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;


	for (int Z = -IntRadius + 1; Z < IntRadius; Z++)
	{
		for (int Y = -IntRadius + 1; Y < IntRadius; Y++)
		{
			for (int X = -IntRadius + 1; X < IntRadius; X++)
			{
				const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
				const float CurrentSolidValue = SolidWorld->GetValue(CurrentPosition);
				if (CurrentSolidValue <= 0)
				{
					WaterWorld->SetValue(CurrentPosition, -CurrentSolidValue);
					//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
				}
				else
				{
					float CurrentValue = WaterWorld->GetValue(CurrentPosition);

					//DrawDebugString(WaterWorld->GetWorld(), WaterWorld->GetTransform().TransformPosition((FVector)CurrentPosition), FString::FromInt(1000 * CurrentValue), 0, FColor::Black, 0.1f, false);

					if (CurrentValue < ValueMultiplier)
					{
						{
							const FIntVector BelowPosition = CurrentPosition + FIntVector(0, 0, -1);
							const float BelowValue = WaterWorld->GetValue(BelowPosition);

							const FIntVector AbovePosition = CurrentPosition + FIntVector(0, 0, 1);
							const float AboveValue = WaterWorld->GetValue(AbovePosition);

							const float BelowSolidValue = SolidWorld->GetValue(BelowPosition);

							float DownDeltaValue = 0;

							if (Z != 1 - IntRadius && BelowValue > -ValueMultiplier && (CurrentValue <= 0 || (BelowValue <= 0 && AboveValue >= 0)) && BelowSolidValue > 0)
							{
								DownDeltaValue = FMath::Clamp(FMath::Min(1 - CurrentValue, 1 + BelowValue), 0.f, DownSpeed) + KINDA_SMALL_NUMBER;

								WaterWorld->SetValue(CurrentPosition, CurrentValue + DownDeltaValue);
								WaterWorld->SetValue(BelowPosition, BelowValue - DownDeltaValue);

								//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
								//WaterWorld->QueueUpdate(BelowPosition, bAsync);

								CurrentValue = CurrentValue + DownDeltaValue;
							}
						}

						{
							const FIntVector ForwardPosition = CurrentPosition + FIntVector(1, 0, 0);
							const float ForwardValue = WaterWorld->GetValue(ForwardPosition);
							const float ForwardSolidValue = SolidWorld->GetValue(ForwardPosition);

							if (X != IntRadius - 1 && CurrentValue < ForwardValue && ForwardValue > -ValueMultiplier && ForwardSolidValue > 0)
							{
								float DeltaValue = FMath::Clamp(ForwardValue - CurrentValue, 0.f, LateralSpeed) / 4 + KINDA_SMALL_NUMBER;

								WaterWorld->SetValue(CurrentPosition, CurrentValue + DeltaValue);
								WaterWorld->SetValue(ForwardPosition, ForwardValue - DeltaValue);

								//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
								//WaterWorld->QueueUpdate(ForwardPosition, bAsync);

								CurrentValue = CurrentValue + DeltaValue;
							}
						}
						{
							const FIntVector BackPosition = CurrentPosition + FIntVector(-1, 0, 0);
							const float BackValue = WaterWorld->GetValue(BackPosition);
							const float BackSolidValue = SolidWorld->GetValue(BackPosition);

							if (X != 1 - IntRadius && CurrentValue < BackValue && BackValue > -ValueMultiplier && BackSolidValue > 0)
							{
								float DeltaValue = FMath::Clamp(BackValue - CurrentValue, 0.f, LateralSpeed) / 3 + KINDA_SMALL_NUMBER;

								WaterWorld->SetValue(CurrentPosition, CurrentValue + DeltaValue);
								WaterWorld->SetValue(BackPosition, BackValue - DeltaValue);

								//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
								//WaterWorld->QueueUpdate(BackPosition, bAsync);

								CurrentValue = CurrentValue + DeltaValue;
							}
						}
						{
							const FIntVector RightPosition = CurrentPosition + FIntVector(0, 1, 0);
							const float RightValue = WaterWorld->GetValue(RightPosition);
							const float RightSolidValue = SolidWorld->GetValue(RightPosition);

							if (Y != IntRadius - 1 && CurrentValue < RightValue && RightValue > -ValueMultiplier && RightSolidValue > 0)
							{
								float DeltaValue = FMath::Clamp(RightValue - CurrentValue, 0.f, LateralSpeed) / 2 + KINDA_SMALL_NUMBER;

								WaterWorld->SetValue(CurrentPosition, CurrentValue + DeltaValue);
								WaterWorld->SetValue(RightPosition, RightValue - DeltaValue);

								//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
								//WaterWorld->QueueUpdate(RightPosition, bAsync);

								CurrentValue = CurrentValue + DeltaValue;
							}
						}
						{
							const FIntVector LeftPosition = CurrentPosition + FIntVector(0, -1, 0);
							const float LeftValue = WaterWorld->GetValue(LeftPosition);
							const float LeftSolidValue = SolidWorld->GetValue(LeftPosition);

							if (Y != 1 - IntRadius && CurrentValue < LeftValue && LeftValue > -ValueMultiplier && LeftSolidValue > 0)
							{
								float DeltaValue = FMath::Clamp(LeftValue - CurrentValue, 0.f, LateralSpeed) / 1 + KINDA_SMALL_NUMBER;

								WaterWorld->SetValue(CurrentPosition, CurrentValue + DeltaValue);
								WaterWorld->SetValue(LeftPosition, LeftValue - DeltaValue);

								//WaterWorld->QueueUpdate(CurrentPosition, bAsync);
								//WaterWorld->QueueUpdate(LeftPosition, bAsync);

								CurrentValue = CurrentValue + DeltaValue;
							}
						}
					}
				}
			}
		}
	}

	WaterWorld->UpdateAll(bAsync);
}

void UVoxelTools::RemoveNonConnectedBlocks(AVoxelWorld* World, FVector Position, float Radius, bool bBordersAreConnected, bool bAsync, float ValueMultiplier)
{
	SCOPE_CYCLE_COUNTER(STAT_RemoveNonConnectedBlocks);

	if (World == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("World is NULL"));
		return;
	}
	check(World);

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;

	TArray<bool> Visited;
	Visited.SetNumZeroed(2 * IntRadius * 2 * IntRadius * 2 * IntRadius);

	std::forward_list<FIntVector> Queue;
	Queue.push_front(FIntVector::ZeroValue);

	for (int X = -IntRadius; X < IntRadius; X++)
	{
		for (int Y = -IntRadius; Y < IntRadius; Y++)
		{
			Queue.push_front(FIntVector(-IntRadius, X, Y));
			Queue.push_front(FIntVector(IntRadius - 1, X, Y));

			Queue.push_front(FIntVector(X, -IntRadius, Y));
			Queue.push_front(FIntVector(X, IntRadius - 1, Y));

			Queue.push_front(FIntVector(X, Y, -IntRadius));
			Queue.push_front(FIntVector(X, Y, IntRadius - 1));
		}
	}

	while (!Queue.empty())
	{
		const FIntVector RelativePosition = Queue.front();
		Queue.pop_front();

		const int X = RelativePosition.X;
		const int Y = RelativePosition.Y;
		const int Z = RelativePosition.Z;

		const int Index = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;

		if ((-IntRadius <= X) && (X < IntRadius)
			&& (-IntRadius <= Y) && (Y < IntRadius)
			&& (-IntRadius <= Z) && (Z < IntRadius)
			&& !Visited[Index]
			&& (World->GetValue(RelativePosition + LocalPosition) < 0))
		{

			Visited[Index] = true;

			Queue.push_front(FIntVector(X - 1, Y - 1, Z - 1));
			Queue.push_front(FIntVector(X + 0, Y - 1, Z - 1));
			Queue.push_front(FIntVector(X + 1, Y - 1, Z - 1));
			Queue.push_front(FIntVector(X - 1, Y + 0, Z - 1));
			Queue.push_front(FIntVector(X + 0, Y + 0, Z - 1));
			Queue.push_front(FIntVector(X + 1, Y + 0, Z - 1));
			Queue.push_front(FIntVector(X - 1, Y + 1, Z - 1));
			Queue.push_front(FIntVector(X + 0, Y + 1, Z - 1));
			Queue.push_front(FIntVector(X + 1, Y + 1, Z - 1));
			Queue.push_front(FIntVector(X - 1, Y - 1, Z + 0));
			Queue.push_front(FIntVector(X + 0, Y - 1, Z + 0));
			Queue.push_front(FIntVector(X + 1, Y - 1, Z + 0));
			Queue.push_front(FIntVector(X - 1, Y + 0, Z + 0));
			Queue.push_front(FIntVector(X + 0, Y + 0, Z + 0));
			Queue.push_front(FIntVector(X + 1, Y + 0, Z + 0));
			Queue.push_front(FIntVector(X - 1, Y + 1, Z + 0));
			Queue.push_front(FIntVector(X + 0, Y + 1, Z + 0));
			Queue.push_front(FIntVector(X + 1, Y + 1, Z + 0));
			Queue.push_front(FIntVector(X - 1, Y - 1, Z + 1));
			Queue.push_front(FIntVector(X + 0, Y - 1, Z + 1));
			Queue.push_front(FIntVector(X + 1, Y - 1, Z + 1));
			Queue.push_front(FIntVector(X - 1, Y + 0, Z + 1));
			Queue.push_front(FIntVector(X + 0, Y + 0, Z + 1));
			Queue.push_front(FIntVector(X + 1, Y + 0, Z + 1));
			Queue.push_front(FIntVector(X - 1, Y + 1, Z + 1));
			Queue.push_front(FIntVector(X + 0, Y + 1, Z + 1));
			Queue.push_front(FIntVector(X + 1, Y + 1, Z + 1));
		}
	}


	uint8 Depth = FMath::CeilToInt(FMath::Log2(2 * IntRadius / 16.f));
	UVoxelWorldGenerator* WorldGenerator = NewObject<UEmptyWorldGenerator>();

	TSharedPtr<FVoxelData> Data = MakeShareable(new FVoxelData(Depth, WorldGenerator));

	std::forward_list<FIntVector> PointPositions;

	for (int Z = -IntRadius; Z < IntRadius; Z++)
	{
		for (int Y = -IntRadius; Y < IntRadius; Y++)
		{
			for (int X = -IntRadius; X < IntRadius; X++)
			{
				const int Index = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;
				const FIntVector RelativePosition = FIntVector(X, Y, Z);
				const FIntVector CurrentPosition = RelativePosition + LocalPosition;
				if (!Visited[Index] && World->GetValue(CurrentPosition) <= 0)
				{
					Data->SetValue(RelativePosition.X, RelativePosition.Y, RelativePosition.Z, World->GetValue(CurrentPosition));
					Data->SetMaterial(RelativePosition.X, RelativePosition.Y, RelativePosition.Z, World->GetMaterial(CurrentPosition));

					// Set external colors
					TArray<FIntVector> L = {
						FIntVector(1, 0, 0),
						FIntVector(0, 1, 0),
						FIntVector(1, 1, 0),
						FIntVector(0, 0, 1),
						FIntVector(1, 0, 1),
						FIntVector(0, 1, 1),
						FIntVector(1, 1, 1),
						FIntVector(-1, 0, 0),
						FIntVector(0, -1, 0),
						FIntVector(-1, -1, 0),
						FIntVector(0, 0, -1),
						FIntVector(-1, 0, -1),
						FIntVector(0, -1, -1),
						FIntVector(-1, -1, -1)
					};
					for (auto P : L)
					{
						Data->SetMaterial(RelativePosition.X + P.X, RelativePosition.Y + P.Y, RelativePosition.Z + P.Z, World->GetMaterial(CurrentPosition + P));
					}

					PointPositions.push_front(RelativePosition);

					World->SetValue(CurrentPosition, ValueMultiplier);
					World->UpdateChunksAtPosition(CurrentPosition, bAsync);
				}
			}
		}
	}

	while (!PointPositions.empty())
	{
		// Find all connected points

		TSharedPtr<FVoxelData> CurrentData = MakeShareable(new FVoxelData(Depth, WorldGenerator));

		FIntVector PointPosition = PointPositions.front();
		PointPositions.pop_front();

		const int X = PointPosition.X;
		const int Y = PointPosition.Y;
		const int Z = PointPosition.Z;
		const int Index = (X + IntRadius) + (Y + IntRadius) * 2 * IntRadius + (Z + IntRadius) * 2 * IntRadius * 2 * IntRadius;

		if (!Visited[Index])
		{
			Queue.resize(0);
			Queue.push_front(PointPosition);

			while (!Queue.empty())
			{
				const FIntVector CurrentPosition = Queue.front();
				Queue.pop_front();

				const int LX = CurrentPosition.X;
				const int LY = CurrentPosition.Y;
				const int LZ = CurrentPosition.Z;

				const int LIndex = (LX + IntRadius) + (LY + IntRadius) * 2 * IntRadius + (LZ + IntRadius) * 2 * IntRadius * 2 * IntRadius;

				if ((-IntRadius <= LX) && (LX < IntRadius)
					&& (-IntRadius <= LY) && (LY < IntRadius)
					&& (-IntRadius <= LZ) && (LZ < IntRadius)
					&& !Visited[LIndex])
				{

					float Value;
					FVoxelMaterial Material;
					Data->GetValueAndMaterial(LX, LY, LZ, Value, Material);

					if (Value < 0)
					{
						Visited[LIndex] = true;
						CurrentData->SetValue(LX, LY, LZ, Value);
						CurrentData->SetMaterial(LX, LY, LZ, Material);

						// Set external colors
						TArray<FIntVector> L = {
							FIntVector(1, 0, 0),
							FIntVector(0, 1, 0),
							FIntVector(1, 1, 0),
							FIntVector(0, 0, 1),
							FIntVector(1, 0, 1),
							FIntVector(0, 1, 1),
							FIntVector(1, 1, 1),
							FIntVector(-1, 0, 0),
							FIntVector(0, -1, 0),
							FIntVector(-1, -1, 0),
							FIntVector(0, 0, -1),
							FIntVector(-1, 0, -1),
							FIntVector(0, -1, -1),
							FIntVector(-1, -1, -1)
						};
						for (auto P : L)
						{
							auto Q = CurrentPosition + P;

							FVoxelMaterial LMaterial;
							float LValue;
							Data->GetValueAndMaterial(LX, LY, LZ, LValue, LMaterial);

							CurrentData->SetMaterial(Q.X, Q.Y, Q.Z, LMaterial);
						}

						Queue.push_front(FIntVector(LX - 1, LY - 1, LZ - 1));
						Queue.push_front(FIntVector(LX + 0, LY - 1, LZ - 1));
						Queue.push_front(FIntVector(LX + 1, LY - 1, LZ - 1));
						Queue.push_front(FIntVector(LX - 1, LY + 0, LZ - 1));
						Queue.push_front(FIntVector(LX + 0, LY + 0, LZ - 1));
						Queue.push_front(FIntVector(LX + 1, LY + 0, LZ - 1));
						Queue.push_front(FIntVector(LX - 1, LY + 1, LZ - 1));
						Queue.push_front(FIntVector(LX + 0, LY + 1, LZ - 1));
						Queue.push_front(FIntVector(LX + 1, LY + 1, LZ - 1));
						Queue.push_front(FIntVector(LX - 1, LY - 1, LZ + 0));
						Queue.push_front(FIntVector(LX + 0, LY - 1, LZ + 0));
						Queue.push_front(FIntVector(LX + 1, LY - 1, LZ + 0));
						Queue.push_front(FIntVector(LX - 1, LY + 0, LZ + 0));
						Queue.push_front(FIntVector(LX + 0, LY + 0, LZ + 0));
						Queue.push_front(FIntVector(LX + 1, LY + 0, LZ + 0));
						Queue.push_front(FIntVector(LX - 1, LY + 1, LZ + 0));
						Queue.push_front(FIntVector(LX + 0, LY + 1, LZ + 0));
						Queue.push_front(FIntVector(LX + 1, LY + 1, LZ + 0));
						Queue.push_front(FIntVector(LX - 1, LY - 1, LZ + 1));
						Queue.push_front(FIntVector(LX + 0, LY - 1, LZ + 1));
						Queue.push_front(FIntVector(LX + 1, LY - 1, LZ + 1));
						Queue.push_front(FIntVector(LX - 1, LY + 0, LZ + 1));
						Queue.push_front(FIntVector(LX + 0, LY + 0, LZ + 1));
						Queue.push_front(FIntVector(LX + 1, LY + 0, LZ + 1));
						Queue.push_front(FIntVector(LX - 1, LY + 1, LZ + 1));
						Queue.push_front(FIntVector(LX + 0, LY + 1, LZ + 1));
						Queue.push_front(FIntVector(LX + 1, LY + 1, LZ + 1));
					}
				}
			}


			AVoxelPart* Part = World->GetWorld()->SpawnActor<AVoxelPart>(Position, World->GetTransform().Rotator());

			Part->SetActorScale3D(World->GetVoxelSize() * FVector::OneVector);

			Part->Init(CurrentData.Get(), World->GetVoxelMaterial(), World);
		}
	}
}
