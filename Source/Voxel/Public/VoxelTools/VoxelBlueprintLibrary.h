// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "VoxelIntBox.h"
#include "VoxelPaintMaterial.h"
#include "VoxelTexture.h"
#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

#include "VoxelBlueprintLibrary.generated.h"

enum class EVoxelTaskType : uint8;
class UVoxelHierarchicalInstancedStaticMeshComponent;
class AVoxelSpawnerActor;
class AVoxelWorld;
class APlayerState;
class AController;
class UStaticMesh;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FVoxelOnChunkGenerationDynamicDelegate, AVoxelWorld*, World, FVoxelIntBox, Bounds);
DECLARE_DYNAMIC_DELEGATE_OneParam(FChunkDynamicDelegate, FVoxelIntBox, Bounds);

USTRUCT(BlueprintType)
struct FVoxelToolRenderingRef
{
	GENERATED_BODY()

	FVoxelToolRenderingId Id;
};

UENUM(BlueprintType)
enum class EVoxelMemoryUsageType : uint8
{
	Total,
	VoxelsDirtyValuesData,
	VoxelsDirtyMaterialsData,
	VoxelsCachedValuesData,
	VoxelsCachedMaterialsData,
	UndoRedo,
	Multiplayer,
	IntermediateBuffers,
	MeshesIndices,
	MeshesTessellationIndices,
	MeshesVertices,
	MeshesColors,
	MeshesUVsAndTangents,
	DataAssets,
	HeightmapAssets,
	UncompressedSaves,
	CompressedSaves
};

UCLASS()
class VOXEL_API UVoxelBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static bool IsVoxelPluginPro();
	
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (DefaultToSelf = "Object", AdvancedDisplay = "Object"))
	static void RaiseInfo(FString Message, UObject* Object = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (DefaultToSelf = "Object", AdvancedDisplay = "Object"))
	static void RaiseWarning(FString Message, UObject* Object = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (DefaultToSelf = "Object", AdvancedDisplay = "Object"))
	static void RaiseError(FString Message, UObject* Object = nullptr);

	// Returns the number of cores the CPU has. Ignores hyperthreading unless -usehyperthreading is passed as a command line argument.
	UFUNCTION(BlueprintPure, Category = "Voxel")
	static int32 NumberOfCores();
	
public:
	// Get the current memory usage of different parts of the plugin
	UFUNCTION(BlueprintPure, Category = "Voxel|Memory")
	static float GetMemoryUsageInMB(EVoxelMemoryUsageType Type);
	
	// Get the peak memory usage of different parts of the plugin
	UFUNCTION(BlueprintPure, Category = "Voxel|Memory")
	static float GetPeakMemoryUsageInMB(EVoxelMemoryUsageType Type);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Memory")
	static void LogMemoryStats();
	
	// Returns the memory used by positions and indices buffers in this voxel world
	// Should give a rough estimate of how much memory is used for physics
	UFUNCTION(BlueprintCallable, Category = "Voxel|Memory", meta = (DefaultToSelf = "World"))
	static float GetEstimatedCollisionsMemoryUsageInMB(AVoxelWorld* World);

