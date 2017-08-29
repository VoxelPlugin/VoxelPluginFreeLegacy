// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "TerrainImporterWorldGenerator.generated.h"


UENUM(BlueprintType)
enum class EBlur : uint8
{
	BE_NoBlur			UMETA(DisplayName = "No Blur"),
	BE_GaussianBlur 	UMETA(DisplayName = "Gaussian Blur"),
	BE_NormalBLur		UMETA(DisplayName = "Normal Blur")
};


/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UTerrainImporterWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UTerrainImporterWorldGenerator() : Bottom(-255), Top(255), LowerLimit(-252, -252), UpperLimit(252, 252), InValue(-1), OutValue(1)
	{
	};

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
		FVector2D LowerLimit;

	UPROPERTY(EditAnywhere)
		FVector2D UpperLimit;

	UPROPERTY(EditAnywhere)
		float InValue;

	UPROPERTY(EditAnywhere)
		float OutValue;

	UPROPERTY(EditAnywhere)
		EBlur Blur;

private:
	bool bCreated = false;
	TArray<float> Values;
	int SizeX;
	int SizeY;

	void Create();

};