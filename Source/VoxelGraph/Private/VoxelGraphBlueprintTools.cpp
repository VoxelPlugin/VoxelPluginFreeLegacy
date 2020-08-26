// Copyright 2020 Phyronnaz

#include "VoxelGraphBlueprintTools.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.h"
#include "VoxelGraphGenerator.h"
#include "VoxelMessages.h"

#define CHECK_WORLDGENERATOR() \
	if (!WorldGenerator) \
	{ \
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid WorldGenerator!")); \
		return false; \
	}

#define CANNOT_FIND_PROPERTY() \
			FVoxelMessages::Error( \
				FText::Format(VOXEL_LOCTEXT("{0}: Could not find property {1}!"), \
					FText::FromString(__FUNCTION__), \
					FText::FromName(UniqueName))); \
			return false;

bool UVoxelGraphBlueprintTools::SetVoxelGraphFloatParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, float Value)
{
	CHECK_WORLDGENERATOR();
	
	if (WorldGenerator->IsA(UVoxelGraphGenerator::StaticClass()))
	{
		CastChecked<UVoxelGraphGenerator>(WorldGenerator)->FloatParameters.Add(UniqueName, Value);
		return true;
	}
	else
	{
		UFloatProperty* Prop = UE_25_SWITCH(FindField, FindFProperty)<UFloatProperty>(WorldGenerator->GetClass(), UniqueName);
		if (Prop)
		{
			Prop->SetPropertyValue_InContainer(WorldGenerator, Value);
			return true;
		}
		else
		{
			CANNOT_FIND_PROPERTY();
		}
	}
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphIntParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, int32 Value)
{
	CHECK_WORLDGENERATOR();
	
	if (WorldGenerator->IsA(UVoxelGraphGenerator::StaticClass()))
	{
		CastChecked<UVoxelGraphGenerator>(WorldGenerator)->IntParameters.Add(UniqueName, Value);
		return true;
	}
	else
	{
		UIntProperty* Prop = UE_25_SWITCH(FindField, FindFProperty)<UIntProperty>(WorldGenerator->GetClass(), UniqueName);
		if (Prop)
		{
			Prop->SetPropertyValue_InContainer(WorldGenerator, Value);
			return true;
		}
		else
		{
			CANNOT_FIND_PROPERTY();
		}
	}
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphBoolParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, bool Value)
{
	CHECK_WORLDGENERATOR();
	
	if (WorldGenerator->IsA(UVoxelGraphGenerator::StaticClass()))
	{
		CastChecked<UVoxelGraphGenerator>(WorldGenerator)->BoolParameters.Add(UniqueName, Value);
		return true;
	}
	else
	{
		UBoolProperty* Prop = UE_25_SWITCH(FindField, FindFProperty)<UBoolProperty>(WorldGenerator->GetClass(), UniqueName);
		if (Prop)
		{
			Prop->SetPropertyValue_InContainer(WorldGenerator, Value);
			return true;
		}
		else
		{
			CANNOT_FIND_PROPERTY();
		}
	}
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphColorParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FLinearColor Value)
{
	CHECK_WORLDGENERATOR();
	
	if (WorldGenerator->IsA(UVoxelGraphGenerator::StaticClass()))
	{
		CastChecked<UVoxelGraphGenerator>(WorldGenerator)->ColorParameters.Add(UniqueName, Value);
		return true;
	}
	else
	{
		UStructProperty* Prop = UE_25_SWITCH(FindField, FindFProperty)<UStructProperty>(WorldGenerator->GetClass(), UniqueName);
		if (Prop && Prop->GetCPPType(nullptr, 0) == "FLinearColor")
		{
			*Prop->ContainerPtrToValuePtr<FLinearColor>(WorldGenerator) = Value;
			return true;
		}
		else
		{
			CANNOT_FIND_PROPERTY();
		}
	}
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphVoxelTextureParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FVoxelFloatTexture Value)
{
	CHECK_WORLDGENERATOR();
	
	if (WorldGenerator->IsA(UVoxelGraphGenerator::StaticClass()))
	{
		CastChecked<UVoxelGraphGenerator>(WorldGenerator)->VoxelTextureParameters.Add(UniqueName, Value);
		return true;
	}
	else
	{
		UStructProperty* Prop = UE_25_SWITCH(FindField, FindFProperty)<UStructProperty>(WorldGenerator->GetClass(), UniqueName);
		if (Prop && Prop->GetCPPType(nullptr, 0) == "FVoxelFloatTexture")
		{
			*Prop->ContainerPtrToValuePtr<FVoxelFloatTexture>(WorldGenerator) = Value;
			return true;
		}
		else
		{
			CANNOT_FIND_PROPERTY();
		}
	}
}

void UVoxelGraphBlueprintTools::ClearVoxelGraphParametersOverrides(UVoxelWorldGenerator* WorldGenerator)
{
	auto* Result = Cast<UVoxelGraphGenerator>(WorldGenerator);
	if (Result)
	{
		Result->ClearParametersOverrides();
	}
}