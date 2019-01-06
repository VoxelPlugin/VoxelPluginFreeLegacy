// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodyInstance.h"

#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelSave.h"
#include "VoxelRender/VoxelRenderFactory.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGenerator.h"
#include "VoxelUtilities.h"
#include "VoxelPoolManager.h"

#include "VoxelWorld.generated.h"

class IVoxelRender;
class FVoxelData;
class FVoxelActorOctreeManager;
class UVoxelInvokerComponent;
class AVoxelWorldEditorInterface;
class AVoxelActor;
class UVoxelMaterialCollection;
class FVoxelPool;
class UVoxelMaterialInterface;
class AVoxelChunksOwner;
struct FVoxelActorSpawnInfo;
class FVoxelActorComputedChunksOctree;
class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldDestroyed);

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnWorldLoaded OnWorldLoaded;

	// Called right before destroying the world. Use this if you want to save data
	UPROPERTY(BlueprintAssignable)
	FOnWorldDestroyed OnWorldDestroyed;

	UPROPERTY()
	FVoxelMaterialsRefHolder MaterialsRef;
		
public:
	AVoxelWorld();
	

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void CreateWorld();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void CreateWorldUsingOtherWorldData(AVoxelWorld* OtherWorld);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void DestroyWorld(bool bClearMeshes = true);

	// Create the world when in editor (not PIE)
	void CreateInEditor(UClass* VoxelWorldEditorClass);
	// Destroy the world when in editor (not PIE)
	void DestroyInEditor();

	void Recreate();

public:
	void UpdateCollisionProfile();

public:
	inline bool IsEditor() const { return PlayType == EPlayType::Editor; }
	inline TSharedRef<FVoxelData, ESPMode::ThreadSafe> GetDataSharedPtr() const { return Data.ToSharedRef(); }
	inline FVoxelData* GetData() const { return Data.Get(); }
	inline FVoxelPool* GetPool() const { return Pool.Get(); }
	inline UMaterialInstanceDynamic* GetMaterialInstance() const { return MaterialInstance; }
	inline const FVoxelWorldGeneratorPicker GetWorldGeneratorPicker() const { return WorldGenerator; }
	inline bool GetColorTransitions() const { return bColorTransitions; }
	FVoxelWorldGeneratorInit GetInitStruct() const;
	FVector GetChunkRelativePosition(const FIntVector& Position) const;

	inline bool IsTessellationEnabled(int InLOD) const
	{
		return GetEnableTessellation() && InLOD <= GetTessellationMaxLOD();
	}

	inline const TArray<TWeakObjectPtr<UVoxelInvokerComponent>>& GetInvokers() const { return Invokers; }
	inline bool IsDedicatedServer() const { return bIsDedicatedServer; }


	UMaterialInterface* GetVoxelMaterial(const FVoxelBlendedMaterial& Index) const;
	UMaterialInterface* GetVoxelMaterialWithTessellation(const FVoxelBlendedMaterial& Index) const;
	inline UMaterialInterface* GetVoxelMaterial(int InLOD, const FVoxelBlendedMaterial& Index) const
	{
		return IsTessellationEnabled(InLOD) ? GetVoxelMaterialWithTessellation(Index) : GetVoxelMaterial(Index);
	}
	inline UMaterialInterface* GetVoxelMaterial(int InLOD) const
	{
		auto* Result = IsTessellationEnabled(InLOD) ? GetVoxelMaterialWithTessellation() : GetVoxelMaterial();
		return Result ? Result : MissingMaterial;
	}
	
public:
	/** Set a MID scalar (float) parameter value */
	UFUNCTION(BlueprintCallable, Category="Voxel|Material", meta=(DisplayName = "Set Voxel Material Scalar Parameter Value", Keywords = "SetFloatParameterValue"))
	void SetScalarParameterValue(FName ParameterName, float Value);

	/** Set an MID texture parameter value */
	UFUNCTION(BlueprintCallable, Category="Voxel|Material", meta=(DisplayName = "Set Voxel Material Texture Parameter Value"))
	void SetTextureParameterValue(FName ParameterName, UTexture* Value);

	/** Set an MID vector parameter value */
	UFUNCTION(BlueprintCallable, Category="Voxel|Material", meta=(DisplayName = "Set Voxel Material Vector Parameter Value", Keywords = "SetColorParameterValue"))
	void SetVectorParameterValue(FName ParameterName, FLinearColor Value);

