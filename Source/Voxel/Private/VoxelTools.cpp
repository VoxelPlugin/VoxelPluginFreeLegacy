// Copyright 2018 Phyronnaz

#include "VoxelTools.h"
#include "VoxelPrivate.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelData.h"
#include "FastNoise.h"
#include "VoxelWorld.h"
#include "VoxelUtilities.h"
#include "Engine/World.h"
#include "VoxelThread.h"
#include "VoxelChunksOwner.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SimulatePhysicsOnFloatingVoxelActors"), STAT_UVoxelTools_SimulatePhysicsOnFloatingVoxelActors, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::*.BeginSet"), STAT_UVoxelTools_BeginSet, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueSphere"), STAT_UVoxelTools_SetValueSphere, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueBox"), STAT_UVoxelTools_SetValueBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialBox"), STAT_UVoxelTools_SetMaterialBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialSphere"), STAT_UVoxelTools_SetMaterialSphere, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::FindModifiedPositionsForRaycasts"), STAT_UVoxelTools_FindModifiedPositionsForRaycasts, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueProjection"), STAT_UVoxelTools_SetValueProjection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialProjection"), STAT_UVoxelTools_SetMaterialProjection, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::Flatten"), STAT_UVoxelTools_Flatten, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::ImportAsset"), STAT_UVoxelTools_ImportAsset, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::RemoveFloatingBlocks"), STAT_UVoxelTools_RemoveFloatingBlocks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::TransformVoxelAsset"), STAT_UVoxelTools_TransformVoxelAsset, STATGROUP_Voxel);


void UVoxelTools::SetValueSphere(AVoxelWorld* World, const FVector Position, const float WorldRadius, const bool bAdd)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueSphere);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueSphere: World is NULL"));
		return;
	}
	const float Radius = WorldRadius / World->GetVoxelSize();

	// Position in voxel space
	FIntVector LocalPosition = World->GlobalToLocal(Position);
	int IntRadius = FMath::CeilToInt(Radius) + 2;
	
	FIntVector R(IntRadius + 1, IntRadius + 1, IntRadius + 1);
	const FIntBox Bounds(LocalPosition - R, LocalPosition + R);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}
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

						Value *= (bAdd ? -1 : 1);

						float OldValue = Data->GetValue(CurrentPosition);

						bool bValid;
						if ((Value <= 0 && bAdd) || (Value > 0 && !bAdd))
						{
							bValid = true;
						}
						else
						{
							bValid = FVoxelUtilities::HaveSameSign(OldValue, Value);
						}
						if (bValid)
						{
							if (LIKELY(Data->IsInWorld(CurrentPosition)))
							{
								Data->SetValue(CurrentPosition, Value, LastOctree);
							}
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetValueBox(AVoxelWorld* World, const FVector Position, const FIntVector Size, const bool bAdd)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueBox);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueBox: World is NULL"));
		return;
	}
	check(World);

	FIntVector LocalPosition = World->GlobalToLocal(Position);
	
	const FIntBox Bounds(LocalPosition, LocalPosition + Size);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}

		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					FIntVector P = LocalPosition + FIntVector(X, Y, Z);

					if (LIKELY(Data->IsInWorld(P)))
					{
						float Value;
						if (X == 0 || X == Size.X - 1 || Y == 0 || Y == Size.Y - 1 || Z == 0 || Z == Size.Z - 1)
						{
							Value = 0;;
						}
						else
						{
							Value = (bAdd ? -1 : 1);
						}

						if ((Value <= 0 && bAdd) || (Value > 0 && !bAdd) || FVoxelUtilities::HaveSameSign(Data->GetValue(P), Value))
						{
							Data->SetValue(P, Value, LastOctree);
						}
					}
				}
			}
		}

		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetMaterialBox(AVoxelWorld* World, FVector Position, FIntVector Size, uint8 MaterialIndex, EVoxelLayer Layer)
{
	
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialBox);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueBox: World is NULL"));
		return;
	}
	check(World);

	FIntVector LocalPosition = World->GlobalToLocal(Position);
	
	const FIntBox Bounds(LocalPosition, LocalPosition + Size);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}

		for (int X = 0; X < Size.X; X++)
		{
			for (int Y = 0; Y < Size.Y; Y++)
			{
				for (int Z = 0; Z < Size.Z; Z++)
				{
					FIntVector P = LocalPosition + FIntVector(X, Y, Z);

					if (LIKELY(Data->IsInWorld(P)))
					{
						FVoxelMaterial Material = Data->GetMaterial(P);

						Material.Alpha = Layer == EVoxelLayer::Layer1 ? 0 : 255;

						// Set index
						if (Layer == EVoxelLayer::Layer1)
						{
							Material.Index1 = MaterialIndex;
						}
						else
						{
							Material.Index2 = MaterialIndex;
						}

						if (Data->IsInWorld(P))
						{
							Data->SetMaterial(P, Material, LastOctree);
						}
					}
				}
			}
		}

		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}

