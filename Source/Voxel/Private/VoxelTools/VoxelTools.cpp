// Copyright 2018 Phyronnaz

#include "VoxelTools/VoxelTools.h"
#include "VoxelLogStatDefinitions.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/HUD.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelData/VoxelData.h"
#include "FastNoise.h"
#include "VoxelWorld.h"
#include "VoxelUtilities.h"
#include "Engine/World.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRender/AsyncWorks/VoxelMCPolygonizerAsyncWork.h"
#include "Curves/CurveFloat.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::*.BeginSet"), STAT_UVoxelTools_BeginSet, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueSphere"),    STAT_UVoxelTools_SetValueSphere,    STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::AddSphere"),		   STAT_UVoxelTools_AddSphere,		   STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::RemoveSphere"),	   STAT_UVoxelTools_RemoveSphere,	   STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetValueBox"),	   STAT_UVoxelTools_SetValueBox,       STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::AddBox"),			   STAT_UVoxelTools_AddBox,	           STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::RemoveBox"),		   STAT_UVoxelTools_RemoveBox,         STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialBox"),    STAT_UVoxelTools_SetMaterialBox,    STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetMaterialSphere"), STAT_UVoxelTools_SetMaterialSphere, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::SetBoxAsDirty"), STAT_UVoxelTools_SetBoxAsDirty, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::Flatten"), STAT_UVoxelTools_Flatten, STATGROUP_Voxel);

#define LOCTEXT_NAMESPACE "VoxelTools"

void UVoxelTools::AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet)
{
	OutSet.Reset();
	for (auto& P : InSet)
	{
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z - 1));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 0));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 1));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::SetValueSphere(AVoxelWorld* World, FIntVector Position, float Radius, float Value)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueSphere);

	CHECK_WORLD_VOXELTOOLS(SetValueSphere);

	FIntVector R(FMath::CeilToInt(Radius) + 3);
	const FIntBox Bounds(Position - R, Position + R);

	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
		{
			float Distance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).Size();

			if (Distance <= Radius)
			{
				OldValue = Value;
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::AddSphere(AVoxelWorld* World, FIntVector Position, float Radius)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_AddSphere);

	CHECK_WORLD_VOXELTOOLS(AddSphere);

	FIntVector R(FMath::CeilToInt(Radius) + 3);
	const FIntBox Bounds(Position - R, Position + R);

	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
		{
			float Distance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).Size();

			if (Distance <= Radius + 2)
			{
				FVoxelValue NewValue = -FMath::Clamp(Radius - Distance, -2.f, 2.f) / 2;

				if (!NewValue.IsEmpty() || FVoxelUtilities::HaveSameSign(OldValue, NewValue))
				{
					OldValue = NewValue;
				}
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::RemoveSphere(AVoxelWorld* World, FIntVector Position, float Radius)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_RemoveSphere);

	CHECK_WORLD_VOXELTOOLS(RemoveSphere);

	FIntVector R(FMath::CeilToInt(Radius) + 3);
	const FIntBox Bounds(Position - R, Position + R);

	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
		{
			float Distance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).Size();

			if (Distance <= Radius + 2)
			{
				FVoxelValue NewValue = FMath::Clamp(Radius - Distance, -2.f, 2.f) / 2;

				if (NewValue.IsEmpty() || FVoxelUtilities::HaveSameSign(OldValue, NewValue))
				{
					OldValue = NewValue;
				}
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::SetMaterialSphere(AVoxelWorld* World, FIntVector Position, float Radius, FVoxelPaintMaterial PaintMaterial, UCurveFloat* StrengthCurve)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialSphere);

	CHECK_WORLD_VOXELTOOLS(SetMaterialSphere);

	FIntVector R(FMath::CeilToInt(Radius) + 3);
	const FIntBox Bounds(Position - R, Position + R);

	const float SquaredRadius = Radius * Radius;

	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
		{
			float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();

			if (SquaredDistance <= SquaredRadius)
			{
				PaintMaterial.ApplyToMaterial(Material, StrengthCurve ? StrengthCurve->FloatCurve.Eval(FMath::Sqrt(SquaredDistance / SquaredRadius)) : 1);
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::SetValueBox(AVoxelWorld* World, FIntBox Bounds, float Value)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetValueBox);

	CHECK_WORLD_VOXELTOOLS(SetValueBox);
	
	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
		{
			OldValue = Value;
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::AddBox(AVoxelWorld* World, FIntBox Bounds)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_AddBox);

	CHECK_WORLD_VOXELTOOLS(AddBox);
	
	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value)
		{
			if (X == Bounds.Min.X || X == Bounds.Max.X - 1 || Y == Bounds.Min.Y || Y == Bounds.Max.Y - 1 || Z == Bounds.Min.Z || Z == Bounds.Max.Z - 1)
			{
				Value = 0;
			}
			else
			{
				Value = -1;
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::RemoveBox(AVoxelWorld* World, FIntBox Bounds)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_RemoveBox);

	CHECK_WORLD_VOXELTOOLS(RemoveBox);
	
	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value)
		{
			if (X == Bounds.Min.X || X == Bounds.Max.X - 1 || Y == Bounds.Min.Y || Y == Bounds.Max.Y - 1 || Z == Bounds.Min.Z || Z == Bounds.Max.Z - 1)
			{
				if (!Value.IsEmpty())
				{
					Value = 0;
				}
			}
			else
			{
				Value = 1;
			}
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::SetMaterialBox(AVoxelWorld* World, FIntBox Bounds, FVoxelPaintMaterial InMaterial)
{	
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetMaterialBox);

	CHECK_WORLD_VOXELTOOLS(SetMaterialBox);
	
	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		Data->SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
		{
			InMaterial.ApplyToMaterial(Material);
		});
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTools::SetBoxAsDirty(AVoxelWorld* World, FIntBox Bounds, bool bSetValuesAsDirty, bool bSetMaterialsAsDirty)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_SetBoxAsDirty);

	CHECK_WORLD_VOXELTOOLS(SetBoxAsDirty);

	FVoxelData* Data = World->GetData();
	{
		FVoxelScopeSetLock Lock(Data, Bounds);

		if (bSetValuesAsDirty)
		{
			Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value)
			{

			});
		}
		if (bSetMaterialsAsDirty)
		{
			Data->SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
			{

			});
		}
	}
}