public:
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetVoxelSize(float NewSize);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	float GetVoxelSize() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetVoxelMassMultiplierInKg(float NewVoxelMassMultiplierInKg);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	float GetVoxelMassMultiplierInKg() const;	
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator);
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass);

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetSaveObject(UVoxelWorldSaveObject* NewSaveObject);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	UVoxelWorldSaveObject* GetSaveObject() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetCreateWorldAutomatically(bool bNewCreateWorldAutomatically);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	bool GetCreateWorldAutomatically() const;
	
	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void AddWorldToUpdateWhenUpdated(AVoxelWorld* World);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	TArray<AVoxelWorld*> GetWorldsToUpdateWhenUpdated() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetEnableUndoRedo(bool bNewEnableUndoRedo);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	bool GetEnableUndoRedo() const;
	
	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void SetEnableWorldRebasing(bool bNewEnableWorldRebasing);
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	bool GetEnableWorldRebasing() const;
	
	// CANNOT be called when created. Add to existing seeds
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void AddSeeds(const TMap<FString, int>& NewSeeds);
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void AddSeed(FString Name, int Value);
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	void ClearSeeds();
	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	TMap<FString, int> GetSeeds() const;

///////////////////////////////////////////////////////////////////////////////

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetOctreeDepth(int NewDepth);
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	int GetOctreeDepth() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetUseCustomWorldBounds(bool bNewUseCustomWorldBounds);
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	bool GetUseCustomWorldBounds() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	void SetCustomWorldBounds(FIntBox NewCustomWorldBounds);
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Size")
	FIntBox GetCustomWorldBounds() const;

///////////////////////////////////////////////////////////////////////////////

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	void SetLODLimit(int NewLODLimit);
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	int GetLODLimit() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	void SetLODLowerLimit(int NewLODLowerLimit);
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	int GetLODLowerLimit() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	void SetLODToMinDistance(const TMap<uint8, float>& NewLODToMinDistance);
	UFUNCTION(BlueprintCallable, Category = "Voxel|LOD Settings")
	TMap<uint8, float> GetLODToMinDistance() const;

///////////////////////////////////////////////////////////////////////////////

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetUVConfig(EVoxelUVConfig Config);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	EVoxelUVConfig GetUVConfig() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetNormalConfig(EVoxelNormalConfig Config);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	EVoxelNormalConfig GetNormalConfig() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetMaterialConfig(EVoxelMaterialConfig Config);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	EVoxelMaterialConfig GetMaterialConfig() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials", meta = (DisplayName = "Set VoxelWorld Material"))
	void SetVoxelMaterial(UMaterialInterface* NewMaterial);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials", meta = (DisplayName = "Get VoxelWorld Material"))
	UMaterialInterface* GetVoxelMaterial() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials", meta = (DisplayName = "Set VoxelWorld Material With Tessellation"))
	void SetVoxelMaterialWithTessellation(UMaterialInterface* NewMaterial);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials", meta = (DisplayName = "Get VoxelWorld Material With Tessellation"))
	UMaterialInterface* GetVoxelMaterialWithTessellation() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetMaterialCollection(UVoxelMaterialCollection* NewMaterialCollection);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	UVoxelMaterialCollection* GetMaterialCollection() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetEnableTessellation(bool NewEnableTessellation);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	bool GetEnableTessellation() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetBoundsExtension(float NewBoundsExtension);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	float GetBoundsExtension() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	void SetTessellationMaxLOD(int NewTessellationMaxLOD);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Materials")
	int GetTessellationMaxLOD() const;
	
///////////////////////////////////////////////////////////////////////////////

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetRenderType(EVoxelRenderType Type);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	EVoxelRenderType GetRenderType() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetChunksCullingLOD(int NewChunksCullingLOD);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	int GetChunksCullingLOD() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetChunksCullingLowerLOD(int NewChunksCullingLOD);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	int GetChunksCullingLowerLOD() const;
	
	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetChunksFadeDuration(float NewChunksFadeDuration);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	float GetChunksFadeDuration() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetProcMeshClass(TSubclassOf<UVoxelProceduralMeshComponent> NewProcMeshClass);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	TSubclassOf<UVoxelProceduralMeshComponent> GetProcMeshClass() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	void SetDontRender(bool bNewDontRender);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Rendering")
	bool GetDontRender() const;


