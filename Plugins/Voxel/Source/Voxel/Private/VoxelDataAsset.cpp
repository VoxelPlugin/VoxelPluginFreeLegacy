// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelDataAsset.h"


UVoxelDataAsset::UVoxelDataAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
};

bool UVoxelDataAsset::GetDecompressedAsset(FDecompressedVoxelAsset*& Asset, const float VoxelSize)
{
	Asset = new FDecompressedVoxelDataAsset();
	return Super::GetDecompressedAsset(Asset, VoxelSize);
}

void UVoxelDataAsset::AddAssetToArchive(FBufferArchive& ToBinary, FDecompressedVoxelAsset* Asset)
{
	ToBinary << *((FDecompressedVoxelDataAsset*)Asset);
}

void UVoxelDataAsset::GetAssetFromArchive(FMemoryReader& FromBinary, FDecompressedVoxelAsset* Asset)
{
	FromBinary << *((FDecompressedVoxelDataAsset*)Asset);
}

void FDecompressedVoxelDataAsset::SetHalfSize(int32 NewHalfSizeX, int32 NewHalfSizeY, int32 NewHalfSizeZ)
{
	HalfSizeX = NewHalfSizeX;
	HalfSizeY = NewHalfSizeY;
	HalfSizeZ = NewHalfSizeZ;

	int Count = HalfSizeX * HalfSizeY * HalfSizeZ * 2 * 2 * 2;

	Values.SetNumUninitialized(Count);
	Materials.SetNumUninitialized(Count);
	VoxelTypes.SetNumUninitialized(Count);
}

float FDecompressedVoxelDataAsset::GetValue(const int X, const int Y, const int Z)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	return Values[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)];
}

FVoxelMaterial FDecompressedVoxelDataAsset::GetMaterial(const int X, const int Y, const int Z)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	return Materials[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)];
}

FVoxelType FDecompressedVoxelDataAsset::GetVoxelType(const int X, const int Y, const int Z)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	return FVoxelType(VoxelTypes[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)]);
}

FVoxelBox FDecompressedVoxelDataAsset::GetBounds()
{
	const FIntVector Bounds;

	FVoxelBox Box;
	Box.Min = FIntVector(-HalfSizeX, -HalfSizeY, -HalfSizeZ);
	Box.Max = FIntVector(HalfSizeX, HalfSizeY, HalfSizeZ);
	return Box;
}

void FDecompressedVoxelDataAsset::SetValue(const int X, const int Y, const int Z, const float NewValue)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	Values[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)] = NewValue;
}

void FDecompressedVoxelDataAsset::SetMaterial(const int X, const int Y, const int Z, const FVoxelMaterial NewMaterial)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	Materials[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)] = NewMaterial;
}

void FDecompressedVoxelDataAsset::SetVoxelType(const int X, const int Y, const int Z, const FVoxelType VoxelType)
{
	check(-HalfSizeX <= X && X < HalfSizeX);
	check(-HalfSizeY <= Y && Y < HalfSizeY);
	check(-HalfSizeZ <= Z && Z < HalfSizeZ);
	VoxelTypes[(X + HalfSizeX) + 2 * HalfSizeX * (Y + HalfSizeY) + 2 * HalfSizeX * 2 * HalfSizeY * (Z + HalfSizeZ)] = VoxelType.Value;
}
