// Copyright 2019 Phyronnaz

#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "IVoxelPool.h"
#include "VoxelDefaultPools.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/VoxelRenderFactory.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelData/VoxelCacheManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelMaterialCollection.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Logging/MessageLog.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/PlatformFilemanager.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#define LOCTEXT_NAMESPACE "Voxel"

DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::Tick"), STAT_VoxelWorld_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateWorldInternal"), STAT_AVoxelWorld_CreateWorldInternal, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::DestroyWorldInternal"), STAT_AVoxelWorld_DestroyWorldInternal, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::DestroyVoxelComponents"), STAT_AVoxelWorld_DestroyVoxelComponents, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::LoadFromSaveObject"), STAT_AVoxelWorld_LoadFromSaveObject, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::ApplyPlaceableItems"), STAT_AVoxelWorld_ApplyPlaceableItems, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateMultiplayerManager"), STAT_AVoxelWorld_CreateMultiplayerManager, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateSpawnerManager"), STAT_AVoxelWorld_CreateSpawnerManager, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateCacheManager"), STAT_AVoxelWorld_CreateCacheManager, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateLODManager"), STAT_AVoxelWorld_CreateLODManager, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateRenderer"), STAT_AVoxelWorld_CreateRenderer, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreatePool"), STAT_AVoxelWorld_CreatePool, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateData"), STAT_AVoxelWorld_CreateData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::CreateDebugManager"), STAT_AVoxelWorld_CreateDebugManager, STATGROUP_Voxel);

#define CHECK_IS_CREATED_IMPL(Name, ReturnValue) if (!IsCreated()) { VoxelLogBlueprintError(FText::Format(LOCTEXT("VoxelWorldIsntCreated", "{}: Voxel World isn't created!"), FText::FromString(FString(Name)))); return ReturnValue; }
#define CHECK_ISNT_CREATED_IMPL(Name, ReturnValue) if (IsCreated()) { VoxelLogBlueprintError(FText::Format(LOCTEXT("VoxelWorldIsCreated", "{}: Voxel World is created!"), FText::FromString(FString(Name)))); return ReturnValue; }

#define CHECK_IS_CREATED() CHECK_IS_CREATED_IMPL(__FUNCTION__, {});
#define CHECK_ISNT_CREATED() CHECK_ISNT_CREATED_IMPL(__FUNCTION__, {});

#define CHECK_IS_CREATED_VOID() CHECK_IS_CREATED_IMPL(__FUNCTION__,);
#define CHECK_ISNT_CREATED_VOID() CHECK_ISNT_CREATED_IMPL(__FUNCTION__,);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void FixWorldBoundsMinMax(int32& Min, int32& Max)
{
	if (Min >= Max)
	{
		if (Max > 0)
		{
			Min = Max - CHUNK_SIZE;
		}
		else
		{
			Max = Min + CHUNK_SIZE;
		}
	}
}

inline void FixWorldBounds(FIntBox& WorldBounds, int32 Depth)
{
	FixWorldBoundsMinMax(WorldBounds.Min.X, WorldBounds.Max.X);
	FixWorldBoundsMinMax(WorldBounds.Min.Y, WorldBounds.Max.Y);
	FixWorldBoundsMinMax(WorldBounds.Min.Z, WorldBounds.Max.Z);
	if (!WorldBounds.IsMultipleOf(CHUNK_SIZE))
	{
		WorldBounds.MakeMultipleOfInclusive(CHUNK_SIZE);
	}
	WorldBounds = FVoxelUtilities::GetBoundsFromDepth<CHUNK_SIZE>(Depth).Overlap(WorldBounds);
	check(WorldBounds.IsMultipleOf(CHUNK_SIZE));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelWorld::AVoxelWorld()
	: LODDynamicSettings(MakeShared<FVoxelLODDynamicSettings>())
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	auto* Box = CreateDefaultSubobject<UBoxComponent>("VoxelWorldPrimitiveComponent");
	Box->SetBoxExtent(FVector::ZeroVector, false);
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrimitiveComponent = Box;

#if WITH_EDITOR
	auto* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> SpriteFinder(TEXT("/Engine/EditorResources/S_Terrain"));
		SpriteComponent->Sprite = SpriteFinder.Object;
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SpriteInfo.Category = TEXT("Voxel World");
		SpriteComponent->SpriteInfo.DisplayName = LOCTEXT("VoxelWorld", "Voxel World");
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->bReceivesDecals = false;
	}
