// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelGraphOutputsConfig.h"
#include "VoxelGraphOutputsConfigFactory.generated.h"

UCLASS()
class UVoxelGraphOutputsConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelGraphOutputsConfigFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelGraphOutputsConfig::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelGraphOutputsConfig>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};