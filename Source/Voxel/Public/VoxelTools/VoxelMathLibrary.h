// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelMathLibrary.generated.h"

USTRUCT(BlueprintType, meta = (HasNativeMake = "Voxel.VoxelMathLibrary.MakeHaltonStream"))
struct FVoxelHaltonStream
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 InitialSeed = 0;
	
	UPROPERTY()
	mutable uint32 Seed = 0;
};

UCLASS()
class VOXEL_API UVoxelMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	/**
	 * Generates a random position on the unit sphere, given some random input between 0 and 1
	 * @param	Random		Random values, between 0 and 1. Can use RandomFloat, but also more complex noises like Halton or Sobol
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Math|Random")
	static FVector GetUnitVectorFromRandom(FVector2D Random);

public:
	UFUNCTION(BlueprintPure, meta = (Keywords = "construct build", NativeMakeFunc), Category = "Voxel|Math|Random")
	static FVoxelHaltonStream MakeHaltonStream(int32 InitialSeed);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Math|Random")
	static void ResetHaltonStream(const FVoxelHaltonStream& Stream);
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Math|Random")
	static float GetHalton1D(const FVoxelHaltonStream& Stream);
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Math|Random")
	static FVector2D GetHalton2D(const FVoxelHaltonStream& Stream);
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Math|Random")
	static FVector GetHalton3D(const FVoxelHaltonStream& Stream);
};