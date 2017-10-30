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

void FDecompressedVoxelDataAsset::SetSize(int32 NewSizeX, int32 NewSizeY, int32 NewSizeZ)
{
	SizeX = NewSizeX;
	SizeY = NewSizeY;
	SizeZ = NewSizeZ;

	int Count = SizeX * SizeY * SizeZ;

	Values.SetNumUninitialized(Count);
	Materials.SetNumUninitialized(Count);
	VoxelTypes.SetNumUninitialized(Count);
}

float FDecompressedVoxelDataAsset::GetValue(const int X, const int Y, const int Z)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	return Values[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)];
}

FVoxelMaterial FDecompressedVoxelDataAsset::GetMaterial(const int X, const int Y, const int Z)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	return Materials[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)];
}

FVoxelType FDecompressedVoxelDataAsset::GetVoxelType(const int X, const int Y, const int Z)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	return FVoxelType(VoxelTypes[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)]);
}

FVoxelBox FDecompressedVoxelDataAsset::GetBounds()
{
	const FIntVector Bounds(SizeX / 2, SizeY / 2, SizeZ / 2);

	FVoxelBox Box;
	Box.Min = Bounds * -1;
	Box.Max = Bounds;
	return Box;
}

void FDecompressedVoxelDataAsset::SetValue(const int X, const int Y, const int Z, const float NewValue)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	Values[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)] = NewValue;
}

void FDecompressedVoxelDataAsset::SetMaterial(const int X, const int Y, const int Z, const FVoxelMaterial NewMaterial)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	Materials[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)] = NewMaterial;
}

void FDecompressedVoxelDataAsset::SetVoxelType(const int X, const int Y, const int Z, const FVoxelType VoxelType)
{
	check(-SizeX / 2 <= X && X < SizeX / 2);
	check(-SizeY / 2 <= Y && Y < SizeY / 2);
	check(-SizeZ / 2 <= Z && Z < SizeZ / 2);
	VoxelTypes[(X + SizeX / 2) + SizeX * (Y + SizeY / 2) + SizeX * SizeY * (Z + SizeZ / 2)] = VoxelType.Value;
}