#endif
}

AVoxelWorld::~AVoxelWorld()
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::CreateWorld()
{
	if (IsCreated())
	{
		FMessageLog("PIE").Error(LOCTEXT("CantCreateWorld", "Can't create world: already created"));
		return;
	}
	bIsPreviewing = false;
	CreateWorldInternal();
	if (bUseCameraIfNoInvokersFound && UVoxelInvokerComponent::GetInvokers(GetWorld()).Num() == 0)
	{
		auto NetMode = GetWorld()->GetNetMode();
		if (NetMode != ENetMode::NM_Standalone)
		{
			if (NetMode != ENetMode::NM_DedicatedServer) // Not spawned yet
			{
				FMessageLog("PIE").Warning(LOCTEXT("CantUseCameraAsInvokerInMultiplayer", "Can't use camera as invoker in multiplayer!"));
			}
		}
		else
		{
			auto* CameraInvoker = NewObject<UVoxelInvokerAutoCameraComponent>(this, NAME_None, RF_Transient);
			CameraInvoker->SetupAttachment(GetRootComponent(), NAME_None);
			CameraInvoker->RegisterComponent();

			UE_LOG(LogVoxel, Log, TEXT("No Voxel Invoker found, using camera as invoker"));
		}
	}
}

void AVoxelWorld::DestroyWorld()
{
	if (!IsCreated())
	{
		FMessageLog("PIE").Error(LOCTEXT("CantDestroyWorld", "Can't destroy world: not created"));
		return;
	}

	OnWorldDestroyed.Broadcast();

	DestroyWorldInternal();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelWorldGeneratorInit AVoxelWorld::GetInitStruct() const
{
	return FVoxelWorldGeneratorInit(Seeds, VoxelSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SetOctreeDepth(int32 NewDepth)
{
	CHECK_ISNT_CREATED_VOID();
	OctreeDepth = FVoxelUtilities::ClampDataDepth(NewDepth);
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(NewDepth);
}

void AVoxelWorld::SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator)
{
	CHECK_ISNT_CREATED_VOID();
	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Object;
	WorldGenerator.WorldGeneratorObject = NewGenerator;
}

void AVoxelWorld::SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass)
{
	CHECK_ISNT_CREATED_VOID();
	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Class;
	WorldGenerator.WorldGeneratorClass = NewGeneratorClass;
}

void AVoxelWorld::AddSeeds(const TMap<FString, int32>& NewSeeds)
{
	CHECK_ISNT_CREATED_VOID();
	Seeds.Append(NewSeeds);
}

void AVoxelWorld::AddSeed(FString Name, int32 Value)
{
	CHECK_ISNT_CREATED_VOID();
	Seeds.Add(Name, Value);
}

void AVoxelWorld::ClearSeeds()
{
	CHECK_ISNT_CREATED_VOID();
	Seeds.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector AVoxelWorld::GlobalToLocal(const FVector& Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position) / VoxelSize;

	FIntVector LocalPosition;

	switch (RenderType)
	{
	case EVoxelRenderType::Cubic:
	{
		LocalPosition = FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z));
		break;
	}
	case EVoxelRenderType::MarchingCubes:
	default:
	{
		LocalPosition = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
		break;
	}
	}

	return LocalPosition - *WorldOffset;
}

FVector AVoxelWorld::GlobalToLocalFloat(const FVector& Position) const
{
	return GetTransform().InverseTransformPosition(Position) / VoxelSize - FVector(*WorldOffset);
}

