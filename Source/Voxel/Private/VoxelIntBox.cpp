// Copyright 2020 Phyronnaz

#include "VoxelIntBox.h"

TArray<FVoxelIntBox, TFixedAllocator<6>> FVoxelIntBox::Difference(const FVoxelIntBox& Other) const
{
    if (!Intersect(Other))
    {
        return { *this };
    }

    TArray<FVoxelIntBox, TFixedAllocator<6>> OutBoxes;

    if (Min.Z < Other.Min.Z)
    {
        // Add bottom
        OutBoxes.Emplace(Min, FIntVector(Max.X, Max.Y, Other.Min.Z));
    }
    if (Other.Max.Z < Max.Z)
    {
        // Add top
        OutBoxes.Emplace(FIntVector(Min.X, Min.Y, Other.Max.Z), Max);
    }

    const int32 MinZ = FMath::Max(Min.Z, Other.Min.Z);
    const int32 MaxZ = FMath::Min(Max.Z, Other.Max.Z);

    if (Min.X < Other.Min.X)
    {
        // Add X min
        OutBoxes.Emplace(FIntVector(Min.X, Min.Y, MinZ), FIntVector(Other.Min.X, Max.Y, MaxZ));
    }
    if (Other.Max.X < Max.X)
    {
        // Add X max
        OutBoxes.Emplace(FIntVector(Other.Max.X, Min.Y, MinZ), FIntVector(Max.X, Max.Y, MaxZ));
    }

    const int32 MinX = FMath::Max(Min.X, Other.Min.X);
    const int32 MaxX = FMath::Min(Max.X, Other.Max.X);

    if (Min.Y < Other.Min.Y)
    {
        // Add Y min
        OutBoxes.Emplace(FIntVector(MinX, Min.Y, MinZ), FIntVector(MaxX, Other.Min.Y, MaxZ));
    }
    if (Other.Max.Y < Max.Y)
    {
        // Add Y max
        OutBoxes.Emplace(FIntVector(MinX, Other.Max.Y, MinZ), FIntVector(MaxX, Max.Y, MaxZ));
    }

    return OutBoxes;
}
