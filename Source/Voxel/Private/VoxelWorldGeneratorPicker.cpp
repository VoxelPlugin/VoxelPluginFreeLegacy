// Copyright 2020 Phyronnaz

#include "VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelMessages.h"
#include "UObject/Package.h"

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(UClass* WorldGeneratorClass)
	: Type(EVoxelWorldGeneratorPickerType::Class)
	, WorldGeneratorClass(WorldGeneratorClass)
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(UVoxelWorldGenerator* WorldGeneratorObject)
	: Type(EVoxelWorldGeneratorPickerType::Object)
	, WorldGeneratorObject(WorldGeneratorObject)
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TSoftClassPtr<UVoxelWorldGenerator> WorldGeneratorClass)
	: Type(EVoxelWorldGeneratorPickerType::Class)
	, WorldGeneratorClass(WorldGeneratorClass)
{
}

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker(TSoftObjectPtr<UVoxelWorldGenerator> WorldGeneratorObject)
	: Type(EVoxelWorldGeneratorPickerType::Object)
	, WorldGeneratorObject(WorldGeneratorObject)
{
}

UVoxelWorldGenerator* FVoxelWorldGeneratorPicker::GetWorldGenerator() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		if (WorldGeneratorClass.LoadSynchronous())
		{
			return WorldGeneratorClass->GetDefaultObject<UVoxelWorldGenerator>();
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return WorldGeneratorObject.LoadSynchronous();
	}
}

UObject* FVoxelWorldGeneratorPicker::GetObject() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		return WorldGeneratorClass.LoadSynchronous();
	}
	else
	{
		return WorldGeneratorObject.LoadSynchronous();
	}
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> FVoxelWorldGeneratorPicker::GetInstance(bool bSilent) const
{
	auto* WorldGenerator = GetWorldGenerator();
	if (!WorldGenerator)
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid World Generator"));
		return MakeVoxelShared<FVoxelEmptyWorldGeneratorInstance>();
	}
	return WorldGenerator->GetInstance();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(UClass* WorldGeneratorClass)
	: Type(EVoxelWorldGeneratorPickerType::Class)
	, WorldGeneratorClass(WorldGeneratorClass)
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(UVoxelTransformableWorldGenerator* WorldGeneratorObject)
	: Type(EVoxelWorldGeneratorPickerType::Object)
	, WorldGeneratorObject(WorldGeneratorObject)
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TSoftClassPtr<UVoxelTransformableWorldGenerator> WorldGeneratorClass)
	: Type(EVoxelWorldGeneratorPickerType::Class)
	, WorldGeneratorClass(WorldGeneratorClass)
{
}

FVoxelTransformableWorldGeneratorPicker::FVoxelTransformableWorldGeneratorPicker(TSoftObjectPtr<UVoxelTransformableWorldGenerator> WorldGeneratorObject)
	: Type(EVoxelWorldGeneratorPickerType::Object)
	, WorldGeneratorObject(WorldGeneratorObject)
{
}

UVoxelTransformableWorldGenerator* FVoxelTransformableWorldGeneratorPicker::GetWorldGenerator() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		if (WorldGeneratorClass.LoadSynchronous())
		{
			return WorldGeneratorClass->GetDefaultObject<UVoxelTransformableWorldGenerator>();
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return WorldGeneratorObject.LoadSynchronous();
	}
}

UObject* FVoxelTransformableWorldGeneratorPicker::GetObject() const
{
	if (Type == EVoxelWorldGeneratorPickerType::Class)
	{
		return WorldGeneratorClass.LoadSynchronous();
	}
	else
	{
		return WorldGeneratorObject.LoadSynchronous();
	}
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> FVoxelTransformableWorldGeneratorPicker::GetInstance(bool bSilent) const
{
	auto* WorldGenerator = GetWorldGenerator();
	if (!WorldGenerator)
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid World Generator"));
		return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
	}
	return WorldGenerator->GetTransformableInstance();
}