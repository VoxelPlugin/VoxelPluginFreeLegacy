// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelDataImpl.h"
#include "VoxelTools/Tools/VoxelTool.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "VoxelToolBase.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DEFINE_TYPED_VOXEL_SPARSE_ARRAY_ID(FVoxelToolRenderingId);

UENUM(BlueprintType)
enum class EVoxelToolAlignment : uint8
{
	// The tool follow the surface. The surface is frozen until the next click.
	Surface,
	// Align with the camera view plane
	View,
	// Align with XY plane
	Ground,
	// Align with the camera view plane, with the Z component zeroed out
	Up
};

USTRUCT(BlueprintType)
struct FVoxelToolBaseConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Materials", EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* OverlayMaterial = nullptr;

	UPROPERTY(Category = "Materials", EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* MeshMaterial = nullptr;

public:
	// Set to 0 to disable
	UPROPERTY(Category = "General", EditAnywhere, BlueprintReadWrite)
	float Stride = 0.f;
	
public:
	// If false will align to movement
	UPROPERTY(Category = "Direction", EditAnywhere, BlueprintReadWrite)
	bool bUseFixedDirection = false;

	// If bUseFixedDirection = true
	UPROPERTY(Category = "Direction", EditAnywhere, BlueprintReadWrite)
	FRotator FixedDirection = FRotator::ZeroRotator;
	
public:
	UPROPERTY(Category = "Normal", EditAnywhere, BlueprintReadWrite)
	bool bUseFixedNormal = false;
	
	// If UseFixedNormal = true
	UPROPERTY(Category = "Normal", EditAnywhere, BlueprintReadWrite)
	FVector FixedNormal = FVector::ZeroVector;
	
public:
	// Whether this tool has an alignment setting
	UPROPERTY(Category = "Aligment", EditAnywhere, BlueprintReadWrite)
	bool bHasAlignment = false;
	
	UPROPERTY(Category = "Aligment", EditAnywhere, BlueprintReadWrite)
	EVoxelToolAlignment Alignment = EVoxelToolAlignment::Surface;
	
	// If Alignment != Surface
	UPROPERTY(Category = "Aligment", EditAnywhere, BlueprintReadWrite)
	bool bAirMode = false;
	
	// If Alignment != Surface and AirMode = true
	UPROPERTY(Category = "Aligment", EditAnywhere, BlueprintReadWrite)
	float DistanceToCamera = 0.f;
	
	// If Alignment != Surface
	UPROPERTY(Category = "Aligment", EditAnywhere, BlueprintReadWrite)
	bool bShowPlanePreview = false;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract, Blueprintable)
class VOXEL_API UVoxelToolBase : public UVoxelTool
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const {}
	virtual void Tick();
	virtual void UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance) {}
	virtual FVoxelIntBoxWithValidity DoEdit() { return {}; }
	//~ End UVoxelToolBase Interface

public:
	// Called on tick
	UFUNCTION(BlueprintNativeEvent, DisplayName = "GetToolConfig")
	void K2_GetToolConfig(FVoxelToolBaseConfig InConfig, FVoxelToolBaseConfig& OutConfig) const;
	// Called first, before DoEdit and UpdateRender
	// Note: Tick is a BlueprintImplementableEvent. The native Tick will always be called before.
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Tick")
	void K2_Tick();
	// Might not always be called - if you want to compute things for the frame, use Tick
	UFUNCTION(BlueprintNativeEvent, DisplayName = "UpdateRender")
	void K2_UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance);
	// Returned bounds will be updated & SaveFrame called on them, as well as RegenerateSpawners if enabled
	// Tick will always be called before
	UFUNCTION(BlueprintNativeEvent, DisplayName = "DoEdit")
	FVoxelIntBoxWithValidity K2_DoEdit();

