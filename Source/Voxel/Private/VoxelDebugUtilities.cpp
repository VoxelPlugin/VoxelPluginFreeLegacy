// Copyright 2019 Phyronnaz

#include "VoxelDebugUtilities.h"
#include "IntBox.h"
#include "VoxelWorld.h"
#include "VoxelBlueprintErrors.h"
#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelToolsHelpers.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#define LOCTEXT_NAMESPACE "Voxel"

void UVoxelDebugUtilities::DrawDebugIntBox(AVoxelWorld* World, const FIntBox& Box, float Lifetime, float Thickness, FLinearColor Color)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVector Min = World->LocalToGlobal(Box.Min);
	FVector Max = World->LocalToGlobal(Box.Max);

	float BorderOffset = Thickness / 2;

	FBox DebugBox(Min + BorderOffset, Max - BorderOffset);

	DrawDebugBox(World->GetWorld(), DebugBox.GetCenter(), DebugBox.GetExtent(), Color.ToFColor(true), false, Lifetime, 0, Thickness);
}

void UVoxelDebugUtilities::DebugVoxelsInsideBounds(
	AVoxelWorld* World,
	FIntBox Bounds,
	FLinearColor Color,
	float Lifetime,
	float Thickness,
	bool bDebugDensities,
	FLinearColor TextColor)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	for (int32 X = Bounds.Min.X; X < Bounds.Max.X; X++)
	{
		for (int32 Y = Bounds.Min.Y; Y < Bounds.Max.Y; Y++)
		{
			for (int32 Z = Bounds.Min.Z; Z < Bounds.Max.Z; Z++)
			{
				DrawDebugIntBox(World, FIntBox(X, Y, Z), Lifetime, Thickness, Color);

				if (bDebugDensities)
				{
					auto& Data = World->GetData();
					FVoxelReadScopeLock Lock(Data, FIntBox(X, Y, Z), "DebugVoxelsInsideBox");
					FVoxelValue Value = Data.GetValue(X, Y, Z, 0);
					DrawDebugString(World->GetWorld(), World->LocalToGlobal(FIntVector(X, Y, Z)), Value.ToString(), nullptr, TextColor.ToFColor(false), Lifetime);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE