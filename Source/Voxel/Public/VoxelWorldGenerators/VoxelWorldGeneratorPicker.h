// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
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

VOXEL_API TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetWorldGeneratorInstance(bool bSilent, UVoxelWorldGenerator* WorldGenerator);
VOXEL_API TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetWorldGeneratorInstance(bool bSilent, UVoxelTransformableWorldGenerator* WorldGenerator);

template<typename TThis, typename TWorldGenerator, typename TWorldGeneratorInstance>
struct TVoxelWorldGeneratorPicker
{
	using WorldGeneratorType = TWorldGenerator;

	// Might return nullptr!
	TWorldGenerator* GetWorldGenerator() const
	{
		if (This().Type == EVoxelWorldGeneratorPickerType::Class)
		{
			if (This().Class)
			{
				return This().Class->template GetDefaultObject<TWorldGenerator>();
			}
			else
			{
				return nullptr;
			}
		}
		else
		{
			return This().Object;
		}
	}
	UObject* GetObject() const
	{
		if (This().Type == EVoxelWorldGeneratorPickerType::Class)
		{
			return (UObject*)This().Class;
		}
		else
		{
			return (UObject*)This().Object;
		}
	}
	// Will default to EmptyWorldGenerator if null
	TVoxelSharedRef<TWorldGeneratorInstance> GetInstance(bool bSilent) const
	{
		return GetWorldGeneratorInstance(bSilent, GetWorldGenerator());
	}
	
	FORCEINLINE bool IsValid() const { return GetObject() != nullptr; }
	FORCEINLINE bool IsClass() const { return This().Type == EVoxelWorldGeneratorPickerType::Class; }
	FORCEINLINE bool IsObject() const { return This().Type == EVoxelWorldGeneratorPickerType::Object; }

private:
	TThis& This() { return static_cast<TThis&>(*this); }
	const TThis& This() const { return static_cast<const TThis&>(*this); }
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelWorldGeneratorPicker
#if CPP
	: public TVoxelWorldGeneratorPicker<
	FVoxelWorldGeneratorPicker,
	UVoxelWorldGenerator,
	FVoxelWorldGeneratorInstance>
#endif
{
	GENERATED_BODY()

public:
	FVoxelWorldGeneratorPicker() = default;
	FVoxelWorldGeneratorPicker(TYPE_OF_NULLPTR);
	FVoxelWorldGeneratorPicker(UClass* InClass);
	FVoxelWorldGeneratorPicker(TSubclassOf<UVoxelWorldGenerator> InClass);
	FVoxelWorldGeneratorPicker(UVoxelWorldGenerator* InObject);
	FVoxelWorldGeneratorPicker(TSoftClassPtr<UVoxelWorldGenerator> InClass);
	FVoxelWorldGeneratorPicker(TSoftObjectPtr<UVoxelWorldGenerator> InObject);

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelWorldGeneratorPickerType Type = EVoxelWorldGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSubclassOf<UVoxelWorldGenerator> Class = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelWorldGenerator* Object = nullptr;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelTransformableWorldGeneratorPicker
#if CPP
	: public TVoxelWorldGeneratorPicker<
	FVoxelTransformableWorldGeneratorPicker,
	UVoxelTransformableWorldGenerator,
	FVoxelTransformableWorldGeneratorInstance>
#endif
{
	GENERATED_BODY()

public:
	FVoxelTransformableWorldGeneratorPicker() = default;
	FVoxelTransformableWorldGeneratorPicker(TYPE_OF_NULLPTR);
	FVoxelTransformableWorldGeneratorPicker(UClass* InClass);
	FVoxelTransformableWorldGeneratorPicker(TSubclassOf<UVoxelTransformableWorldGenerator> InClass);
	FVoxelTransformableWorldGeneratorPicker(UVoxelTransformableWorldGenerator* InObject);
	FVoxelTransformableWorldGeneratorPicker(TSoftClassPtr<UVoxelTransformableWorldGenerator> InClass);
	FVoxelTransformableWorldGeneratorPicker(TSoftObjectPtr<UVoxelTransformableWorldGenerator> InObject);

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelWorldGeneratorPickerType Type = EVoxelWorldGeneratorPickerType::Class;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSubclassOf<UVoxelTransformableWorldGenerator> Class = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelTransformableWorldGenerator* Object = nullptr;
};