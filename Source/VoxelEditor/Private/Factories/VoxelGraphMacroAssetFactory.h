// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelGraphMacroAssetFactory.generated.h"

UCLASS()
class UVoxelGraphMacroAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelGraphMacroAssetFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelGraphMacro::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelGraphMacro>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};