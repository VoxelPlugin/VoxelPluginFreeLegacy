// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPaintMaterial.h"
#include "IntBox.h"
#include "VoxelBlueprintLibrary.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class AVoxelActor;
class AVoxelWorld;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FVoxelOnChunkGenerationDynamicDelegate, AVoxelWorld*, World, FIntBox, Bounds);

UCLASS()
class VOXEL_API UVoxelBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		

public:
	/**
	 * FVoxelCacheManager helpers
	 */
	
	// Clear manually cached chunks not inside one of the BoundsToKeepCached bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cache")
	static void ClearCache(AVoxelWorld* World, const TArray<FIntBox>& BoundsToKeepCached);
	// Clear all manually cached chunks
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cache")
	static void ClearAllCache(AVoxelWorld* World);
	// Cache a zone of the world
	UFUNCTION(BlueprintCallable, Category = "Voxel|Cache")
	static void Cache(AVoxelWorld* World, const TArray<FIntBox>& BoundsToCache, bool bCacheValues = true, bool bCacheMaterials = true);

public:
	/**
	 * FVoxelData helpers
	 */
	
	// Undo last frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo")
	static void Undo(AVoxelWorld* World);
	// Redo last undone frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo")
	static void Redo(AVoxelWorld* World);
	// Save the edits since the last call to SaveFrame/Undo/Redo and clear the redo stack. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo")
	static void SaveFrame(AVoxelWorld* World);
	// Clear all the frames. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel|UndoRedo")
	static void ClearFrames(AVoxelWorld* World);

	// Get the normal at Position using the density gradient. May differ from the mesh normal
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (Keywords = "gradient"))
	static FVector GetNormal(AVoxelWorld* World, const FIntVector& Position);
	
	// Get a custom float output value
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
	static float GetFloatOutput(AVoxelWorld* World, FName Name, int X, int Y, int Z);

	// Bounds of this world
	UFUNCTION(BlueprintPure, Category = "Voxel|Data")
	static FIntBox GetBounds(AVoxelWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data")
	static void BindDelegateToChunkGeneration(AVoxelWorld* World, FVoxelOnChunkGenerationDynamicDelegate Event);

public:
	/**
	 * IVoxelLODManager helpers
	 */
	
	// Update the chunks overlapping Position
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render")
	static void UpdatePosition(AVoxelWorld* World, const FIntVector& Position);
	// Update the chunks overlapping Bounds. 
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render", meta = (AdvancedDisplay = "bWaitForAllChunksToFinishUpdating"))
	static void UpdateBounds(AVoxelWorld* World, const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating = false);
	// Update all the chunks
	UFUNCTION(BlueprintCallable, Category = "Voxel|Render")
	static void UpdateAll(AVoxelWorld* World);

public:
	/**
	 * IVoxelRenderer helpers
	 */

	// Number of mesh processing tasks not finished
	UFUNCTION(BlueprintPure, Category = "Voxel|Render")
	static int32 GetTaskCount(AVoxelWorld* World);


public:
	/**
	 * IVoxelPool helpers
	 */

	/**
	 * Create the global voxel thread pool. Must not be already created.
	 * @param	MeshThreadCount		At least 1
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads")
	 static void CreateGlobalVoxelThreadPool(int MeshThreadCount = 2);
	
	// Destroy the global voxel thread pool
	UFUNCTION(BlueprintCallable, Category = "Voxel|Threads")
	static void DestroyGlobalVoxelThreadPool();

	// Is the global voxel thread pool created?
	UFUNCTION(BlueprintPure, Category = "Voxel|Threads")
	static bool IsGlobalVoxelPoolCreated();

public:
	/**
	 * FIntBox helpers
	 */
	// From Floor(-Radius) (included) to Ceil(Radius) (excluded)
	UFUNCTION(BlueprintPure, Category = "Voxel|Utilities")
	static FIntBox MakeBoxFromGlobalPositionAndRadius(AVoxelWorld* World, const FVector& GlobalPosition, float RadiusInVoxels);

public:
	/**
	 * FIntVector helpers
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector + IntVector", CompactNodeTitle = "+", Keywords = "+ add plus"), Category = "Math|IntVector")
	static FIntVector Add_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector - IntVector", CompactNodeTitle = "-", Keywords = "- subtract minus"), Category = "Math|IntVector")
	static FIntVector Substract_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorIntVector(const FIntVector& Left, const FIntVector& Right);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector / int", CompactNodeTitle = "/", Keywords = "/ divide"), Category = "Math|IntVector")
	static FIntVector Divide_IntVectorInt(const FIntVector& Left, int Right);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "IntVector * int", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntVectorInt(const FIntVector& Left, int Right);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "int * IntVector", CompactNodeTitle = "*", Keywords = "* multiply"), Category = "Math|IntVector")
	static FIntVector Multiply_IntIntVector(int Left, const FIntVector& Right);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Max"), Category = "Math|IntVector")
	static int GetMax_Intvector(const FIntVector& Vector);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Min"), Category = "Math|IntVector")
	static int GetMin_Intvector(const FIntVector& Vector);
		
public:
	/**
	 * FVoxelPaintMaterial helpers
	 */

	 // Create from color
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials", DisplayName = "Create RGB Paint Material")
	static FVoxelPaintMaterial CreateRGBPaintMaterial(FLinearColor Color, float Amount = 1.f, bool bPaintR = true, bool bPaintG = true, bool bPaintB = true, bool bPaintA = true);

	// Create from index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateIndexPaintMaterial(uint8 Index);

	// Create from double index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateDoubleIndexSetPaintMaterial(uint8 IndexA, uint8 IndexB, float Blend, bool bSetIndexA = true, bool bSetIndexB = true, bool bSetBlend = true);

	// Create from double index
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateDoubleIndexBlendPaintMaterial(uint8 Index, float Amount);

	// Create from grass id
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateGrassPaintMaterial(uint8 GrassId);

	// Create from actor id
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelPaintMaterial CreateActorPaintMaterial(uint8 ActorId);

	// Apply a Paint Material to a Voxel Material
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial ApplyPaintMaterial(FVoxelMaterial Material, FVoxelPaintMaterial PaintMaterial);

public:
	/**
	 * FVoxelMaterial helpers
	 */
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FLinearColor GetColor(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetColor(FVoxelMaterial Material, FLinearColor Color);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial CreateMaterialFromColor(FLinearColor Color);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetIndex(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetIndex(FVoxelMaterial Material, uint8 Index);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial CreateMaterialFromIndex(uint8 Index);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetIndexA(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetIndexB(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetBlend(FVoxelMaterial Material);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetIndexA(FVoxelMaterial Material, uint8 Index);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetIndexB(FVoxelMaterial Material, uint8 Index);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetBlend(FVoxelMaterial Material, float Blend);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial CreateMaterialFromDoubleIndex(uint8 IndexA, uint8 IndexB, float Blend);
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetVoxelActorId(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetVoxelActorId(FVoxelMaterial Material, uint8 VoxelSpawnedActorId);

	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static uint8 GetVoxelGrassId(FVoxelMaterial Material);
	UFUNCTION(BlueprintPure, Category = "Voxel|Materials")
	static FVoxelMaterial SetVoxelGrassId(FVoxelMaterial Material, uint8 VoxelGrassId);
};