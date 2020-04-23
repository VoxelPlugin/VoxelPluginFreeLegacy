// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "IntBox.h"
#include "VoxelPaintMaterial.h"
#include "VoxelTexture.h"
#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelRender/VoxelToolRendering.h"

#include "VoxelBlueprintLibrary.generated.h"

enum class EVoxelTaskType : uint8;
class UVoxelHierarchicalInstancedStaticMeshComponent;
class AVoxelSpawnerActor;
class AVoxelWorld;
class APlayerState;
class AController;
class UStaticMesh;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FVoxelOnChunkGenerationDynamicDelegate, AVoxelWorld*, World, FIntBox, Bounds);
DECLARE_DYNAMIC_DELEGATE_OneParam(FChunkDynamicDelegate, FIntBox, Bounds);

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
	VoxelsDirtyFoliageData,
	VoxelsCachedValuesData,
	VoxelsCachedMaterialsData,
	VoxelsCachedFoliageData,
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
	 * FVoxelInstancedMeshManager helpers
	 */
	
public:
	// Will replace instanced static mesh instances by actors
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void SpawnVoxelSpawnerActorsInArea(
		TArray<AVoxelSpawnerActor*>& OutActors, 
		AVoxelWorld* World,
		FIntBox Bounds,
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
	static void RegenerateSpawners(AVoxelWorld* World, FIntBox Bounds);

	// Mark spawners as dirty so that they don't get trash if they go out of scope
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void MarkSpawnersDirty(AVoxelWorld* World, FIntBox Bounds);

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
	static bool Undo(AVoxelWorld* World, TArray<FIntBox>& OutUpdatedBounds);
	static bool Undo(AVoxelWorld* World);
	// Redo last undone frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	// @return Success
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo", meta = (DefaultToSelf = "World"))
	static bool Redo(AVoxelWorld* World, TArray<FIntBox>& OutUpdatedBounds);
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
	static FIntBox GetBounds(AVoxelWorld* World);

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
	static void UpdateBounds(AVoxelWorld* World, FIntBox Bounds);
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
	 * @param	NumberOfThreads		At least 1
	 * @param	bConstantPriorities	If true won't recompute the tasks priorities once added. Useful if you have many tasks, but will give bad task scheduling when moving fast
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "PriorityCategoriesOverrides, PriorityOffsetsOverrides"))
	static void CreateGlobalVoxelThreadPool(
		UObject* WorldContextObject,
		const TMap<EVoxelTaskType, int32>& PriorityCategoriesOverrides,
		const TMap<EVoxelTaskType, int32>& PriorityOffsetsOverrides,
		int32 NumberOfThreads = 2,
		bool bConstantPriorities = false);

	// Destroy the global voxel thread pool
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads", meta = (WorldContext = "WorldContextObject"))
	static void DestroyGlobalVoxelThreadPool(UObject* WorldContextObject);

	// Is the global voxel thread pool created?
	UFUNCTION(BlueprintPure, Category = "Voxel|Threads", meta = (WorldContext = "WorldContextObject"))
	static bool IsGlobalVoxelPoolCreated(UObject* WorldContextObject);

public:
	/**
	 * FIntBox helpers
	 */
	
