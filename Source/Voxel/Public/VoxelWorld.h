// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"

#include "IntBox.h"
#include "VoxelMathUtilities.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelWorldInterface.h"
#include "VoxelEditorDelegatesInterface.h"
#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelWorld.generated.h"

class UVoxelSpawnerConfig;
class UVoxelWorldSaveObject;
class UVoxelWorldRootComponent;
class UVoxelMultiplayerInterface;
class UVoxelMaterialCollectionBase;
class UVoxelProceduralMeshComponent;
class IVoxelPool;
class IVoxelRenderer;
class IVoxelLODManager;
class FVoxelData;
class FVoxelDebugManager;
class FVoxelProcGenManager;
class FVoxelSpawnerManager;
class FVoxelMultiplayerManager;
class FVoxelInstancedMeshManager;
class FVoxelToolRenderingManager;
struct FVoxelWorldGeneratorInit;
struct FVoxelLODDynamicSettings;
struct FVoxelRendererDynamicSettings;
enum class EVoxelTaskType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldDestroyed);

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld : public AVoxelWorldInterface, public IVoxelEditorDelegatesInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWorldLoaded OnWorldLoaded;

	// Called right before destroying the world. Use this if you want to save data
	UPROPERTY(BlueprintAssignable)
	FOnWorldDestroyed OnWorldDestroyed;

public:
	UPROPERTY(Category = "Voxel", VisibleAnywhere, BlueprintReadOnly)
	UVoxelWorldRootComponent* WorldRoot;

