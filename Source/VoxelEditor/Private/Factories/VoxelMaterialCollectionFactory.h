// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelRender/MaterialCollections/VoxelBasicMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelMaterialCollectionFactory.generated.h"

UCLASS()
class UVoxelBasicMaterialCollectionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelBasicMaterialCollectionFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelBasicMaterialCollection::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelBasicMaterialCollection>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};

UCLASS()
class UVoxelInstancedMaterialCollectionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelInstancedMaterialCollectionFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelInstancedMaterialCollection::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelInstancedMaterialCollection>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};