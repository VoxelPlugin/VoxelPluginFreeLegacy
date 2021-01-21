// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"
#include "Templates/SubclassOf.h"
#include "PhysicsEngine/BodyInstance.h"
#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelRender/VoxelLODMaterials.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"

enum class EVoxelPlayType;
enum class EVoxelSubsystemFlags;
enum class EVoxelUVConfig : uint8;
enum class EVoxelTaskType : uint8;
enum class EVoxelRenderType : uint8;
enum class EVoxelRGBHardness : uint8;
enum class EVoxelNormalConfig : uint8;
enum class EVoxelMaterialConfig : uint8;
enum class EVoxelFoliageWorldType : uint8;

struct FVoxelGeneratorInit;

class FVoxelData;
class IVoxelSubsystem;
class FInvokerPositionsArray;

class AVoxelWorld;
class AVoxelRuntimeActor;

class UMaterialInterface;
class UVoxelFoliageBiomeType;
class UVoxelFoliageCollectionBase;
class UVoxelPlaceableItemManager;
class UVoxelMultiplayerInterface;
class UVoxelProceduralMeshComponent;
class UVoxelMaterialCollectionBase;

class UVoxelLODSubsystemProxy;
class UVoxelRendererSubsystemProxy;

// Note: We don't store settings on the runtime, as subsystems can each have different settings when they are recreated
class VOXEL_API FVoxelRuntimeSettings
{
public:
	EVoxelPlayType PlayType;
	
	// Used for errors
	TWeakObjectPtr<UObject> Owner;
	TWeakObjectPtr<UWorld> World;
	// Actor components outer
	TWeakObjectPtr<AActor> ComponentsOwner;
	// The primitive component to copy properties from & to attach scene components to
	TWeakObjectPtr<UPrimitiveComponent> RootComponent;
	// Use the most abstract one
	TWeakObjectPtr<const AVoxelRuntimeActor> Runtime;
	TWeakObjectPtr<const AVoxelWorld> VoxelWorld;
	
	TSubclassOf<UVoxelRendererSubsystemProxy> RendererSubsystem;
	TSubclassOf<UVoxelLODSubsystemProxy> LODSubsystem;

public:
	TVoxelSharedPtr<FVoxelData> DataOverride;

public:
	float VoxelSize;
	FVoxelWeakGeneratorPicker Generator;
	TWeakObjectPtr<UVoxelPlaceableItemManager> PlaceableItemManager;
	bool bCreateWorldAutomatically;
	bool bUseCameraIfNoInvokersFound;
	bool bEnableUndoRedo;
	bool bUseAbsoluteTransforms;
	bool bMergeAssetActors;
	bool bMergeDisableEditsBoxes;
	bool bDisableOnScreenMessages;
	bool bDisableDebugManager;
	
public:
	int32 RenderOctreeDepth;
	uint32 WorldSizeInVoxel;
	bool bUseCustomWorldBounds;
	FVoxelIntBox CustomWorldBounds;
	
public:
	int32 MaxLOD;
	int32 MinLOD;
	float InvokerDistanceThreshold;
	float MinDelayBetweenLODUpdates;
	bool bConstantLOD;
	
public:
	EVoxelMaterialConfig MaterialConfig;
	bool bUseMaterialCollection;
	TWeakObjectPtr<UMaterialInterface> VoxelMaterial;
	TWeakObjectPtr<UVoxelMaterialCollectionBase> MaterialCollection;
	TArray<FVoxelLODMaterials> LODMaterials; 
	TArray<FVoxelLODMaterialCollections> LODMaterialCollections; 
	EVoxelUVConfig UVConfig;
	float UVScale;
	EVoxelNormalConfig NormalConfig;
	EVoxelRGBHardness RGBHardness;
	TMap<FString, float> MaterialsHardness;
	bool bHardColorTransitions;
	TArray<uint8> HolesMaterials;
	TMap<uint8, FVoxelMeshConfig> MaterialsMeshConfigs;
	bool bHalfPrecisionCoordinates;
	bool bInterpolateColors;
	bool bInterpolateUVs;
	bool bSRGBColors;
	
public:
	EVoxelRenderType RenderType;
	int32 RenderSharpness;
	bool bCreateMaterialInstances;
	bool bDitherChunks;
	float ChunksDitheringDuration;
	bool bCastFarShadow;
	TSubclassOf<UVoxelProceduralMeshComponent> ProcMeshClass;
	int32 ChunksCullingLOD;
	bool bRenderWorld;
	bool bContributesToStaticLighting;
	bool bUseStaticPath;
	bool bStaticWorld;
	bool bGreedyCubicMesher;
	bool bSingleIndexGreedy;
	int32 TexturePoolTextureSize;
	bool bOptimizeIndices;
	bool bGenerateDistanceFields;
	int32 MaxDistanceFieldLOD;
	int32 DistanceFieldBoundsExtension;
	int32 DistanceFieldResolutionDivisor;
	float DistanceFieldSelfShadowBias;
	bool bEnableTransitions;
	bool bMergeChunks;
	int32 ChunksClustersSize;
	bool bDoNotMergeCollisionsAndNavmesh;
	float BoundsExtension;
	
public:
	TArray<TWeakObjectPtr<UVoxelFoliageCollectionBase>> FoliageCollections;
	EVoxelFoliageWorldType FoliageWorldType;
	bool bIsFourWayBlend;