FVector AVoxelWorld::LocalToGlobal(const FIntVector& Position) const
{
	return GetTransform().TransformPosition(VoxelSize * FVector(Position + *WorldOffset));
}

FVector AVoxelWorld::LocalToGlobalFloat(const FVector& Position) const
{
	return GetTransform().TransformPosition(VoxelSize * (Position + FVector(*WorldOffset)));
}

TArray<FIntVector> AVoxelWorld::GetNeighboringPositions(const FVector& GlobalPosition) const
{
	FVector P = GlobalToLocalFloat(GlobalPosition);
	return TArray<FIntVector>({
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z))
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor)
{
	CHECK_IS_CREATED_VOID();

	if (bMoveActor)
	{
		SetActorLocation(GetTransform().TransformPosition(VoxelSize * FVector(OffsetInVoxels)));
	}
	*WorldOffset -= OffsetInVoxels;
	Renderer->RecomputeMeshPositions();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	UpdateCollisionProfile();
	
	if (!IsCreated() && bCreateWorldAutomatically)
	{
		CreateWorld();
	}
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (IsCreated())
	{
		DestroyWorld();
	}
}

void AVoxelWorld::BeginDestroy()
{
	Super::BeginDestroy();

	if (IsCreated())
	{
		DestroyWorld();
	}
}

void AVoxelWorld::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_Tick);

	Super::Tick(DeltaTime);

	if (IsCreated())
	{
		DebugManager->Tick(DeltaTime, this);
		if (bIsPreviewing && Data->IsDirty())
		{
			MarkPackageDirty();
		}
	}
}

void AVoxelWorld::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Temp fix
	if (!IsTemplate())
	{
		CollisionPresets.FixupData(this);
	}
}

void AVoxelWorld::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	if (!IsCreated() || !bEnableWorldRebasing)
	{
		Super::ApplyWorldOffset(InOffset, bWorldShift);
	}
	else
	{
		const FVector RelativeOffset = InOffset / VoxelSize;
		const FIntVector IntegerOffset = FVoxelUtilities::RoundToInt(RelativeOffset);
		const FVector GlobalIntegerOffset = (FVector)IntegerOffset * VoxelSize;
		const FVector Diff = InOffset - GlobalIntegerOffset;

		Super::ApplyWorldOffset(Diff, bWorldShift);

		*WorldOffset += IntegerOffset;

		Renderer->RecomputeMeshPositions();
	}
}

void AVoxelWorld::PostLoad()
{
	Super::PostLoad();
	
	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}

	OctreeDepth = FVoxelUtilities::ClampDataDepth(OctreeDepth);
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
	MaxCacheSizeInMB = FVoxelDataCellUtilities::GetCacheSizeInMB(MaxCacheSize);
}

