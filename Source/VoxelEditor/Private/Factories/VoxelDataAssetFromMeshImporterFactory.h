// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VoxelDataAssetFromMeshImporterFactory.generated.h"

class AVoxelMeshImporter;

UCLASS()
class UVoxelDataAssetFromMeshImporterFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AVoxelMeshImporter* MeshImporter;

	UVoxelDataAssetFromMeshImporterFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FString GetDefaultNewAssetName() const override;
	// End of UFactory interface
};