	int32 HISMChunkSize;
	int32 FoliageCollisionDistanceInVoxel;
	int64 MaxNumberOfFoliageInstances;
	
public:
	bool bEnableCollisions;
	FBodyInstance CollisionPresets;
	TEnumAsByte<ECollisionTraceFlag> CollisionTraceFlag;
	bool bComputeVisibleChunksCollisions;
	int32 VisibleChunksCollisionsMaxLOD;
	bool bSimpleCubicCollision;
	int32 SimpleCubicCollisionLODBias;
	int32 NumConvexHullsPerAxis;
	bool bCleanCollisionMeshes;
	
public:
	bool bEnableNavmesh;
	bool bComputeVisibleChunksNavmesh;
	int32 VisibleChunksNavmeshMaxLOD;
	
public:
	TMap<EVoxelTaskType, int32> PriorityCategories;
	TMap<EVoxelTaskType, int32> PriorityOffsets;
	float MeshUpdatesBudget;
	float EventsTickRate;
	int32 DataOctreeInitialSubdivisionDepth;

public:
	bool bEnableMultiplayer;
	TSubclassOf<UVoxelMultiplayerInterface> MultiplayerInterface;
	float MultiplayerSyncRate;

public:
	FVoxelRuntimeSettings();

	void SetFromRuntime(const AVoxelRuntimeActor& Runtime);
	void ConfigurePreview();
	void Fixup();

public:
	static FVoxelIntBox GetWorldBounds(bool bUseCustomWorldBounds, const FVoxelIntBox& CustomWorldBounds, int32 RenderOctreeDepth);

public:
	FVoxelIntBox GetWorldBounds() const;
	FVoxelGeneratorInit GetGeneratorInit() const;

	void SetupComponent(USceneComponent& Component) const;
	void SetComponentPosition(USceneComponent& Component, const FIntVector& Position, bool bScaleByVoxelSize) const;

	// TransformsOffset is used to reduce precision errors
	FIntVector ComputeFoliageTransformsOffset(const FVoxelIntBox& Bounds) const
	{
		return FVoxelUtilities::DivideFloor(Bounds.Min, HISMChunkSize) * HISMChunkSize;
	}

	uint32 GetTextureDataStride() const
	{
		return bSingleIndexGreedy ? sizeof(uint8) : sizeof(FColor);
	}
};

class VOXEL_API FVoxelRuntimeDynamicSettings
{
public:
	int32 MinLOD;
	int32 MaxLOD;
	int32 ChunksCullingLOD;

	// In world space
	float InvokerDistanceThreshold;
	// Dynamic setting for LOD, static setting for render
	bool bRenderWorld;

	bool bEnableCollisions;
	bool bComputeVisibleChunksCollisions;
	int32 VisibleChunksCollisionsMaxLOD;
	
	bool bEnableNavmesh;
	bool bComputeVisibleChunksNavmesh;
	int32 VisibleChunksNavmeshMaxLOD;
	
	struct FLODMaterialSettings
	{
		TWeakObjectPtr<UMaterialInterface> Material;
		TWeakObjectPtr<UVoxelMaterialCollectionBase> MaterialCollection;
		FThreadSafeBool bEnableCubicFaces = false;
		FThreadSafeCounter MaxMaterialIndices = 1;
	};
	TVoxelStaticArray<FLODMaterialSettings, 32> MaterialSettings{ ForceInit };

public:
	FVoxelRuntimeDynamicSettings();
	UE_NONCOPYABLE(FVoxelRuntimeDynamicSettings);
	
	void SetFromRuntime(const AVoxelRuntimeActor& Runtime);
	void ConfigurePreview();
	void Fixup();

public:
	const FLODMaterialSettings& GetLODMaterialSettings(int32 LOD) const
	{
		return MaterialSettings[FMath::Clamp(LOD, 0, 32)];
	}
};

class VOXEL_API FVoxelRuntimeData
{
public:
	FSimpleMulticastDelegate OnWorldLoaded;
	FSimpleMulticastDelegate OnMaxFoliageInstancesReached;
	FSimpleMulticastDelegate OnRecomputeComponentPositions;

