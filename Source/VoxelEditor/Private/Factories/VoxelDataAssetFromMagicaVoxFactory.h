// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "VoxelDataAssetFromMagicaVoxFactory.generated.h"

UCLASS()
class UVoxelDataAssetFromMagicaVoxFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	// If false, the material index will be set to the palette index (Single Index material config)
	// If true, the palette will be read and the color will be imported instead (RGB material config)
	// If you're not sure, leave to true
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	bool bUsePalette = true;

public:
	UVoxelDataAssetFromMagicaVoxFactory();

	// UFactory interface
	virtual bool ConfigureProperties() override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	// End of UFactory interface

	//~ Begin FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler Interface
};