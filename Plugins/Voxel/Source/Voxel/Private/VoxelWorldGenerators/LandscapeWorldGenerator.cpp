#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"
#include "LandscapeWorldGenerator.h"

ULandscapeWorldGenerator::ULandscapeWorldGenerator()
	: InstancedWorldGenerator(nullptr)
	, VoxelSize(0)
{
	DefaultWorldGenerator = TSubclassOf<UVoxelWorldGenerator>(UFlatWorldGenerator::StaticClass());
	TArray<float> Heights;
	TArray<FVoxelMaterial> Materials;
}

float ULandscapeWorldGenerator::GetDefaultValue(int X, int Y, int Z)
{
	const int HalfSize = Asset.Size / 2;

	if (-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize)
	{
		return Asset.GetValue(X + HalfSize, Y + HalfSize, Z, VoxelSize);
	}
	else
	{
		return InstancedWorldGenerator->GetDefaultValue(X, Y, Z);
	}
}

FVoxelMaterial ULandscapeWorldGenerator::GetDefaultMaterial(int X, int Y, int Z)
{
	const int HalfSize = Asset.Size / 2;

	if (-HalfSize <= X && X < HalfSize && -HalfSize <= Y && Y < HalfSize)
	{
		return Asset.GetMaterial(X + HalfSize, Y + HalfSize, Z, VoxelSize);
	}
	else
	{
		return InstancedWorldGenerator->GetDefaultMaterial(X, Y, Z);
	}
}

void ULandscapeWorldGenerator::SetVoxelWorld(AVoxelWorld* VoxelWorld)
{
	VoxelSize = VoxelWorld->GetVoxelSize();
	CreateGeneratorAndDecompressedAsset();
}

void ULandscapeWorldGenerator::CreateGeneratorAndDecompressedAsset()
{
	check(!InstancedWorldGenerator);

	InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), DefaultWorldGenerator);
	if (InstancedWorldGenerator == nullptr)
	{
		UE_LOG(VoxelLog, Error, TEXT("LandscapeWorldGenerator: Invalid world generator"));
		InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), UFlatWorldGenerator::StaticClass());
	}

	bool bSuccess = Landscape && Landscape->GetDecompressedAsset(Asset);
	if (!bSuccess)
	{
		Asset.Size = -2; // Prevent crash
		UE_LOG(VoxelLog, Error, TEXT("LandscapeWorldGenerator: Invalid Landscape"));
	}
}