#if WITH_EDITOR
bool AVoxelWorld::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	MaxCacheSizeInMB = FVoxelDataCellUtilities::GetCacheSizeInMB(MaxCacheSize);
	
	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}

	if (auto* Property = PropertyChangedEvent.MemberProperty)
	{
		FName Name = Property->GetFName();
		if (Name == GET_MEMBER_NAME_CHECKED(AVoxelWorld, OctreeDepth))
		{
			OctreeDepth = FVoxelUtilities::ClampDataDepth(OctreeDepth);
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_CHECKED(AVoxelWorld, WorldSizeInVoxel))
		{
			OctreeDepth = FVoxelUtilities::ClampDataDepth(FVoxelUtilities::GetDepthFromSize<CHUNK_SIZE>(WorldSizeInVoxel));
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_CHECKED(AVoxelWorld, LODToMinDistance))
		{
			if (!LODToMinDistance.Contains("0"))
			{
				LODToMinDistance.Add("0", 1000);
			}

			for (auto& It : LODToMinDistance)
			{
				if (!It.Key.IsEmpty())
				{
					It.Key = FString::FromInt(FVoxelUtilities::ClampChunkDepth(TCString<TCHAR>::Atoi(*It.Key)));
				}
			}
			LODToMinDistance.KeySort([](const FString& A, const FString& B) { return TCString<TCHAR>::Atoi(*A) < TCString<TCHAR>::Atoi(*B); });

			float LastValue = 0;
			for (auto& It : LODToMinDistance)
			{
				It.Value = FMath::Max(LastValue, It.Value);
				LastValue = It.Value;
			}
		}

		if (IsCreated() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			if (Property->HasMetaData("Recreate"))
			{
				DestroyWorldInternal();
				CreateWorldInternal();

				OnRecreate.Broadcast();
			}
			else if (Property->HasMetaData("RecreateRender"))
			{
				SyncRefs();
				ensure(LODManager.GetSharedReferenceCount() == 1);
				LODManager.Reset();
				ensure(Renderer.GetSharedReferenceCount() == 1);
				Renderer.Reset();
				
				DestroyVoxelComponents();
				
				Renderer = CreateRenderer();
				LODManager = CreateLODManager();

				OnRecreateRender.Broadcast();
			}
			else if (Property->HasMetaData("UpdateAll"))
			{
				SyncRefs();
				GetLODManager().UpdateBounds(FIntBox::Infinite);

				OnUpdateAll.Broadcast();
			}
			else if (Property->HasMetaData("UpdateLODs"))
			{
				SyncRefs();
				GetLODManager().ForceLODsUpdate();

				OnUpdateLODs.Broadcast();
			}
		}
	}
	
	FixWorldBounds(CustomWorldBounds, OctreeDepth);
	
	// Call it AFTER possibly destroying components, as it registers all components
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AVoxelWorld::UpdateCollisionProfile()
{
#if WITH_EDITOR
	// This is needed to restore transient collision profile data.
	CollisionPresets.LoadProfileData(false);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SyncRefs()
{
	LODDynamicSettings->LODLimit = FVoxelUtilities::ClampChunkDepth(LODLimit);
	LODDynamicSettings->LODToMinDistance.Reset();
	for (auto& It : LODToMinDistance)
	{
		LODDynamicSettings->LODToMinDistance.Add(TCString<TCHAR>::Atoi(*It.Key), It.Value);
	}
	
	LODDynamicSettings->InvokerDistanceThreshold = InvokerDistanceThreshold;
	
	LODDynamicSettings->ChunksCullingLOD = FVoxelUtilities::ClampChunkDepth(ChunksCullingLOD);

	LODDynamicSettings->bEnableRender = bRenderWorld;
	
	LODDynamicSettings->bEnableCollisions = bIsPreviewing ? true : bEnableCollisions;
	LODDynamicSettings->bComputeVisibleChunksCollisions = bIsPreviewing ? true : bComputeVisibleChunksCollisions;
	LODDynamicSettings->VisibleChunksCollisionsMaxLOD = bIsPreviewing ? 5 : FVoxelUtilities::ClampChunkDepth(VisibleChunksCollisionsMaxLOD);
	
	LODDynamicSettings->bEnableNavmesh = bEnableNavmesh;

	LODDynamicSettings->bEnableTessellation = bEnableTessellation;
	LODDynamicSettings->TessellationDistance = TessellationDistance;
}

FIntBox AVoxelWorld::GetWorldBounds() const
{
	FIntBox Bounds = FVoxelUtilities::GetBoundsFromDepth<CHUNK_SIZE>(OctreeDepth);
	if (bUseCustomWorldBounds)
	{
		Bounds = CustomWorldBounds;
	}
	FixWorldBounds(Bounds, OctreeDepth);
	return Bounds;
}


void AVoxelWorld::OnWorldLoadedCallback()
{
	UE_LOG(LogVoxel, Log, TEXT("%s took %fs to generate"), *GetName(), FPlatformTime::Seconds() - TimeOfCreation);
	OnWorldLoaded.Broadcast();
}

TSharedRef<FVoxelDebugManager, ESPMode::ThreadSafe> AVoxelWorld::CreateDebugManager() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateDebugManager);
	return FVoxelDebugManager::Create();
}

