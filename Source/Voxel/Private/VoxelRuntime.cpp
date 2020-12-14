// Copyright 2020 Phyronnaz

#include "VoxelRuntime.h"
#include "VoxelRuntimeActor.h"
#include "VoxelSubsystem.h"
#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelPriorityHandler.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelSpawners/VoxelFoliageCollection.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "UObject/UObjectHash.h"

FVoxelRuntimeSettings::FVoxelRuntimeSettings()
{
	PlayType = EVoxelPlayType::Game;
	SetFromRuntime(*GetDefault<AVoxelRuntimeActor>());
}

void FVoxelRuntimeSettings::SetFromRuntime(const AVoxelRuntimeActor& InRuntime)
{
#define SET(Name) Name = decltype(Name)(InRuntime.Name)

	Owner = const_cast<AVoxelRuntimeActor*>(&InRuntime);
	World = InRuntime.GetWorld();
	ComponentsOwner = const_cast<AVoxelRuntimeActor*>(&InRuntime);
	RootComponent = Cast<UPrimitiveComponent>(InRuntime.GetRootComponent());
	
	Runtime = &InRuntime;
	VoxelWorld = Cast<const AVoxelWorld>(&InRuntime);

	SET(RendererSubsystem);
	SET(LODSubsystem);

	SET(VoxelSize);
	SET(Generator);
	SET(PlaceableItemManager);
	SET(bCreateWorldAutomatically);
	SET(bUseCameraIfNoInvokersFound);
	SET(bEnableUndoRedo);
	SET(bEnableCustomWorldRebasing);
	SET(bMergeAssetActors);
	SET(bMergeDisableEditsBoxes);
	SET(bDisableOnScreenMessages);
	SET(bDisableDebugManager);

	SET(RenderOctreeDepth);
	SET(WorldSizeInVoxel);
	SET(bUseCustomWorldBounds);
	SET(CustomWorldBounds);

	SET(MaxLOD);
	SET(MinLOD);
	SET(InvokerDistanceThreshold);
	SET(MinDelayBetweenLODUpdates);
	SET(bConstantLOD);

	SET(MaterialConfig);
	SET(VoxelMaterial);
	SET(MaterialCollection);
	SET(LODMaterials);
	SET(LODMaterialCollections);
	SET(UVConfig);
	SET(UVScale);
	SET(NormalConfig);
	SET(RGBHardness);
	SET(MaterialsHardness);
	SET(bHardColorTransitions);
	SET(bOneMaterialPerCubeSide);
	SET(HolesMaterials);
	SET(MaterialsMeshConfigs);
	SET(bHalfPrecisionCoordinates);
	SET(bInterpolateColors);
	SET(bInterpolateUVs);
	SET(bSRGBColors);

	SET(RenderType);
	SET(RenderSharpness);
	SET(bCreateMaterialInstances);
	SET(bDitherChunks);
	SET(ChunksDitheringDuration);
	SET(bCastFarShadow);
	SET(ProcMeshClass);
	SET(ChunksCullingLOD);
	SET(bRenderWorld);
	SET(bContributesToStaticLighting);
	SET(bUseStaticPath);
	SET(bStaticWorld);
	SET(bGreedyCubicMesher);
	SET(TexturePoolTextureSize);
	SET(bOptimizeIndices);
	SET(bGenerateDistanceFields);
	SET(MaxDistanceFieldLOD);
	SET(DistanceFieldBoundsExtension);
	SET(DistanceFieldResolutionDivisor);
	SET(DistanceFieldSelfShadowBias);
	SET(bEnableTransitions);
	SET(bMergeChunks);
	SET(ChunksClustersSize);
	SET(bDoNotMergeCollisionsAndNavmesh);
	SET(BoundsExtension);

	SET(FoliageCollections);
	SET(FoliageWorldType);
	SET(bIsFourWayBlend);
	SET(HISMChunkSize);
	SET(SpawnersCollisionDistanceInVoxel);
	SET(MaxNumberOfFoliageInstances);

	SET(bEnableCollisions);
	SET(CollisionPresets);
	SET(CollisionTraceFlag);
	SET(bComputeVisibleChunksCollisions);
	SET(VisibleChunksCollisionsMaxLOD);
	SET(bSimpleCubicCollision);
	SET(SimpleCubicCollisionLODBias);
	SET(NumConvexHullsPerAxis);
	SET(bCleanCollisionMeshes);

	SET(bEnableNavmesh);
	SET(bComputeVisibleChunksNavmesh);
	SET(VisibleChunksNavmeshMaxLOD);

	SET(PriorityCategories);
	SET(PriorityOffsets);
	SET(MeshUpdatesBudget);
	SET(EventsTickRate);
	SET(DataOctreeInitialSubdivisionDepth);

	SET(bEnableMultiplayer);
	SET(MultiplayerInterface);
	SET(MultiplayerSyncRate);

#undef SET

	Fixup();
}

