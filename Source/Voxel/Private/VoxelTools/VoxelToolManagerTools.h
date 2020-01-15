// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "IntBox.h"
#include "UObject/GCObject.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelTools/VoxelToolManager.h"

struct FVoxelMeshImporterRenderTargetCache;
struct FVoxelDataAssetData;
struct FVoxelMeshImporterInputData;
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
		
		bool bSupportStride = false;
		bool const* bEnableStride = nullptr;
		float const* Stride = nullptr;
		
		bool bNeedToolRendering = false;
		UMaterialInterface* const* ToolMaterial = nullptr;

		bool bNeedToolMesh = false;
		UStaticMesh* const* ToolMesh = nullptr;
		UMaterialInterface* const* ToolMeshMaterial = nullptr;
		
		bool bSupportToolDirection = false;
		bool const* bAlignToMovement = nullptr;
		FRotator const* Direction = nullptr;

		bool bViewportSpaceMovement = false;
		EVoxelToolManagerAlignment const* Alignment = nullptr;
		bool const* bAirMode = nullptr;
		float const* DistanceToCamera = nullptr;
		
		bool bWaitForUpdates = false;
		bool bSaveFrameOnEndClick = false;
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
	FVector GetToolNormal() const;
	FVector GetToolDirection() const;

	inline float GetMouseMovementSize() const { return MouseMovementSize; }
	
	UMaterialInstanceDynamic& GetToolMaterialInstance() const;
	
	void UpdateWorld(AVoxelWorld& World, const FIntBox& Bounds);
	void SaveFrameOnEndClick(const FIntBox& Bounds);
	void SetToolRenderingBounds(AVoxelWorld& World, const FBox& Bounds);
	void SetToolMeshTransform(UWorld* World, const FTransform& Transform);
	FIntBox GetAndDebugBoundsToCache(AVoxelWorld& World, const FIntBox& Bounds, const FVoxelToolManagerTickData& TickData) const;

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	TWeakObjectPtr<AVoxelWorld> VoxelWorld;

	int32 NumPendingUpdates = 0;
	
	FVoxelToolRenderingId ToolRenderingId;
	
	FVector CurrentPosition = FVector::ZeroVector;
	FVector CurrentNormal = FVector::UpVector;
	
	FVector MovementTangent = FVector::RightVector;
	FVector LastPositionUsedForTangent = FVector::ZeroVector;

	FVector StridePosition = FVector::ZeroVector;
	FVector StrideNormal = FVector::UpVector;
	FVector StrideDirection = FVector::ForwardVector;
	double LastStridePositionUpdate = 0;

	float MouseMovementSize = 0;
	
	FPlane LastClickPlane = FPlane(FVector::ZeroVector, FVector::UpVector);
	FVector LastClickNormal = FVector::UpVector;

	FVoxelToolManagerTickData LastFrameTickData;
	
	FIntBoxWithValidity PendingFrameBounds;

	TWeakObjectPtr<AStaticMeshActor> StaticMeshActor;
	UMaterialInstanceDynamic* ToolMaterialInstance = nullptr;
};

class FVoxelToolManagerTool_Surface : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_SurfaceSettings& SurfaceSettings;
	
	explicit FVoxelToolManagerTool_Surface(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};

class FVoxelToolManagerTool_Trim : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_TrimSettings& TrimSettings;
	
	explicit FVoxelToolManagerTool_Trim(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};

class FVoxelToolManagerTool_Smooth : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_SmoothSettings& SmoothSettings;
	
	explicit FVoxelToolManagerTool_Smooth(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};

class FVoxelToolManagerTool_Sphere : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_SphereSettings& SphereSettings;
	
	explicit FVoxelToolManagerTool_Sphere(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};

