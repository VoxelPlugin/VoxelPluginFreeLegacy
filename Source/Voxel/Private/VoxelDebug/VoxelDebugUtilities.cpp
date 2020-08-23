// Copyright 2020 Phyronnaz

#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelIntBox.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelToolHelpers.h"

#include "Components/LineBatchComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

void UVoxelDebugUtilities::DrawDebugIntBox(
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	FTransform Transform,
	float Lifetime,
	float Thickness,
	FLinearColor Color)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();
	
	// no debug line drawing on dedicated server
	if (GEngine->GetNetMode(World->GetWorld()) == NM_DedicatedServer) return;

	DrawDebugIntBox(
		*World,
		World->GetLineBatchComponent(),
		Transform,
		Bounds,
		Lifetime,
		Thickness,
		Color);
}

void UVoxelDebugUtilities::DrawDebugIntBox(
	const AVoxelWorldInterface* World,
	FVoxelIntBox Box,
	float Lifetime,
	float Thickness,
	FLinearColor Color)
{
	DrawDebugIntBox(Cast<AVoxelWorld>(const_cast<AVoxelWorldInterface*>(World)), Box, FTransform(), Lifetime, Thickness, Color);
}

void UVoxelDebugUtilities::DrawDebugIntBox(
	const IVoxelWorldInterface& World,
	UVoxelLineBatchComponent& LineBatchComponent, 
	FTransform Transform,
	FVoxelIntBox Box, 
	float Lifetime, 
	float Thickness,
	FLinearColor Color)
{
	VOXEL_FUNCTION_COUNTER();
	
	const float LineLifeTime = (Lifetime > 0.f) ? Lifetime : LineBatchComponent.DefaultLifeTime;
	
	// Put it in local voxel world space
	const FVector Min = LineBatchComponent.GetComponentTransform().InverseTransformPosition(World.LocalToGlobal(Box.Min));
	const FVector Max = LineBatchComponent.GetComponentTransform().InverseTransformPosition(World.LocalToGlobal(Box.Max));

	const float BorderOffset = Thickness / 2;

	const FBox DebugBox(Min + BorderOffset, Max - BorderOffset);
	const FVector Extent = DebugBox.GetExtent();
	const FVector Center = DebugBox.GetCenter();
	const uint8 DepthPriority = 0;

	Transform = LineBatchComponent.GetComponentTransform() * Transform;

	{
		VOXEL_SCOPE_COUNTER("DrawLines");
		auto& Lines = LineBatchComponent.BatchedLines;

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
	}
	{
		VOXEL_SCOPE_COUNTER("MarkRenderStateDirty");
		LineBatchComponent.MarkRenderStateDirty();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDebugUtilities::DebugVoxelsInsideBounds(
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	FLinearColor Color,
	float Lifetime,
	float Thickness,
	bool bDebugDensities,
	FLinearColor TextColor)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();

	for (int32 X = Bounds.Min.X; X < Bounds.Max.X; X++)
	{
		for (int32 Y = Bounds.Min.Y; Y < Bounds.Max.Y; Y++)
		{
			for (int32 Z = Bounds.Min.Z; Z < Bounds.Max.Z; Z++)
			{
				DrawDebugIntBox(World, FVoxelIntBox(X, Y, Z), Lifetime, Thickness, Color);

				if (bDebugDensities)
				{
					auto& Data = World->GetData();
					FVoxelReadScopeLock Lock(Data, FVoxelIntBox(X, Y, Z), "DebugVoxelsInsideBox");
					float Value = Data.GetValue(X, Y, Z, 0).ToFloat();
					DrawDebugString(World->GetWorld(), World->LocalToGlobal(FIntVector(X, Y, Z)), LexToString(Value), nullptr, TextColor.ToFColor(false), Lifetime);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void UVoxelDebugUtilities::DrawDataOctreeImpl(const FVoxelData& Data, const FDrawDataOctreeSettings& Settings)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](const FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForRead());
		
		auto& LeafData = Leaf.GetData<T>();
		const auto Draw = [&](FColor Color)
		{
			UVoxelDebugUtilities::DrawDebugIntBox(Settings.World, Leaf.GetBounds(), Settings.Lifetime, 0, FLinearColor(Color));;
		};
		if (LeafData.HasData())
		{
			if (LeafData.HasAllocation())
			{
				if (LeafData.IsDirty())
				{
					Draw(Settings.DirtyColor);
				}
				else
				{
					if (Settings.bShowCached)
					{
						Draw(Settings.CachedColor);
					}
				}
			}
			else if (Settings.bShowSingle)
			{
				if (LeafData.IsDirty())
				{
					Draw(Settings.SingleDirtyColor);
				}
				else
				{
					Draw(Settings.SingleColor);
				}
			}
		}
	});
}

template VOXEL_API void UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelValue   >(const FVoxelData&, const FDrawDataOctreeSettings&);
template VOXEL_API void UVoxelDebugUtilities::DrawDataOctreeImpl<FVoxelMaterial>(const FVoxelData&, const FDrawDataOctreeSettings&);

void UVoxelDebugUtilities::DrawDataOctree(
	AVoxelWorld* World, 
	EVoxelDataType DataType, 
	float Lifetime, 
	bool bShowSingle,
	bool bShowCached,
	FColor SingleColor, 
	FColor SingleDirtyColor, 
	FColor CachedColor, 
	FColor DirtyColor)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	const FDrawDataOctreeSettings Settings
	{
		World,
		Lifetime,
		bShowSingle,
		bShowCached,
		SingleColor,
		SingleDirtyColor,
		CachedColor,
		DirtyColor
	};

	auto& Data = World->GetData();

	FVoxelReadScopeLock Lock(Data, FVoxelIntBox::Infinite, FUNCTION_FNAME);

	if (DataType == EVoxelDataType::Values) DrawDataOctreeImpl<FVoxelValue>(Data, Settings);
	if (DataType == EVoxelDataType::Materials) DrawDataOctreeImpl<FVoxelMaterial>(Data, Settings);
}