public:
	// Automatically loaded on creation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Save", meta = (Recreate))
	UVoxelWorldSaveObject* SaveObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save", meta = (FilePathFilter = "voxelsave"))
	FFilePath SaveFilePath;

	// If true, will save the world to SaveFilePath each time it's saved to the save object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save")
	bool bAutomaticallySaveToFile = false;

	// If true, will add the current time & date to the filepath when saving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Save")
	bool bAppendDateToSavePath = false;

	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	bool bRecomputeNormalsBeforeBaking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	UStaticMesh* BakedMeshTemplate = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake")
	TSubclassOf<UStaticMeshComponent> BakedMeshComponentTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Bake", meta = (RelativeToGameContentDir))
	FFilePath BakedDataPath = { "/Game/VoxelStaticData" };

	//////////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview", meta = (Recreate, ClampMin = 1))
	int32 NumberOfThreadsForPreview = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Preview", meta = (Recreate))
	bool bEnableFoliageInEditor = true;

	//////////////////////////////////////////////////////////////////////////////

	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", meta = (Recreate, ClampMin = 0.0001))
	float VoxelSize = 100;

	// Generator of this world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", meta = (Recreate))
	FVoxelWorldGeneratorPicker WorldGenerator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - General", meta = (Recreate))
	TMap<FName, int32> Seeds;
		
	UPROPERTY(EditDefaultsOnly, Category = "Voxel - General")
	bool bCreateWorldAutomatically = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - General", meta = (DisplayName = "Use camera if no invokers found"))
	bool bUseCameraIfNoInvokersFound = false;
	
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

	//////////////////////////////////////////////////////////////////////////////

	// WorldSizeInVoxel = CHUNK_SIZE * 2^Depth.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "Voxel - World Size", meta = (Recreate, ClampMin = 1, ClampMax = 24, UIMin = 1, UIMax = 24))
	int32 OctreeDepth = 10;

	// Size of an edge of the world
	UPROPERTY(EditAnywhere, Category = "Voxel - World Size", meta = (Recreate, ClampMin = 1, DisplayName = "World Size (in voxel)"))
	int32 WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(10);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - World Size", meta = (Recreate, InlineEditConditionToggle))
	bool bUseCustomWorldBounds = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - World Size", meta = (Recreate, EditCondition = "bUseCustomWorldBounds"))
	FIntBox CustomWorldBounds;
	
	//////////////////////////////////////////////////////////////////////////////

	// Chunks can't have a LOD strictly higher than this. Useful is background has a too low resolution.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - LOD Settings", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int32 MaxLOD = MAX_WORLD_DEPTH - 1;

	// In world space. Chunks under this distance from voxel invokers will have at least the given LOD.
	// CANNOT be used to force a higher (= lower resolution) LOD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - LOD Settings", meta = (UpdateLODs, DisplayName = "LODs Min Distances"))
	TMap<FString, float> LODToMinDistance = { {"0", 10000.f } };
	
	// Chunks can't have a LOD strictly lower than this. Useful when in space for instance, combined with a manual BP call to ApplyLODSettings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int32 MinLOD = 0;
	
	// In world space. If invokers move by less than this distance LODs won't be updated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (UpdateLODs))
	float InvokerDistanceThreshold = 100;
	
	// Min delay between 2 LOD updates, in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (RecreateRender, ClampMin = 0), DisplayName = "Min Delay Between LOD Updates")
	float MinDelayBetweenLODUpdates = 0.1;

	// If true, the LODs will be updated only once at start. Useful when used with a Max LOD of 0 for worlds that have the highest resolution LOD everywhere
	// LODs can still be updated using ForceLODsUpdate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - LOD Settings", meta = (RecreateRender))
	bool bConstantLOD = false;
	
	//////////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (Recreate /* also used by world generator */))
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	// Only used if Material Config = RGB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (UpdateRenderer))
	UMaterialInterface* VoxelMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials", meta = (UpdateRenderer))
	UVoxelMaterialCollectionBase* MaterialCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|UVs", meta = (RecreateRender, DisplayName = "UV Config"))
	EVoxelUVConfig UVConfig = EVoxelUVConfig::GlobalUVs;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|UVs", meta = (RecreateRender, DisplayName = "UV Scale"))
	float UVScale = 1;

	// You should leave this to Gradient Normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Normals", meta = (RecreateRender))
	EVoxelNormalConfig NormalConfig = EVoxelNormalConfig::GradientNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Tessellation", meta = (RecreateRender))
	bool bEnableTessellation = false;

	// Only used if Material Config = RGB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Tessellation", meta = (UpdateRenderer, EditCondition ="bEnableTessellation"))
	UMaterialInterface* TessellatedVoxelMaterial = nullptr;
	
	// Tessellation distance in world space.
	// Chunks under this distance will have their adjacency indices computed.
	// For a more precise control use the material tessellation multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Tessellation", meta = (UpdateLODs, ClampMin = 0, EditCondition ="bEnableTessellation"))
	float TessellationDistance = 10000;

	// Increases the chunks bounding boxes, useful when using tessellation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Tessellation", meta = (RecreateRender, EditCondition ="bEnableTessellation"))
	float TessellationBoundsExtension = 0;

	// If true, will use the voxel material alpha as hardness if the material config is RGB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Hardness")
	bool bUseAlphaAsHardness = false;

	// Material Index -> Hardness, for Single/Double index
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Materials|Hardness")
	TMap<FString, float> MaterialsHardness;

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
	// In SingleIndex, will interpolate DataA and DataB
	// In DoubleIndex, will interpolate Blend and Data
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

	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Rendering", meta = (RecreateRender))
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int32 ChunksCullingLOD = MAX_WORLD_DEPTH - 1;

	// Whether to render the world, or to just use it for collisions/navmesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (Recreate))
	bool bRenderWorld = true;

	// Will destroy any intermediate render data to free up memory
	// Does not support any kind of updates
	// Note: if MergeChunks is true, chunk meshes memory won't be cleared as it can't know if a new mesh will be added to the cluster
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender))
	bool bStaticWorld = false;
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Rendering", meta = (RecreateRender, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition = "bGenerateDistanceFields"))
	int32 MaxDistanceFieldLOD = 1;
	
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
	
	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Spawners", meta = (Recreate))
	UVoxelSpawnerConfig* SpawnerConfig;

	// The number of instances to put in each hierarchical instanced mesh before creating a new one
	// Lower = higher draw calls/object count
	// Higher = more delay when building the occlusion tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Spawners", meta = (Recreate))
	int32 NumberOfInstancesPerHISM = 100000;

	// Only nearby instances have collisions
	// Configure the distance using this
	// In voxels!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Spawners", meta = (Recreate, ClampMin = 0))
	int32 SpawnersCollisionDistanceInVoxel = 64;

	//////////////////////////////////////////////////////////////////////////////

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
	TEnumAsByte<ECanBeCharacterBase> CanCharacterStepUpOn = ECanBeCharacterBase(1); // ECB_Yes
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Collisions|Visible Chunks", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition = "bComputeVisibleChunksCollisions && bEnableCollisions"))
	int32 VisibleChunksCollisionsMaxLOD = 5;
	
	/**	Allows you to override the PhysicalMaterial to use for simple collision on this body. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	UPhysicalMaterial* PhysMaterialOverride = nullptr;
	
	/** If true Continuous Collision Detection (CCD) will be used for this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Collisions", meta = (Recreate, EditCondition = bEnableCollisions))
	bool bUseCCD = false;

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
	
	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (RecreateRender))
	bool bEnableNavmesh = false;

	// If false, use only invokers navmesh settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (UpdateLODs, EditCondition = bEnableNavmesh))
	bool bComputeVisibleChunksNavmesh = true;

	// Max LOD to compute navmesh on. Inclusive. Does not affect invokers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Navmesh", meta = (UpdateLODs, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition = "bEnableNavmesh && bComputeVisibleChunksNavmesh"))
	int32 VisibleChunksNavmeshMaxLOD = 0;
	
	//////////////////////////////////////////////////////////////////////////////

	// If you have more than one voxel world, set this to false and call CreateGlobalVoxelThreadPool at BeginPlay (for instance in your level blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Performance")
	bool bCreateGlobalPool = true;

	// Number of threads allocated for the voxel background processing. Setting it too high may impact performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Performance", meta = (Recreate, ClampMin = 1, EditCondition = "bCreateGlobalPool"))
	int32 NumberOfThreads = 2;

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

	// If true, won't recompute task priorities once they are queued
	// If false, will recompute task priorities with the new voxel invoker positions every PriorityDuration seconds
	// True: useful if you have many tasks
	// False: useful if you want precise task scheduling, eg if you are moving relatively fast
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (Recreate, EditCondition = "bCreateGlobalPool"))
	bool bConstantPriorities = false;

	// Only used if ConstantPriorities is false
	// Time, in seconds, during which a task priority is valid and does not need to be recomputed
	// Lowering this will increase async cost to recompute priorities, but will lead to more precise scheduling
	// Increasing this will decreasing async cost to recompute priorities, but might lead to imprecise scheduling if the invokers are moving fast
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (RecreateRender, ClampMin = 0, EditCondition = "!bConstantPriorities"))
	float PriorityDuration = 0.5;

	// Max time in milliseconds to spend on mesh updates per tick
	// If this is too low world will generate very slowly
	// If this is too high you will get lag spikes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (RecreateRender, ClampMin = 0.001))
	float MeshUpdatesBudget = 1000;

	// The rate at which generation events are fired (number of updates per seconds). Used for foliage spawning, foliage collision, multiplayer, binded BP events...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (RecreateRender, UIMin = 1, UIMax = 60))
	float GenerationEventsTickRate = 15;
	
	// Depth to which to subdivide the data octree on start
	// Will create 8^X nodes, so keep low!
	// Useful to avoid update tasks locking the entire octree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Performance", meta = (Recreate, ClampMin = 0))
	int32 DataOctreeInitialSubdivisionDepth = 4;

	//////////////////////////////////////////////////////////////////////////////
	
	// Is this world synchronized using the plugin multiplayer system?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Multiplayer", meta = (Recreate))
	bool bEnableMultiplayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel - Multiplayer", meta = (Recreate, EditCondition = "bMultiplayer"))
	TSubclassOf<UVoxelMultiplayerInterface> MultiplayerInterface;
	
	// Number of sync per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel - Multiplayer", meta = (Recreate, EditCondition = "bMultiplayer"))
	float MultiplayerSyncRate = 15;

public:
	AVoxelWorld();
	~AVoxelWorld();

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void CreateWorld();
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void DestroyWorld();

public:
	// Will use this data in CreateWorld if it's valid
	TVoxelSharedPtr<FVoxelData> PendingData;
	
public:
	IVoxelPool& GetPool() const { return *Pool; }
	FVoxelData& GetData() const { return *Data; }
	IVoxelLODManager& GetLODManager() const { return *LODManager; }
	IVoxelRenderer& GetRenderer() const { return *Renderer; }
	FVoxelDebugManager& GetDebugManager() const { return *DebugManager; }
	FVoxelProcGenManager& GetProcGenManager() const { return *ProcGenManager; }
	FVoxelToolRenderingManager& GetToolRenderingManager() const { return *ToolRenderingManager; }

	const TVoxelSharedPtr<FVoxelData>& GetDataSharedPtr() const { return Data; }
	const TVoxelSharedPtr<IVoxelLODManager>& GetLODManagerSharedPtr() const { return LODManager; }
	const TVoxelSharedPtr<IVoxelPool>& GetPoolSharedPtr() const { return Pool; }
	const TVoxelSharedRef<FIntVector>& GetWorldOffsetPtr() const { return WorldOffset; }
	const TVoxelSharedRef<FVoxelRendererDynamicSettings>& GetRendererDynamicSettings() const { return RendererDynamicSettings; }
	EVoxelPlayType GetPlayType() const { return PlayType; }
	
	FVoxelWorldGeneratorInit GetInitStruct() const;
	FIntBox GetWorldBounds() const;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass);

	// Set the octree depth: between 1 and 25
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetOctreeDepth(int32 NewDepth);

public:
	// Is this world created?
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	inline bool IsCreated() const { return bIsCreated; }

public:
	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @param	Rounding	How to round. For cubic will always be round down
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|General|Coordinates", meta = (DisplayName = "World Position to Voxel", AdvancedDisplay = "Rounding"))
	virtual FIntVector GlobalToLocal(const FVector& Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const override final;
	// Precision errors with large world offset
	UFUNCTION(BlueprintCallable, Category = "Voxel|General|Coordinates", meta = (DisplayName = "World Position to Voxel Float"))
	virtual FVector GlobalToLocalFloat(const FVector& Position) const override final;

	/**
	 * Convert position from voxel space to world space
	 * @param	Position	Position in voxel space
	 * @return	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|General|Coordinates", meta = (DisplayName = "Voxel Position to World"))
	virtual FVector LocalToGlobal(const FIntVector& Position) const override final;
	// Precision errors with large world offset
	UFUNCTION(BlueprintCallable, Category = "Voxel|General|Coordinates", meta = (DisplayName = "Voxel Position to World Float"))
	virtual FVector LocalToGlobalFloat(const FVector& Position) const override final;

	/**
	 * Get the 8 neighbors in voxel space of GlobalPosition
	 * @param	GlobalPosition	The position in world space
	 * @return	The 8 neighbors in voxel space 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|General|Coordinates")
	TArray<FIntVector> GetNeighboringPositions(const FVector& GlobalPosition) const;
	
	/**
	 * Set the voxel world voxel offset
	 * @param	OffsetInVoxels	Offset
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetOffset(const FIntVector& OffsetInVoxels);
	
	/**
	 * Add an offset to the world coordinate system (eg for rebasing)
	 * @param	OffsetInVoxels	Offset
	 * @param	bMoveActor		If false, the actor will keep its current position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor = true);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer")
	UVoxelMultiplayerInterface* CreateMultiplayerInterfaceInstance();
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Multiplayer")
	UVoxelMultiplayerInterface* GetMultiplayerInterfaceInstance() const;

public:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void UnregisterAllComponents(bool bForReregister = false) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif // WITH_EDITOR
	//~ End AActor Interface

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PreEditChange(UProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface

	void UpdateCollisionProfile();

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	AActor* VoxelWorldEditor;
#endif

	UPROPERTY(Transient)
	mutable UVoxelMultiplayerInterface* MultiplayerInterfaceInstance;
	
	UPROPERTY()
	bool bIsToggled = false;

	bool bIsCreated = false;
	EVoxelPlayType PlayType = EVoxelPlayType::Game;
	double TimeOfCreation = 0;

	// Temporary variable set in PreEditChange to avoid re-registering proc meshes
	bool bDisableComponentUnregister = false;

private:	
	TVoxelSharedPtr<FVoxelDebugManager> DebugManager;
	TVoxelSharedPtr<FVoxelData> Data;
	TVoxelSharedPtr<IVoxelPool> Pool;
	TVoxelSharedPtr<IVoxelRenderer> Renderer;
	TVoxelSharedPtr<IVoxelLODManager> LODManager;
	TVoxelSharedPtr<FVoxelProcGenManager> ProcGenManager;
	TVoxelSharedPtr<FVoxelToolRenderingManager> ToolRenderingManager;

	TVoxelSharedRef<FIntVector> WorldOffset = MakeVoxelShared<FIntVector>(FIntVector::ZeroValue);
	TVoxelSharedRef<FVoxelLODDynamicSettings> LODDynamicSettings = TVoxelSharedPtr<FVoxelLODDynamicSettings>().ToSharedRef(); // else the VTABLE constructor doesn't compile...
	TVoxelSharedRef<FVoxelRendererDynamicSettings> RendererDynamicSettings = TVoxelSharedPtr<FVoxelRendererDynamicSettings>().ToSharedRef();

private:
	void OnWorldLoadedCallback();

	TVoxelSharedRef<FVoxelDebugManager> CreateDebugManager() const;
	TVoxelSharedRef<FVoxelData> CreateData() const;
	TVoxelSharedRef<IVoxelPool> CreatePool() const;
	TVoxelSharedRef<IVoxelRenderer> CreateRenderer() const;
	TVoxelSharedRef<IVoxelLODManager> CreateLODManager() const;
	TVoxelSharedPtr<FVoxelProcGenManager> CreateProcGenManager() const;
	TVoxelSharedPtr<FVoxelToolRenderingManager> CreateToolRenderingManager() const;

	void CreateWorldInternal();
	void DestroyWorldInternal();
	void DestroyVoxelComponents();

public:
	void LoadFromSaveObject();
	void ApplyPlaceableItems();
	void UpdateDynamicLODSettings() const;
	void UpdateDynamicRendererSettings() const;

#if WITH_EDITOR
	FSimpleMulticastDelegate OnPropertyChanged;
	FSimpleMulticastDelegate OnPropertyChanged_Interactive;

	void Toggle();
	void CreateInEditor();
	void SaveData();
	void LoadFromSaveObjectEditor();
	bool SaveToFile(const FString& Path, FText& Error);
	bool LoadFromFile(const FString& Path, FText& Error);
	FString GetDefaultFilePath() const;
	
	//~ Begin IVoxelEditorDelegatesInterface Interface
	virtual void OnPreSaveWorld(uint32 SaveFlags, UWorld* World) override;
	virtual void OnPreBeginPIE(bool bIsSimulating) override;
	virtual void OnEndPIE(bool bIsSimulating) override;
	virtual void OnPrepareToCleanseEditorObject(UObject* Object) override;
	virtual void OnPreExit() override;
	//~ End IVoxelEditorDelegatesInterface Interface
#endif
};

#if CPP // UHT is dumb
#if WITH_EDITOR
class VOXEL_API IVoxelWorldEditor
{
public:
	virtual ~IVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() = 0;
	virtual UClass* GetVoxelWorldEditorClass() = 0;

public:
	// Sets the voxel world editor implementation.*
	static void SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor);
	inline static IVoxelWorldEditor* GetVoxelWorldEditor() { return VoxelWorldEditor.Get(); }

private:
	// Ptr to interface to voxel editor operations.
	static TSharedPtr<IVoxelWorldEditor> VoxelWorldEditor;
};
#endif
#endif