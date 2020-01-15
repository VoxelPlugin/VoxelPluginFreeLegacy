// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFoliage.h"
#include "VoxelMaterial.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelToolsBase.generated.h"

USTRUCT(BlueprintType)
struct FModifiedVoxelValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float OldValue = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float NewValue = 0;
};

USTRUCT(BlueprintType)
struct FModifiedVoxelMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial OldMaterial = FVoxelMaterial(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial NewMaterial = FVoxelMaterial(ForceInit);
};

USTRUCT(BlueprintType)
struct FModifiedVoxelFoliage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFoliage OldFoliage = FVoxelFoliage(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelFoliage NewFoliage = FVoxelFoliage(ForceInit);
};

UCLASS()
class VOXEL_API UVoxelToolsBase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Base")
	static FIntBox GetModifiedVoxelValuesBounds(const TArray<FModifiedVoxelValue>& ModifiedVoxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Base")
	static FIntBox GetModifiedVoxelMaterialsBounds(const TArray<FModifiedVoxelMaterial>& ModifiedVoxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Base")
	static FIntBox GetModifiedVoxelFoliagesBounds(const TArray<FModifiedVoxelFoliage>& ModifiedVoxels);
};