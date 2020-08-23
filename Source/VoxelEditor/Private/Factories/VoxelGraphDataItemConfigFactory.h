// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "VoxelGraphDataItemConfig.h"
#include "VoxelGraphDataItemConfigFactory.generated.h"

UCLASS()
class UVoxelGraphDataItemConfigFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVoxelGraphDataItemConfigFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
		bCreateNew = true;
		bEditAfterNew = true;
		SupportedClass = UVoxelGraphDataItemConfig::StaticClass();
	}

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		return NewObject<UVoxelGraphDataItemConfig>(InParent, Class, Name, Flags | RF_Transactional);
	}
	// End of UFactory interface
};