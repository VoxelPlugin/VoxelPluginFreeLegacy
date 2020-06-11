// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Engine/EngineTypes.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/SubclassOf.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelInstancedMeshSettings.generated.h"

class UVoxelHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class AVoxelSpawnerActor;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelInt32Interval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 Min = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	int32 Max = 0;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelInstancedMeshSettings
{
	GENERATED_BODY()

	FVoxelInstancedMeshSettings();
	
public:
	// Distance from camera at which each instance begins/completely to fade out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Settings")
	FVoxelInt32Interval CullDistance = { 100000, 200000 };

	/** Controls whether the foliage should cast a shadow or not. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Settings")
	bool bCastShadow = true;

	/** Controls whether the foliage should inject light into the Light Propagation Volume.  This flag is only used if CastShadow is true. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", meta=(EditCondition="bCastShadow"))
	bool bAffectDynamicIndirectLighting = false;

	/** Controls whether the primitive should affect dynamic distance field lighting methods.  This flag is only used if CastShadow is true. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", meta=(EditCondition="bCastShadow"))
	bool bAffectDistanceFieldLighting = false;

	/** Whether this foliage should cast dynamic shadows as if it were a two sided material. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", meta=(EditCondition="bCastShadow"))
	bool bCastShadowAsTwoSided = false;

	/** Whether the foliage receives decals. */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings")
	bool bReceivesDecals = true;

	/**
	 * If enabled, foliage will render a pre-pass which allows it to occlude other primitives, and also allows 
	 * it to correctly receive DBuffer decals. Enabling this setting may have a negative performance impact.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings")
	bool bUseAsOccluder = false;

	/** Custom collision for foliage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Settings", meta = (ShowOnlyInnerProperties))
	FBodyInstance BodyInstance;

	/** Force navmesh */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Instance Settings")
	TEnumAsByte<EHasCustomNavigableGeometry::Type> CustomNavigableGeometry = {};

	/**
	 * Lighting channels that placed foliage will be assigned. Lights with matching channels will affect the foliage.
	 * These channels only apply to opaque materials, direct lighting, and dynamic lighting and shadowing.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings")
	FLightingChannels LightingChannels{};

	/** If true, the foliage will be rendered in the CustomDepth pass (usually used for outlines) */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", meta=(DisplayName = "Render CustomDepth Pass"))
	bool bRenderCustomDepth = false;

	/** Optionally write this 0-255 value to the stencil buffer in CustomDepth pass (Requires project setting or r.CustomDepth == 3) */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings",  meta=(UIMin = "0", UIMax = "255", editcondition = "bRenderCustomDepth", DisplayName = "CustomDepth Stencil Value"))
	int32 CustomDepthStencilValue = 0;

	// If more instances are added before BuildDelay seconds elapsed, the tree build is queued
	// This is useful to avoid spending lots of time building the tree for nothing.
	// However, it can lead to delays in foliage spawning.
	// To disable this feature entirely, set it to 0
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", meta = (ClampMin = 0, DisplayName = "Culling Tree Build Delay"))
	float BuildDelay = 0.1;

	// If you want to edit the HISM properties create a BP inheriting from HierarchicalInstancedStaticMeshComponent and set it here
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Instance Settings", AdvancedDisplay, meta = (DisplayName = "HISM Template"))
	TSubclassOf<UVoxelHierarchicalInstancedStaticMeshComponent> HISMTemplate;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelSpawnerActorSettings
{
	GENERATED_BODY()

	FVoxelSpawnerActorSettings();

public:
	// Actor to spawn to replace the instanced mesh. After spawn, the SetStaticMesh event will be called on the actor with Mesh as argument
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	TSubclassOf<AVoxelSpawnerActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings", meta = (ShowOnlyInnerProperties))
	FBodyInstance BodyInstance;
	
	// Set the lifespan of this actor. When it expires the object will be destroyed.
	// Set to 0 to disable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings", meta = (ClampMin = 0))
	float Lifespan = 5.f;
};

struct FVoxelInstancedMeshAndActorSettings
{
	FVoxelInstancedMeshAndActorSettings() = default;
	FVoxelInstancedMeshAndActorSettings(
		TWeakObjectPtr<UStaticMesh> Mesh, 
		const TMap<int32, UMaterialInterface*>& SectionMaterials, 
		FVoxelInstancedMeshSettings MeshSettings, 
		FVoxelSpawnerActorSettings ActorSettings);
	
	TWeakObjectPtr<UStaticMesh> Mesh;
	// Index in the array = mesh section index
	TArray<TWeakObjectPtr<UMaterialInterface>> MaterialsOverrides;
	FVoxelInstancedMeshSettings MeshSettings;
	FVoxelSpawnerActorSettings ActorSettings;

	TMap<int32, UMaterialInterface*> GetSectionsMaterials() const;
	void SetSectionsMaterials(const TMap<int32, UMaterialInterface*>& SectionMaterials);
};

VOXEL_API bool operator==(const FVoxelInstancedMeshAndActorSettings& A, const FVoxelInstancedMeshAndActorSettings& B);
VOXEL_API uint32 GetTypeHash(const FVoxelInstancedMeshAndActorSettings& Settings);