// Copyright 2018 Phyronnaz

#include "VoxelWorldGenerator.h"
#include "VoxelPrivate.h"
#include "VoxelWorldGenerators/EmptyWorldGenerator.h"

float FVoxelWorldGeneratorInstance::GetValue(int X, int Y, int Z) const
{
	float Value;
	GetValuesAndMaterialsAndVoxelTypes(&Value, nullptr, nullptr, FIntVector(X, Y, Z), FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
	return Value;
}

FVoxelMaterial FVoxelWorldGeneratorInstance::GetMaterial(int X, int Y, int Z) const
{
	FVoxelMaterial Material;
	GetValuesAndMaterialsAndVoxelTypes(nullptr, &Material, nullptr, FIntVector(X, Y, Z), FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
	return Material;
}

FVoxelType FVoxelWorldGeneratorInstance::GetVoxelType(int X, int Y, int Z) const
{
	FVoxelType VoxelType;
	GetValuesAndMaterialsAndVoxelTypes(nullptr, nullptr, &VoxelType, FIntVector(X, Y, Z), FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
	return VoxelType;
}

void FVoxelWorldGeneratorInstance::GetValueAndMaterialAndVoxelType(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial, FVoxelType& OutVoxelType)
{
	GetValuesAndMaterialsAndVoxelTypes(&OutValue, &OutMaterial, &OutVoxelType, FIntVector(X, Y, Z), FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
}

void FVoxelWorldGeneratorInstance::GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial)
{
	GetValuesAndMaterialsAndVoxelTypes(&OutValue, &OutMaterial, nullptr, FIntVector(X, Y, Z), FIntVector::ZeroValue, 1, FIntVector(1, 1, 1), FIntVector(1, 1, 1));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FVoxelWorldGeneratorPicker::FVoxelWorldGeneratorPicker()
	: UseClassOrObject(EVoxelWorldGeneratorClassOrObject::Object)
{

}

TSharedRef<FVoxelWorldGeneratorInstance> FVoxelWorldGeneratorPicker::GetWorldGenerator() const
{
	if (UseClassOrObject == EVoxelWorldGeneratorClassOrObject::Class)
	{
		UVoxelWorldGenerator* InstancedWorldGenerator = nullptr;

		if (WorldGeneratorClass)
		{
			InstancedWorldGenerator = NewObject<UVoxelWorldGenerator>((UObject*)GetTransientPackage(), WorldGeneratorClass);
		}
		if (!InstancedWorldGenerator)
		{
			UE_LOG(LogVoxel, Error, TEXT("VoxelWorldGeneratorPicker: Invalid world generator class"));
			return MakeShareable(new FEmptyWorldGeneratorInstance());
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
			UE_LOG(LogVoxel, Error, TEXT("VoxelWorldGeneratorPicker: Invalid world generator object"));
			return MakeShareable(new FEmptyWorldGeneratorInstance());
		}
	}
}

FString FVoxelWorldGeneratorPicker::GetName() const
{
	if (UseClassOrObject == EVoxelWorldGeneratorClassOrObject::Class)
	{
		if (WorldGeneratorClass)
		{
			return WorldGeneratorClass->GetName();
		}
		else
		{
			return UEmptyWorldGenerator::StaticClass()->GetName();
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
			return UEmptyWorldGenerator::StaticClass()->GetName();
		}
	}
}
