// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDataAsset.h"
#include "VoxelMeshImporter.generated.h"

/**
 *
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering"))
class VOXEL_API AVoxelMeshImporter : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Save Path: /Game/", RelativeToGameContentDir), Category = "Save configuration")
		FDirectoryPath SavePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save configuration")
		FString FileName;



	UPROPERTY(EditAnywhere, Category = "Import configuration")
		UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, Category = "Import configuration", meta = (ClampMin = "0", UIMin = "0"))
		float MeshVoxelSize;

	UPROPERTY(EditAnywhere, Category = "Import configuration", meta = (ClampMin = "1", UIMin = "1"))
		int HalfFinalVoxelSizeDivisor;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
		AActor* ActorInsideTheMesh;

	UPROPERTY(EditAnywhere, Category = "Debug")
		bool bDrawPoints;

	AVoxelMeshImporter();

	void ImportToAsset(FDecompressedVoxelDataAsset& Asset);
};