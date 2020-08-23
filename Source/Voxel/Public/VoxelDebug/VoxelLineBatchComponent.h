// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Components/LineBatchComponent.h"
#include "VoxelLineBatchComponent.generated.h"

UCLASS()
class VOXEL_API UVoxelLineBatchComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	// Buffer of lines to draw. No support for depth priority
	TArray<struct FBatchedLine> BatchedLines;
	// Buffer or points to draw
	TArray<struct FBatchedPoint> BatchedPoints;
	// Buffer of simple meshes to draw
	TArray<struct FBatchedMesh> BatchedMeshes;
	
	// Default time that lines/points will draw for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float DefaultLifeTime = 1.0f;

	// Whether to calculate a tight accurate bounds (encompassing all points), or use a giant bounds that is fast to compute
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bCalculateAccurateBounds = false;

	UVoxelLineBatchComponent();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UActorComponent Interface.
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	//~ End UActorComponent Interface.

	// Clear all batched lines, points and meshes
	void Flush();
};

class VOXEL_API FVoxelLineBatcherSceneProxy : public FPrimitiveSceneProxy
{
public:
	explicit FVoxelLineBatcherSceneProxy(const UVoxelLineBatchComponent* InComponent);

	//~ Begin FPrimitiveSceneProxy Interface
	virtual SIZE_T GetTypeHash() const override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;
	//~ End FPrimitiveSceneProxy Interface

	uint32 GetAllocatedSize() const;

private:
	TArray<FBatchedLine> Lines;
	TArray<FBatchedPoint> Points;
	TArray<FBatchedMesh> Meshes;
};