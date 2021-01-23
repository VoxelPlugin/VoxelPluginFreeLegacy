// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelGeneratorOutputPicker.h"
#include "VoxelFoliageDensity.generated.h"

UENUM(BlueprintType)
enum class EVoxelFoliageDensityType : uint8
{
	// Use a constant as density
	Constant,
	// Use a generator output
	GeneratorOutput,
	// Use one of the material RGBA channels
	MaterialRGBA,
	// Use the material UV channels
	MaterialUVs,
	// Use a five way blend strength
	MaterialFiveWayBlend,
	// Use a single index channel
	SingleIndex,
	// Use a multi index channel
	MultiIndex
};

USTRUCT(BlueprintType)
struct FVoxelFoliageDensity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelFoliageDensityType Type = EVoxelFoliageDensityType::Constant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Constant = 1.f;

	// Whether to use the voxel world generator, or a custom one
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bUseMainGenerator = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelGeneratorPicker CustomGenerator;
	
	// Your generator needs to have a float output named like this
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (OutputType = Float))
	FVoxelGeneratorOutputPicker GeneratorOutputName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisplayName = "RGBA Channel"))
	EVoxelRGBA RGBAChannel = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisplayName = "UV Channel", ClampMin = 0, ClampMax = 3))
	int32 UVChannel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (DisplayName = "UV Axis"))
	EVoxelUVAxis UVAxis = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 4))
	int32 FiveWayBlendChannel = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 255))
	TArray<int32> SingleIndexChannels = { 0 };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 255))
	TArray<int32> MultiIndexChannels = { 0 };

	// If true, will use 1 - Density instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bInvertDensity = false;

public:
	// Only set if bUseMainGenerator = false and Type = GeneratorOutput
	TVoxelSharedPtr<FVoxelGeneratorInstance> GeneratorInstance;
	
#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
	{
		return Type == EVoxelFoliageDensityType::GeneratorOutput && !bUseMainGenerator && CustomGenerator.GetObject() == Object;
	}
#endif
};