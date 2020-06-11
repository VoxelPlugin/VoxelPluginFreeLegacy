// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelTools/VoxelToolManager.h"
#include "UObject/GCObject.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInstanceDynamic;
class UTexture;

class FVoxelToolManagerTool : public FGCObject, public TVoxelSharedFromThis<FVoxelToolManagerTool>
{
public:
	struct FToolSettings
	{
		FName ToolName;
		
		const float* Ptr_Stride = nullptr;
		
		UMaterialInterface* const* Ptr_ToolOverlayMaterial = nullptr;
		UMaterialInterface* const* Ptr_ToolMeshMaterial = nullptr;

		const bool* Ptr_bAlignToMovement = nullptr;
		const FRotator* Ptr_Direction = nullptr;

		const bool* Ptr_bFixedNormal = nullptr;
		FVector FixedNormal = FVector::ZeroVector;

		bool bViewportSpaceMovement = false;
		const bool* Ptr_bShowPlanePreview = nullptr;
		const EVoxelToolManagerAlignment* Ptr_Alignment = nullptr;
		const bool* Ptr_bAirMode = nullptr;
		const float* Ptr_DistanceToCamera = nullptr;
	};
	
	const UVoxelToolManager& ToolManager;
	const FToolSettings ToolSettings;

	FVoxelToolManagerTool(const UVoxelToolManager& ToolManager, const FToolSettings& ToolSettings);
	virtual ~FVoxelToolManagerTool();
	
	void TriggerTick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData, const FHitResult& Hit);
	void ClearVoxelWorld();
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) = 0;
	//~ End FVoxelToolManagerTool Interface

protected:
	FVector GetToolPosition() const;
	FVector GetToolPreviewPosition() const;
	FVector GetToolNormal() const;
	FVector GetToolDirection() const;

	bool CanEdit() const { return bCanEdit; }
	bool LastFrameCanEdit() const { return bLastFrameCanEdit; }
	float GetMouseMovementSize() const { return MouseMovementSize; }
	const FVoxelToolManagerTickData& GetLastFrameTickData() const { return LastFrameTickData; }

	// Delta time accounting for the skipped frame waiting for updates
	float GetDeltaTime() const { return DeltaTime; }
	
	UMaterialInstanceDynamic& GetToolOverlayMaterialInstance() const;
	UMaterialInstanceDynamic& GetToolMeshMaterialInstance() const;
	
	void UpdateWorld(AVoxelWorld& World, const FVoxelIntBox& Bounds);
	void SaveFrameOnEndClick(const FVoxelIntBox& Bounds);
	void SetToolRenderingBounds(AVoxelWorld& World, const FBox& Bounds);
	// Note: Material will not be updated if the mesh did not change
	void UpdateToolMesh(
		UWorld* World, 
		UStaticMesh* Mesh, 
		UMaterialInterface* Material,
		const FTransform& Transform, 
		FName Id = NAME_None);
	FVoxelIntBox GetAndDebugBoundsToCache(AVoxelWorld& World, const FVoxelIntBox& Bounds, const FVoxelToolManagerTickData& TickData) const;

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	TWeakObjectPtr<AVoxelWorld> VoxelWorld;

	int32 NumPendingUpdates = 0;
	
	FVoxelToolRenderingId ToolRenderingId;
	
	bool bCanEdit = true;
	bool bLastFrameCanEdit = true;

	double LastTickTime = FPlatformTime::Seconds();
	float DeltaTime = 0.f;
	
	FVoxelToolManagerTickData LastFrameTickData;
	
	FVector CurrentPosition = FVector::ZeroVector;
	FVector CurrentNormal = FVector::UpVector;
	
	FVector MovementTangent = FVector::RightVector;
	FVector LastPositionUsedForTangent = FVector::ZeroVector;

	FVector StridePosition = FVector::ZeroVector;
	FVector StrideNormal = FVector::UpVector;
	FVector StrideDirection = FVector::ForwardVector;

	float MouseMovementSize = 0;

	struct FViewportSpaceMovement
	{
		FPlane LastClickPlane = FPlane(FVector::ZeroVector, FVector::UpVector);
		FVector LastClickPoint = FVector::ZeroVector;
		FVector LastClickNormal = FVector::UpVector;
	};
	FViewportSpaceMovement ViewportSpaceMovement;
	
	FVoxelIntBoxWithValidity PendingFrameBounds;

	// Map from id to mesh actor, to allow having multple meshes
	TMap<FName, TWeakObjectPtr<AStaticMeshActor>> StaticMeshActors;
	
	UMaterialInstanceDynamic* ToolOverlayMaterialInstance = nullptr;
	UMaterialInstanceDynamic* ToolMeshMaterialInstance = nullptr;
	UMaterialInstanceDynamic* PlaneMeshMaterialInstance = nullptr;
};