void FVoxelRuntimeSettings::ConfigurePreview()
{
	PlayType = EVoxelPlayType::Preview;
	
	bEnableMultiplayer = false;
	bEnableUndoRedo = true;
	ProcMeshClass = nullptr;
	bCreateMaterialInstances = false;
	MeshUpdatesBudget = 1000;
	bStaticWorld = false;
	bConstantLOD = false;

	if (VoxelWorld.IsValid() && !VoxelWorld->bEnableFoliageInEditor)
	{
		FoliageCollections.Reset();
	}

	Fixup();
}

void FVoxelRuntimeSettings::Fixup()
{
	// Ordering matters! If you set something after it's checked, the logic will fail
	
	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}
	if (RenderType == EVoxelRenderType::Cubic)
	{
		bHardColorTransitions = false;
	}
	if (bMergeChunks)
	{
		bCreateMaterialInstances = false;
	}
	else
	{
		bDoNotMergeCollisionsAndNavmesh = false;
	}
	if (!bCreateMaterialInstances)
	{
		bDitherChunks = false;
	}
	if (!bGenerateDistanceFields)
	{
		MaxDistanceFieldLOD = -1;
	}
	if (MaterialConfig != EVoxelMaterialConfig::RGB)
	{
		bGreedyCubicMesher = false;
	}
	if (MaterialConfig != EVoxelMaterialConfig::SingleIndex)
	{
		bOneMaterialPerCubeSide = false;
	}
	if (!bGreedyCubicMesher)
	{
		bSimpleCubicCollision = false;
	}

	MeshUpdatesBudget = FMath::Max(0.001f, MeshUpdatesBudget);
	RenderSharpness = FMath::Max(0, RenderSharpness);
	ChunksClustersSize = FMath::Max(RENDER_CHUNK_SIZE, ChunksClustersSize);
	SimpleCubicCollisionLODBias = FMath::Clamp(SimpleCubicCollisionLODBias, 0, 4);
	RenderOctreeDepth = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(FMath::Max(1, RenderOctreeDepth));
	TexturePoolTextureSize = FMath::Clamp(TexturePoolTextureSize, 128, 16384);
	EventsTickRate = FMath::Max(SMALL_NUMBER, EventsTickRate);
	MultiplayerSyncRate = FMath::Max(SMALL_NUMBER, MultiplayerSyncRate);
	SpawnersCollisionDistanceInVoxel = FMath::Max(0, SpawnersCollisionDistanceInVoxel);
	HISMChunkSize = FMath::Max(32, HISMChunkSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelIntBox FVoxelRuntimeSettings::GetWorldBounds(bool bUseCustomWorldBounds, const FVoxelIntBox& CustomWorldBounds, int32 RenderOctreeDepth)
{
	if (bUseCustomWorldBounds)
	{
		return
			FVoxelUtilities::GetCustomBoundsForDepth<RENDER_CHUNK_SIZE>(
				FVoxelIntBox::SafeConstruct(CustomWorldBounds.Min, CustomWorldBounds.Max),
				RenderOctreeDepth);
	}
	else
	{
		return FVoxelUtilities::GetBoundsFromDepth<RENDER_CHUNK_SIZE>(RenderOctreeDepth);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelIntBox FVoxelRuntimeSettings::GetWorldBounds() const
{
	return GetWorldBounds(bUseCustomWorldBounds, CustomWorldBounds, RenderOctreeDepth);
}

FVoxelGeneratorInit FVoxelRuntimeSettings::GetGeneratorInit() const
{
	return FVoxelGeneratorInit(
		VoxelSize,
		FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(RenderOctreeDepth),
		RenderType,
		MaterialConfig,
		MaterialCollection.Get(),
		VoxelWorld.Get());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntimeDynamicSettings::FVoxelRuntimeDynamicSettings()
{
	SetFromRuntime(*GetDefault<AVoxelRuntimeActor>());
}

void FVoxelRuntimeDynamicSettings::SetFromRuntime(const AVoxelRuntimeActor& Runtime)
{
#define SET(Name) Name = Runtime.Name
	
	SET(MinLOD);
	SET(MaxLOD);
	SET(ChunksCullingLOD);
	
	SET(InvokerDistanceThreshold);
	SET(bRenderWorld);

	SET(bEnableCollisions);
	SET(bComputeVisibleChunksCollisions);
	SET(VisibleChunksCollisionsMaxLOD);

	SET(bEnableNavmesh);
	SET(bComputeVisibleChunksNavmesh);
	SET(VisibleChunksNavmeshMaxLOD);

	TArray<UVoxelMaterialCollectionBase*> MaterialCollectionsToInitialize;
	for (int32 LOD = 0; LOD < 32; LOD++)
	{
		auto& Settings = MaterialSettings[LOD];

		// Copy materials
		Settings.Material = nullptr;
		for (auto& It : Runtime.LODMaterials)
		{
			if (It.StartLOD <= LOD && LOD <= It.EndLOD)
			{
				if (Settings.Material.IsValid())
				{
					FVoxelMessages::Warning(FString::Printf(TEXT("Multiple materials are assigned to LOD %d!"), LOD), &Runtime);
				}
				Settings.Material = It.Material;
			}
		}
		if (!Settings.Material.IsValid())
		{
			Settings.Material = Runtime.VoxelMaterial;
		}

		// Copy material collection
		Settings.MaterialCollection = nullptr;
		for (auto& It : Runtime.LODMaterialCollections)
		{
			if (It.StartLOD <= LOD && LOD <= It.EndLOD)
			{
				if (Settings.MaterialCollection.IsValid())
				{
					FVoxelMessages::Warning(FString::Printf(TEXT("Multiple material collections are assigned to LOD %d!"), LOD), &Runtime);
				}
				Settings.MaterialCollection = It.MaterialCollection;
			}
		}
		if (!Settings.MaterialCollection.IsValid())
		{
			Settings.MaterialCollection = Runtime.MaterialCollection;
		}

		// Set MaxMaterialIndices
		if (auto* Collection = Settings.MaterialCollection.Get())
		{
			Settings.MaxMaterialIndices.Set(FMath::Max(Collection->GetMaxMaterialIndices(), 1));
			MaterialCollectionsToInitialize.AddUnique(Collection);
		}
		else
		{
			Settings.MaxMaterialIndices.Set(1);
		}
	}

	// Initialize all used collections
	for (auto* Collection : MaterialCollectionsToInitialize)
	{
		Collection->InitializeCollection();
	}

	Fixup();
}

void FVoxelRuntimeDynamicSettings::ConfigurePreview()
{
	bEnableCollisions = true;
	bComputeVisibleChunksCollisions = true;
	VisibleChunksCollisionsMaxLOD = 32;

	// bEnableNavmesh is needed for path previews in editor, so don't disable it

	Fixup();
}

void FVoxelRuntimeDynamicSettings::Fixup()
{
	MinLOD = FVoxelUtilities::ClampMesherDepth(MinLOD);
	MaxLOD = FVoxelUtilities::ClampMesherDepth(MaxLOD);
	
	ChunksCullingLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(ChunksCullingLOD);
	VisibleChunksCollisionsMaxLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(VisibleChunksCollisionsMaxLOD);
	VisibleChunksNavmeshMaxLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(VisibleChunksNavmeshMaxLOD);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRuntimeData::FVoxelRuntimeData()
	: InvokersPositionsForPriorities(MakeVoxelShared<FInvokerPositionsArray>(32))
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelRuntime> FVoxelRuntime::Create(const FVoxelRuntimeSettings& Settings)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	const auto Runtime = FVoxelUtilities::MakeGameThreadDeleterPtr<FVoxelRuntime>();
	{
		const TGuardValue<bool> InitGuard(Runtime->bIsInit, true);

		// Add static subsystems
		{
			TArray<UClass*> Classes;
			GetDerivedClasses(UVoxelStaticSubsystemProxy::StaticClass(), Classes);

			for (auto* Class : Classes)
			{
				if (!Class->HasAnyClassFlags(CLASS_Abstract))
				{
					Runtime->AddSubsystem(Class, Settings);
				}
			}
		}

		// Add dynamic subsystems
		{
			const auto AddMainSubsystem = [&](UClass* Class, UClass* DefaultClass)
			{
				if (!Class || Class->HasAnyClassFlags(CLASS_Abstract))
				{
					Class = DefaultClass;
				}

				Runtime->AddSubsystem(Class, Settings);
			};

			AddMainSubsystem(Settings.RendererSubsystem, UVoxelDefaultRendererSubsystemProxy::StaticClass());
			AddMainSubsystem(Settings.LODSubsystem, UVoxelDefaultLODSubsystemProxy::StaticClass());
		}

		for (auto& Subsystem : Runtime->AllSubsystems)
		{
			ensure(Runtime->SubsystemsBeingInitialized.Num() == 0);
			Runtime->InitializeSubsystem(Subsystem);
			ensure(Runtime->SubsystemsBeingInitialized.Num() == 0);
		}
		ensure(Runtime->InitializedSubsystems.Num() == Runtime->AllSubsystems.Num());
		// Note: we can't clear InitializedSubsystems, due to RecreateSubsystem
	}
	
	for (auto& Subsystem : Runtime->AllSubsystems)
	{
		Subsystem->PostCreate();
	}
	
	return Runtime;
}

FVoxelRuntime::~FVoxelRuntime()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!ensure(bIsDestroyed))
	{
		Destroy();
	}
}

void FVoxelRuntime::Destroy()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	ensure(!bIsDestroyed);
	bIsDestroyed = true;
	
	for (auto& Subsystem : AllSubsystems)
	{
		ensure(Subsystem->State == IVoxelSubsystem::EState::Before_Destroy);
		Subsystem->Destroy();
		ensure(Subsystem->State == IVoxelSubsystem::EState::Destroy);
	}
}

void FVoxelRuntime::InitializeSubsystem(const TVoxelSharedPtr<IVoxelSubsystem>& Subsystem)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	check(bIsInit);
	check(Subsystem);
	
	if (InitializedSubsystems.Contains(Subsystem))
	{
		return;
	}
	
	checkf(!SubsystemsBeingInitialized.Contains(Subsystem), TEXT("Recursive dependencies!"));
	SubsystemsBeingInitialized.Add(Subsystem);
	
	ensure(Subsystem->State == IVoxelSubsystem::EState::Before_Create);
	Subsystem->Create();
	ensure(Subsystem->State == IVoxelSubsystem::EState::Create);

	SubsystemsBeingInitialized.Remove(Subsystem);
	InitializedSubsystems.Add(Subsystem);
}

