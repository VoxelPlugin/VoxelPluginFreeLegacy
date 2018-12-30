// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelWorldSaveObjectFactory.generated.h"

UCLASS()
class UVoxelWorldSaveObjectFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelWorldSaveObjectFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};