TSharedRef<FVoxelData, ESPMode::ThreadSafe> AVoxelWorld::CreateData() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateData);

	FVoxelDataSettings Settings;

	Settings.Depth = FVoxelUtilities::ClampDataDepth(FVoxelUtilities::GetDataDepthFromChunkDepth(OctreeDepth));
	Settings.WorldBounds = GetWorldBounds();

	auto WorldGeneratorInstance = WorldGenerator.GetWorldGenerator();
	WorldGeneratorInstance->Init(GetInitStruct());
	Settings.WorldGenerator = WorldGeneratorInstance;
	
	Settings.bEnableMultiplayer = false;
	
	Settings.bEnableUndoRedo = bEnableUndoRedo || bIsPreviewing;
	Settings.bCacheLOD0Chunks = bEnableAutomaticCache && bCacheLOD0Chunks;

	return FVoxelData::Create(Settings);
}

TSharedRef<IVoxelPool> AVoxelWorld::CreatePool() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreatePool);

	if (bCreateGlobalPool)
	{
		if (IVoxelPool::IsGlobalVoxelPoolCreated())
		{
			FMessageLog("PIE").Info(LOCTEXT("GlobalPoolCreated", "CreateGlobalPool = true but global pool is already created! Using existing one NOTE: THIS WARNING IS OBSOLETE"));
		}
		}
		else
		{
		if (!IVoxelPool::IsGlobalVoxelPoolCreated())
		{
			FMessageLog("PIE").Info(LOCTEXT("GlobalPoolNotCreated", "CreateGlobalPool = false but global pool isn't created! Creating it NOTE: THIS WARNING IS OBSOLETE"));
		}
	}

	if (!IVoxelPool::IsGlobalVoxelPoolCreated())
	{
		FVoxelDefaultPool::CreateGlobalPool(FMath::Max(1, MeshThreadCount));
	}

	return IVoxelPool::GetGlobalPool().ToSharedRef();
}

TSharedRef<IVoxelRenderer> AVoxelWorld::CreateRenderer() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateRenderer);

	check(Data.IsValid());
	check(Pool.IsValid());
	check(DebugManager.IsValid());

	FVoxelRendererSettings Settings{};
	Settings.VoxelSize = VoxelSize;
	Settings.OctreeDepth = FVoxelUtilities::ClampChunkDepth(OctreeDepth);
	Settings.WorldBounds = GetWorldBounds();
	Settings.ProcMeshClass = (ProcMeshClass && !bIsPreviewing) ? (UClass*)ProcMeshClass : UVoxelProceduralMeshComponent::StaticClass();
	Settings.WorldOffset = WorldOffset;
	
	Settings.World = GetWorld();
	Settings.ComponentsOwner = const_cast<AVoxelWorld*>(this);
	Settings.Data = Data.ToSharedRef();
	Settings.Pool = Pool.ToSharedRef();
	Settings.DebugManager = DebugManager.ToSharedRef();

	Settings.UVConfig = UVConfig;
	Settings.UVScale = UVScale;
	Settings.NormalConfig = NormalConfig;
	Settings.MaterialConfig = MaterialConfig;
	Settings.VoxelMaterialWithoutTessellation = VoxelMaterial;
	Settings.VoxelMaterialWithTessellation = TessellatedVoxelMaterial;
	Settings.MaterialCollection = MaterialCollection;

	Settings.TessellationBoundsExtension = TessellationBoundsExtension;
	Settings.WaitForOtherChunksToAvoidHolesTimeout = WaitForOtherChunksToAvoidHolesTimeout;

	Settings.CollisionPresets = CollisionPresets;

	Settings.RenderType = RenderType;
	Settings.ChunksDitheringDuration = bIsPreviewing ? 0.1 : ChunksDitheringDuration;
	Settings.bOptimizeIndices = bOptimizeIndices;

	return FVoxelRenderFactory::GetVoxelRenderer(RenderType, Settings);
}