void FVoxelRuntime::RecreateSubsystem(TVoxelSharedPtr<IVoxelSubsystem> OldSubsystem, const FVoxelRuntimeSettings& Settings)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	if (!OldSubsystem)
	{
		// Allowed: useful to recreate subsystems whose class didn't want to be created
		return;
	}
	
	FScopeLock Lock(&RecreateSection);
	
	OldSubsystem->Destroy();
	AllSubsystems.Remove(OldSubsystem);

	for (auto It = SubsystemsMap.CreateIterator(); It; ++It)
	{
		if (It.Value() == OldSubsystem)
		{
			It.RemoveCurrent();
		}
	}
	
	UClass* Class = OldSubsystem->GetProxyClass();
	OldSubsystem.Reset();

	TVoxelSharedPtr<IVoxelSubsystem> NewSubsystem;
	{
		check(!bIsInit);
		const TGuardValue<bool> InitGuard(bIsInit, true);

		NewSubsystem = AddSubsystem(Class, Settings);
		if (!NewSubsystem)
		{
			// ShouldCreateSubsystem returned false
			return;
		}

		ensure(SubsystemsBeingInitialized.Num() == 0);
		InitializeSubsystem(NewSubsystem);
		ensure(SubsystemsBeingInitialized.Num() == 0);
	}
	NewSubsystem->PostCreate();
}