void UVoxelTools::SetMaterialSphere(AVoxelWorld* World, const FVector Position, const float WorldRadius, const uint8 MaterialIndex, const EVoxelLayer Layer, const float FadeDistance, const float Exponent)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialSphere);

	if (World == nullptr)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetMaterialSphere: World is NULL"));
		return;
	}

	const float Radius = WorldRadius / World->GetVoxelSize();
	const FIntVector LocalPosition = World->GlobalToLocal(Position);
	const int Size = FMath::CeilToInt(Radius + FadeDistance);
	const float VoxelDiagonalLength = 1.73205080757f;

	FIntVector R(Size + 1, Size + 1, Size + 1);
	const FIntBox Bounds = FIntBox(LocalPosition - R, LocalPosition + R);

	FValueOctree* LastOctree = nullptr;
	FVoxelData* Data = World->GetData();

	{
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(Bounds);
		}
		for (int X = -Size; X <= Size; X++)
		{
			for (int Y = -Size; Y <= Size; Y++)
			{
				for (int Z = -Size; Z <= Size; Z++)
				{
					const FIntVector CurrentPosition = LocalPosition + FIntVector(X, Y, Z);
					const float Distance = FVector(X, Y, Z).Size();

					if (Distance <= Radius + FadeDistance + VoxelDiagonalLength)
					{
						FVoxelMaterial Material = Data->GetMaterial(CurrentPosition);

						// Set alpha
						float Blend = FMath::Clamp((Radius + FadeDistance - Distance) / FMath::Max(1.f, FadeDistance), 0.f, 1.f);
						if (Layer == EVoxelLayer::Layer1)
						{
							Blend = 1 - Blend;
						}

						int8 Alpha = FMath::Clamp<int>(FMath::Pow(Blend, Exponent) * 255, 0, 255);
						 
						if ((Layer == EVoxelLayer::Layer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
						{
							// Same index, don't override alpha if smaller
							Alpha = Layer == EVoxelLayer::Layer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
						}
						Material.Alpha = Alpha;

						// Set index
						if (Layer == EVoxelLayer::Layer1)
						{
							Material.Index1 = MaterialIndex;
						}
						else
						{
							Material.Index2 = MaterialIndex;
						}

						if (LIKELY(Data->IsInWorld(CurrentPosition)))
						{
							// Apply changes
							Data->SetMaterial(CurrentPosition, Material, LastOctree);
						}
					}
				}
			}
		}
		Data->EndSet(Octrees);
	}
	World->UpdateChunksOverlappingBox(Bounds);
}


