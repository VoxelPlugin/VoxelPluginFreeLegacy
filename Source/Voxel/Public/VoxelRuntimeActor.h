// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"

#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelRender/VoxelLODMaterials.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelRuntimeActor.generated.h"

class UVoxelLODSubsystemProxy;
class UVoxelRendererSubsystemProxy;

class UVoxelFoliageCollection;
class UVoxelMultiplayerInterface;
class UVoxelPlaceableItemManager;
class UVoxelProceduralMeshComponent;

UCLASS()
class VOXEL_API AVoxelRuntimeActor : public AActor
{
	GENERATED_BODY()

public:
	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", meta = (Recreate, ClampMin = 0.0001))
	float VoxelSize = 100;

	// Generator of this world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", meta = (Recreate))
	FVoxelGeneratorPicker Generator;

	// Will be automatically created if not set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", Instanced, meta = (Recreate))
	UVoxelPlaceableItemManager* PlaceableItemManager = nullptr;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bCreateWorldAutomatically = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General", meta = (DisplayName = "Use camera if no invokers found"))
	bool bUseCameraIfNoInvokersFound = true;
	
	// Keep all the changes in memory to enable undo/redo. Can be expensive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General", meta = (Recreate))
	bool bEnableUndoRedo = false;

	// If true, the voxel world will try to stay near its original coordinates when rebasing, and will offset the voxel coordinates instead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bEnableCustomWorldRebasing = false;

	// If true, will merge asset actors in the scene on create.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bMergeAssetActors = true;

	// If true, will merge disable edits boxes in the scene on create.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bMergeDisableEditsBoxes = true;

	// Will hide voxel messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bDisableOnScreenMessages = false;

	// Will disable all debug features
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General")
	bool bDisableDebugManager = false;
	
public:
	// WorldSizeInVoxel = RENDER_CHUNK_SIZE * 2^DataOctreeDepth.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "Voxel - World Size", meta = (Recreate, ClampMin = 1, ClampMax = 26, UIMin = 1, UIMax = 26))
	int32 RenderOctreeDepth = 10;

	// Size of an edge of the world
	UPROPERTY(EditAnywhere, Category = "Voxel - World Size", meta = (Recreate, ClampMin = 1, DisplayName = "World Size (in voxel)"))
	uint32 WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(10);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - World Size", meta = (Recreate, InlineEditConditionToggle))
	bool bUseCustomWorldBounds = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - World Size", meta = (Recreate, EditCondition = "bUseCustomWorldBounds"))
	FVoxelIntBox CustomWorldBounds;
	
public:
	// Chunks can't have a LOD strictly higher than this. Useful is background has a too low resolution.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - LOD Settings", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int32 MaxLOD = FVoxelUtilities::ClampMesherDepth(32);
	
	// Chunks can't have a LOD strictly lower than this. Useful when in space for instance, combined with a manual BP call to ApplyLODSettings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int32 MinLOD = 0;
	
	// In world space. If invokers move by less than this distance LODs won't be updated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (UpdateLODs))
	float InvokerDistanceThreshold = 100;
	
	// Min delay between two LOD updates, in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (RecreateRender, ClampMin = 0), DisplayName = "Min Delay Between LOD Updates")
	float MinDelayBetweenLODUpdates = 0.1;

