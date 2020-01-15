// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelData/VoxelSave.h"
#include "VoxelWorldSaveObjectFactory.generated.h"

UCLASS()
class UVoxelWorldSaveObjectFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelWorldSaveObjectFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelWorldSaveObject::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelWorldSaveObject>(InParent, Class, Name, Flags);
	}
	// End of UFactory interface
};