// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VG_Example_MultiIndex.generated.h"

UCLASS(Blueprintable)
class UVG_Example_MultiIndex : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UMaterialInterface> Layer_2 = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/CoralMud/MI_CoralMud.MI_CoralMud"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UMaterialInterface> Layer_3 = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/Snow/MI_Snow.MI_Snow"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UMaterialInterface> Layer_0 = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/AerialGrassRock/MI_AerialGrassRock.MI_AerialGrassRock"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UMaterialInterface> Layer_1 = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath("/Voxel/Examples/Shared/Textures/TextureHaven/BrownMudRocks/MI_BrownMudRocks.MI_BrownMudRocks"));
	
	UVG_Example_MultiIndex();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
