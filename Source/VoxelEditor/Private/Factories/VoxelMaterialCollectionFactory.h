// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelRender/VoxelMaterialCollection.h"
#include "VoxelMaterialCollectionFactory.generated.h"

UCLASS()
class UVoxelMaterialCollectionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelMaterialCollectionFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelMaterialCollection::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelMaterialCollection>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};


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