public:
	void K2_GetToolConfig_Implementation(FVoxelToolBaseConfig InConfig, FVoxelToolBaseConfig& OutConfig) const
	{
		GetToolConfig(InConfig);
		OutConfig = InConfig;
	}
	void K2_UpdateRender_Implementation(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
	{
		UpdateRender(OverlayMaterialInstance, MeshMaterialInstance);
	}
	FVoxelIntBoxWithValidity K2_DoEdit_Implementation()
	{
		return DoEdit();
	}

public:
	virtual void EnableTool() override;
	virtual void DisableTool() override;

	virtual void CallTool(AVoxelWorld* VoxelWorld, const FVoxelToolTickData& TickData, const FCallToolParameters& Parameters) final override;

	virtual AVoxelWorld* GetVoxelWorld() const override final { return VoxelWorld; }
	
public:
	UFUNCTION(BlueprintCallable, Category = "Tool|Transform")
	FVector GetToolPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Tool|Transform")
	FVector GetToolPreviewPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Tool|Transform")
	FVector GetToolNormal() const;

	UFUNCTION(BlueprintCallable, Category = "Tool|Transform")
	FVector GetToolDirection() const;

public:
	UFUNCTION(BlueprintCallable, Category = "Tool")
	bool CanEdit() const { return bCanEdit; }
	
	UFUNCTION(BlueprintCallable, Category = "Tool")
	bool LastFrameCanEdit() const { return bLastFrameCanEdit; }
	
	UFUNCTION(BlueprintCallable, Category = "Tool")
	const FVoxelToolTickData& GetTickData() const { return TickData; }
	
	UFUNCTION(BlueprintCallable, Category = "Tool")
	const FVoxelToolTickData& GetLastFrameTickData() const { return LastFrameTickData; }

public:
	UFUNCTION(BlueprintCallable, Category = "Tool")
	float GetMouseMovementSize() const { return MouseMovementSize; }

	// Delta time accounting for the skipped frame waiting for updates
	UFUNCTION(BlueprintCallable, Category = "Tool")
	float GetDeltaTime() const { return DeltaTime; }

	// Will also debug them
	UFUNCTION(BlueprintCallable, Category = "Tool")
	FVoxelIntBox GetBoundsToCache(const FVoxelIntBox& Bounds) const;

	UFUNCTION(BlueprintCallable, Category = "Tool")
	float GetValueAfterAxisInput(FName AxisName, float CurrentValue, float Min = 0.f, float Max = 1.f) const;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Tool|Render")
	void SetToolOverlayBounds(const FBox& Bounds);
	
	// Note: Material will not be updated if the mesh did not change
	UFUNCTION(BlueprintCallable, Category = "Tool|Render")
	void UpdateToolMesh(UStaticMesh* Mesh, 
		UMaterialInterface* Material,
		const FTransform& Transform, 
		FName Id = NAME_None);

protected:
	TVoxelDataImpl<> GetDataImpl(FVoxelData& Data) const
	{
		return TVoxelDataImpl<>(Data, SharedConfig->bMultiThreaded, false);
	}
	template<typename T>
	TVoxelDataImpl<T> GetDataImpl(FVoxelData& Data) const
	{
		return TVoxelDataImpl<T>(Data, SharedConfig->bMultiThreaded, true);
	}
	template<typename T, typename TData>
	void CacheData(TData& Data, const FVoxelIntBox& Bounds)
	{
		if (SharedConfig->bCacheData)
		{
			Data.template CacheBounds<T>(Bounds, SharedConfig->bMultiThreaded);
		}
	}
	
private:
	UPROPERTY(Transient)
	AVoxelWorld* VoxelWorld = nullptr;

	FVoxelToolBaseConfig ToolBaseConfig;

	int32 NumPendingUpdates = 0;
	
	FVoxelToolRenderingId ToolRenderingId;
	
	bool bCanEdit = true;
	bool bLastFrameCanEdit = true;
	
	FVoxelToolTickData TickData;
	FVoxelToolTickData LastFrameTickData;

	double LastTickTime = FPlatformTime::Seconds();
	float DeltaTime = 0.f;
	
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

	// Map from id to mesh actor, to allow having multiple meshes
	TMap<FName, TWeakObjectPtr<AStaticMeshActor>> StaticMeshActors;
	
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* ToolOverlayMaterialInstance = nullptr;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* ToolMeshMaterialInstance = nullptr;
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* PlaneMeshMaterialInstance = nullptr;

	void ClearVoxelWorld();
	void ApplyPendingFrameBounds();
};