void FindModifiedPositionsForRaycasts(AVoxelWorld* World, const FVector StartPosition, const FVector Direction, const float Radius, const float ToolHeight, const float Precision,
	const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, TArray<TTuple<FIntVector, float>>& OutModifiedPositionsAndDistances, bool bMultitrace)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_FindModifiedPositionsForRaycasts);

	const FVector ToolPosition = StartPosition - Direction * ToolHeight;

	/**
	* Create a 2D basis from (Tangent, Bitangent)
	*/
	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
	if (Direction.Z != 0)
	{
		Tangent.X = 1;
		Tangent.Y = 1;
		Tangent.Z = -1 / Direction.Z * (Direction.X * Tangent.X + Direction.Y * Tangent.Y);
	}
	else
	{
		Tangent = FVector(1, 0, 0);
	}
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
				FVector End = Start + Direction * ToolHeight * 2;
				if (bShowRaycasts)
				{
					DrawDebugLine(World->GetWorld(), Start, End, FColor::Magenta, false, 1);
				}
				bool bSuccess;
				if (bMultitrace)
				{
					TArray<FHitResult> Hits;
					if (World->GetWorld()->LineTraceMultiByChannel(Hits, Start, End, ECollisionChannel::ECC_WorldStatic))
					{
						bSuccess = false;
						for (auto& SingleHit : Hits)
						{
							if (SingleHit.Actor == World || (Cast<AVoxelChunksOwner>(SingleHit.Actor.Get()) && Cast<AVoxelChunksOwner>(SingleHit.Actor.Get())->World == World))
							{
								Hit = SingleHit;
								bSuccess = true;
								break;
							}
						}
					}
					else
					{
						bSuccess = false;
					}
				}
				else
				{
					if (World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_WorldStatic) && (Hit.Actor == World || (Cast<AVoxelChunksOwner>(Hit.Actor.Get()) && Cast<AVoxelChunksOwner>(Hit.Actor.Get())->World == World)))
					{
						bSuccess = true;
					}
					else
					{
						bSuccess = false;
					}
				}

				if (bSuccess)
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
						if (!AddedPoints.Contains(Point) && LIKELY(World->IsInWorld(Point)))
						{
							AddedPoints.Add(Point);
							OutModifiedPositionsAndDistances.Add(TTuple<FIntVector, float>(Point, Distance));
						}
					}
				}
			}
		}
	}
}

void UVoxelTools::SetValueProjection(TArray<FIntVector>& ModifiedPositions, AVoxelWorld* World, const FVector StartPosition, const FVector Direction, const float Radius, const float Strength, const bool bAdd,
	const float ToolHeight, const float Precision, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, const bool bEnableMultiTrace)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueProjection);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValueProjection: World is NULL"));
		return;
	}

	TArray<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, StartPosition, Direction, Radius, ToolHeight, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances, bEnableMultiTrace);
	
	if (ModifiedPositionsAndDistances.Num() > 0)
	{
		FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
		FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			Min.X = FMath::Min(Min.X, Point.X);
			Min.Y = FMath::Min(Min.Y, Point.Y);
			Min.Z = FMath::Min(Min.Z, Point.Z);

			Max.X = FMath::Max(Max.X, Point.X);
			Max.Y = FMath::Max(Max.Y, Point.Y);
			Max.Z = FMath::Max(Max.Z, Point.Z);
		}

		FVoxelData* Data = World->GetData();
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(FIntBox(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1)));
		}

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			const float Distance = Tuple.Get<1>();
			if (World->IsInWorld(Point))
			{
				if (bAdd)
				{
					Data->SetValue(Point, FMath::Clamp<float>(Data->GetValue(Point) - Strength, -1, 1));
				}
				else
				{
					Data->SetValue(Point, FMath::Clamp<float>(Data->GetValue(Point) + Strength, -1, 1));
				}
				World->UpdateChunksAtPosition(Point);
			}
		}

		Data->EndSet(Octrees);
		
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			World->UpdateChunksAtPosition(Point);
			ModifiedPositions.Add(Point);
		}
	}
}