TSharedRef<IVoxelLODManager> AVoxelWorld::CreateLODManager() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateLODManager);

	check(Renderer.IsValid());
	check(Pool.IsValid());
	check(DebugManager.IsValid());

	FVoxelLODSettings Settings{};
	Settings.VoxelSize = VoxelSize;
	Settings.OctreeDepth = FVoxelUtilities::ClampChunkDepth(OctreeDepth);
	Settings.WorldBounds = GetWorldBounds();
	Settings.LODUpdateRate = LODUpdateRate;
	
	Settings.World = GetWorld();
	Settings.Renderer = Renderer.ToSharedRef();
	Settings.Pool = Pool.ToSharedRef();
	Settings.DebugManager = DebugManager.ToSharedRef();

	Settings.bWaitForOtherChunksToAvoidHoles = !bIsPreviewing && bWaitForOtherChunksToAvoidHoles;

	return FVoxelDefaultLODManager::Create(Settings, this, LODDynamicSettings);
}

TSharedRef<FVoxelCacheManager> AVoxelWorld::CreateCacheManager() const
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateCacheManager);

	check(Data.IsValid());
	check(DebugManager.IsValid());
	check(Pool.IsValid());

	FVoxelCacheSettings Settings{};
	Settings.DataOctreeCompactDelayInSeconds = DataOctreeCompactDelayInSeconds;
	Settings.bEnableAutomaticCache = bEnableAutomaticCache;
	Settings.CacheUpdateDelayInSeconds = CacheUpdateDelayInSeconds;
	Settings.CacheAccessThreshold = CacheAccessThreshold;
	Settings.MaxCacheSize = MaxCacheSize;

	Settings.Data = Data.ToSharedRef();
	Settings.DebugManager = DebugManager.ToSharedRef();
	Settings.Pool = Pool.ToSharedRef();
	Settings.World = GetWorld();

	return FVoxelCacheManager::Create(Settings);
}


void AVoxelWorld::CreateWorldInternal()
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_CreateWorldInternal);

	check(!IsCreated());

	UE_LOG(LogVoxel, Log, TEXT("Loading world"));

	bIsCreated = true;
	TimeOfCreation = FPlatformTime::Seconds();

	*WorldOffset = FIntVector::ZeroValue;
	SyncRefs();
	
	DebugManager = CreateDebugManager();
	Data = CreateData();
	Pool = CreatePool();
	Renderer = CreateRenderer();
	Renderer->OnWorldLoaded.AddUObject(this, &AVoxelWorld::OnWorldLoadedCallback);
	LODManager = CreateLODManager();
	CacheManager = CreateCacheManager();

	// Load if possible
	if (SaveObject)
	{
		LoadFromSaveObject();
	}
	
	// Add placeable items AFTER loading
	ApplyPlaceableItems();
}

void AVoxelWorld::DestroyWorldInternal()
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_DestroyWorldInternal);

	check(IsCreated());

#if WITH_EDITOR
	if (bIsPreviewing)
	{
		SaveData();
	}
#endif

	UE_LOG(LogVoxel, Log, TEXT("Unloading world"));

	bIsCreated = false;

	DebugManager.Reset();
	Data.Reset();
	Pool.Reset();
	Renderer.Reset();
	LODManager.Reset();
	CacheManager.Reset();

	DestroyVoxelComponents();
}

void AVoxelWorld::DestroyVoxelComponents()
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_DestroyVoxelComponents);

	auto Components = GetComponents(); // need a copy as we are modifying it when destroying comps
	for (auto& Component : Components)
	{
		if (Component->HasAnyFlags(RF_Transient) && (Component->IsA<UVoxelProceduralMeshComponent>() || Component->IsA<UHierarchicalInstancedStaticMeshComponent>()))
		{
			Component->DestroyComponent();
		}
	}
}

