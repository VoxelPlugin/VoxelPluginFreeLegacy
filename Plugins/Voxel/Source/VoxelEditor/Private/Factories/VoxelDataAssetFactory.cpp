// Copyright 2017 Phyronnaz

#include "VoxelDataAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelDataAsset.h"

UVoxelDataAssetFactory::UVoxelDataAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
}

UObject* UVoxelDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewDataAsset = NewObject<UVoxelDataAsset>(InParent, Class, Name, Flags);

	FDecompressedVoxelDataAsset Asset;
	Asset.SetSize(100, 100, 100);

	static const float Sqrt2 = 1.41421356237f;

	for (int X = 0; X < 100; X++)
	{
		for (int Y = 0; Y < 100; Y++)
		{
			for (int Z = 0; Z < 100; Z++)
			{
				FVector Position(X - 50, Y - 50, Z - 50);
				float Distance = Position.Size();
				float DistanceToIso = Distance - 45;

				Asset.SetValue(X, Y, Z, FMath::Clamp(DistanceToIso, -2.f, 2.f) / 2);
				Asset.SetMaterial(X, Y, Z, FVoxelMaterial(1, 0, FMath::Clamp<uint8>(255 * (Distance / 45), 0, 255)));
				Asset.SetVoxelType(X, Y, Z, DistanceToIso < 0 ? EVoxelType::UseValue : DistanceToIso < Sqrt2 ? UseValueIfSameSign : IgnoreValue);
			}
		}
	}

	NewDataAsset->Init(Asset);

	return NewDataAsset;
}

uint32 UVoxelDataAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
