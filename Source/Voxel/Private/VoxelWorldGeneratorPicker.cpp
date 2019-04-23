// Copyright 2019 Phyronnaz

#include "VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelBlueprintErrors.h"

#define LOCTEXT_NAMESPACE "Voxel"

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker()
	: Type(EVoxelWorldGeneratorPickerType::Object)
{

}

TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> FVoxelWorldGeneratorPicker::GetWorldGenerator() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		UVoxelWorldGenerator* InstancedWorldGenerator = nullptr;

		if (WorldGeneratorClass)
		{
			InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), WorldGeneratorClass);
		}
		if (!InstancedWorldGenerator)
		{
			FVoxelBPErrors::Error(LOCTEXT("VoxelWorldGeneratorPickerInvalidClass", "VoxelWorldGeneratorPicker: Invalid world generator class"));
			return MakeShared<FEmptyWorldGeneratorInstance, ESPMode::ThreadSafe>();
		}
		else
		{
			return InstancedWorldGenerator->GetWorldGenerator();
		}
	}
	else
	{
		if (WorldGeneratorObject)
		{
			return WorldGeneratorObject->GetWorldGenerator();
		}
		else
		{
			FVoxelBPErrors::Error(LOCTEXT("VoxelWorldGeneratorPickerInvalidObject", "VoxelWorldGeneratorPicker: Invalid world generator object"));
			return MakeShared<FEmptyWorldGeneratorInstance, ESPMode::ThreadSafe>();
		}
	}
}

FString FVoxelWorldGeneratorPicker::GetName() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		if (WorldGeneratorClass)
		{
			return WorldGeneratorClass->GetName();
		}
		else
		{
			return "Invalid class";
		}
	}
	else
	{
		if (WorldGeneratorObject)
		{
			return WorldGeneratorObject->GetName();
		}
		else
		{
			return "Invalid object";
		}
	}
}

#undef LOCTEXT_NAMESPACE
