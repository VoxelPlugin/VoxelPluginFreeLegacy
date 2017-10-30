// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDataAsset.h"
#include "VoxelMeshAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering"))
class VOXEL_API AVoxelMeshAsset : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Save Path: /Game/", RelativeToGameContentDir), Category = "Save configuration")
		FDirectoryPath SavePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save configuration")
		FString FileName;



	UPROPERTY(EditAnywhere, Category = "Import configuration")
		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
		int MeshVoxelSize;

	// TODO: UIMIN = 1
	UPROPERTY(EditAnywhere, Category = "Import configuration")
		int HalfFinalVoxelSizeDivisor;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
		bool bDrawPoints;

	AVoxelMeshAsset();

	void ImportToAsset(FDecompressedVoxelDataAsset& Asset);
};