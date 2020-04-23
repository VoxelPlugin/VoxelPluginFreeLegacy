// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemActor.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelEditorDelegatesInterface.h"
#include "VoxelAssetActor.generated.h"

class UBoxComponent;
class FVoxelData;
class FVoxelDebugManager;
class IVoxelRenderer;
class FVoxelFixedResolutionLODManager;
class IVoxelPool;

UENUM()
enum class EVoxelAssetActorPreviewUpdateType
{
	// Will only update when Update is clicked, or when a property is changed
	Manually,
	// Will update after each move
	EndOfMove,
	// Will update while moving
	RealTime
};

UCLASS()
class VOXEL_API UAssetActorPrimitiveComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
};

UCLASS()
class VOXEL_API AVoxelAssetActor : public AVoxelPlaceableItemActor, public IVoxelEditorDelegatesInterface
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings")
	FVoxelTransformableWorldGeneratorPicker WorldGenerator;

	// Overrides the voxel world seeds
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings")
	TMap<FName, int32> Seeds;

	// Higher priority assets will be on top
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings")
	int32 Priority = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings", meta = (InlineEditConditionToggle))
	bool bOverrideAssetBounds = false;	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings", meta = (EditCondition = "bOverrideAssetBounds"))
	FIntBox AssetBounds = FIntBox(-25, 25);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings")
	bool bImportAsReference = true;

	UPROPERTY(EditAnywhere, Category = "Asset Actor Settings", meta = (EditCondition = "!bImportAsReference"))
	bool bSubtractiveAsset = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor Settings", meta = (EditCondition = "!bImportAsReference"))
	EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials;

public:
#if WITH_EDITORONLY_DATA
	// The lower, the better looking but the slower
	UPROPERTY(EditAnywhere, Category = "Preview Settings", meta = (ClampMin = "0", ClampMax = "24", UIMin = "0", UIMax = "10"))
	int32 PreviewLOD = 0;

	UPROPERTY(EditAnywhere, Category = "Preview Settings")
	EVoxelAssetActorPreviewUpdateType UpdateType = EVoxelAssetActorPreviewUpdateType::EndOfMove;

	// If true, the voxel asset actor position will be rounded to the nearest voxel position when moved
	UPROPERTY(EditAnywhere, Category = "Preview Settings")
	bool bRoundAssetPosition = false;
#endif

public:
	AVoxelAssetActor();

	
	// If VoxelWorldData is null, will only return the bounds
	FIntBox AddItemToData(
		AVoxelWorld* VoxelWorld, 
		FVoxelData* VoxelWorldData) const;
	
#if WITH_EDITOR
	void UpdatePreview();
#endif

private:
	UPROPERTY()
	USceneComponent* Root;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UAssetActorPrimitiveComponent* PrimitiveComponent;
	
	UPROPERTY()
	UBoxComponent* Box;
#endif

protected:
#if WITH_EDITOR
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

private:
#if WITH_EDITOR
	TVoxelSharedPtr<FVoxelData> Data;
	TVoxelSharedPtr<IVoxelRenderer> Renderer;
	TVoxelSharedPtr<FVoxelFixedResolutionLODManager> LODManager;
	TVoxelSharedPtr<FVoxelDebugManager> DebugManager;

	bool IsPreviewCreated() const;
	void CreatePreview();
	void DestroyPreview();
	void UpdateBox();
	void ClampTransform();
#endif
	
public:
#if WITH_EDITOR
	//~ Begin IVoxelEditorDelegatesInterface Interface
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) override;
	//~ End IVoxelEditorDelegatesInterface Interface
#endif

public:
#if WITH_EDITOR
	static TVoxelSharedPtr<IVoxelPool> StaticPool;
#endif
};