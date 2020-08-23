// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelAssetSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"
#include "VoxelSpawnersFactories.generated.h"

UCLASS()
class UVoxelMeshSpawnerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelMeshSpawnerFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelMeshSpawner::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelMeshSpawner>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};

UCLASS()
class UVoxelMeshSpawnerGroupFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelMeshSpawnerGroupFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelMeshSpawnerGroup::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelMeshSpawnerGroup>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};

UCLASS()
class UVoxelAssetSpawnerFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelAssetSpawnerFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelAssetSpawner::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelAssetSpawner>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};

UCLASS()
class UVoxelSpawnerGroupFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelSpawnerGroupFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelSpawnerGroup::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelSpawnerGroup>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};