void UVoxelTools::RoundVoxels(AVoxelWorld* World, FIntBox Bounds)
{
	CHECK_WORLD_VOXELTOOLS(RoundVoxels);

	const FIntVector Size = Bounds.Size();

	TArray<FVoxelValue> Values;
	Values.SetNumUninitialized(Size.X * Size.Y * Size.Z);

	FVoxelData* Data = World->GetData();
	FVoxelScopeSetLock Lock(Data, Bounds);

	Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value) {}); // Make sure values are cached for faster access
	Data->GetValuesAndMaterials(Values.GetData(), nullptr, FVoxelWorldGeneratorQueryZone(Bounds, Size, 0), 0);
	
	FVoxelData::LastOctreeAccelerator OctreeAccelerator(Data);

#define VOXELINDEX(A, B, C) (A - Bounds.Min.X) + Size.X * (B - Bounds.Min.Y) + Size.X * Size.Y * (C - Bounds.Min.Z)
#define CHECKVOXEL(A, B, C) if (!Bounds.IsInside(A, B, C) || Values[VOXELINDEX(A, B, C)].IsEmpty() != bEmpty) continue;

	for (int Z = Bounds.Min.Z; Z < Bounds.Max.Z; Z++)
	{
		for (int Y = Bounds.Min.Y; Y < Bounds.Max.Y; Y++)
		{
			for (int X = Bounds.Min.X; X < Bounds.Max.X; X++)
			{
				auto& Value = Values[VOXELINDEX(X, Y, Z)];
				if(Value.IsTotallyEmpty() || Value.IsTotallyFull()) continue;
				bool bEmpty = Value.IsEmpty();
				CHECKVOXEL(X - 1, Y, Z);
				CHECKVOXEL(X + 1, Y, Z);
				CHECKVOXEL(X, Y - 1, Z);
				CHECKVOXEL(X, Y + 1, Z);
				CHECKVOXEL(X, Y, Z - 1);
				CHECKVOXEL(X, Y, Z + 1);
				OctreeAccelerator.SetValue(X, Y, Z, bEmpty ? FVoxelValue::Empty : FVoxelValue::Full);
			}
		}
	}
#undef CHECKVOXEL
#undef VOXELINDEX
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelTools::Flatten(AVoxelWorld* World, FVector Position, FVector Normal, float WorldRadius, float Strength, bool bDontModifyVoxelsAroundPosition, bool bDontModifyEmptyVoxels, bool bDontModifyFullVoxels, int TimeoutInMicroSeconds, bool bShowModifiedVoxels, bool bShowTestedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_Flatten);
	
	CHECK_WORLD_VOXELTOOLS(Flatten, false);

	const FVector LocalPosition = (FVector)World->GlobalToLocal(Position);
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
	TArray<FVoxelId> Octrees;
	{
		SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_BeginSet);
		if (!Data->TryBeginSet(Bounds, TimeoutInMicroSeconds, Octrees))
		{
			return false;
		}
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
			const FVoxelValue Value = Data->GetValue(P, 0);
			if ((!Value.IsTotallyFull() || !bDontModifyFullVoxels) &&
				(!Value.IsTotallyEmpty() || !bDontModifyEmptyVoxels))
			{
				if (Data->IsInWorld(P))
				{
					Data->SetValue(P, FMath::Clamp<float>(Value.ToFloat() + (F - Value.ToFloat()) * Strength, -1, 1));
				}
			}
		}
	}

	Data->EndSet(Octrees);

	World->UpdateChunksOverlappingBox(Bounds, true);

	return true;
}
#undef LOCTEXT_NAMESPACE
