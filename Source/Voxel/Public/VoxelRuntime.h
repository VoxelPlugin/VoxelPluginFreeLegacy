// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "PhysicsEngine/BodyInstance.h"
#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelRender/VoxelLODMaterials.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"

enum class EVoxelPlayType;
enum class EVoxelUVConfig : uint8;
enum class EVoxelTaskType : uint8;
enum class EVoxelRenderType : uint8;
enum class EVoxelRGBHardness : uint8;
enum class EVoxelNormalConfig : uint8;
enum class EVoxelMaterialConfig : uint8;

struct FVoxelGeneratorInit;

class FVoxelData;
class IVoxelSubsystem;
class FInvokerPositionsArray;

class AVoxelWorld;
class AVoxelRuntimeActor;

class UMaterialInterface;
class UVoxelSpawnerConfig;
class UVoxelPlaceableItemManager;
class UVoxelMultiplayerInterface;
class UVoxelProceduralMeshComponent;
class UVoxelMaterialCollectionBase;

class UVoxelLODSubsystemProxy;
class UVoxelRendererSubsystemProxy;

class VOXEL_API FVoxelRuntimeSettings
{
public:
	EVoxelPlayType PlayType;
	
	// Used for errors
	TWeakObjectPtr<UObject> Owner;
	TWeakObjectPtr<UWorld> World;
	// For actor components
	TWeakObjectPtr<AActor> ComponentsOwner;
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
	bool bEnableCustomWorldRebasing;
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
	bool bOneMaterialPerCubeSide;
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
	TWeakObjectPtr<UVoxelSpawnerConfig> SpawnerConfig;
	int32 HISMChunkSize;
	int32 SpawnersCollisionDistanceInVoxel;
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
	FVoxelIntBox GetWorldBounds() const;
	static FVoxelIntBox GetWorldBounds(bool bUseCustomWorldBounds, const FVoxelIntBox& CustomWorldBounds, int32 RenderOctreeDepth);

	FVoxelGeneratorInit GetGeneratorInit() const;
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
	
public:
	FIntVector WorldOffset = FIntVector::ZeroValue;
	FSimpleMulticastDelegate OnWorldOffsetChanged;

	void SetWorldOffset(const FIntVector& NewWorldOffset)
	{
		check(IsInGameThread());
		WorldOffset = NewWorldOffset;
		OnWorldOffsetChanged.Broadcast();
	}

public:
	FThreadSafeCounter64 CollisionMemory;
	TVoxelSharedRef<FInvokerPositionsArray> InvokersPositionsForPriorities;

	FVoxelRuntimeData();
};

class VOXEL_API FVoxelRuntime : public TVoxelSharedFromThis<FVoxelRuntime>
{
public:
	const FVoxelRuntimeSettings Settings;
	const TVoxelSharedRef<FVoxelRuntimeDynamicSettings> DynamicSettings = MakeVoxelShared<FVoxelRuntimeDynamicSettings>();
	const TVoxelSharedRef<FVoxelRuntimeData> RuntimeData = MakeVoxelShared<FVoxelRuntimeData>();

	static TVoxelSharedRef<FVoxelRuntime> Create(const FVoxelRuntimeSettings& Settings);
	
	explicit FVoxelRuntime(const FVoxelRuntimeSettings& Settings);
	~FVoxelRuntime();
	
	template<typename T>
	TVoxelSharedPtr<T> GetSubsystem() const
	{
		ensureMsgf(!bIsInit, TEXT("Use InitializeDependency instead!"));
		FScopeLock Lock(&CriticalSection);
		return StaticCastSharedPtr<T>(SubsystemsMap.FindRef(T::ProxyClass::StaticClass()));
	}
	template<typename T>
	TVoxelSharedRef<T> GetSubsystemChecked() const
	{
		// Unsafe to do in async threads
		check(IsInGameThread());
		return GetSubsystem<T>().ToSharedRef();
	}
	
	template<typename T>
	TVoxelSharedRef<T> InitializeDependency()
	{
		check(bIsInit);
		const auto Subsystem = SubsystemsMap.FindRef(T::ProxyClass::StaticClass());
		InitializeSubsystem(Subsystem);
		return StaticCastSharedRef<T>(Subsystem.ToSharedRef());
	}
	template<typename T>
	void RecreateSubsystem()
	{
		RecreateSubsystem(GetSubsystem<T>());
	}

private:
	// The lock is needed for RecreateSubsystem
	mutable FCriticalSection CriticalSection;
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> AllSubsystems;
	TMap<UClass*, TVoxelSharedPtr<IVoxelSubsystem>> SubsystemsMap;

	bool bIsInit = false;
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> InitializedSubsystems;
	TSet<TVoxelSharedPtr<IVoxelSubsystem>> SubsystemsBeingInitialized;
	
	void InitializeSubsystem(const TVoxelSharedPtr<IVoxelSubsystem>& Subsystem);
	void RecreateSubsystem(TVoxelSharedPtr<IVoxelSubsystem> OldSubsystem);
	TVoxelSharedPtr<IVoxelSubsystem> AddSubsystem(UClass* Class);
};