// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "LandscapeFileFormatInterface.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelImporters/VoxelLandscapeImporter.h"
#include "VoxelHeightmapAssetFactory.generated.h"

class ULandscapeComponent;

UCLASS()
class UVoxelHeightmapAssetFloatFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelHeightmapAssetFloatFactory();

	UPROPERTY()
	EVoxelHeightmapImporterMaterialConfig MaterialConfig;
	
	UPROPERTY()
	TArray<FVoxelLandscapeImporterLayerInfo> LayerInfos;
	
	UPROPERTY()
	TArray<ULandscapeComponent*> Components;
	
	UPROPERTY()
	FVector ActorLocation;

	UPROPERTY()
	FString AssetName;

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FString GetDefaultNewAssetName() const override;
	// End of UFactory interface
};

namespace FVoxelHeightmapImportersHelpers
{
	struct FWeightmap
	{
		TArray<uint8> Data;
		EVoxelRGBA Layer = EVoxelRGBA::R;
		uint8 Index = 0;

		FWeightmap() = default;
	};
}

UCLASS(HideCategories=Object, CollapseCategories)
class UVoxelHeightmapAssetUINT16Factory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UVoxelHeightmapAssetUINT16Factory();

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	FFilePath Heightmap;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	EVoxelHeightmapImporterMaterialConfig MaterialConfig;

	UPROPERTY(EditAnywhere, Category = "Import configuration", meta = (DisplayName = "Weightmaps"))
	TArray<FVoxelHeightmapImporterWeightmapInfos> WeightmapsInfos;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FString GetDefaultNewAssetName() const override;
	//~ End UFactory Interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface

private:
	int32 Width = 0;
	int32 Height = 0;
	FLandscapeHeightmapImportData HeightmapImportData;
	TArray<FVoxelHeightmapImportersHelpers::FWeightmap> Weightmaps;

	bool TryLoad();
	bool DoImport(UVoxelHeightmapAssetUINT16* Asset);
};