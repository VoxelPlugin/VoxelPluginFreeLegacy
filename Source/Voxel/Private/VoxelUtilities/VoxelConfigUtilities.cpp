// Copyright 2020 Phyronnaz

#include "VoxelUtilities/VoxelConfigUtilities.h"
#include "VoxelMinimal.h"

#include "Misc/ConfigCacheIni.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"

void FVoxelConfigUtilities::SaveConfig(UObject* Object, const FString& BaseSectionName, const FString& Filename)
{
	UClass* Class = Object->GetClass();
	const UObject* CDO = Class->GetDefaultObject();

	for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		auto& Property = **It;
		if (Property.HasAnyPropertyFlags(CPF_Transient)) continue;
		if (!ensure(Property.ArrayDim == 1)) continue;

		const FString Section = BaseSectionName + TEXT(".") + Class->GetName();

		FString	Value;
		if (Property.ExportText_InContainer(0, Value, Object, CDO, Object, PPF_None))
		{
			GConfig->SetString(*Section, *Property.GetName(), *Value, Filename);
		}
		else
		{
			GConfig->RemoveKey(*Section, *Property.GetName(), Filename);
		}
	}
}

void FVoxelConfigUtilities::LoadConfig(UObject* Object, const FString& BaseSectionName, const FString& Filename)
{
	UClass* Class = Object->GetClass();
	for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		auto& Property = **It;
		if (Property.HasAnyPropertyFlags(CPF_Transient)) continue;
		if (!ensure(Property.ArrayDim == 1)) continue;
		
		const FString Section = BaseSectionName + TEXT(".") + Class->GetName();

		FString Value;
		if (GConfig->GetString(*Section, *Property.GetName(), Value, Filename))
		{
			Property.ImportText(*Value, Property.ContainerPtrToValuePtr<void>(Object), PPF_None, Object);
		}
	}
}