///////////////////////////////////////////////////////////////////////////////
	

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	void SetEnableCollisions(bool bNewEnableCollisions);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	bool GetEnableCollisions() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	void SetMaxCollisionsLOD(int NewMaxCollisionsLOD);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	int GetMaxCollisionsLOD() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	void SetCollisionPresets(FBodyInstance NewCollisionPresets);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	FBodyInstance GetCollisionPresets() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	void SetEnableNavmesh(bool bNewEnableNavmesh);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	bool GetEnableNavmesh() const;
	
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	void SetMaxNavmeshLOD(int NewNavmeshLOD);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Collisions & Navmesh")
	int GetMaxNavmeshLOD() const;
		
///////////////////////////////////////////////////////////////////////////////
		
	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetLODUpdateRate(float NewLODUpdateRate);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	float GetLODUpdateRate() const;
	
	
	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetEnableAutomaticCache(bool bNewEnableAutomaticCache);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	bool GetEnableAutomaticCache() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetCacheUpdateDelayInSeconds(float NewCacheUpdateDelayInSeconds);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	float GetCacheUpdateDelayInSeconds() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetCacheAccessThreshold(int NewCacheAccessThreshold);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	int GetCacheAccessThreshold() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetMaxCacheSize(int NewMaxCacheSize);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	int GetMaxCacheSize() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetDontUseGlobalPool(bool bNewDontUseGlobalPool);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	bool GetDontUseGlobalPool() const;

	// CANNOT be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetMeshThreadCount(int NewMeshThreadCount);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	int GetMeshThreadCount() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetOptimizeIndices(bool bNewOptimizeIndices);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	bool GetOptimizeIndices() const;

	// CAN be called when created
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	void SetWaitForOtherChunksToAvoidHoles(bool bNewWaitForOtherChunksToAvoidHoles);
	UFUNCTION(BlueprintCallable, Category = "Voxel|Performance")
	bool GetWaitForOtherChunksToAvoidHoles() const;
	
///////////////////////////////////////////////////////////////////////////////
	
	
public:
	// Is this world created?
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsCreated() const;

	// Bounds of this world
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntBox GetBounds() const;

	// Number of mesh processing tasks not finished
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	int32 GetTaskCount() const;

