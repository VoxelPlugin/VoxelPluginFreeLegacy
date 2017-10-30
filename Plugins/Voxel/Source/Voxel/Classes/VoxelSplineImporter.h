// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.h"
#include "Components/SphereComponent.h"
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

	UPROPERTY(EditAnywhere, Category = "Splines configuration")
		float VoxelSize;


	AVoxelSplineImporter();

	void ImportToAsset(FDecompressedVoxelDataAsset& Asset);

protected:
#if WITH_EDITOR
	void Tick(float DeltaTime) override;
	bool ShouldTickIfViewportsOnly() const override;
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
		TArray<USphereComponent*> Spheres;
};
