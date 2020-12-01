// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "VoxelData/VoxelSave.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelSpawners/VoxelAssetSpawner.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelSpawnerConfig.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"
#include "VoxelRender/MaterialCollections/VoxelBasicMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelLandscapeMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelGraphOutputsConfig.h"
#include "VoxelGraphDataItemConfig.h"
#include "VoxelFactories.generated.h"

UCLASS(Abstract)
class UVoxelFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
	}

	virtual UClass* GetSupportedClass_Voxel() const { ensure(false); return nullptr; }

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UObject>(InParent, Class, Name, Flags);
	}
	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();

		if (GetClass() != StaticClass())
		{
			ensure(!SupportedClass);
			SupportedClass = GetSupportedClass_Voxel();
		}
	}
	//~ End UFactory Interface
};

#define GENERATED_VOXEL_FACTORY_BODY(Class) \
	virtual UClass* GetSupportedClass_Voxel() const override { return Class::StaticClass(); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UVoxelWorldSaveObjectFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelWorldSaveObject)
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UVoxelMeshSpawnerFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelMeshSpawner)
};

UCLASS()
class UVoxelMeshSpawnerGroupFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelMeshSpawnerGroup)
};

UCLASS()
class UVoxelAssetSpawnerFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelAssetSpawner)
};

UCLASS()
class UVoxelSpawnerGroupFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelSpawnerGroup)
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UVoxelSpawnerConfigFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelSpawnerConfig)
};

UCLASS()
class UVoxelSpawnerCollectionFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelSpawnerCollection)
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UVoxelBasicMaterialCollectionFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelBasicMaterialCollection)
};

UCLASS()
class UVoxelInstancedMaterialCollectionTemplatesFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelInstancedMaterialCollectionTemplates)
};

UCLASS()
class UVoxelInstancedMaterialCollectionFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelInstancedMaterialCollection)
};

UCLASS()
class UVoxelInstancedMaterialCollectionInstanceFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelInstancedMaterialCollectionInstance)
};

UCLASS()
class UVoxelLandscapeMaterialCollectionFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelLandscapeMaterialCollection)
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS()
class UVoxelGraphOutputsConfigFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelGraphOutputsConfig)
};

UCLASS()
class UVoxelGraphMacroAssetFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelGraphMacro)
};

UCLASS()
class UVoxelGraphGeneratorAssetFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelGraphGenerator)
};

UCLASS()
class UVoxelGraphDataItemConfigFactory : public UVoxelFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelGraphDataItemConfig)
};