public:
	/**
	 * Make IntBox from global position and radius
	 * @param	Radius	in cm
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Utilities", meta = (DefaultToSelf = "World"))
	static FIntBox MakeIntBoxFromGlobalPositionAndRadius(AVoxelWorld* World, FVector GlobalPosition, float Radius);

	// eg if you want to cache all the data that's going to be used by render chunks when updating the world
	UFUNCTION(BlueprintPure, Category = "Voxel|Utilities", meta = (DefaultToSelf = "World"))
	static FIntBox GetRenderBoundsOverlappingDataBounds(AVoxelWorld* World, FIntBox DataBounds, int32 LOD = 0);

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
	static FVoxelPaintMaterial CreateRGBPaintMaterial(
		FLinearColor Color, 
		bool bPaintR = true, 
		bool bPaintG = true, 
		bool bPaintB = true, 
		bool bPaintA = true)
	{
		return FVoxelPaintMaterial::CreateRGB(Color, bPaintR, bPaintG, bPaintB, bPaintA);
	}
	
	/**
	 * Create paint material for 5 way blend
	 * @param	Channel		Between 0 and 4. 1,2,3,4 => R,G,B,A. 0 => material displayed by default
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials", DisplayName = "Create RGB Paint Material")
	static FVoxelPaintMaterial CreateFiveWayBlendPaintMaterial(
		int32 Channel,
		float TargetValue = 1.f,
		bool bPaintR = true,
		bool bPaintG = true,
		bool bPaintB = true,
		bool bPaintA = true);

	// Create from index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateSingleIndexPaintMaterial(uint8 Index)
	{
		return FVoxelPaintMaterial::CreateSingleIndex(Index);
	}

	// Create from double index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateDoubleIndexSetPaintMaterial(
		uint8 IndexA, 
		uint8 IndexB, 
		float Blend, 
		bool bSetIndexA = true,
		bool bSetIndexB = true, 
		bool bSetBlend = true)
	{
		return FVoxelPaintMaterial::CreateDoubleIndexSet(IndexA, IndexB, Blend, bSetIndexA, bSetIndexB, bSetBlend);
	}

	// Create from double index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateDoubleIndexBlendPaintMaterial(uint8 Index)
	{
		return FVoxelPaintMaterial::CreateDoubleIndexBlend(Index);
	}

	// Create UV paint
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials", DisplayName = "Create UV Paint Material")
	static FVoxelPaintMaterial CreateUVPaintMaterial(
		uint8 Channel, 
		FVector2D UV, 
		bool bPaintU = true, 
		bool bPaintV = true)
	{
		return FVoxelPaintMaterial::CreateUV(Channel, UV, bPaintU, bPaintV);
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
	static FVoxelMaterial CreateMaterialFromColor(FLinearColor Color)
	{
		return FVoxelMaterial::CreateFromColor(Color);
	}

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static void GetSingleIndex(FVoxelMaterial Material, uint8& Index, float& DataA, float& DataB, float& DataC)
	{
		Index = Material.GetSingleIndex_Index();
		DataA = Material.GetSingleIndex_DataA_AsFloat();
		DataB = Material.GetSingleIndex_DataB_AsFloat();
		DataC = Material.GetSingleIndex_DataC_AsFloat();
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial CreateMaterialFromSingleIndex(uint8 Index, float DataA, float DataB, float DataC)
	{
		return FVoxelMaterial::CreateFromSingleIndex(Index, DataA, DataB, DataC);
	}

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static void GetDoubleIndex(FVoxelMaterial Material, uint8& IndexA, uint8& IndexB, float& Blend, float& Data)
	{
		IndexA = Material.GetDoubleIndex_IndexA();
		IndexB = Material.GetDoubleIndex_IndexB();
		Blend = Material.GetDoubleIndex_Blend_AsFloat();
		Data = Material.GetDoubleIndex_Data_AsFloat();
	}
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial CreateMaterialFromDoubleIndex(uint8 IndexA, uint8 IndexB, float Blend, float Data)
	{
		return FVoxelMaterial::CreateFromDoubleIndex(IndexA, IndexB, Blend, Data);
	}

public:
	/**
	 * FVoxelFloatTexture helpers
	 */
	
public:
	// Will create Texture if null, and set it
	// Returns Texture for convenience
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateOrUpdateTextureFromVoxelTexture(FVoxelFloatTexture VoxelTexture, UPARAM(ref) UTexture2D*& Texture)
	{
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static UTexture2D* CreateTextureFromVoxelTexture(FVoxelFloatTexture VoxelTexture)
	{
		UTexture2D* Texture = nullptr;
		FVoxelTextureUtilities::CreateOrUpdateUTexture2D(VoxelTexture.Texture, Texture);
		return Texture;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static FVoxelFloatTexture CreateVoxelTextureFromTextureChannel(UTexture2D* Texture, EVoxelRGBA Channel)
	{
		return { FVoxelTextureUtilities::CreateFromTexture_Float(Texture, Channel) };
	}
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static FIntPoint GetVoxelTextureSize(FVoxelFloatTexture Texture)
	{
		return { Texture.Texture.GetSizeX(), Texture.Texture.GetSizeY() };
	}
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Voxel Texture")
	static bool IsVoxelTextureValid(FVoxelFloatTexture Texture)
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