	// If true, the LODs will be updated only once at start
	// LODs can still be updated using ForceLODsUpdate or ApplyLODSettings
	// For example, can be useful when used with a Max LOD of 0 for worlds that have the highest resolution LOD everywhere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (RecreateRender))
	bool bConstantLOD = false;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (Recreate /* also used by generator */))
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	// Only used if Material Config = RGB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (UpdateRenderer))
	UMaterialInterface* VoxelMaterial = nullptr;

	// The material collection to use in Single Index or Double Index material config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (UpdateRenderer))
	UVoxelMaterialCollectionBase* MaterialCollection;

	// Per LOD material overrides
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|LOD", meta = (UpdateRenderer, DisplayName = "LOD Materials"))
	TArray<FVoxelLODMaterials> LODMaterials; 

	// Per LOD material collections overrides
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|LOD", meta = (UpdateRenderer, DisplayName = "LOD Material Collections"))
	TArray<FVoxelLODMaterialCollections> LODMaterialCollections; 

	// UV Config, ignored if Greedy Meshing is enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|UVs", meta = (RecreateRender, DisplayName = "UV Config"))
	EVoxelUVConfig UVConfig = EVoxelUVConfig::GlobalUVs;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|UVs", meta = (RecreateRender, DisplayName = "UV Scale"))
	float UVScale = 1;

	// Normal config, only respected by Marching Cubes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Normals", meta = (RecreateRender))
	EVoxelNormalConfig NormalConfig = EVoxelNormalConfig::GradientNormal;

	// Hardness settings for RGB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Hardness", meta = (DisplayName = "RGB Hardness"))
	EVoxelRGBHardness RGBHardness = EVoxelRGBHardness::FiveWayBlend;

	// Material Index -> Hardness, for Single/Multi index, or RGB if Four/Five Way Blend
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Hardness")
	TMap<FString, float> MaterialsHardness;

	// If true, then in RGB mode additional vertices will be created to ensure that no colors are ever blended
	// Does nothing in cubic mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	bool bHardColorTransitions = false;
	
	// Only for Cubic mode. If true, the material index will be 3 x Index + 0 for top, 3 x Index + 1 for sides and 3 x Index + 2 for bottom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	bool bOneMaterialPerCubeSide = false;

	// These materials won't be rendered nor have any collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	TArray<uint8> HolesMaterials;

	// Apply custom mesh settings per material index
	// Will create more mesh components!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	TMap<uint8, FVoxelMeshConfig> MaterialsMeshConfigs;

	// Use 16 bits float instead of 32 bits. Halves the UVs memory usage, but lower precision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	bool bHalfPrecisionCoordinates = false;

	// If true, will interpolate the adjacent voxels colors to find the exact vertex color
	// In SingleIndex, will interpolate DataA/B/C
	// In MultiIndex, will interpolate Blend and Wetness
	// Twice as expensive, as requires to make twice as many material queries!
	// Might not look as great if the material outside of the voxel surface isn't set to something nice
	// Only works with marching cubes for now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	bool bInterpolateColors = false;
	
	// If true, will interpolate the adjacent voxels uvs to find the exact vertex uvs
	// Twice as expensive, as requires to make twice as many material queries!
	// Only works with marching cubes for now
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender))
	bool bInterpolateUVs = false;

	// When ticked, will convert the color stored in the material (as a 4 bytes color) from sRGB to Linear
	// However, since the target will still be 4 bytes, the conversion won't be perfect
	// This is a limitation of vertex colors sadly
	// NOTE: It is recommended to leave this off, and to tick bLinearColor when painting colors instead
	// That way color operations are done in linear space, which is recommended
	// NOTE: DO NOT enable in Multi Index, it will just mess up the blend parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Materials", meta = (RecreateRender, DisplayName = "sRGB Colors"))
	bool bSRGBColors = false;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Rendering", meta = (RecreateRender))
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;

	// For marching cubes only
	// If 0, will do nothing
	// If above zero, will round the vertices positions to the nearest multiple of (1 / RenderSharpness)
	// Visually, it will give a more "sharp" look, 1 being the sharpest, 2 3 etc being less and less sharp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, UIMin = 0, UIMax = 10, ClampMin = 0))
	int32 RenderSharpness = 0;
	
	// If true, a dynamic instance will be created for each chunk. Else, the material will be used directly
	// Disable this if you want to use dynamic material instances as voxel world materials
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bCreateMaterialInstances = true;

	// Whether to dither chunks
	// Requires CreateMaterialInstances
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, EditCondition = bCreateMaterialInstances))
	bool bDitherChunks = true;

	// Dithering duration when changing LODs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 0, EditCondition = bDitherChunks))
	float ChunksDitheringDuration = 1;

	// When enabled, the component will be rendering into the far shadow cascades (only for directional lights).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bCastFarShadow = false;
	
	// Custom procedural mesh class to use
	// Use this to override specific rendering settings such as cast shadows, render custom depth...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	TSubclassOf<UVoxelProceduralMeshComponent> ProcMeshClass;

	// Chunks with a LOD strictly higher than this won't be rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (UpdateLODs, ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26))
	int32 ChunksCullingLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(32);

	// Whether to render the world, or to just use it for collisions/navmesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (Recreate))
	bool bRenderWorld = true;

	// If true, will create static meshes for proc meshes that are overlapping lightmass importance volumes
	// allowing to have static lighting through volumetric lightmaps
	// You can toggle static meshes using voxel.renderer.ShowStaticMeshComponents 1
	// and force an update using voxel.renderer.UpdateStaticMeshComponents
	// In the editor, this will force LOD 0 on chunks overlapping lightmass importance volumes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bContributesToStaticLighting = false;

	// If true, will try to use the static path when possible. Using the static path is much cheaper on the render thread
	// Should always be on unless you see some issues
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bUseStaticPath = false;
	
	// Will destroy any intermediate render data to free up memory
	// Does not support any kind of updates INCLUDING LOD updates: your LODs will be frozen!
	// Note: if MergeChunks is true, chunk meshes memory won't be cleared as it can't know if a new mesh will be added to the cluster
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bStaticWorld = false;

	// If true, will merge quads generated by the cubic mesher
	// Will ignore UV Config
	// Only works with the RGB Material config
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bGreedyCubicMesher = false;

	// The size of the textures in the pool used by the greedy cubic mesher to store the colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 64, UIMin = 128, UIMax = 2048))
	int32 TexturePoolTextureSize = 1024;
	
	// If true, the mesh indices will be sorted to improve GPU cache performance. Adds a cost to the async mesh building. If you don't see any perf difference, leave it off
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bOptimizeIndices = false;

	// Will generate distance fields on LOD 0 chunks
	// Has a cost of around 1 ms per chunk (on async thread)
	// Doesn't work with chunks merging or single/double index material config with different materials per chunk
	// Requires UE 4.23+
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bGenerateDistanceFields = false;

	// Chunks with LOD <= this will have distance fields
	// Be careful when increasing because of the memory usage caused by distance fields
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26, EditCondition = "bGenerateDistanceFields"))
	int32 MaxDistanceFieldLOD = 4;

	// By how many voxels to extend the chunks distance fields (on every side)
	// This is needed so that distance fields nicely overlap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 0, ClampMax = 32, UIMin = 0, UIMax = 8, EditCondition = "bGenerateDistanceFields"))
	int32 DistanceFieldBoundsExtension = 4;

	// By how much to divide the distance field resolution
	// By default it'll be 32x32x32: if the divisor is 2, it'll be 16x16x16, if 4 8x8x8...
	// Increasing this decreases quality of the distance field, but saves huge amount of VRAM
	// NOTE: increasing this can lead to messy distance fields as some signs are messy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 1, ClampMax = 32, UIMin = 1, UIMax = 8, EditCondition = "bGenerateDistanceFields"))
	int32 DistanceFieldResolutionDivisor = 1;
	
	/** Useful for reducing self shadowing from distance field methods when using world position offset to animate the mesh's vertices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, EditCondition = "bGenerateDistanceFields"))
	float DistanceFieldSelfShadowBias = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bEnableTransitions = true;

	// Will merge chunks together to reduce draw calls.
	// Only merges chunks of the same LOD!
	// Enabling this disables CreateMaterialInstances and DitherChunks.
	// When turning this on, it is recommended to reduce the priority of the Mesh Merge category (eg set it to 0)
	// Else mesh merge are done before meshing tasks, even if these meshing tasks would have made the merge invalid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bMergeChunks = false;

	// Size in voxels of the clusters. Scales with LOD (eg if 64, for LOD 3 it will be 64 * 2 * 2 * 2 = 512)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, EditCondition = "bMergeChunks"))
	int32 ChunksClustersSize = 64;

	// If true, additional meshes with the normal chunk size will be spawned only for collisions & navmesh
	// Recommended, as cooking collision for merged chunks takes forever
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, EditCondition = "bMergeChunks"))
	bool bDoNotMergeCollisionsAndNavmesh = true;

	// Increases the chunks bounding boxes, useful when using tessellation
	// Setting it to 0 can cause issues on flat worlds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	float BoundsExtension = 100;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Foliage", meta = (RecreateSpawners))
	TArray<UVoxelFoliageCollection*> FoliageCollections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Foliage", meta = (RecreateSpawners))
	EVoxelFoliageWorldType FoliageWorldType = EVoxelFoliageWorldType::Flat;
	
	// If true, will ignore Alpha when reading material strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Foliage", meta = (RecreateSpawners))
	bool bIsFourWayBlend = false;
	
	// The chunk size, in voxels, of a single HISM component
	// Lower = higher draw calls/object count
	// Higher = more delay when building the occlusion tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Foliage", meta = (RecreateSpawners, DisplayName = "HISM Chunk Size"))
	int32 HISMChunkSize = 2048;

	// Only nearby instances have collisions
	// Configure the distance using this
	// In voxels!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Foliage", meta = (RecreateSpawners, ClampMin = 0))
	int32 SpawnersCollisionDistanceInVoxel = 64;

	// If more instances than this are spawned, they will not be displayed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Foliage", meta = (RecreateSpawners, ClampMin = 0))
	int64 MaxNumberOfFoliageInstances = MAX_int32;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions", meta = (RecreateRender))
	bool bEnableCollisions = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions, ShowOnlyInnerProperties))
	FBodyInstance CollisionPresets;
	
	// Whether to compute simple collision meshes or not
	// Change this only if you want to use the voxel world as a rigidbody
	// Simple collision won't match the geometry exactly
	// Using simple collision is not less expensive than using complex collisions, as the convex hulls are far from optimized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	TEnumAsByte<ECollisionTraceFlag> CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;

	/**
	 * Determine whether a Character can step up onto this component.
	 * This controls whether they can try to step up on it when they bump in to it, not whether they can walk on it after landing on it.
	 * @see FWalkableSlopeOverride
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	TEnumAsByte<ECanBeCharacterBase> CanCharacterStepUpOn = ECB_Yes;
	
	/**	Should 'Hit' events fire when this object collides during physics simulation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions|Events", meta = (Recreate, EditCondition = bEnableCollisions, DisplayName = "Simulation Generates Hit Events"))
	bool bNotifyRigidBodyCollision = false;
	
	/**
	 * If true, this component will generate overlap events when it is overlapping other components (eg Begin Overlap).
	 * Both components (this and the other) must have this enabled for overlap events to occur.
	 *
	 * @see [Overlap Events](https://docs.unrealengine.com/latest/INT/Engine/Physics/Collision/index.html#overlapandgenerateoverlapevents)
	 * @see UpdateOverlaps(), BeginComponentOverlap(), EndComponentOverlap()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions|Events", meta = (Recreate, EditCondition = bEnableCollisions))
	bool bGenerateOverlapEvents = false;
	
	// If false, use only invokers collisions settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions|Visible Chunks", meta = (UpdateLODs, EditCondition = bEnableCollisions))
	bool bComputeVisibleChunksCollisions = true;

	// Max LOD to compute collisions on. Inclusive. If not 0 collisions won't be precise. Does not affect invokers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions|Visible Chunks", meta = (UpdateLODs, ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26, EditCondition = "bComputeVisibleChunksCollisions && bEnableCollisions"))
	int32 VisibleChunksCollisionsMaxLOD = 5;
	
	/**	Allows you to override the PhysicalMaterial to use for simple collision on this body. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	UPhysicalMaterial* PhysMaterialOverride = nullptr;
	
	/** If true Continuous Collision Detection (CCD) will be used for this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	bool bUseCCD = false;

	// If true and the greedy cubic mesher is enabled, will use cubes as simple collision
	// Cubes will be created greedily, so they'll be as large as possible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	bool bSimpleCubicCollision = true;

	// Will use a lower LOD for cubic collisions, making them much faster to simulate at the cost of accuracy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, ClampMin = 0, ClampMax = 4, EditCondition = bEnableCollisions))
	int32 SimpleCubicCollisionLODBias = 0;
	
	// Number of convex hulls to create per chunk per axis for simple collisions
	// More hulls = more precise collisions, but much more expensive physics
	// You can check the result in the Player Collision view
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (RecreateRender, ClampMin = 1, ClampMax = 32, UIMin = 1, UIMax = 32, EditCondition = bEnableCollisions))
	int32 NumConvexHullsPerAxis = 2;
	
	// Clean collisions meshes when cooking them.
	// Disabling this makes cooking collision slightly faster, but might lead to physx crashing in case of invalid geometry.
	// Enable this if you are getting crashes in the physx code
	// To check the performance improvements: voxel.LogCollisionCookingTimes 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (RecreateRender, EditCondition = bEnableCollisions))
	bool bCleanCollisionMeshes = true;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (RecreateRender))
	bool bEnableNavmesh = false;

	// If false, use only invokers navmesh settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (UpdateLODs, EditCondition = bEnableNavmesh))
	bool bComputeVisibleChunksNavmesh = true;

	// Max LOD to compute navmesh on. Inclusive. Does not affect invokers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (UpdateLODs, ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26, EditCondition = "bEnableNavmesh && bComputeVisibleChunksNavmesh"))
	int32 VisibleChunksNavmeshMaxLOD = 0;
	
public:
	// Async tasks are sorted based on 2 values:
	// - first, their priority category
	// - then, their own priority (most of the time their distance from voxel invokers)
	// Using priority categories, you can determine which tasks to compute first
	// Setting 2 task type to the same category will allow to sort them only based on their distance from a voxel invoker
	// eg, for foliage and meshing tasks: meshes will spawn at the same time as the foliage on top of them
	// If you want to spawn the meshes slightly before foliage, you can offset the tasks own priorities using the PriorityOffsets below
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (Recreate, EditCondition = "bCreateGlobalPool"))
	TMap<EVoxelTaskType, int32> PriorityCategories;

	// Allows to offset tasks own priorities
	// Only useful for tasks that have the same priority category!
	// Most values are in voxel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (Recreate, EditCondition = "bCreateGlobalPool"))
	TMap<EVoxelTaskType, int32> PriorityOffsets;

	// Max time in milliseconds to spend on mesh updates per tick
	// If this is too low world will generate very slowly
	// If this is too high you will get lag spikes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (RecreateRender, ClampMin = 0.001))
	float MeshUpdatesBudget = 1000;

	// The rate at which events are fired (number of updates per seconds). Used for foliage spawning, foliage collision, binded BP events...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (RecreateRender, UIMin = 1, UIMax = 60))
	float EventsTickRate = 15;
	
	// Depth to which to subdivide the data octree on start
	// Will create 8^X nodes, so keep low!
	// Useful to avoid update tasks locking the entire octree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (Recreate, ClampMin = 0))
	int32 DataOctreeInitialSubdivisionDepth = 4;

public:
	// Is this world synchronized using the plugin multiplayer system?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Multiplayer", meta = (Recreate))
	bool bEnableMultiplayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Multiplayer", meta = (Recreate, EditCondition = "bMultiplayer"))
	TSubclassOf<UVoxelMultiplayerInterface> MultiplayerInterface;
	
	// Number of sync per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Multiplayer", meta = (Recreate, EditCondition = "bMultiplayer"))
	float MultiplayerSyncRate = 15;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Subsystems")
	TSubclassOf<UVoxelRendererSubsystemProxy> RendererSubsystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Subsystems")
	TSubclassOf<UVoxelLODSubsystemProxy> LODSubsystem;

public:
	UPROPERTY(EditAnywhere, Category = "Voxel - Debug")
	mutable TArray<TWeakObjectPtr<UTexture>> DebugTextures;
};