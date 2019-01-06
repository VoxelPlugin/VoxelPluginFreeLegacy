// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelMaterialCollectionFactory.generated.h"

UCLASS()
class UVoxelMaterialCollectionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelMaterialCollectionFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};