// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.h"
#include "VoxelSplineImporter.generated.h"

class USplineComponent;

/**
*
*/
UCLASS(Blueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering"))
class VOXEL_API AVoxelSplineImporter : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Save Path: /Game/", RelativeToGameContentDir), Category = "Save configuration")
		FDirectoryPath SavePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save configuration")
		FString FileName;


	UPROPERTY(EditAnywhere, Category = "Splines configuration")
		TArray<USplineComponent*> Splines;

	UPROPERTY(EditAnywhere, Category = "Splines configuration")
		bool bSubstrative;

	UPROPERTY(EditAnywhere, Category = "Splines configuration")
		bool bSetMaterial;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bSetMaterial"), Category = "Splines configuration")
		FVoxelMaterial Material;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bSetMaterial"), Category = "Splines configuration")
		float VoxelSize;


	AVoxelSplineImporter();

	void ImportToAsset(FDecompressedVoxelDataAsset& Asset);

};
