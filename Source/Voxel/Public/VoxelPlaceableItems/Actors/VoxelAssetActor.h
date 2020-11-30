// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActor.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelEditorDelegatesInterface.h"
#include "VoxelAssetActor.generated.h"

class UBoxComponent;
class FVoxelRuntime;

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

	//~ Begin UPrimitiveComponent Interface
#if WITH_EDITOR
	virtual bool IgnoreBoundsForEditorFocus() const override { return true; }
#endif
	//~ End UPrimitiveComponent Interface
};

UCLASS(HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HOLD", "LOD", "Cooking", "Collision"))
class VOXEL_API AVoxelAssetActor : public AVoxelPlaceableItemActor, public IVoxelEditorDelegatesInterface
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor")
	FVoxelTransformableGeneratorPicker Generator;

	// Higher priority assets will be on top
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor")
	int32 Priority = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor", meta = (InlineEditConditionToggle))
	bool bOverrideAssetBounds = false;	
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor", meta = (EditCondition = "bOverrideAssetBounds"))
	FVoxelIntBox AssetBounds = FVoxelIntBox(-25, 25);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor")
	bool bImportAsReference = true;

	UPROPERTY(EditAnywhere, Category = "Asset Actor", meta = (EditCondition = "!bImportAsReference"))
	bool bSubtractiveAsset = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Asset Actor", meta = (EditCondition = "!bImportAsReference"))
	EVoxelAssetMergeMode MergeMode = EVoxelAssetMergeMode::InnerValuesAndInnerMaterials;

public:
	// If true, will not add to any voxel world and will spawn a new one instead
	// Useful for physics
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel World")
	bool bSpawnNewVoxelWorld = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Voxel World", meta = (EditCondition = "bSpawnNewVoxelWorld"))
	bool bSimulatePhysics = true;

public:
	// The lower, the better looking but the slower
	UPROPERTY(EditAnywhere, Category = "Preview", meta = (ClampMin = "0", ClampMax = "24", UIMin = "0", UIMax = "10"))
	int32 PreviewLOD = 0;

	UPROPERTY(EditAnywhere, Category = "Preview")
	EVoxelAssetActorPreviewUpdateType UpdateType = EVoxelAssetActorPreviewUpdateType::EndOfMove;

	// If true, the voxel asset actor position will be rounded to the nearest voxel position when moved
	// Always on in cubic mode
	UPROPERTY(EditAnywhere, Category = "Preview")
	bool bRoundAssetPosition = false;
	
	// If true, the voxel asset actor rotation will be rounded to the nearest valid rotation (90/180/-90)
	// Always on in cubic mode
	UPROPERTY(EditAnywhere, Category = "Preview")
	bool bRoundAssetRotation = false;

	// Increase this if you want a higher quality preview
	// Be careful: might freeze Unreal if too high!
	UPROPERTY(EditAnywhere, Category = "Preview", AdvancedDisplay)
	uint32 MaxPreviewChunks = 1024;

public:
	AVoxelAssetActor();

	//~ Begin AVoxelPlaceableItemActor Interface
	virtual void AddItemToWorld(AVoxelWorld* World) override;
	virtual int32 GetPriority() const override;
	//~ End AVoxelPlaceableItemActor Interface
	
	// If VoxelWorldData is null, will only return the bounds
	FVoxelIntBox AddItemToData(
		AVoxelWorld* VoxelWorld, 
		FVoxelData* VoxelWorldData) const;
	void ClampTransform();
	
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
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End AActor Interface

private:
#if WITH_EDITOR
	TVoxelSharedPtr<FVoxelRuntime> Runtime;

	bool IsPreviewCreated() const;
	void CreatePreview();
	void DestroyPreview();
	void UpdateBox();
#endif
	
public:
#if WITH_EDITOR
	//~ Begin IVoxelEditorDelegatesInterface Interface
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) override;
	//~ End IVoxelEditorDelegatesInterface Interface
#endif
};