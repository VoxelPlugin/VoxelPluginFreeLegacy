// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelTools.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorld.h"
#include "VoxelMathUtilities.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelDebug/VoxelDebugManager.h"

#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelTools::Flatten"), STAT_UVoxelTools_Flatten, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelTools::Flatten(
	AVoxelWorld* World, 
	FVector Position,
	FVector Normal, 
	float WorldRadius, 
	float Strength,
	bool bDontModifyVoxelsAroundPosition,
	bool bDontModifyEmptyVoxels, 
	bool bDontModifyFullVoxels, 
	float LockTimeoutInSeconds,
	bool bShowModifiedVoxels,
	bool bShowTestedVoxels)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelTools_Flatten);
	CHECK_VOXELWORLD_IS_CREATED();

	const FVector LocalPosition = (FVector)World->GlobalToLocal(Position);
	const float Radius = WorldRadius / World->VoxelSize;
	const int32 IntRadius = FMath::CeilToInt(Radius);

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

	for (int32 X = -IntRadius; X <= IntRadius; X++)
	{
		for (int32 Y = -IntRadius; Y <= IntRadius; Y++)
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

	FVoxelData& Data = World->GetData();
	{
		FVoxelReadWriteScopeTryLock Lock(Data, Bounds, LockTimeoutInSeconds, "Flatten");
		if (!Lock.Success())
		{
			return false;
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
				const FVoxelValue Value = Data.GetValue(P, 0);
				if ((!Value.IsTotallyFull() || !bDontModifyFullVoxels) &&
					(!Value.IsTotallyEmpty() || !bDontModifyEmptyVoxels))
				{
					if (Data.IsInWorld(P))
					{
						Data.SetValue(P, FMath::Clamp<float>(Value.ToFloat() + (F - Value.ToFloat()) * Strength, -1, 1));
					}
				}
			}
		}
	}

	World->GetLODManager().UpdateBounds(Bounds, true);

	return true;
}
#undef LOCTEXT_NAMESPACE
