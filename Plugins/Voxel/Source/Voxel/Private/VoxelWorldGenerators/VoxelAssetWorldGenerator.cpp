#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "VoxelAssetWorldGenerator.h"

UVoxelAssetWorldGenerator::UVoxelAssetWorldGenerator()
	: InstancedWorldGenerator(nullptr)
	, DecompressedAsset(nullptr)
{
	DefaultWorldGenerator = TSubclassOf<UVoxelWorldGenerator>(UFlatWorldGenerator::StaticClass());
}

float UVoxelAssetWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	if (Bounds.IsInside(X, Y, Z))
	{
		float AssetValue = DecompressedAsset->GetValue(X, Y, Z);
		EVoxelType VoxelType = DecompressedAsset->GetVoxelType(X, Y, Z);

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

FVoxelMaterial UVoxelAssetWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	if (Bounds.IsInside(X, Y, Z))
	{
		EVoxelType VoxelType = DecompressedAsset->GetVoxelType(X, Y, Z);

		if (VoxelType == EVoxelType::UseValue || VoxelType == EVoxelType::UseValueIfSameSign)
		{
			return DecompressedAsset->GetMaterial(X, Y, Z);
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

void UVoxelAssetWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	CreateGeneratorAndDecompressedAsset(VoxelWorld->GetVoxelSize());
}

void UVoxelAssetWorldGenerator::CreateGeneratorAndDecompressedAsset(const float VoxelSize)
{
	check(!InstancedWorldGenerator);

	InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), DefaultWorldGenerator);
	if (InstancedWorldGenerator == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("VoxelAssetWorldGenerator: Invalid world generator"));
		InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), UFlatWorldGenerator::StaticClass());
	}

	bool bSuccess = Asset && Asset->GetDecompressedAsset(DecompressedAsset, VoxelSize);
	
	if (bSuccess)
	{
		Bounds = DecompressedAsset->GetBounds();
	}
	else
	{
		Bounds.Min = FIntVector(2, 2, 2);
		Bounds.Max = FIntVector(-2, -2, -2);
		UE_LOG(VoxelLog, Error, TEXT("VoxelAssetWorldGenerator: Invalid asset"));
	}
}
