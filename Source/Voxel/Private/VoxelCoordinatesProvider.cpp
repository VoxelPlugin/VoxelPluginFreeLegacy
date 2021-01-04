// Copyright 2021 Phyronnaz

#include "VoxelCoordinatesProvider.h"
#include "VoxelUtilities/VoxelVectorUtilities.h"

FVoxelVector FVoxelCoordinatesProvider::GlobalToLocalFloat(const FVoxelVector& Position) const
{
	return GetVoxelTransform().InverseTransformPosition(Position) / GetVoxelSize();
}

FVoxelVector FVoxelCoordinatesProvider::LocalToGlobalFloat(const FVoxelVector& Position) const
{
	return GetVoxelTransform().TransformPosition(Position * GetVoxelSize());
}

FIntVector FVoxelCoordinatesProvider::GlobalToLocal(const FVoxelVector& Position, EVoxelWorldCoordinatesRounding Rounding) const
{
	const FVoxelDoubleVector LocalPosition = GlobalToLocalFloat(Position);

	switch (Rounding)
	{
	default: ensure(false);
	case EVoxelWorldCoordinatesRounding::RoundToNearest: return FVoxelUtilities::RoundToInt(LocalPosition);
	case EVoxelWorldCoordinatesRounding::RoundUp: return FVoxelUtilities::CeilToInt(LocalPosition);
	case EVoxelWorldCoordinatesRounding::RoundDown: return FVoxelUtilities::FloorToInt(LocalPosition);
	}
}

FVoxelVector FVoxelCoordinatesProvider::LocalToGlobal(const FIntVector& Position) const
{
	return LocalToGlobalFloat(FVoxelVector(Position));
}

FBox FVoxelCoordinatesProvider::LocalToGlobalBounds(const FVoxelIntBox& Bounds) const
{
	return Bounds.ApplyTransformFloatImpl([&](const FIntVector& Position) { return LocalToGlobal(Position); });
}

FVoxelIntBox FVoxelCoordinatesProvider::GlobalToLocalBounds(const FBox& Bounds) const
{
	FVoxelIntBoxWithValidity Result;

	FVector Vertices[8] =
	{
		FVector(Bounds.Min),
		FVector(Bounds.Min.X, Bounds.Min.Y, Bounds.Max.Z),
		FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Min.Z),
		FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Min.Z),
		FVector(Bounds.Max.X, Bounds.Max.Y, Bounds.Min.Z),
		FVector(Bounds.Max.X, Bounds.Min.Y, Bounds.Max.Z),
		FVector(Bounds.Min.X, Bounds.Max.Y, Bounds.Max.Z),
		FVector(Bounds.Max)
	};
	for (FVector& Vertex : Vertices)
	{
		Result += GlobalToLocal(Vertex);
	}

	return Result.GetBox();
}