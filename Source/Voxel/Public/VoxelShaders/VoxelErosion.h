// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RHIResources.h"
#include "VoxelTexture.h"
#include "VoxelErosion.generated.h"

class FVoxelErosionParameters;
class UTexture2D;

UCLASS(Blueprintable, BlueprintType)
class VOXEL_API UVoxelErosion : public UObject
{
	GENERATED_BODY()

public:
	// Must be a multiple of 32
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    int32 Size = 1024;

	// Time elapsed between each simulation step. Smaller = more stable, but slower
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float DeltaTime = 0.005;

	// The scale of the simulation. Should leave to default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Scale = 1;

	// Gravity, use to compute the speed of the water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Gravity = 10;

	// How much sediment a volume of water can carry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentCapacity = 0.05;

	// How much sediment is removed from height by the water in each step
	// This controls the "strength" of the erosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentDissolving = 0.001;

	// How much sediment can go from the water to the height
	// This controls how far the sediments are carried
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentDeposition = 0.0001;

	// Amount of water added per step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float RainStrength = 2;

	// Controls the evaporation of the water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Evaporation = 1;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init Parameters")
	FVoxelFloatTexture RainMapInit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init Parameters")
	FVoxelFloatTexture HeightmapInit;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	void Step(int32 Count = 10);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetTerrainHeightTexture();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetWaterHeightTexture();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetSedimentTexture();

private:
    int32 RealSize = 0; // Can't be changed through BP after init
	bool bIsInit = false;
	
	FUnorderedAccessViewRHIRef RainMapUAV;
	FUnorderedAccessViewRHIRef TerrainHeightUAV;
	FUnorderedAccessViewRHIRef TerrainHeight1UAV;
	FUnorderedAccessViewRHIRef WaterHeightUAV;
	FUnorderedAccessViewRHIRef WaterHeight1UAV;
	FUnorderedAccessViewRHIRef WaterHeight2UAV;
	FUnorderedAccessViewRHIRef SedimentUAV;
	FUnorderedAccessViewRHIRef Sediment1UAV;
	FUnorderedAccessViewRHIRef OutflowUAV;
	FUnorderedAccessViewRHIRef VelocityUAV;

	FTexture2DRHIRef RainMap;
	FTexture2DRHIRef TerrainHeight;
	FTexture2DRHIRef TerrainHeight1;
	FTexture2DRHIRef WaterHeight;
	FTexture2DRHIRef WaterHeight1;
	FTexture2DRHIRef WaterHeight2;
	FTexture2DRHIRef Sediment;
	FTexture2DRHIRef Sediment1;
	FTexture2DRHIRef Outflow;
	FTexture2DRHIRef Velocity;

	template<typename T>
	void RunShader(const FVoxelErosionParameters& Parameters);

	void CopyTextureToRHI(const TVoxelTexture<float>& Texture, const FTexture2DRHIRef& RHITexture);
	void CopyRHIToTexture(const FTexture2DRHIRef& RHITexture, TVoxelSharedRef<TVoxelTexture<float>::FTextureData>& Texture);

	static void CopyTextureToRHI_RenderThread(const TVoxelTexture<float>& Texture, const FTexture2DRHIRef& RHITexture);
	static void CopyRHIToTexture_RenderThread(const FTexture2DRHIRef& RHITexture, TVoxelTexture<float>::FTextureData& Texture);
	
	void Init_RenderThread();
	void Step_RenderThread(const FVoxelErosionParameters& Parameters, int32 Count);
};