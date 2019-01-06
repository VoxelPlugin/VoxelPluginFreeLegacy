// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelWorldGeneratorPicker.generated.h"

class UVoxelWorldGenerator;
class FVoxelWorldGeneratorInstance;

UENUM(BlueprintType)
enum class EVoxelWorldGeneratorPickerType : uint8
{
	Class,
	Object
};

/**
 * Used to select a world generator
 */
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelWorldGeneratorPicker
{
	GENERATED_BODY()

public:
	FVoxelWorldGeneratorPicker();

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelWorldGeneratorPickerType Type;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TSubclassOf<UVoxelWorldGenerator> WorldGeneratorClass;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelWorldGenerator* WorldGeneratorObject;

	TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator() const;
	FString GetName() const;
};

