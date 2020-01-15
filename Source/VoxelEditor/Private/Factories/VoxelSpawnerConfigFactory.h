// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelSpawnerConfigFactory.generated.h"

UCLASS()
class UVoxelSpawnerConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelSpawnerConfigFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelSpawnerConfig::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelSpawnerConfig>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};
