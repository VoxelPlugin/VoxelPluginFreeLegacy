// Copyright 2017 Phyronnaz

/**
* Factory for VoxelGrassType assets
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelDataAssetFactory.generated.h"

UCLASS()
class UVoxelDataAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelDataAssetFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	// End of UFactory interface
};
