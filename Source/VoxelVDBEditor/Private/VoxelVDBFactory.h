// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "VoxelVDBAsset.h"
#include "VoxelVDBFactory.generated.h"

USTRUCT()
struct FVoxelVDBFactoryChannelMapping
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	FName ChannelName;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelVDBImportChannelConfig ChannelConfig;
};

UCLASS(HideCategories=Object, CollapseCategories)
class UVoxelVDBFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UVoxelVDBFactory();

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	TArray<FVoxelVDBFactoryChannelMapping> Mappings;

	// UFactory interface
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	// End of UFactory interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface

private:
	bool DoImport(UVoxelVDBAsset& Asset, const FString& Path, const TMap<FName, FVoxelVDBImportChannelConfig>& ExistingChannelConfigs);
};