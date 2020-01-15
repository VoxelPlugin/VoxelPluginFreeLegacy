// Copyright 2020 Phyronnaz

#include "VoxelDebugUtilities.h"
#include "IntBox.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelToolHelpers.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Components/LineBatchComponent.h"

#define LOCTEXT_NAMESPACE "Voxel"

void UVoxelDebugUtilities::DrawDebugIntBox(
	AVoxelWorld* World,
	FIntBox Bounds,
	FTransform Transform,
	float Lifetime,
	float Thickness,
	FLinearColor Color)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();

	// Put it in local voxel world space
	const FVector Min = World->GetTransform().InverseTransformPosition(World->LocalToGlobal(Bounds.Min));
	const FVector Max = World->GetTransform().InverseTransformPosition(World->LocalToGlobal(Bounds.Max));

	const float BorderOffset = Thickness / 2;

	const FBox DebugBox(Min + BorderOffset, Max - BorderOffset);
	const FVector Extent = DebugBox.GetExtent();
	const FVector Center = DebugBox.GetCenter();
	const uint8 DepthPriority = 0;

	UWorld* const SceneWorld = World->GetWorld();

	Transform = World->GetTransform() * Transform;

	// no debug line drawing on dedicated server
	if (GEngine->GetNetMode(SceneWorld) != NM_DedicatedServer)
	{
		// this means foreground lines can't be persistent 
		ULineBatchComponent* const LineBatcher = (Lifetime > 0.f) ? SceneWorld->PersistentLineBatcher : SceneWorld->LineBatcher;
		if (LineBatcher)
		{
			float const LineLifeTime = (Lifetime > 0.f) ? Lifetime : LineBatcher->DefaultLifeTime;
			TArray<struct FBatchedLine> Lines;

			FVector Start = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, Extent.Z));
			FVector End = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, -Extent.Z));
			End = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, -Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, -Extent.Z));
			End = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, -Extent.Z));
			new(Lines)FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(Extent.X, Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(Extent.X, -Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			Start = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, Extent.Z));
			End = Transform.TransformPosition(Center + FVector(-Extent.X, Extent.Y, -Extent.Z));
			new(Lines) FBatchedLine(Start, End, Color, LineLifeTime, Thickness, DepthPriority);

			LineBatcher->DrawLines(Lines);
		}
	}
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
	CHECK_BOUNDS_ARE_VALID_VOID();

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
					float Value = Data.GetValue(X, Y, Z, 0).ToFloat();
					DrawDebugString(World->GetWorld(), World->LocalToGlobal(FIntVector(X, Y, Z)), LexToString(Value), nullptr, TextColor.ToFColor(false), Lifetime);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE