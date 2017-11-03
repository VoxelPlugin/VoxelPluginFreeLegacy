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
#include "Fluids.h"

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
	World->UpdateChunksOverlappingBox(FVoxelBox(LocalPosition + FIntVector(1, 1, 1) * -(IntRadius + 1), LocalPosition + FIntVector(1, 1, 1) * (IntRadius + 1)), bAsync);
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
	World->UpdateChunksOverlappingBox(FVoxelBox(LocalPosition + FIntVector(1, 1, 1) * -(Size + 1), LocalPosition + FIntVector(1, 1, 1) * (Size + 1)), bAsync);
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
							NewValue = 0;
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

void UVoxelTools::ApplyWaterEffect(AVoxelWorld* World, const int N, const bool bInit, TArray<float>& Dens0, TArray<float>& U0, TArray<float>& V0, TArray<float>& W0, const float Visc, const float Diff, const float Dt, TArray<float>& Dens, TArray<float>&U, TArray<float>& V, TArray<float>& W)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyWaterEffect);

	if (!World)
	{
		UE_LOG(VoxelLog, Error, TEXT("ApplyWaterEffect: World is NULL"));
		return;
	}

	FVoxelData* Data = World->GetData();
	if (Data)
	{
		if (Dens0.Num() == 0 || U0.Num() == 0 || V0.Num() == 0 || W0.Num() == 0 || bInit)
		{
			Dens0.SetNum((N + 2) * (N + 2) * (N + 2));
			U0.SetNum((N + 2) * (N + 2) * (N + 2));
			V0.SetNum((N + 2) * (N + 2) * (N + 2));
			W0.SetNum((N + 2) * (N + 2) * (N + 2));

			for (int i = 1; i < N + 1; i++)
			{
				for (int j = 1; j < N + 1; j++)
				{
					for (int k = 1; k < N + 1; k++)
					{
						Dens0[i + (N + 2) * j + (N + 2) * (N + 2) * k] = (FVector(i - 1, j - 1, k - 1) - FVector(N / 2, N / 2, N / 2)).Size() > N / 4 ? 0 : 1;
					}
				}
			}
		}

		for (int i = 1; i < N + 1; i++)
		{
			for (int j = 1; j < N + 1; j++)
			{
				for (int k = 2; k < N + 1; k++)
				{
					W0[i + (N + 2) * j + (N + 2) * (N + 2) * k] -= 10 * Dt;
				}
			}
		}

		if (Dens.Num() == 0 || U.Num() == 0 || V.Num() == 0)
		{
			Dens.SetNum((N + 2) * (N + 2) * (N + 2));
			U.SetNum((N + 2) * (N + 2) * (N + 2));
			V.SetNum((N + 2) * (N + 2) * (N + 2));
			W.SetNum((N + 2) * (N + 2) * (N + 2));
		}

		FluidStep(N, Dens0, U0, V0, W0, Visc, Diff, Dt, Dens, U, V, W);

		{
			Data->BeginSet();
			for (int i = 1; i < N + 1; i++)
			{
				for (int j = 1; j < N + 1; j++)
				{
					for (int k = 1; k < N + 1; k++)
					{
						Data->SetValue(i - 1, j - 1, k - 1, 1 - 2 * Dens[i + (N + 2) * j + (N + 2) *(N + 2) *k]);
					}
				}
			}
			Data->EndSet();
		}

		World->UpdateChunksOverlappingBox(FVoxelBox(FIntVector(-1, -1, -1), FIntVector(N + 1, N + 1, N + 1)), false);

		for (int i = 1; i < N + 1; i++)
		{
			for (int j = 1; j < N + 1; j++)
			{
				for (int k = 1; k < N + 1; k++)
				{
					Dens0[i + (N + 2) * j + (N + 2) *(N + 2) *k] = 0;
					U0[i + (N + 2) * j + (N + 2) *(N + 2) *k] = 0;
					V0[i + (N + 2) * j + (N + 2) *(N + 2) *k] = 0;
					W0[i + (N + 2) * j + (N + 2) *(N + 2) *k] = 0;
				}
			}
		}
	}
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
