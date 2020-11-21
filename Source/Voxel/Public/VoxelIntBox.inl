// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelIntBox.h"

template<typename T>
FORCEINLINE bool FVoxelIntBox::IterateBorders_TrueForAll(T Lambda) const
{
    if (!ensure(IsValid())) return true;

    // Z Min
    for (int32 X = Min.X; X < Max.X; X++)
    {
        for (int32 Y = Min.Y; Y < Max.Y; Y++)
        {
            if (!Lambda(X, Y, Min.Z))
            {
                return false;
            }
        }
    }

    // Z Max
    if (Min.Z != Max.Z - 1)
    {
        for (int32 X = Min.X; X < Max.X; X++)
        {
            for (int32 Y = Min.Y; Y < Max.Y; Y++)
            {
                if (!Lambda(X, Y, Max.Z - 1))
                {
                    return false;
                }
            }
        }
    }

    // For X/Y, exclude Z Min/Max as they were already added above

    // X Min
    for (int32 Z = Min.Z + 1; Z < Max.Z - 1; Z++)
    {
        for (int32 Y = Min.Y; Y < Max.Y; Y++)
        {
            if (!Lambda(Min.X, Y, Z))
            {
                return false;
            }
        }
    }

    // X Max
    if (Min.X != Max.X - 1)
    {
        for (int32 Z = Min.Z + 1; Z < Max.Z - 1; Z++)
        {
            for (int32 Y = Min.Y; Y < Max.Y; Y++)
            {
                if (!Lambda(Max.X - 1, Y, Z))
                {
                    return false;
                }
            }
        }
    }

    // For Y, ignore X Min/Max as well

    // Y Min
    for (int32 Z = Min.Z + 1; Z < Max.Z - 1; Z++)
    {
        for (int32 X = Min.X + 1; X < Max.X - 1; X++)
        {
            if (!Lambda(X, Min.Y, Z))
            {
                return false;
            }
        }
    }

    // Y Max
    if (Min.Y != Max.Y - 1)
    {
        for (int32 Z = Min.Z + 1; Z < Max.Z - 1; Z++)
        {
            for (int32 X = Min.X + 1; X < Max.X - 1; X++)
            {
                if (!Lambda(X, Max.Y - 1, Z))
                {
                    return false;
                }
            }
        }
    }

    return true;
}