// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelMaterialCollection.h"
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