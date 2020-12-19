// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelGeneratorOutputPicker.h"
#include "VoxelFoliage/VoxelFoliageSpawnSettings.h"
#include "VoxelFoliageBiome.generated.h"

class UVoxelFoliage;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFoliageBiomeTypeEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	int32 Seed = 0;

	// The individual foliage asset settings will be ignored and these will be used instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	FVoxelFoliageSpawnSettings SpawnSettings;
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelFoliageBiomeType : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayName = "Generator (for autocomplete only)"))
	FVoxelGeneratorPicker OutputPickerGenerator;

	// The generator output used to select the biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (OutputType = int))
	FVoxelGeneratorOutputPicker BiomeOutput; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	TArray<FVoxelFoliageBiomeTypeEntry> Entries;

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	FSimpleMulticastDelegate OnPostEditChangeProperty;

	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const;
#endif
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFoliageBiomeEntry
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Biomes")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes", Instanced)
	UVoxelFoliage* Foliage = nullptr;
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelFoliageBiome : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	UVoxelFoliageBiomeType* Type = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
	TArray<FVoxelFoliageBiomeEntry> Entries;
	
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	//~ End UObject Interface

	void Fixup();
	void BindDelegate();

#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const;
#endif
};