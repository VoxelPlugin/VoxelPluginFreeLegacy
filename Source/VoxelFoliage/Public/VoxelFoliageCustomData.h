// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelGeneratorOutputPicker.h"
#include "VoxelFoliageCustomData.generated.h"

UENUM(BlueprintType)
enum class EVoxelFoliageCustomDataType : uint8
{
	// 8 bits per channel, internally packed into a float
	// Use the TODO material function to extract the color
	ColorGeneratorOutput,
	// 32 bits float
	FloatGeneratorOutput,
	// Pass the material color
	// Use the TODO material function to extract the color
	MaterialColor,
	// Pass the material single index
	MaterialSingleIndex,
	// Pass an UV channel
	MaterialUV,
};

USTRUCT(BlueprintType)
struct FVoxelFoliageCustomData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelFoliageCustomDataType Type = EVoxelFoliageCustomDataType::ColorGeneratorOutput;

	// Whether to use the voxel world generator, or a custom one
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bUseMainGenerator = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelGeneratorPicker CustomGenerator;
	
	// Your generator needs to have a color output named like this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (OutputType = Color))
	FVoxelGeneratorOutputPicker ColorGeneratorOutputName;
	
	// Your generator needs to have a float output named like this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (OutputType = Float))
	FVoxelGeneratorOutputPicker FloatGeneratorOutputName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisplayName = "UV Channel", ClampMin = 0, ClampMax = 3))
	int32 UVChannel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisplayName = "UV Axis"))
	EVoxelUVAxis UVAxis = {};

public:
	// Only set if bUseMainGenerator = false and Type = GeneratorOutput
	TVoxelSharedPtr<FVoxelGeneratorInstance> GeneratorInstance;
	
#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
	{
		return (Type == EVoxelFoliageCustomDataType::ColorGeneratorOutput || Type == EVoxelFoliageCustomDataType::FloatGeneratorOutput)
			&& !bUseMainGenerator
			&& CustomGenerator.GetObject() == Object;
	}
#endif
};