// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelAssetItem.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelWorldGenerator.h"
#include "VoxelMessages.h"
#include "VoxelWorld.h"

FVoxelAssetItem::FVoxelAssetItem(
	const TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance>& WorldGenerator,
	const FIntBox& WorldBounds,
	const FTransform& LocalToWorld,
	int32 Priority)
	: FVoxelPlaceableItem(StaticId(), WorldBounds, Priority)
	, WorldGenerator(WorldGenerator)
	, LocalToWorld(LocalToWorld)
	, WorldBounds(WorldBounds)
{
}

FString FVoxelAssetItem::GetDescription() const
{
	return FString::Printf(TEXT("Asset %s"), *WorldGenerator->Class->GetPathName());
}

void FVoxelAssetItem::Save(FArchive& Ar) const
{
	FString ClassPath = WorldGenerator->Class->GetPathName();
	FIntBox WorldBoundsCopy = WorldBounds;
	FTransform LocalToWorldCopy = LocalToWorld;
	int32 PriorityCopy = Priority;

	Ar << ClassPath;
	Ar << WorldBoundsCopy;
	Ar << LocalToWorldCopy;
	Ar << PriorityCopy;

	WorldGenerator->Class->GetDefaultObject<UVoxelTransformableWorldGenerator>()->SaveInstance(*WorldGenerator, Ar);
}

FVoxelAssetItemLoader FVoxelAssetItemLoader::AssetLoader;

TVoxelSharedRef<FVoxelPlaceableItem> FVoxelAssetItemLoader::Load(FArchive& Ar, const AVoxelWorld* VoxelWorld) const
{
	FString ClassPath;
	FIntBox WorldBounds;
	FTransform LocalToWorld;
	int32 Priority;
	
	Ar << ClassPath;
	Ar << WorldBounds;
	Ar << LocalToWorld;
	Ar << Priority;

	TVoxelSharedPtr<FVoxelTransformableWorldGeneratorInstance> WorldGenerator;
	if (UClass* Class = LoadClass<UVoxelTransformableWorldGenerator>(nullptr, *ClassPath))
	{
		if (auto* WorldGeneratorObject = Cast<UVoxelTransformableWorldGenerator>(Class->GetDefaultObject()))
		{
			WorldGenerator = WorldGeneratorObject->LoadInstance(Ar);
		}
		else
		{
			Ar.SetError();
			FVoxelMessages::Error(FString::Printf(TEXT("Can't load asset: class '%s' isn't a VoxelWorldGenerator"), *ClassPath));
		}
	}
	else
	{
		Ar.SetError();
		FVoxelMessages::Error(FString::Printf(TEXT("Can't load asset: invalid class path '%s'"), *ClassPath));
	}

	if (!WorldGenerator.IsValid())
	{
		WorldGenerator = MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
		Ar.Reset();
	}

	WorldGenerator->Init(VoxelWorld->GetInitStruct());

	return MakeVoxelShared<FVoxelAssetItem>(
		WorldGenerator.ToSharedRef(),
		WorldBounds,
		LocalToWorld,
		Priority);
}