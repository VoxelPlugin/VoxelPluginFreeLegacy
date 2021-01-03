// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Engine/EngineTypes.h"
#include "VoxelDataAssetFactory.generated.h"

class AVoxelMeshImporter;

UCLASS()
class UVoxelDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelDataAssetFactory();

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};