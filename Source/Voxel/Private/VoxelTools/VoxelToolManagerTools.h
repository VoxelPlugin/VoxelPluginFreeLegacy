// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelTexture.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelTools/VoxelToolManager.h"

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
		
		UMaterialInterface* const* Ptr_ToolMaterial = nullptr;

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

	inline bool CanEdit() const { return bCanEdit; }
	inline bool LastFrameCanEdit() const { return bLastFrameCanEdit; }
	inline float GetMouseMovementSize() const { return MouseMovementSize; }
	inline const FVoxelToolManagerTickData& GetLastFrameTickData() const { return LastFrameTickData; }
	
	UMaterialInstanceDynamic& GetToolMaterialInstance() const;
	
	void UpdateWorld(AVoxelWorld& World, const FIntBox& Bounds);
	void SaveFrameOnEndClick(const FIntBox& Bounds);
	void SetToolRenderingBounds(AVoxelWorld& World, const FBox& Bounds);
	// Note: Material will not be updated if the mesh did not change
	void UpdateToolMesh(
		UWorld* World, 
		UStaticMesh* Mesh, 
		UMaterialInterface* Material,
		const FTransform& Transform, 
		FName Id = NAME_None);
	FIntBox GetAndDebugBoundsToCache(AVoxelWorld& World, const FIntBox& Bounds, const FVoxelToolManagerTickData& TickData) const;

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	TWeakObjectPtr<AVoxelWorld> VoxelWorld;

	int32 NumPendingUpdates = 0;
	
	FVoxelToolRenderingId ToolRenderingId;
	
	bool bCanEdit = true;
	bool bLastFrameCanEdit = true;

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
	
	FIntBoxWithValidity PendingFrameBounds;

	// Map from id to mesh actor, to allow having multple meshes
	TMap<FName, TWeakObjectPtr<AStaticMeshActor>> StaticMeshActors;
	
	UMaterialInstanceDynamic* ToolMaterialInstance = nullptr;
	UMaterialInstanceDynamic* PlaneMeshMaterialInstance = nullptr;
};

class FVoxelToolManagerTool_Surface : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_SurfaceSettings& SurfaceSettings;
	
	explicit FVoxelToolManagerTool_Surface(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface

	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

private:
	struct FMaskWorldGeneratorCache
	{
		UObject* GeneratorObject = nullptr;
		float Scale = 0;
		float Radius = 0;
		bool bScaleWithRadius = false;

		TMap<FName, int32> Seeds;
		TVoxelTexture<float> Texture;
		UTexture2D* RenderTexture = nullptr;
	};
	FMaskWorldGeneratorCache MaskWorldGeneratorCache;
};

class FVoxelToolManagerTool_Flatten : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_FlattenSettings& FlattenSettings;
	
	explicit FVoxelToolManagerTool_Flatten(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface

private:
	FVector LastClickFlattenPosition = FVector::ZeroVector;
	FVector LastClickFlattenNormal = FVector::UpVector;
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

class FVoxelToolManagerTool_Level : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_LevelSettings& LevelSettings;
	
	explicit FVoxelToolManagerTool_Level(const UVoxelToolManager& ToolManager);
	
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


class FVoxelToolManagerTool_Revert : public FVoxelToolManagerTool
{
public:
	const FVoxelToolManager_RevertSettings& RevertSettings;
	
	explicit FVoxelToolManagerTool_Revert(const UVoxelToolManager& ToolManager);
	
	//~ Begin FVoxelToolManagerTool Interface
	virtual void Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData) override;
	//~ End FVoxelToolManagerTool Interface
};