void UVoxelTools::SetMaterialProjection(AVoxelWorld * World, const FVector StartPosition, const FVector Direction, const float Radius, const uint8 MaterialIndex, const EVoxelLayer Layer,
	const float FadeDistance, const float Exponent, const float ToolHeight, const float Precision, const bool bShowRaycasts, const bool bShowHitPoints, const bool bShowModifiedVoxels, const bool bEnableMultiTrace)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialProjection);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetMaterialProjection: World is NULL"));
		return;
	}

	const float VoxelDiagonalLength = 1.73205080757f * World->GetVoxelSize();

	TArray<TTuple<FIntVector, float>> ModifiedPositionsAndDistances;
	FindModifiedPositionsForRaycasts(World, StartPosition, Direction, Radius + FadeDistance + 2 * VoxelDiagonalLength, ToolHeight, Precision, bShowRaycasts, bShowHitPoints, bShowModifiedVoxels, ModifiedPositionsAndDistances, bEnableMultiTrace);
	
	if (ModifiedPositionsAndDistances.Num() > 0)
	{
		FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
		FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			Min.X = FMath::Min(Min.X, Point.X);
			Min.Y = FMath::Min(Min.Y, Point.Y);
			Min.Z = FMath::Min(Min.Z, Point.Z);

			Max.X = FMath::Max(Max.X, Point.X);
			Max.Y = FMath::Max(Max.Y, Point.Y);
			Max.Z = FMath::Max(Max.Z, Point.Z);
		}

		FVoxelData* Data = World->GetData();
		TArray<uint64> Octrees;
		{
			SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
			Octrees = Data->BeginSet(FIntBox(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1)));
		}

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector CurrentPosition = Tuple.Get<0>();
			const float Distance = Tuple.Get<1>();

			if (Distance <= Radius + FadeDistance + VoxelDiagonalLength)
			{
				FVoxelMaterial Material = Data->GetMaterial(CurrentPosition);

				// Set alpha
				float Blend = FMath::Clamp((Radius + FadeDistance - Distance) / FMath::Max(1.f, FadeDistance), 0.f, 1.f);
				if (Layer == EVoxelLayer::Layer1)
				{
					Blend = 1 - Blend;
				}

				int8 Alpha = FMath::Clamp<int>(FMath::Pow(Blend, Exponent) * 255, 0, 255);

				if ((Layer == EVoxelLayer::Layer1 ? Material.Index1 : Material.Index2) == MaterialIndex)
				{
					// Same index, don't override alpha if smaller
					Alpha = Layer == EVoxelLayer::Layer1 ? FMath::Min<uint8>(Alpha, Material.Alpha) : FMath::Max<uint8>(Alpha, Material.Alpha);
				}
				Material.Alpha = Alpha;

				// Set index
				if (Layer == EVoxelLayer::Layer1)
				{
					Material.Index1 = MaterialIndex;
				}
				else
				{
					Material.Index2 = MaterialIndex;
				}

				if (Data->IsInWorld(CurrentPosition))
				{
					Data->SetMaterial(CurrentPosition, Material);
				}
			}
		}

		Data->EndSet(Octrees);

		for (auto& Tuple : ModifiedPositionsAndDistances)
		{
			const FIntVector Point = Tuple.Get<0>();
			World->UpdateChunksAtPosition(Point);
		}
	}
}