	FThreadSafeCounter64 CollisionMemory;
	TVoxelSharedRef<FInvokerPositionsArray> InvokersPositionsForPriorities;

	FVoxelRuntimeData();
};

class VOXEL_API FVoxelRuntime : public TVoxelSharedFromThis<FVoxelRuntime>
{
public:
	const TVoxelSharedRef<FVoxelRuntimeData> RuntimeData = MakeVoxelShared<FVoxelRuntimeData>();
	const TVoxelSharedRef<FVoxelRuntimeDynamicSettings> DynamicSettings = MakeVoxelShared<FVoxelRuntimeDynamicSettings>();

	static TVoxelSharedRef<FVoxelRuntime> Create(const FVoxelRuntimeSettings& Settings);
	
	FVoxelRuntime() = default;
	~FVoxelRuntime();

	void Destroy();
	void RecreateSubsystems(EVoxelSubsystemFlags Flags, const FVoxelRuntimeSettings& Settings);
	
	template<typename T>
	TVoxelSharedPtr<T> GetSubsystem() const
	{
		const auto Subsystem = GetSubsystemImpl<T>();
		if (bIsInit && Subsystem)
		{
			// This will happen if GetSubsystem is called in a subsystem Create
			const_cast<FVoxelRuntime*>(this)->InitializeSubsystem(Subsystem);
		}
		return GetTypedSubsystem<T>(Subsystem);
	}
	template<typename T>
	T& GetSubsystemChecked() const
	{
		// Unsafe to do in async threads
		check(IsInGameThread());
		return GetSubsystem<T>().ToSharedRef().Get();
	}
	
	template<typename T>
	void RecreateSubsystem(const FVoxelRuntimeSettings& Settings)
	{
		RecreateSubsystem(GetSubsystemImpl<T>(), Settings);
	}

private:
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> AllSubsystems;

	// The lock is needed for RecreateSubsystem
	// We lock for the entire Recreate
	// We never lock GetSubsystemImpl on the game thread since Recreate always runs on the game thread
	// It also avoids recursive locks
	mutable FCriticalSection RecreateSection;
	
	TMap<UClass*, TVoxelSharedPtr<IVoxelSubsystem>> SubsystemsMap;

	bool bIsInit = false;
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> InitializedSubsystems;
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> SubsystemsBeingInitialized;

	bool bIsDestroyed = false;
	
	void InitializeSubsystem(const TVoxelSharedPtr<IVoxelSubsystem>& Subsystem);
	void RecreateSubsystem(TVoxelSharedPtr<IVoxelSubsystem> OldSubsystem, const FVoxelRuntimeSettings& Settings);
	TVoxelSharedPtr<IVoxelSubsystem> AddSubsystem(UClass* Class, const FVoxelRuntimeSettings& Settings);

	template<typename T>
	TVoxelSharedPtr<IVoxelSubsystem> GetSubsystemImpl() const
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();

		const bool bNeedLock = !IsInGameThread();

		if (bNeedLock) RecreateSection.Lock();
		const auto Result = SubsystemsMap.FindRef(GetSubsystemProxyClass<T>());
		if (bNeedLock) RecreateSection.Unlock();

		return Result;
	}
	
	template<typename T>
	static FORCEINLINE typename TEnableIf<TIsDerivedFrom<T, IVoxelSubsystem>::IsDerived, UClass*>::Type GetSubsystemProxyClass()
	{
		return T::ProxyClass::StaticClass();
	}
	template<typename T>
	static FORCEINLINE typename TEnableIf<!TIsDerivedFrom<T, IVoxelSubsystem>::IsDerived, UClass*>::Type GetSubsystemProxyClass()
	{
		return GetSubsystemProxyClass<typename T::SubsystemClass>();
	}
	
	template<typename T>
	static FORCEINLINE typename TEnableIf<TIsDerivedFrom<T, IVoxelSubsystem>::IsDerived, TVoxelSharedPtr<T>>::Type GetTypedSubsystem(const TVoxelSharedPtr<IVoxelSubsystem>& Subsystem)
	{
		return StaticCastSharedPtr<T>(Subsystem);
	}
	template<typename T>
	static FORCEINLINE typename TEnableIf<!TIsDerivedFrom<T, IVoxelSubsystem>::IsDerived, TVoxelSharedPtr<T>>::Type GetTypedSubsystem(const TVoxelSharedPtr<IVoxelSubsystem>& Subsystem)
	{
		return Subsystem ? T::GetFromSubsystem(static_cast<typename T::SubsystemClass&>(*Subsystem)) : nullptr;
	}
};