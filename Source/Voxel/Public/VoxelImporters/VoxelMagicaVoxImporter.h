// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelMagicaVoxImporter.generated.h"

class AVoxelWorld;
class UVoxelDataAsset;
class AVoxelAssetActor;
class UVoxelMagicaVoxScene;
struct FVoxelDataAssetData;

struct ogt_vox_scene;
struct ogt_vox_palette;
struct ogt_vox_transform;
struct ogt_vox_model;
struct ogt_vox_instance;
struct ogt_vox_layer;
struct ogt_vox_group;

class VOXEL_API FVoxelMagicaVoxScene
{
public:
	const ogt_vox_scene& Scene;
	const TArrayView<const ogt_vox_model*> Models;
	const TArrayView<const ogt_vox_instance> Instances;
	const TArrayView<const ogt_vox_layer> Layers;
	const TArrayView<const ogt_vox_group> Groups;

	explicit FVoxelMagicaVoxScene(const ogt_vox_scene& Scene);
	~FVoxelMagicaVoxScene();
	UE_NONCOPYABLE(FVoxelMagicaVoxScene);

	UVoxelMagicaVoxScene* Import(
		UObject* Parent,
		FName NamePrefix,
		EObjectFlags Flags,
		bool bUsePalette,
		TArray<UVoxelDataAsset*>& OutAssets) const;
	
	bool ImportModel(FVoxelDataAssetData& Asset, int32 ModelIndex, bool bUsePalette) const;

public:
	static TVoxelSharedPtr<FVoxelMagicaVoxScene> LoadScene(const FString& Filename, FString& OutError);
	static void ImportModel(FVoxelDataAssetData& Asset, const ogt_vox_model& Model, bool bUsePalette, const ogt_vox_palette& Palette);
	static FTransform ConvertTransform(const ogt_vox_transform& Transform);
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMagicaVoxSceneEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UVoxelDataAsset* Asset = nullptr;

	// Center of the asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FString Layer;
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelMagicaVoxScene : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	TArray<FVoxelMagicaVoxSceneEntry> Entries;

	UPROPERTY(VisibleAnywhere, Category = "Import")
	FString ImportPath;
};

UCLASS(BlueprintType)
class VOXEL_API AVoxelMagicaVoxSceneActor : public AActor
{
	GENERATED_BODY()
	
public:
	AVoxelMagicaVoxSceneActor();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
	float VoxelSize = 100;

	UPROPERTY(VisibleAnywhere, Category = "Scene")
	TMap<AVoxelAssetActor*, FTransform> ActorTransforms;

	UPROPERTY(VisibleAnywhere, Category = "Scene")
	AVoxelWorld* VoxelWorld = nullptr;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void ApplyVoxelSize();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetScene(UVoxelMagicaVoxScene* Scene);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};