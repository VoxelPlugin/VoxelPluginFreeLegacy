// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelGlobals.h"
#include "VoxelWorldGeneratorPicker.generated.h"

class UVoxelWorldGenerator;
class UVoxelTransformableWorldGenerator;
class FVoxelWorldGeneratorInstance;
class FVoxelTransformableWorldGeneratorInstance;

UENUM(BlueprintType)
enum class EVoxelWorldGeneratorPickerType : uint8
{
	Class,
	Object
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelWorldGeneratorPicker
{
	GENERATED_BODY()

public:
	using WorldGeneratorType = UVoxelWorldGenerator;
	
	FVoxelWorldGeneratorPicker() = default;
	FVoxelWorldGeneratorPicker(UClass* WorldGeneratorClass);
	FVoxelWorldGeneratorPicker(UVoxelWorldGenerator* WorldGeneratorObject);
	FVoxelWorldGeneratorPicker(TSoftClassPtr<UVoxelWorldGenerator> WorldGeneratorClass);
	FVoxelWorldGeneratorPicker(TSoftObjectPtr<UVoxelWorldGenerator> WorldGeneratorObject);
	FVoxelWorldGeneratorPicker(TYPE_OF_NULLPTR)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelWorldGeneratorPickerType Type = EVoxelWorldGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftClassPtr<UVoxelWorldGenerator> WorldGeneratorClass;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftObjectPtr<UVoxelWorldGenerator> WorldGeneratorObject = nullptr;

	// Might return nullptr!
	UVoxelWorldGenerator* GetWorldGenerator() const;
	// Will return WorldGeneratorObject or WorldGeneratorClass
	UObject* GetObject() const;
	// Will default to EmptyWorldGenerator if null
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance(bool bSilent) const;
	
	FORCEINLINE bool IsValid() const { return GetObject() != nullptr; }
	FORCEINLINE bool IsClass() const { return Type == EVoxelWorldGeneratorPickerType::Class; }
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelTransformableWorldGeneratorPicker
{
	GENERATED_BODY()

public:
	using WorldGeneratorType = UVoxelTransformableWorldGenerator;
	
	FVoxelTransformableWorldGeneratorPicker() = default;
	FVoxelTransformableWorldGeneratorPicker(UClass* WorldGeneratorClass);
	FVoxelTransformableWorldGeneratorPicker(UVoxelTransformableWorldGenerator* WorldGeneratorObject);
	FVoxelTransformableWorldGeneratorPicker(TSoftClassPtr<UVoxelTransformableWorldGenerator> WorldGeneratorClass);
	FVoxelTransformableWorldGeneratorPicker(TSoftObjectPtr<UVoxelTransformableWorldGenerator> WorldGeneratorObject);
	FVoxelTransformableWorldGeneratorPicker(TYPE_OF_NULLPTR)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelWorldGeneratorPickerType Type = EVoxelWorldGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftClassPtr<UVoxelTransformableWorldGenerator> WorldGeneratorClass;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSoftObjectPtr<UVoxelTransformableWorldGenerator> WorldGeneratorObject = nullptr;
	
	// Might return nullptr!
	UVoxelTransformableWorldGenerator* GetWorldGenerator() const;
	// Will return WorldGeneratorObject or WorldGeneratorClass
	UObject* GetObject() const;
	// Will default to EmptyWorldGenerator if null
	TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetInstance(bool bSilent) const;
	
	FORCEINLINE bool IsValid() const { return GetObject() != nullptr; }
	FORCEINLINE bool IsClass() const { return Type == EVoxelWorldGeneratorPickerType::Class; }
};