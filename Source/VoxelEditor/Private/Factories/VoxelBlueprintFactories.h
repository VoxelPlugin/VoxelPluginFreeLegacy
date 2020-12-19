// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Factories/VoxelFactories.h"
#include "Factories/VoxelAssetClassParentFilter.h"
#include "KismetCompilerModule.h"
#include "Modules/ModuleManager.h"
#include "Kismet2/SClassPickerDialog.h"
#include "VoxelFoliage/VoxelFoliage.h"
#include "VoxelFoliage/VoxelFoliageActor.h"
#include "VoxelBlueprintFactories.generated.h"

UCLASS(Abstract)
class UVoxelBlueprintFactory : public UVoxelFactory
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSubclassOf<UObject> ParentClass;

public:
	virtual void SetupCDO_Voxel(UObject* Object) {}
	
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override
	{
		ParentClass = nullptr;

		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;

		const auto Filter = MakeShared<FVoxelAssetClassParentFilter>();
		Options.ClassFilter = Filter;

		Filter->DisallowedClassFlags = CLASS_Deprecated | CLASS_NewerVersionExists;
		Filter->AllowedChildrenOfClasses.Add(SupportedClass);

		const FText TitleText = VOXEL_LOCTEXT("Pick Parent Class");
		UClass* ChosenClass = nullptr;
		const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, SupportedClass);

		if (bPressedOk)
		{
			ParentClass = ChosenClass;
		}

		return bPressedOk;
	}
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
	{
		if (!ParentClass)
		{
			return nullptr;
		}
		
		UClass* BlueprintClass = nullptr;
		UClass* BlueprintGeneratedClass = nullptr;

		IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		KismetCompilerModule.GetBlueprintTypesForClass(ParentClass, BlueprintClass, BlueprintGeneratedClass);

		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass, NAME_None);
		if (!ensure(Blueprint->GeneratedClass))
		{
			return nullptr;
		}

		UObject* CDO = Blueprint->GeneratedClass->GetDefaultObject();
		SetupCDO_Voxel(CDO);
		CDO->PostEditChange();
		CDO->MarkPackageDirty();

		// Compile changes to CDO
		FKismetEditorUtilities::CompileBlueprint(Blueprint);

		return Blueprint;
	}
	//~ End UFactory Interface
};

UCLASS()
class UVoxelFoliageFactory : public UVoxelBlueprintFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(UVoxelFoliage)

	virtual void SetupCDO_Voxel(UObject* Object) override
	{
		CastChecked<UVoxelFoliage>(Object)->Guid = FGuid::NewGuid();
	}
};

UCLASS()
class UVoxelFoliageActorFactory : public UVoxelBlueprintFactory
{
	GENERATED_BODY()
	GENERATED_VOXEL_FACTORY_BODY(AVoxelFoliageActor)
};