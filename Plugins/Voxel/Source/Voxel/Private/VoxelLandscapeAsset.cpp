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
	, ScaleMultiplier(1)
	, bShrink(false)
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
	LandscapeAsset->ScaleMultiplier = ScaleMultiplier;
	LandscapeAsset->bShrink = bShrink;

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
	const int HalfSize = bShrink ? (Size / 2 / ScaleMultiplier) : (Size * ScaleMultiplier / 2);

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	const int IndexX = bShrink ? ((X + HalfSize) * ScaleMultiplier) : ((X + HalfSize) / ScaleMultiplier);
	const int IndexY = bShrink ? ((Y + HalfSize) * ScaleMultiplier) : ((Y + HalfSize) / ScaleMultiplier);
	const float CurrentHeight = Heights[IndexX + Size * IndexY];

	const int RealZ = bShrink ? (Z * ScaleMultiplier) : (Z / ScaleMultiplier);


	if (CurrentHeight > (RealZ + Precision) * VoxelSize)
	{
		// If voxel over us is in, we're entirely in
		return -HardnessMultiplier;
	}
	else if ((RealZ - Precision) * VoxelSize > CurrentHeight)
	{
		// If voxel under us is out, we're entirely out
		return HardnessMultiplier;
	}
	else
	{
		float Alpha = (RealZ * VoxelSize - CurrentHeight) / VoxelSize / Precision;

		return Alpha * HardnessMultiplier;
	}
}

FVoxelMaterial FDecompressedVoxelLandscapeAsset::GetMaterial(const int X, const int Y, const int Z)
{
	const int HalfSize = bShrink ? (Size / 2 / ScaleMultiplier) : (Size * ScaleMultiplier / 2);

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	const int IndexX = bShrink ? ((X + HalfSize) * ScaleMultiplier) : ((X + HalfSize) / ScaleMultiplier);
	const int IndexY = bShrink ? ((Y + HalfSize) * ScaleMultiplier) : ((Y + HalfSize) / ScaleMultiplier);

	return Materials[IndexX + Size * IndexY];
}

FVoxelType FDecompressedVoxelLandscapeAsset::GetVoxelType(const int X, const int Y, const int Z)
{
	const int HalfSize = bShrink ? (Size / 2 / ScaleMultiplier) : (Size * ScaleMultiplier / 2);

	check(-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize);

	const int IndexX = bShrink ? ((X + HalfSize) * ScaleMultiplier) : ((X + HalfSize) / ScaleMultiplier);
	const int IndexY = bShrink ? ((Y + HalfSize) * ScaleMultiplier) : ((Y + HalfSize) / ScaleMultiplier);
	const float CurrentHeight = Heights[IndexX + Size * IndexY];

	const int RealZ = bShrink ? (Z * ScaleMultiplier) : (Z / ScaleMultiplier);


	if ((RealZ - Precision) * VoxelSize <= CurrentHeight || CurrentHeight <= (RealZ + Precision) * VoxelSize)
	{
		return (RealZ * VoxelSize - CurrentHeight <= 0) ? FVoxelType(UseValue, UseMaterial) : FVoxelType(UseValueIfSameSign, UseMaterial);
	}
	else
	{
		return FVoxelType(IgnoreValue, IgnoreMaterial);
	}
}

FVoxelBox FDecompressedVoxelLandscapeAsset::GetBounds()
{
	const int HalfSize = bShrink ? (Size / 2 / ScaleMultiplier) : (Size * ScaleMultiplier / 2);

	FIntVector Bound(HalfSize, HalfSize, HalfSize);

	FVoxelBox Box;
	Box.Min = Bound * -1;
	Box.Max = Bound;

	return Box;
}
