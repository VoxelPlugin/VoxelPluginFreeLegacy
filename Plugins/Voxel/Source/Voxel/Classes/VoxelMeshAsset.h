// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/LineBatchComponent.h"
#include <forward_list>
#include "VoxelMeshAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class VOXEL_API AVoxelMeshAsset : public AActor
{
	GENERATED_BODY()

public:
	AVoxelMeshAsset();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* ActorToImport;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UPrimitiveComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* MinPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* MaxPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float VoxelSizeInUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAdd;


	UPROPERTY(EditAnywhere)
		float MinValue;

	UPROPERTY(EditAnywhere)
		float MaxValue;

	// Avoid crash
	UPROPERTY(EditAnywhere, AdvancedDisplay)
		int MaxResolution;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<float> Values;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<bool> IsInside;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		FIntVector Size;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ImportIntoWorld(AVoxelWorld* World, FIntVector Position, bool bAsync, bool bDebugPoints);

	void Import();

	void UpdateLines();

private:
	UPROPERTY()
		ULineBatchComponent* DebugLineBatch;

	void LineTrace(UPrimitiveComponent* Component, TArray<std::forward_list<float>>& ValuesLists, FIntVector IMin, int X, int Y, int Z, int DeltaX, int DeltaY, int DeltaZ);
};