// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldGenerator.h"
#include "CirclesWorldGenerator.generated.h"

UCLASS(Blueprintable)
class PROCEDURAL_API UCirclesWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UCirclesWorldGenerator() : Scale(10, 10), TopColor(230, 255, 0, 255), MiddleColor(255, 0, 26, 255), BottomColor(13, 0, 255, 255), MaxValue(1), MinValue(-1)
	{
	};

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);
	virtual float GetDefaultValue_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;

	UPROPERTY(EditAnywhere)
		FVector2D Scale;

	UPROPERTY(EditAnywhere)
		FColor TopColor;

	UPROPERTY(EditAnywhere)
		FColor MiddleColor;

	UPROPERTY(EditAnywhere)
		FColor BottomColor;

	UPROPERTY(EditAnywhere)
		float MaxValue;
	UPROPERTY(EditAnywhere)
		float MinValue;
};
