// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VG_Example_Erosion.generated.h"

UCLASS(Blueprintable)
class UVG_Example_Erosion : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Erosion Material Offset"))
	float Erosion_Material_Offset = 0.65;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Erosion Material Strength"))
	float Erosion_Material_Strength = 3.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Erosion Strength"))
	float Erosion_Strength = 0.008;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Height"))
	float Height = 500.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Rocks"))
	TSoftObjectPtr<UMaterialInterface> Rocks = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/BrownMudRocks/MI_BrownMudRocks.MI_BrownMudRocks"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Snow"))
	TSoftObjectPtr<UMaterialInterface> Snow = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/Snow/MI_Snow.MI_Snow"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Valleys Height"))
	float Valleys_Height = -0.5;
	
	UVG_Example_Erosion();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