void AVoxelWorld::LoadFromSaveObject()
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_LoadFromSaveObject);

	check(SaveObject);
	
	FVoxelUncompressedWorldSave Save;
	UVoxelSaveUtilities::DecompressVoxelSave(SaveObject->Save, Save);

	if (Save.GetDepth() == -1)
	{
		FMessageLog("PIE").Error(LOCTEXT("LoadFromSaveObjectInvalidDepth", "Invalid Save Object!"));
		return;
	}
	if (Save.GetDepth() > Data->Depth)
	{
		FMessageLog("PIE").Info(LOCTEXT("LoadFromSaveObjectDepthBigger", "Save Object depth is bigger than world depth, the save data outside world bounds will be ignored"));
	}

	TArray<FIntBox> BoundsToUpdate;
	Data->LoadFromSave(Save, BoundsToUpdate);
	LODManager->UpdateBounds(BoundsToUpdate);
}

void AVoxelWorld::ApplyPlaceableItems()
{
	SCOPE_CYCLE_COUNTER(STAT_AVoxelWorld_ApplyPlaceableItems);

}

#if WITH_EDITOR
void AVoxelWorld::CreateForPreview(UClass* VoxelWorldEditorClass)
{
	check(VoxelWorldEditorClass);

	if (!VoxelWorldEditor)
	{
		// Create/Find VoxelWorldEditor
		for (TActorIterator<AActor> It(GetWorld(), VoxelWorldEditorClass); It; ++It)
		{
			VoxelWorldEditor = *It;
			break;
		}
		if (!VoxelWorldEditor)
		{
			FActorSpawnParameters Transient;
			Transient.ObjectFlags = RF_Transient;
			VoxelWorldEditor = GetWorld()->SpawnActor<AActor>(VoxelWorldEditorClass, Transient);
		}
	}

	IVoxelWorldEditor::GetVoxelWorldEditor()->BindEditorDelegates(this);

	if (IsCreated())
	{
		DestroyWorldInternal();
	}
	bIsPreviewing = true;
	CreateWorldInternal();
}

void AVoxelWorld::SaveData()
{
	check(IsCreated());
	if (Data->IsDirty())
	{
		if (!SaveObject && ensure(IVoxelWorldEditor::GetVoxelWorldEditor()))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedToCreateSaveObject", "The voxel world Save Object is null. You need to create one now if you don't want to loose your changes."));
			Modify();
			SaveObject = IVoxelWorldEditor::GetVoxelWorldEditor()->CreateSaveObject();
		}
		if (SaveObject)
		{
			FVoxelUncompressedWorldSave Save;
			Data->GetSave(Save);
			UVoxelSaveUtilities::CompressVoxelSave(Save, SaveObject->Save);
			SaveObject->PostEditChange(); // Fixup depth
			SaveObject->MarkPackageDirty();

			FNotificationInfo Info = FNotificationInfo(LOCTEXT("VoxelWorldSaved", "Voxel world saved!"));
			Info.CheckBoxState = ECheckBoxState::Checked;
			FSlateNotificationManager::Get().AddNotification(Info);

			Data->ClearDirtyFlag();
		}
		else
		{
			FNotificationInfo Info = FNotificationInfo(LOCTEXT("VoxelWorldSaved", "Voxel world not saved: no Save Object!"));
			Info.CheckBoxState = ECheckBoxState::Unchecked;
			FSlateNotificationManager::Get().AddNotification(Info);
		}

		if (bAutomaticallySaveToFile)
		{
			FText Error;
			if (!SaveToFile(GetDefaultFilePath(), Error))
			{
				FMessageDialog::Open(EAppMsgType::Ok, Error);
			}
		}
	}
}

inline bool CanLoad(const TSharedPtr<FVoxelData, ESPMode::ThreadSafe>& Data)
{
	if (Data->IsDirty())
	{
		auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("WorldDetailsUnsavedChanges", "There are unsaved changes. Loading will overwrite them. Confirm?"));
		if (Result != EAppReturnType::Yes)
		{
			return false;
		}
	}
	return true;
}

void AVoxelWorld::LoadFromSaveObjectEditor()
{
	check(SaveObject);
	if (!CanLoad(Data))
	{
		return;
	}
	
	LoadFromSaveObject();

	FNotificationInfo Info(LOCTEXT("SuccessfullyLoaded", "Loaded!"));
	FSlateNotificationManager::Get().AddNotification(Info);
}