public:
	/**
	 * Transform a box in global space to voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (DefaultToSelf = "World"))
	static FVoxelIntBox TransformGlobalBoxToVoxelBox(AVoxelWorld* World, FBox Box);
	
	/**
	 * Transform a box in voxel space to global space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (DefaultToSelf = "World"))
	static FBox TransformVoxelBoxToGlobalBox(AVoxelWorld* World, FVoxelIntBox Box);

public:
	/**
	 * Iterate all voxel worlds in the scene, and return the first one that contains Position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (WorldContext = "WorldContextObject"))
	static AVoxelWorld* GetVoxelWorldContainingPosition(UObject* WorldContextObject, FVector Position);	
	/**
	 * Iterate all voxel worlds in the scene, and return all the ones that contains Position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (WorldContext = "WorldContextObject"))
	static TArray<AVoxelWorld*> GetAllVoxelWorldsContainingPosition(UObject* WorldContextObject, FVector Position);
	
	/**
	 * Iterate all voxel worlds in the scene, and return the first one that overlaps Box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (WorldContext = "WorldContextObject"))
	static AVoxelWorld* GetVoxelWorldOverlappingBox(UObject* WorldContextObject, FBox Box);	
	/**
	 * Iterate all voxel worlds in the scene, and return all the ones that overlaps Box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (WorldContext = "WorldContextObject"))
	static TArray<AVoxelWorld*> GetAllVoxelWorldsOverlappingBox(UObject* WorldContextObject, FBox Box);
	
	/**
	 * Iterate all voxel worlds in the scene, and return the first one that overlaps the actor bounding box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (DefaultToSelf = "Actor"))
	static AVoxelWorld* GetVoxelWorldOverlappingActor(AActor* Actor);	
	/**
	 * Iterate all voxel worlds in the scene, and return all the ones that overlaps the actor bounding box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Helpers", meta = (DefaultToSelf = "Actor"))
	static TArray<AVoxelWorld*> GetAllVoxelWorldsOverlappingActor(AActor* Actor);
	
public:
	/**
	 * FVoxelInstancedMeshManager helpers
	 */
	
public:
	// Will replace instanced static mesh instances by actors
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void SpawnVoxelSpawnerActorsInArea(
		TArray<AVoxelSpawnerActor*>& OutActors, 
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		EVoxelSpawnerActorSpawnType SpawnType = EVoxelSpawnerActorSpawnType::OnlyFloating);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static AVoxelSpawnerActor* SpawnVoxelSpawnerActorByInstanceIndex(
		AVoxelWorld* World,
		UVoxelHierarchicalInstancedStaticMeshComponent* Component,
		int32 InstanceIndex);

	/**
	 * Add instances to a voxel world foliage system
	 * @param World						The voxel world
	 * @param Mesh						The mesh to use
	 * @param Transforms				The transforms, relative to the voxel world (but not in voxel space!)
	 * @param Colors					The colors to send to the instance material (use GetVoxelMaterialFromPerInstanceRandom to get it)
	 * @param FloatingDetectionOffset	Increase this if your foliage is enabling physics too soon
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World", AdvancedDisplay = "FloatingDetectionOffset"))
	static void AddInstances(
		AVoxelWorld* World,
		UStaticMesh* Mesh,
		const TArray<FTransform>& Transforms,
		const TArray<FLinearColor>& Colors,
		FVoxelInstancedMeshSettings InstanceSettings,
		FVoxelSpawnerActorSettings ActorSettings,
		FVector FloatingDetectionOffset = FVector(0, 0, -10));
	
public:
	/**
	 * IVoxelSpawnerManager helpers
	 */

	// Regenerate spawners in an aera
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void RegenerateSpawners(AVoxelWorld* World, FVoxelIntBox Bounds);

	// Mark spawners as dirty so that they don't get trash if they go out of scope
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void MarkSpawnersDirty(AVoxelWorld* World, FVoxelIntBox Bounds);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static FVoxelSpawnersSave GetSpawnersSave(AVoxelWorld* World);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void LoadFromSpawnersSave(AVoxelWorld* World, const FVoxelSpawnersSave& Save);

public:
	/**
	 * FVoxelData helpers
	 */
	
