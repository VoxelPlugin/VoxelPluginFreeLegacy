// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphGeneratorAssetFactory.generated.h"

UCLASS()
class UVoxelGraphGeneratorAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelGraphGeneratorAssetFactory()
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelGraphGenerator::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelGraphGenerator>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};