bool AVoxelWorld::SaveToFile(const FString& Path, FText& Error)
{
	check(IsCreated());

	if (Path.IsEmpty())
	{
		Error = LOCTEXT("EmptyPath", "Empty Save File Path");
		return false;
	}
	else if (!FPaths::ValidatePath(Path, &Error))
	{
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(Path));

	FBufferArchive Archive;
	FVoxelCompressedWorldSave CompressedSave;
	UVoxelDataTools::GetCompressedSave(this, CompressedSave);
	CompressedSave.Serialize(Archive);

	if (FFileHelper::SaveArrayToFile(Archive, *Path))
	{
		FNotificationInfo Info(FText::Format(LOCTEXT("SuccessfullyCreated", "{0} was successfully created"), FText::FromString(Path)));
		FSlateNotificationManager::Get().AddNotification(Info);
		return true;
	}
	else
	{
		Error = FText::Format(LOCTEXT("SaveError", "Error when creating {0}"), FText::FromString(Path));
		return false;
	}
}

bool AVoxelWorld::LoadFromFile(const FString& Path, FText& Error)
{
	if (!CanLoad(Data))
	{
		Error = LOCTEXT("Canceled", "Canceled");
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<uint8> Array;
	if (!FFileHelper::LoadFileToArray(Array, *Path))
	{
		Error = FText::Format(LOCTEXT("LoadError", "Error when reading {0}"), FText::FromString(Path));
		return false;
	}

	FMemoryReader Reader(Array);
	FVoxelCompressedWorldSave CompressedSave;
	CompressedSave.Serialize(Reader);
	UVoxelDataTools::LoadFromCompressedSave(this, CompressedSave);

	FNotificationInfo Info(FText::Format(LOCTEXT("SuccessfullyLoaded", "{0} was successfully loaded"), FText::FromString(Path)));
	FSlateNotificationManager::Get().AddNotification(Info);
	return true;
}

FString AVoxelWorld::GetDefaultFilePath() const
{
	FString Path = SaveFilePath.FilePath;
	if (Path.IsEmpty())
	{
		return Path;
	}
	Path.RemoveFromEnd(".voxelsave");
	FString Sep = TEXT("_");
	int32 Year = 0, Month = 0, DayOfWeek = 0, Day = 0, Hour = 0, Min = 0, Sec = 0, MSec = 0;
	FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);
	FString Date = FString::FromInt(Year) +
		Sep + FString::FromInt(Month) +
		Sep + FString::FromInt(Day) +
		Sep + FString::FromInt(Hour) +
		Sep + FString::FromInt(Min) +
		Sep + FString::FromInt(Sec) +
		Sep + FString::FromInt(MSec);
	Path += Date;
	Path += ".voxelsave";
	return Path;
}

void AVoxelWorld::OnPreSaveWorld(uint32 SaveFlags, UWorld* World)
{
	if (IsCreated() && ensure(bIsPreviewing))
	{
		SaveData();
	}
}

void AVoxelWorld::OnPreBeginPIE(bool bIsSimulating)
{
	if (IsCreated() && ensure(bIsPreviewing))
	{
		DestroyWorldInternal();
	}
}

void AVoxelWorld::OnEndPIE(bool bIsSimulating)
{
	if (ensure(!IsCreated()) && bIsPreviewing)
	{
		CreateWorldInternal();
	}
}

void AVoxelWorld::OnPrepareToCleanseEditorObject(UObject* Object)
{
	if (IsCreated() && ensure(bIsPreviewing))
	{
		DestroyWorldInternal();
	}
}

void AVoxelWorld::OnPreExit()
{
	bIsPreviewing = false; // Disable saving data
}

void IVoxelWorldEditor::SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor)
{
	check(!VoxelWorldEditor.IsValid());
	VoxelWorldEditor = InVoxelWorldEditor;
}

TSharedPtr<IVoxelWorldEditor> IVoxelWorldEditor::VoxelWorldEditor;
#endif

#undef LOCTEXT_NAMESPACE