public:
	// Undo last frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	// @return Success
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static bool Undo(AVoxelWorld* World, TArray<FVoxelIntBox>& OutUpdatedBounds);
	static bool Undo(AVoxelWorld* World);
	// Redo last undone frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	// @return Success
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static bool Redo(AVoxelWorld* World, TArray<FVoxelIntBox>& OutUpdatedBounds);
	static bool Redo(AVoxelWorld* World);
	// Save the edits since the last call to SaveFrame/Undo/Redo and clear the redo stack. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static void SaveFrame(AVoxelWorld* World);
	// Clear all the frames. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static void ClearFrames(AVoxelWorld* World);
	// Get the current history position
	UFUNCTION(BlueprintPure, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static int32 GetHistoryPosition(AVoxelWorld* World);

	// Get the normal at Position using the density gradient. May differ from the mesh normal
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (Keywords = "gradient", DefaultToSelf = "World"))
	static FVector GetNormal(AVoxelWorld* World, FIntVector Position);
	
	// Get a custom float output value
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static float GetFloatOutput(AVoxelWorld* World, FName Name, float X, float Y, float Z, float DefaultValue);
	
	// Get a custom int32 output value
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static int32 GetIntOutput(AVoxelWorld* World, FName Name, float X, float Y, float Z, int32 DefaultValue);

	// Bounds of this world
	UFUNCTION(BlueprintPure, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static FVoxelIntBox GetBounds(AVoxelWorld* World);

	// TODO: delegate to chunk creation to setup material
	// Will cleanup the code there as well, as no need to manually call world gen thingy

	// Clear all the data in the world, including items/assets
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static void ClearAllData(AVoxelWorld* World, bool bUpdateRender = true);

	// Clear all the value data in the world
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static void ClearValueData(AVoxelWorld* World, bool bUpdateRender = true);

	// Clear all the material data in the world
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static void ClearMaterialData(AVoxelWorld* World, bool bUpdateRender = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static bool HasValueData(AVoxelWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static bool HasMaterialData(AVoxelWorld* World);
	
	// Clear all the edited data in the world, excluding items/assets
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bUpdateRender"))
	static void ClearDirtyData(AVoxelWorld* World, bool bUpdateRender = true);

	// Scale the voxel world data. Will recreate the voxel world!
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void ScaleData(AVoxelWorld* World, const FVector& Scale);
	
public:
	/**
	 * IVoxelLODManager helpers
	 */
	
public:
	// Update the chunks overlapping Position
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void UpdatePosition(AVoxelWorld* World, FIntVector Position);
	// Update the chunks overlapping Bounds. 
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void UpdateBounds(AVoxelWorld* World, FVoxelIntBox Bounds);
	// Update all the chunks
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void UpdateAll(AVoxelWorld* World);
	// Call this after changing the voxel world LODs setting while created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World", DisplayName = "Apply LOD Settings"))
	static void ApplyLODSettings(AVoxelWorld* World);
	/**
	 * Returns whether voxel collisions are enabled at Position 
	 * @param World						The voxel world
	 * @param Position					The position to query, in world space if ConvertToVoxelSpace is true
	 * @param LOD						The LOD at that position
	 * @param bConvertToVoxelSpace		If true, the position will be converted to voxel space. Else it will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static bool AreCollisionsEnabled(AVoxelWorld* World, FVector Position, int32& LOD, bool bConvertToVoxelSpace = true);

public:
	/**
	 * IVoxelRenderer helpers
	 */
	
public:
	// Number of mesh processing tasks not finished
	UFUNCTION(BlueprintPure, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static int32 GetTaskCount(AVoxelWorld* World);

	// Returns true if there are still mesh tasks queued
	UFUNCTION(BlueprintPure, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static bool IsVoxelWorldMeshLoading(AVoxelWorld* World);

	// Returns true if there are still foliage tasks queued
	UFUNCTION(BlueprintPure, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static bool IsVoxelWorldFoliageLoading(AVoxelWorld* World);
	
	// Call this after changing a voxel world VoxelMaterial/MaterialCollection to apply it
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void ApplyNewMaterials(AVoxelWorld* World);

	// Call this to recreate the voxel world rendering entirely, keeping data intact
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void RecreateRender(AVoxelWorld* World);

	// Call this to recreate the voxel world spawners
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void RecreateSpawners(AVoxelWorld* World);

	// Call this to recreate the voxel world entirely, optionally keeping data intact by saving it
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (DefaultToSelf = "World"))
	static void Recreate(AVoxelWorld* World, bool bSaveData = true);
	
public:
	/**
	 * FVoxelEventManager helpers
	 */
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Proc Gen", meta = (DefaultToSelf = "World", AdvancedDisplay = "bFireExistingOnes"))
	static void BindVoxelChunkEvents(
		AVoxelWorld* World, 
		FChunkDynamicDelegate OnActivate,
		FChunkDynamicDelegate OnDeactivate,
		bool bFireExistingOnes = false,
		int32 ChunkSize = 32, 
		int32 ActivationDistanceInChunks = 2);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Proc Gen", meta = (DefaultToSelf = "World", AdvancedDisplay = "bFireExistingOnes"))
	static void BindVoxelGenerationEvent(
		AVoxelWorld* World, 
		FChunkDynamicDelegate OnGenerate,
		bool bFireExistingOnes = false,
		int32 ChunkSize = 32, 
		int32 GenerationDistanceInChunks = 2);

public:
	/**
	 * FVoxelToolRenderingManager helpers
	 */
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static bool IsValidRef(AVoxelWorld* World, FVoxelToolRenderingRef Ref);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static FVoxelToolRenderingRef CreateToolRendering(AVoxelWorld* World);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static void DestroyToolRendering(AVoxelWorld* World, FVoxelToolRenderingRef Ref);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static void SetToolRenderingMaterial(AVoxelWorld* World, FVoxelToolRenderingRef Ref, UMaterialInterface* Material);

	// Bounds: In world space
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static void SetToolRenderingBounds(AVoxelWorld* World, FVoxelToolRenderingRef Ref, FBox Bounds);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tool Rendering", meta = (DefaultToSelf = "World"))
	static void SetToolRenderingEnabled(AVoxelWorld* World, FVoxelToolRenderingRef Ref, bool bEnabled = true);

public:
	/**
	 * IVoxelPool helpers
	 */
	
public:
	/**
	 * Create the global voxel thread pool. Must not be already created.
	 * CreateWorldVoxelThreadPool is preferred, as pools will be per level
	 * @param	NumberOfThreads		At least 1
	 * @param	bConstantPriorities	If true won't recompute the tasks priorities once added. Useful if you have many tasks, but will give bad task scheduling when moving fast
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads", meta = (AdvancedDisplay = "PriorityCategoriesOverrides, PriorityOffsetsOverrides"))
	static void CreateGlobalVoxelThreadPool(
		const TMap<EVoxelTaskType, int32>& PriorityCategoriesOverrides,
		const TMap<EVoxelTaskType, int32>& PriorityOffsetsOverrides,
		int32 NumberOfThreads = 2,
		bool bConstantPriorities = false);

	// Destroy the global voxel thread pool
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads")
	static void DestroyGlobalVoxelThreadPool();

	// Is the global voxel thread pool created?
	UFUNCTION(BlueprintPure, Category = "Voxel|Threads")
	static bool IsGlobalVoxelPoolCreated();
	
	/**
	 * Create the voxel thread pool for a specific world. Must not be already created.
	 * @param	NumberOfThreads		At least 1
	 * @param	bConstantPriorities	If true won't recompute the tasks priorities once added. Useful if you have many tasks, but will give bad task scheduling when moving fast
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads", meta = (AdvancedDisplay = "PriorityCategoriesOverrides, PriorityOffsetsOverrides"))
	static void CreateWorldVoxelThreadPool(
		UWorld* World,
		const TMap<EVoxelTaskType, int32>& PriorityCategoriesOverrides,
		const TMap<EVoxelTaskType, int32>& PriorityOffsetsOverrides,
		int32 NumberOfThreads = 2,
		bool bConstantPriorities = false);

	// Destroy the world voxel thread pool
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads")
	static void DestroyWorldVoxelThreadPool(UWorld* World);

	// Is the global voxel thread pool created?
	UFUNCTION(BlueprintPure, Category = "Voxel|Threads")
	static bool IsWorldVoxelPoolCreated(UWorld* World);

public:
	/**
	 * FVoxelIntBox helpers
	 */
	
public:
	/**
	 * Make IntBox from global position and radius
	 * @param	Radius	in cm
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Utilities", meta = (DefaultToSelf = "World"))
	static FVoxelIntBox MakeIntBoxFromGlobalPositionAndRadius(AVoxelWorld* World, FVector GlobalPosition, float Radius);

	// eg if you want to cache all the data that's going to be used by render chunks when updating the world
	UFUNCTION(BlueprintPure, Category = "Voxel|Utilities", meta = (DefaultToSelf = "World"))
	static FVoxelIntBox GetRenderBoundsOverlappingDataBounds(AVoxelWorld* World, FVoxelIntBox DataBounds, int32 LOD = 0);

public:
	/**
	 * FIntVector helpers
	 */
	
public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector + IntVector", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|IntVector")
	static FIntVector Add_IntVectorIntVector(FIntVector Left, FIntVector Right)
	{
		return Left + Right;
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector - IntVector", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|IntVector")
	static FIntVector Substract_IntVectorIntVector(FIntVector Left, FIntVector Right)
	{
		return Left - Right;
	}
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorIntVector(FIntVector Left, FIntVector Right)
	{
		return FIntVector(Left.X * Right.X, Left.Y * Right.Y, Left.Z * Right.Z);
	}

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector / int", CompactNodeTitle = "/", Keywords = "/ divide"), Category = "Math|IntVector")
	static FIntVector Divide_IntVectorInt(FIntVector Left, int32 Right)
	{
		return Left / Right;
	}
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * int", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorInt(FIntVector Left, int32 Right)
	{
		return Left * Right;
	}
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "int * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntIntVector(int32 Left, FIntVector Right)
	{
		return Right * Left;
	}
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Max"), Category = "Math|IntVector")
	static int32 GetMax_Intvector(FIntVector Vector)
	{
		return Vector.GetMax();
	}
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Min"), Category = "Math|IntVector")
	static int32 GetMin_Intvector(FIntVector Vector)
	{
		return Vector.GetMin();
	}
		
public:
	/**
	 * FVoxelPaintMaterial helpers
	 */

public:
	 // Create from color
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials", DisplayName = "Create RGB Paint Material")
	static FVoxelPaintMaterial CreateColorPaintMaterial(FVoxelPaintMaterialColor Color)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::Color;
		PaintMaterial.Color = Color;
		return PaintMaterial;
	}
	
	/**
	 * Create paint material for 5 way blend
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateFiveWayBlendPaintMaterial(FVoxelPaintMaterialFiveWayBlend FiveWayBlend);

	// Create for single index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateSingleIndexPaintMaterial(FVoxelPaintMaterialSingleIndex SingleIndex)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::SingleIndex;
		PaintMaterial.SingleIndex = SingleIndex;
		return PaintMaterial;
	}

	// Create for multi index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateMultiIndexPaintMaterial(FVoxelPaintMaterialMultiIndex MultiIndex)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::MultiIndex;
		PaintMaterial.MultiIndex = MultiIndex;
		return PaintMaterial;
	}

	// Create for multi index wetness
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateMultiIndexWetnessPaintMaterial(FVoxelPaintMaterialMultiIndexWetness MultiIndexWetness)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::MultiIndexWetness;
		PaintMaterial.MultiIndexWetness = MultiIndexWetness;
		return PaintMaterial;
	}

	// Create for multi index, setting the data directly
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateMultiIndexRawPaintMaterial(FVoxelPaintMaterialMultiIndexRaw MultiIndexRaw)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::MultiIndexRaw;
		PaintMaterial.MultiIndexRaw = MultiIndexRaw;
		return PaintMaterial;
	}

	// Create UV paint
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials", DisplayName = "Create UV Paint Material")
	static FVoxelPaintMaterial CreateUVPaintMaterial(FVoxelPaintMaterialUV UV)
	{
		FVoxelPaintMaterial PaintMaterial;
		PaintMaterial.Type = EVoxelPaintMaterialType::UV;
		PaintMaterial.UV = UV;
		return PaintMaterial;
	}

	// Apply a Paint Material to a Voxel Material
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial ApplyPaintMaterial(FVoxelMaterial Material, FVoxelPaintMaterial PaintMaterial, float Strength = 1.f)
	{
		PaintMaterial.ApplyToMaterial(Material, Strength);
		return Material;
	}

public:
	/**
	 * FVoxelMaterial helpers
	 */
	
public:
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FLinearColor GetColor(FVoxelMaterial Material)
	{
		return Material.GetLinearColor();
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetSingleIndex(FVoxelMaterial Material)
	{
		return Material.GetSingleIndex();
	}
	// If SortByStrength is true, Index 0 will have the highest strength, Index 1 the second highest etc
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static void GetMultiIndex(
		FVoxelMaterial Material, 
		bool bSortByStrength,
		float& Strength0, uint8& Index0,
		float& Strength1, uint8& Index1,
		float& Strength2, uint8& Index2,
		float& Strength3, uint8& Index3,
		float& Wetness);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVector2D GetUV(FVoxelMaterial Material, int32 Channel)
	{
		return Material.GetUV_AsFloat(Channel);
	}

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static void GetRawMaterial(
		FVoxelMaterial Material,
		uint8& R, uint8& G, uint8& B, uint8& A,
		uint8& U0, uint8& V0,
		uint8& U1, uint8& V1,
		uint8& U2, uint8& V2,
		uint8& U3, uint8& V3)
	{
		R = Material.Impl_GetR();
		G = Material.Impl_GetG();
		B = Material.Impl_GetB();
		A = Material.Impl_GetA();
		U0 = Material.Impl_GetU0();
		V0 = Material.Impl_GetV0();
		U1 = Material.Impl_GetU1();
		V1 = Material.Impl_GetV1();
		U2 = Material.Impl_GetU2();
		V2 = Material.Impl_GetV2();
		U3 = Material.Impl_GetU3();
		V3 = Material.Impl_GetV3();
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial MakeRawMaterial(
		uint8 R, uint8 G, uint8 B, uint8 A,
		uint8 U0, uint8 V0,
		uint8 U1, uint8 V1,
		uint8 U2, uint8 V2,
		uint8 U3, uint8 V3)
	{
		FVoxelMaterial Material{ ForceInit };
		Material.Impl_SetR(R);
		Material.Impl_SetG(G);
		Material.Impl_SetB(B);
		Material.Impl_SetA(A);
		Material.Impl_SetU0(U0);
		Material.Impl_SetV0(V0);
		Material.Impl_SetU1(U1);
		Material.Impl_SetV1(V1);
		Material.Impl_SetU2(U2);
		Material.Impl_SetV2(V2);
		Material.Impl_SetU3(U3);
		Material.Impl_SetV3(V3);
		return Material;
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static int32 MakeMaterialMask(
		bool R, bool G, bool B, bool A,
		bool U0, bool V0,
		bool U1, bool V1,
		bool U2, bool V2,
		bool U3, bool V3)
	{
		return
			EVoxelMaterialMask::R * R |
			EVoxelMaterialMask::G * G |
			EVoxelMaterialMask::B * B |
			EVoxelMaterialMask::A * A |
			EVoxelMaterialMask::U0 * U0 |
			EVoxelMaterialMask::U1 * U1 |
			EVoxelMaterialMask::U2 * U2 |
			EVoxelMaterialMask::U3 * U3 |
			EVoxelMaterialMask::V0 * V0 |
			EVoxelMaterialMask::V1 * V1 |
			EVoxelMaterialMask::V2 * V2 |
			EVoxelMaterialMask::V3 * V3;
	}

public:
	/**
	 * FVoxelTexture helpers
	 */
	
public:
	/**
	 * Will create Texture if null, and set it
	 * Returns Texture for convenience
	 *
	 * Texture will have the following config:
	 * Pixel format: PF_R32_FLOAT
	 * Compression settings: TC_HDR
	 * SRGB: false
	 * Filter: TF_Bilinear
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateOrUpdateTextureFromVoxelFloatTexture(FVoxelFloatTexture VoxelTexture, UPARAM(ref) UTexture2D*& Texture)
	{
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	/**
	 * Same as CreateOrUpdateTextureFromVoxelFloatTexture with nullptr in input
	 *
	 * Texture will have the following config:
	 * Pixel format: PF_R32_FLOAT
	 * Compression settings: TC_HDR
	 * SRGB: false
	 * Filter: TF_Bilinear
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateTextureFromVoxelFloatTexture(FVoxelFloatTexture VoxelTexture)
	{
		UTexture2D* Texture = nullptr;
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	/**
	 * Creates a voxel float texture from the color channel of a texture
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static FVoxelFloatTexture CreateVoxelFloatTextureFromTextureChannel(UTexture2D* Texture, EVoxelRGBA Channel)
	{
		return { FVoxelTextureUtilities::CreateFromTexture_Float(Texture, Channel) };
	}
	
public:
	/**
	 * Will create Texture if null, and set it
	 * Returns Texture for convenience
	 *
	 * Texture will have the following config:
	 * Pixel format: PF_B8G8R8A8
	 * Compression settings: TC_VectorDisplacementmap
	 * SRGB: false
	 * Filter: TF_Bilinear
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateOrUpdateTextureFromVoxelColorTexture(FVoxelColorTexture VoxelTexture, UPARAM(ref) UTexture2D*& Texture)
	{
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	/**
	 * Same as CreateOrUpdateTextureFromVoxelFloatTexture with nullptr in input
	 *
	 * Texture will have the following config:
	 * Pixel format: PF_B8G8R8A8
	 * Compression settings: TC_VectorDisplacementmap
	 * SRGB: false
	 * Filter: TF_Bilinear
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateTextureFromVoxelColorTexture(FVoxelColorTexture VoxelTexture)
	{
		UTexture2D* Texture = nullptr;
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	/**
	 * Creates a voxel color texture by putting a float texture into a specific channel
	 * @param bNormalize	If true, the float texture min value will be mapped to 0, and max value to 1
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static FVoxelColorTexture CreateVoxelColorTextureFromVoxelFloatTexture(FVoxelFloatTexture Texture, EVoxelRGBA Channel, bool bNormalize = true)
	{
		return { FVoxelTextureUtilities::CreateColorTextureFromFloatTexture(Texture.Texture, Channel, bNormalize) };
	}

public:
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static FIntPoint GetVoxelFloatTextureSize(FVoxelFloatTexture Texture)
	{
		return { Texture.Texture.GetSizeX(), Texture.Texture.GetSizeY() };
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static FIntPoint GetVoxelColorTextureSize(FVoxelColorTexture Texture)
	{
		return { Texture.Texture.GetSizeX(), Texture.Texture.GetSizeY() };
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static bool IsVoxelFloatTextureValid(FVoxelFloatTexture Texture)
	{
		return FMath::Max(Texture.Texture.GetSizeX(), Texture.Texture.GetSizeY()) > 1;
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static bool IsVoxelColorTextureValid(FVoxelFloatTexture Texture)
	{
		return FMath::Max(Texture.Texture.GetSizeX(), Texture.Texture.GetSizeY()) > 1;
	}
	
public:
	/**
	 * General helpers
	 */
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Utilities")
	static void AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet);
};