void UVoxelTools::Flatten(AVoxelWorld* World, FVector Position, FVector Normal, float WorldRadius, float Strength, bool bDontModifyVoxelsAroundPosition, bool bDontModifyEmptyVoxels, bool bDontModifyFullVoxels, bool bShowModifiedVoxels, bool bShowTestedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_Flatten);

	if (!World)
	{
		UE_LOG(LogVoxel, Error, TEXT("Flatten: World is NULL"));
		return;
	}

	const FVector LocalPosition = World->GlobalToLocalFloat(Position);
	const float Radius = WorldRadius / World->GetVoxelSize();
	const int IntRadius = FMath::CeilToInt(Radius);

	/**
	 * Create a 2D basis from (Tangent, Bitangent)
	 */
	 // Compute tangent
	FVector Tangent;
	{
		// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
		if (Normal.Z != 0)
		{
			Tangent.X = 1;
			Tangent.Y = 1;
			Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
		}
		else
		{
			Tangent = FVector(1, 0, 0);
		}
		Tangent.Normalize();
	}

	// Compute bitangent
	const FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();
	const FPlane Plane(LocalPosition, Normal);

	TSet<TTuple<FIntVector, float>> Positions;
	TSet<FIntVector> AddedPositions;

	for (int X = -IntRadius; X <= IntRadius; X++)
	{
		for (int Y = -IntRadius; Y <= IntRadius; Y++)
		{
			if (FVector2D(X, Y).Size() <= Radius)
			{
				for (float Z = -1; Z < 1 + KINDA_SMALL_NUMBER; Z += 0.5)
				{
					FVector P = Tangent * X + Bitangent * Y + LocalPosition + Z * Normal;
					for (auto& N : World->GetNeighboringPositions(World->LocalToGlobalFloat(P)))
					{
						if (!AddedPositions.Contains(N))
						{
							if (bShowTestedVoxels)
							{
								DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(N), 5, Plane.PlaneDot((FVector)N) < 0 ? FColor::Purple : FColor::Cyan, false, 1);
							}

							Positions.Add(TTuple<FIntVector, float>(N, Plane.PlaneDot((FVector)N)));
							AddedPositions.Add(N);
						}
					}
				}
			}
		}
	}

	// We don't want to modify the normal
	if (bDontModifyVoxelsAroundPosition)
	{
		TSet<FIntVector> SafePoints(World->GetNeighboringPositions(Position));
		AddedPositions = AddedPositions.Difference(SafePoints);
	}

	FIntVector Min(MAX_int32, MAX_int32, MAX_int32);
	FIntVector Max(MIN_int32, MIN_int32, MIN_int32);
	for (auto& Point : AddedPositions)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
		Min.Z = FMath::Min(Min.Z, Point.Z);

		Max.X = FMath::Max(Max.X, Point.X);
		Max.Y = FMath::Max(Max.Y, Point.Y);
		Max.Z = FMath::Max(Max.Z, Point.Z);
	}

	FIntBox Bounds(Min, FIntVector(Max.X + 1, Max.Y + 1, Max.Z + 1));

	FVoxelData* Data = World->GetData();
	TArray<uint64> Octrees;
	{
		SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
		Octrees = Data->BeginSet(Bounds);
	}

	for (auto& T : Positions)
	{
		FIntVector P = T.Get<0>();
		float F = T.Get<1>();

		if (AddedPositions.Contains(P))
		{
			if (bShowModifiedVoxels)
			{
				DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(P), 10, FColor::Red, false, 1);
			}
			float Value = Data->GetValue(P);
			if ((KINDA_SMALL_NUMBER - 1.f < Value || !bDontModifyFullVoxels) &&
				(Value < 1.f - KINDA_SMALL_NUMBER || !bDontModifyEmptyVoxels))
			{
				if (Data->IsInWorld(P))
				{
					Data->SetValue(P, FMath::Clamp<float>(Value + (F - Value) * Strength, -1, 1));
				}
			}
		}
	}

	Data->EndSet(Octrees);

	World->UpdateChunksOverlappingBox(Bounds);
}


void UVoxelTools::GetVoxelWorld(FVector WorldPosition, FVector WorldDirection, float MaxDistance, APlayerController* PlayerController, bool bMultipleHits, AVoxelWorld*& World, FVector& HitPosition, FVector& HitNormal)
{
	if (!PlayerController)
	{
		UE_LOG(LogVoxel, Error, TEXT("GetVoxelWorld: Invalid PlayerController"));
		return;
	}

	TArray<FHitResult> HitResults;
	if (PlayerController->GetWorld()->LineTraceMultiByChannel(HitResults, WorldPosition, WorldPosition + WorldDirection * MaxDistance, ECC_WorldStatic))
	{
		for (int Index = 0; Index < HitResults.Num(); Index++)
		{
			auto& HitResult = HitResults[Index];
			HitPosition = HitResult.ImpactPoint;
			HitNormal = HitResult.ImpactNormal;

			if (HitResult.Actor->IsA(AVoxelWorld::StaticClass()))
			{
				World = Cast<AVoxelWorld>(HitResult.Actor.Get());
				check(World);
			}
		}
	}
}

void UVoxelTools::GetMouseWorldPositionAndDirection(APlayerController* PlayerController, FVector& WorldPosition, FVector& WorldDirection, EBlueprintSuccess& Branches)
{
	if (!PlayerController)
	{
		UE_LOG(LogVoxel, Error, TEXT("GetMouseWorldPositionAndDirection: Invalid PlayerController"));
		Branches = EBlueprintSuccess::Failed;
		return;
	}
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
					Branches = EBlueprintSuccess::Success;
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

