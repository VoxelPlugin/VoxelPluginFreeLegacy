// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "VoxelLandscapeAsset.h"
#include "Engine/World.h"
#include "BufferArchive.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "MemoryReader.h"
#include "Engine/Texture2D.h"


UVoxelLandscapeAsset::UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Precision(1)
	, HardnessMultiplier(1)
{
};

bool UVoxelLandscapeAsset::GetDecompressedAsset(FDecompressedVoxelAsset*& Asset, const float VoxelSize)
{
	Asset = new FDecompressedVoxelLandscapeAsset();
	bool bSuccess = Super::GetDecompressedAsset(Asset, VoxelSize);
	FDecompressedVoxelLandscapeAsset* LandscapeAsset = (FDecompressedVoxelLandscapeAsset*)Asset;
	LandscapeAsset->Precision = Precision;
	LandscapeAsset->HardnessMultiplier = HardnessMultiplier;
	LandscapeAsset->VoxelSize = VoxelSize;

	return bSuccess;
}

void UVoxelLandscapeAsset::AddAssetToArchive(FBufferArchive& ToBinary, FDecompressedVoxelAsset* Asset)
{
	ToBinary << *((FDecompressedVoxelLandscapeAsset*)Asset);
}

void UVoxelLandscapeAsset::GetAssetFromArchive(FMemoryReader& FromBinary, FDecompressedVoxelAsset* Asset)
{
	FromBinary << *((FDecompressedVoxelLandscapeAsset*)Asset);
}

float FDecompressedVoxelLandscapeAsset::GetValue(const int X, const int Y, const int Z)
{
	const int HalfSize = Size / 2;

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	const float CurrentHeight = Heights[(X + HalfSize) + Size * (Y + HalfSize)];

	if (CurrentHeight > (Z + Precision) * VoxelSize)
	{
		// If voxel over us is in, we're entirely in
		return -HardnessMultiplier;
	}
	else if ((Z - Precision) * VoxelSize > CurrentHeight)
	{
		// If voxel under us is out, we're entirely out
		return HardnessMultiplier;
	}
	else
	{
		float Alpha = (Z * VoxelSize - CurrentHeight) / VoxelSize / Precision;

		return Alpha * HardnessMultiplier;
	}
}

FVoxelMaterial FDecompressedVoxelLandscapeAsset::GetMaterial(const int X, const int Y, const int Z)
{
	const int HalfSize = Size / 2;

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	return Materials[(X + HalfSize) + Size * (Y + HalfSize)];
}

EVoxelType FDecompressedVoxelLandscapeAsset::GetVoxelType(const int X, const int Y, const int Z)
{
	const int HalfSize = Size / 2;

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	const float CurrentHeight = Heights[(X + HalfSize) + Size * (Y + HalfSize)];

	if ((Z - Precision) * VoxelSize < CurrentHeight || CurrentHeight < (Z + Precision) * VoxelSize)
	{
		return (Z * VoxelSize - CurrentHeight <= 0) ? EVoxelType::UseValue : EVoxelType::UseValueIfSameSign;
	}
	else
	{
		return EVoxelType::IgnoreValue;
	}
}

FVoxelBox FDecompressedVoxelLandscapeAsset::GetBounds()
{
	FIntVector Bound(Size / 2, Size / 2, Size / 2);

	FVoxelBox Box;
	Box.Min = Bound * -1;
	Box.Max = Bound;

	return Box;
}
