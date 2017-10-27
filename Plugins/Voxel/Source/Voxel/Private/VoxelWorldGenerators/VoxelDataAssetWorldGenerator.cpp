#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "VoxelDataAssetWorldGenerator.h"

UVoxelDataAssetWorldGenerator::UVoxelDataAssetWorldGenerator()
	: InstancedWorldGenerator(nullptr)
{
	DefaultWorldGenerator = TSubclassOf<UVoxelWorldGenerator>(UFlatWorldGenerator::StaticClass());
}

float UVoxelDataAssetWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	const int HalfSizeX = DecompressedAsset.SizeX / 2;
	const int HalfSizeY = DecompressedAsset.SizeY / 2;
	const int HalfSizeZ = DecompressedAsset.SizeZ / 2;

	if (-HalfSizeX <= X && X < HalfSizeX && -HalfSizeY <= Y && Y < HalfSizeY && -HalfSizeZ <= Z && Z < HalfSizeZ)
	{
		float AssetValue = DecompressedAsset.GetValue(X + HalfSizeX, Y + HalfSizeY, Z + HalfSizeZ);
		EVoxelType VoxelType = DecompressedAsset.GetVoxelType(X + HalfSizeX, Y + HalfSizeY, Z + HalfSizeZ);

		if (VoxelType == EVoxelType::UseValue)
		{
			return AssetValue;
		}
		else
		{
			float DefaultValue = InstancedWorldGenerator->GetDefaultValue(X, Y, Z);
			if (VoxelType == EVoxelType::UseValueIfSameSign && DefaultValue * AssetValue >= 0)
			{
				return AssetValue;
			}
			else
			{
				return DefaultValue;
			}
		}
	}
	else
	{
		return InstancedWorldGenerator->GetDefaultValue(X, Y, Z);
	}
}

FVoxelMaterial UVoxelDataAssetWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	const int HalfSizeX = DecompressedAsset.SizeX / 2;
	const int HalfSizeY = DecompressedAsset.SizeY / 2;
	const int HalfSizeZ = DecompressedAsset.SizeZ / 2;

	if (-HalfSizeX <= X && X < HalfSizeX && -HalfSizeY <= Y && Y < HalfSizeY && -HalfSizeZ <= Z && Z < HalfSizeZ)
	{
		EVoxelType VoxelType = DecompressedAsset.GetVoxelType(X + HalfSizeX, Y + HalfSizeY, Z + HalfSizeZ);

		if (VoxelType == EVoxelType::UseValue || VoxelType == EVoxelType::UseValueIfSameSign)
		{
			return DecompressedAsset.GetMaterial(X + HalfSizeX, Y + HalfSizeY, Z + HalfSizeZ);
		}
		else
		{
			return InstancedWorldGenerator->GetDefaultMaterial(X, Y, Z);
		}
	}
	else
	{
		return InstancedWorldGenerator->GetDefaultMaterial(X, Y, Z);
	}
}

void UVoxelDataAssetWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	CreateGeneratorAndDecompressedAsset();
}

void UVoxelDataAssetWorldGenerator::CreateGeneratorAndDecompressedAsset()
{
	check(!InstancedWorldGenerator);

	InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), DefaultWorldGenerator);
	if (InstancedWorldGenerator == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("VoxelDataAssetWorldGenerator: Invalid world generator"));
		InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), UFlatWorldGenerator::StaticClass());
	}

	bool bSuccess = Asset && Asset->GetDecompressedAsset(DecompressedAsset);
	if (!bSuccess)
	{
		// Prevent crash
		DecompressedAsset.SizeX = -2;
		DecompressedAsset.SizeY = -2;
		DecompressedAsset.SizeZ = -2;

		UE_LOG(VoxelLog, Error, TEXT("VoxelDataAssetWorldGenerator: Invalid Asset"));
	}
}