public:
	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntVector GlobalToLocal(const FVector& Position) const;
	// Precision errors with large world offset
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector GlobalToLocalFloat(const FVector& Position) const;

	/**
	 * Convert position from voxel space to world space
	 * @param	Position	Position in voxel space
	 * @return	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobal(const FIntVector& Position) const;
	// Precision errors with large world offset
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobalFloat(const FVector& Position) const;

	/**
	 * Get the 8 neighbors in voxel space of GlobalPosition
	 * @param	GlobalPosition	The position in world space
	 * @return	The 8 neighbors in voxel space 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	TArray<FIntVector> GetNeighboringPositions(const FVector& GlobalPosition) const;

public:
	/**
	 * Add chunks at position to update queue
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksAtPosition(const FIntVector& Position);
	/**
	 * Add chunks overlapping box to update queue
	 * @param	Box			Box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksOverlappingBox(const FIntBox& Box, bool bRemoveHoles);
	void UpdateChunksOverlappingBox(const FIntBox& Box, TFunction<void()> CallbackWhenUpdated);
	/**
	 * Update all the chunks
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateAll();

public:
	// Get the render chunks LOD at Position
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	int GetLODAt(const FIntVector& Position) const;
	// Is position in this world?
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	bool IsInWorld(const FIntVector& Position) const;
		
	/**
	 * Is the position inside the world mesh? If false then it's outside, if true then it might inside
	 * @param	Position	Position in global space
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	bool IsInside(const FVector& Position) const;

	/**
	 * Get the intersection using voxel data. Doesn't depend on LOD. Useful for short distances, but really expensive for big ones
	 * @param	Start				The start of the raycast. The start and the end must have only one coordinate not in common
	 * @param	End					The end of the raycast. The start and the end must have only one coordinate not in common
	 * @param	GlobalPosition		The world position of the intersection if found
	 * @param	VoxelPosition		The voxel position of the intersection if found
	 * @return	Has intersected?
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	bool GetIntersection(const FIntVector& Start, const FIntVector& End, FVector& GlobalPosition, FIntVector& VoxelPosition) const;

	/**
	 * Get the normal at the voxel position Position using gradient. May differ from the mesh normal
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	FVector GetNormal(const FIntVector& Position) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	float GetValue(const FIntVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetValue(const FIntVector& Position, float Value, bool bUpdateRender = true);
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	FVoxelMaterial GetMaterial(const FIntVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetMaterial(const FIntVector& Position, const FVoxelMaterial& Material, bool bUpdateRender = true);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	float GetFloatOutput(FName Name, int X, int Y, int Z) const;

public:
	/**
	 * Get a save of the current world
	 * @return	SaveArray
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void GetSave(FVoxelUncompressedWorldSave& OutSave) const;
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void GetCompressedSave(FVoxelCompressedWorldSave& OutSave) const;
	/**
	 * Load world from save
	 * @param	Save	Save to load from
	 * @param	bReset	Reset existing world? Set to false only if current world is unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "1"))
	void LoadFromSave(const FVoxelUncompressedWorldSave& Save);
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "1"))
	void LoadFromCompressedSave(UPARAM(ref) FVoxelCompressedWorldSave& Save);
	

public:
	// Undo last frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void Undo();
	// Redo last undone frame. bEnableUndoRedo must be true, and SaveFrame must have been called after any edits
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void Redo();
	// Save the edits since the last call to SaveFrame/Undo/Redo and clear the redo stack. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SaveFrame();
	// Clear all the frames. bEnableUndoRedo must be true
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void ClearFrames();

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void DrawDebugIntBox(const FIntBox& Box, float Lifetime = 1, float Thickness = 10, FLinearColor Color = FLinearColor::Red) const;
	void DrawDebugIntBox(const FIntBox& Box, float Lifetime = 1, float Thickness = 10, FColor Color = FColor::Red, float BorderOffset = 0) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void DebugVoxelsInsideBox(
		FIntBox Box, 
		FLinearColor Color = FLinearColor::Red, 
		float Lifetime = 1, 
		float Thickness = 1, 
		bool bDebugDensities = true, 
		FLinearColor TextColor = FLinearColor::Black) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void ClearCache(const TArray<FIntBox>& BoundsToKeepCached);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void ClearAllCache() { ClearCache({}); }

	/**
	 * Cache a zone of the world
	 * @param	BoundsToCache			The bounds to cache
	 * @param	bCheckIfChunksAreEmpty	If true, the chunks will be iterated to see if all their values have the same sign.
										Useful to reduce memory usage, but not if you want to query/edit the world as it'll need to be cached again.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void Cache(const TArray<FIntBox>& BoundsToCache, bool bCheckIfChunksAreEmpty = false);
	/**
	 * Cache a zone of the world
	 * @param	BoundsToCache			The bounds to cache
	 * @param	bCheckIfChunksAreEmpty	If true, the chunks will be iterated to see if all their values have the same sign.
										Useful to reduce memory usage, but not if you want to query/edit the world as it'll need to be cached again.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel")
	void CacheBounds(FIntBox BoundsToCache, bool bCheckIfChunksAreEmpty = false) { Cache({ BoundsToCache }, bCheckIfChunksAreEmpty); }

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor = true);


public:
	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	void Tick(float DeltaTime) override;
	void Serialize(FArchive& Ar) override;
	void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	void PostLoad() override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const override;
	bool CanEditChange(const UProperty* InProperty) const override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End AActor Interface

protected:
	UPROPERTY()
	int LOD_DEPRECATED;
	
	UPROPERTY()
	int ChunkCullingLOD_DEPRECATED;

	UPROPERTY()
	FIntBox WorldBounds_DEPRECATED;

	UPROPERTY()
	bool bOverrideBounds_DEPRECATED;
	
	//////////////////////////////////////////////////////////////////////////////

	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditWhenCreated, Recreate, ClampMin = 0.0001))
	float VoxelSize = 100;

	// Mass in kg of a voxel (multiplied by its density)
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditWhenCreated))
	float VoxelMassMultiplierInKg = 1;

	// Generator of this world
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	FVoxelWorldGeneratorPicker WorldGenerator;

	// Automatically loaded on creation
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditWhenCreated))
	UVoxelWorldSaveObject* SaveObject = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	bool bCreateWorldAutomatically = false;
	
	// Share the data between multiple worlds
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditCondition = bCreateWorldAutomatically))
	AVoxelWorld* VoxelWorldToCopyDataFrom = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditWhenCreated))
	TArray<TWeakObjectPtr<AVoxelWorld>> WorldsToUpdateWhenUpdated;

	// Keep all the changes in memory to enable undo/redo. Can be expensive
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	bool bEnableUndoRedo = false;

	// If true, the voxel world will try to stay near its original coordinates when rebasing, and will offset the voxel coordinates instead
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (EditWhenCreated))
	bool bEnableWorldRebasing = false;

	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	TMap<FString, int> Seeds;

	//////////////////////////////////////////////////////////////////////////////

	// WorldSizeInVoxel = CHUNK_SIZE * 2^Depth. Has little impact on performance
	UPROPERTY(EditAnywhere, Category = "Voxel|World Size", meta = (ClampMin = 1, ClampMax = 24, UIMin = 1, UIMax = 24))
	int OctreeDepth = 10;

	// Size of an edge of the world
	UPROPERTY(EditAnywhere, Category = "Voxel|World Size", meta = (ClampMin = 1))
	int WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(10);

	UPROPERTY()
	bool bUseCustomWorldBounds = false;

	UPROPERTY(EditAnywhere, Category = "Voxel|World Size", meta = (EditCondition = "bUseCustomWorldBounds"))
	FIntBox CustomWorldBounds;
	
	//////////////////////////////////////////////////////////////////////////////

	// Chunks can't have a LOD strictly higher than this. Useful is background has a too low resolution. WARNING: Don't set this too low, 5 under Octree Depth should be safe
	UPROPERTY(EditAnywhere, Category = "Voxel|LOD Settings", meta = (EditWhenCreated, DisplayName = "LOD Limit", ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int LODLimit = MAX_WORLD_DEPTH - 1;

	// Chunks can't have a LOD strictly lower than this. Mainly useful when you have multiple voxel worlds using the same data
	UPROPERTY(EditAnywhere, Category = "Voxel|LOD Settings", meta = (EditWhenCreated, DisplayName = "LOD Lower Limit", ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int LODLowerLimit = 0;

	// In world space. Chunks under this distance from voxel invokers will have at least the given LOD
	UPROPERTY(EditAnywhere, Category = "Voxel|LOD Settings", meta = (EditWhenCreated, DisplayName = "LOD to Min Distance"))
	TMap<FString, float> LODToMinDistance = { {"0", 1000.f } };
	
	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, UpdateAll, DisplayName = "UV Config"))
	EVoxelUVConfig UVConfig = EVoxelUVConfig::GlobalUVs;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, UpdateAll))
	EVoxelNormalConfig NormalConfig = EVoxelNormalConfig::GradientNormal;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, Recreate))
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	// The material of the world
	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, Recreate, DisplayName = "VoxelWorld Material"))
	UMaterialInterface* VoxelMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, Recreate, DisplayName = "VoxelWorld Material With Tessellation", EditCondition ="bEnableTessellation"))
	UMaterialInterface* VoxelMaterialWithTessellation = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, Recreate))
	UVoxelMaterialCollection* MaterialCollection;

	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, Recreate))
	bool bEnableTessellation = false;

	// Increases the chunks bounding boxes, useful when using tessellation
	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, UpdateAll, EditCondition ="bEnableTessellation"))
	float BoundsExtension = 0;
	
	// Inclusive. Chunks with a LOD higher than that will use a non tessellated material
	UPROPERTY(EditAnywhere, Category = "Voxel|Materials", meta = (EditWhenCreated, UpdateAll, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition ="bEnableTessellation"))
	int TessellationMaxLOD = 0;
	
	//////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, Recreate))
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;

	// Chunks with a LOD strictly higher than this won't be rendered
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, Recreate, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int ChunksCullingLOD = MAX_WORLD_DEPTH - 1;

	// Chunks with a LOD strictly lower than this won't be rendered
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, Recreate, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25))
	int ChunksCullingLowerLOD = 0;

	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, ClampMin = 0))
	float ChunksFadeDuration = 1;
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, Recreate))
	TSubclassOf<UVoxelProceduralMeshComponent> ProcMeshClass;

	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (EditWhenCreated, Recreate))
	bool bDontRender = false;

	//////////////////////////////////////////////////////////////////////////////


	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions & Navmesh", meta = (EditWhenCreated, Recreate))
	bool bEnableCollisions = true;

	// Max LOD to compute collisions on. Inclusive.
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions & Navmesh", meta = (EditWhenCreated, Recreate, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition = bEnableCollisions))
	int MaxCollisionsLOD = 3;

	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions & Navmesh", meta = (EditWhenCreated, Recreate, EditCondition = bEnableCollisions))
	FBodyInstance CollisionPresets;

	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions & Navmesh", meta = (EditWhenCreated, Recreate))
	bool bEnableNavmesh = false;

	// Max LOD to compute navmesh on. Inclusive.
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions & Navmesh", meta = (EditWhenCreated, Recreate, ClampMin = 0, ClampMax = 25, UIMin = 0, UIMax = 25, EditCondition = bEnableNavmesh))
	int MaxNavmeshLOD = 3;
	
	//////////////////////////////////////////////////////////////////////////////
	
	// Number of LOD update per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = 0.000001), DisplayName = "LOD Update Rate")
	float LODUpdateRate = 15;

	
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (EditWhenCreated))
	bool bEnableAutomaticCache = true;

	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = 0, EditCondition = bEnableAutomaticCache))
	float CacheUpdateDelayInSeconds = 1;

	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (EditWhenCreated, EditCondition = bEnableAutomaticCache, ClampMin = 0))
	int CacheAccessThreshold = 3;

	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (EditWhenCreated, EditCondition = bEnableAutomaticCache, ClampMin = 0))
	int MaxCacheSize = 1000;

	UPROPERTY(EditAnywhere, Category = "Voxel|Performance")
	bool bDontUseGlobalPool = true;

	// Number of threads allocated for the mesh processing. Setting it too high may impact performance
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = 1, EditCondition = "bDontUseGlobalPool"))
	int MeshThreadCount = 2;
	
	// If true, the mesh indices will be sorted to improve GPU cache performance. Adds a cost to the async mesh building. If you don't see any perf difference, leave it off
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (EditWhenCreated, UpdateAll))
	bool bOptimizeIndices = false;

	// Less holes, but might look more laggy
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (EditWhenCreated))
	bool bWaitForOtherChunksToAvoidHoles = true;
	
	//////////////////////////////////////////////////////////////////////////////

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug")
	bool bShowPopupIfNoInvokers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bShowWorldBounds = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated, EditCondition = "bShowWorldBounds"))
	float WorldBoundsThickness = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bShowAllRenderChunks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bColorTransitions = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bShowInvokersPositions = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated, EditCondition = "bShowInvokersPositions"))
	float InvokersPointSize = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bShowDataOctreeLeavesStatus = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated, EditCondition = "bShowDataOctreeLeavesStatus"))
	bool bHideEmptyDataOctreeLeaves = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated, EditCondition = "bShowDataOctreeLeavesStatus"))
	float DataOctreeLeavesThickness = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bShowUpdatedChunks = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated, EditCondition = "bShowUpdatedChunks"))
	float UpdatedChunksThickness = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bLogRenderOctreeStats = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	int MaxRenderOctreeLeaves = 10000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel|Debug", meta = (EditWhenCreated))
	bool bLogCacheStats = false;

private:
	UPROPERTY()
	UMaterialInterface* MissingMaterial;

	UPROPERTY(Transient)
	AVoxelWorldEditorInterface* VoxelWorldEditor;

	// Material to copy interp param from
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* MaterialInstance;

	enum class EPlayType
	{
		Destroyed,
		Editor,
		Normal
	};
	EPlayType PlayType = EPlayType::Destroyed;

	FVoxelPoolRef Pool;	

	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data;
	TSharedPtr<IVoxelRender> Render;
	TSharedPtr<class FVoxelCacheManager> CacheManager;
	
	bool bIsCreated = false;
	int InvokerComponentChangeVersion = -1;
	bool bWorldLoadAlreadyFinished = false;
	bool bIsDedicatedServer = false;
	bool bIsOwningData = false;

	FVoxelTimer CacheDebugTimer;
	FVoxelTimer AutomaticCacheTimer;

	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;

	// Position of the voxel world actor in voxel space
	FIntVector WorldOffset = FIntVector::ZeroValue;


private:
	// Create the world
	void CreateWorldInternal(
		EPlayType InPlayType,
		AVoxelWorld* WorldToCopyDataFrom,
		bool bInMultiplayer,
		bool bInEnableUndoRedo,
		bool bInDontUseGlobalPool);

	// Destroy the world
	void DestroyWorldInternal();

};

/**
 * Interface to use the VoxelWorldEditor in the VoxelEditor module
 */
UCLASS(notplaceable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API AVoxelWorldEditorInterface : public AActor
{
	GENERATED_BODY()

public:
	virtual void Init(TWeakObjectPtr<AVoxelWorld> NewWorld) {}
};