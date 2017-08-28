// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "TerrainImporterWorldGenerator.generated.h"

/**
 * Not working
 */
UCLASS(Blueprintable)
class VOXEL_API UTerrainImporterWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);
	virtual float GetDefaultValue_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;

	UPROPERTY(EditAnywhere)
		UTexture2D* Heightmap;

	UPROPERTY(EditAnywhere)
		float Bottom;

	UPROPERTY(EditAnywhere)
		float Top;

	UPROPERTY(EditAnywhere)
		FVector2D Lower;

	UPROPERTY(EditAnywhere)
		FVector2D Upper;

	UPROPERTY(EditAnywhere)
		float InValue;

	UPROPERTY(EditAnywhere)
		float OutValue;

private:
	bool bCreated = false;
	TArray<float> Values;
	uint32 SizeX;
	uint32 SizeY;

	void Create();

};