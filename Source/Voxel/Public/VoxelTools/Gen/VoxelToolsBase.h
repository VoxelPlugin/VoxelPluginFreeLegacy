// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelToolsBase.generated.h"

class AVoxelWorld;

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

	FModifiedVoxelValue() = default;

	FModifiedVoxelValue(const FIntVector& Position, float OldValue, float NewValue)
		: Position(Position)
		, OldValue(OldValue)
		, NewValue(NewValue)
	{
	}

	FModifiedVoxelValue(const FIntVector& Position, FVoxelValue OldValue, FVoxelValue NewValue)
		: Position(Position)
		, OldValue(OldValue.ToFloat())
		, NewValue(NewValue.ToFloat())
	{
	}
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

DECLARE_DELEGATE(FOnVoxelToolComplete);
DECLARE_DELEGATE_OneParam(FOnVoxelToolComplete_WithModifiedValues, const TArray<FModifiedVoxelValue>&);
DECLARE_DELEGATE_OneParam(FOnVoxelToolComplete_WithModifiedMaterials, const TArray<FModifiedVoxelMaterial>&);

UCLASS()
class VOXEL_API UVoxelToolsBase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Base")
	static FVoxelIntBox GetModifiedVoxelValuesBounds(const TArray<FModifiedVoxelValue>& ModifiedVoxels);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Base")
	static FVoxelIntBox GetModifiedVoxelMaterialsBounds(const TArray<FModifiedVoxelMaterial>& ModifiedVoxels);
};