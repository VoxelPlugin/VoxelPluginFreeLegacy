// Copyright 2020 Phyronnaz

#include "VoxelWorldGeneratorPicker.h"
#include "VoxelMessages.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"

TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetWorldGeneratorInstance(bool bSilent, UVoxelWorldGenerator* WorldGenerator)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (WorldGenerator)
	{
		return WorldGenerator->GetInstance();
	}
	else
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid World Generator"));
		return MakeVoxelShared<FVoxelEmptyWorldGeneratorInstance>();
	}
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetWorldGeneratorInstance(bool bSilent, UVoxelTransformableWorldGenerator* WorldGenerator)
{
	VOXEL_FUNCTION_COUNTER();

	if (WorldGenerator)
	{
		return WorldGenerator->GetTransformableInstance();
	}
	else
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid World Generator"));
		return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TYPE_OF_NULLPTR)
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(UClass* InClass)
{
	Type = EVoxelWorldGeneratorPickerType::Class;
	Class = InClass;
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TSubclassOf<UVoxelWorldGenerator> InClass)
	: FVoxelWorldGeneratorPicker(InClass.Get())
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(UVoxelWorldGenerator* InObject)
{
	Type = EVoxelWorldGeneratorPickerType::Object;
	Object = InObject;
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TSoftClassPtr<UVoxelWorldGenerator> InClass)
	: FVoxelWorldGeneratorPicker(InClass.LoadSynchronous())
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TSoftObjectPtr<UVoxelWorldGenerator> InObject)
	: FVoxelWorldGeneratorPicker(InObject.LoadSynchronous())
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TYPE_OF_NULLPTR)
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(UClass* InClass)
{
	Type = EVoxelWorldGeneratorPickerType::Class;
	Class = InClass;
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TSubclassOf<UVoxelTransformableWorldGenerator> InClass)
	: FVoxelTransformableWorldGeneratorPicker(InClass.Get())
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(UVoxelTransformableWorldGenerator* InObject)
{
	Type = EVoxelWorldGeneratorPickerType::Object;
	Object = InObject;
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TSoftClassPtr<UVoxelTransformableWorldGenerator> InClass)
	: FVoxelTransformableWorldGeneratorPicker(InClass.LoadSynchronous())
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TSoftObjectPtr<UVoxelTransformableWorldGenerator> InObject)
	: FVoxelTransformableWorldGeneratorPicker(InObject.LoadSynchronous())
{
}