TVoxelSharedPtr<IVoxelSubsystem> FVoxelRuntime::AddSubsystem(UClass* Class, const FVoxelRuntimeSettings& Settings)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	check(Class && !Class->HasAnyClassFlags(CLASS_Abstract));

	const bool bStaticSubsystem = Class->IsChildOf<UVoxelStaticSubsystemProxy>();
	check(bStaticSubsystem || Class->IsChildOf<UVoxelDynamicSubsystemProxy>());
	
	if (bStaticSubsystem && !Class->GetDefaultObject<UVoxelStaticSubsystemProxy>()->ShouldCreateSubsystem(*this, Settings))
	{
		return nullptr;
	}

	const TVoxelSharedRef<IVoxelSubsystem> Subsystem = Class->GetDefaultObject<UVoxelSubsystemProxy>()->GetSubsystem(*this, Settings);
	AllSubsystems.Add(Subsystem);

	// Add to the whole hierarchy so that we can query using parent classes
	for (UClass* ClassIt = Class;
		ClassIt != UVoxelStaticSubsystemProxy::StaticClass() &&
		ClassIt != UVoxelDynamicSubsystemProxy::StaticClass();
		ClassIt = ClassIt->GetSuperClass())
	{
		ensure(!SubsystemsMap.Contains(ClassIt));
		SubsystemsMap.Add(ClassIt, Subsystem);
	}

	return Subsystem;
}