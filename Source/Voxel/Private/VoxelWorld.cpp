// Copyright 2020 Phyronnaz

#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "IVoxelPool.h"
#include "VoxelSettings.h"
#include "VoxelDefaultPool.h"
#include "VoxelWorldRootComponent.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelMaterialCollection.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelMultiplayer/VoxelMultiplayerTcp.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelMessages.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemActor.h"
#include "VoxelPlaceableItems/VoxelAssetActor.h"
#include "VoxelPlaceableItems/VoxelDisableEditsBox.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelProcGen/VoxelProcGenManager.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/NetConnection.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "Misc/UObjectToken.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/FeedbackContext.h"
#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/PlatformFilemanager.h"
#include "TimerManager.h"

#include "Materials/Material.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#define LOCTEXT_NAMESPACE "Voxel"

#define CHECK_IS_CREATED_IMPL(Name, ReturnValue) if (!IsCreated()) { FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World isn't created!"), *FString(Name))); return ReturnValue; }
#define CHECK_IS_NOT_CREATED_IMPL(Name, ReturnValue) if (IsCreated()) { FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World is created!"), *FString(Name))); return ReturnValue; }

#define CHECK_IS_CREATED() CHECK_IS_CREATED_IMPL(__FUNCTION__, {});
#define CHECK_IS_NOT_CREATED() CHECK_IS_NOT_CREATED_IMPL(__FUNCTION__, {});

#define CHECK_IS_CREATED_VOID() CHECK_IS_CREATED_IMPL(__FUNCTION__,);
#define CHECK_IS_NOT_CREATED_VOID() CHECK_IS_NOT_CREATED_IMPL(__FUNCTION__,);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelWorld::AVoxelWorld()
	: LODDynamicSettings(MakeVoxelShared<FVoxelLODDynamicSettings>())
	, RendererDynamicSettings(MakeVoxelShared<FVoxelRendererDynamicSettings>())
{
	MultiplayerInterface = UVoxelMultiplayerTcpInterface::StaticClass();
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = true;

	RootComponent = WorldRoot = CreateDefaultSubobject<UVoxelWorldRootComponent>("Root");

#if WITH_EDITOR
	auto* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> SpriteFinder(TEXT("/Engine/EditorResources/S_Terrain"));
		SpriteComponent->Sprite = SpriteFinder.Object;
		SpriteComponent->SetRelativeScale3D(FVector(0.5f));
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SpriteInfo.Category = TEXT("Voxel World");
		SpriteComponent->SpriteInfo.DisplayName = FText::FromString("Voxel World");
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
	VOXEL_FUNCTION_COUNTER();
	
	if (IsCreated())
	{
		FVoxelMessages::Error("Can't create world: already created");
		return;
	}

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor ||
		GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		// Called in editor - probably by a BP construction script
		// Forward to CreateInEditor
		CreateInEditor();
		return;
	}
#endif
	
	PlayType = EVoxelPlayType::Game;
	CreateWorldInternal();
	if (bUseCameraIfNoInvokersFound && UVoxelInvokerComponent::GetInvokers(GetWorld()).Num() == 0)
	{
		const auto NetMode = GetWorld()->GetNetMode();
		if (NetMode != ENetMode::NM_Standalone)
		{
			if (NetMode != ENetMode::NM_DedicatedServer) // Not spawned yet
			{
				FVoxelMessages::Warning("Voxel World: Can't use camera as invoker in multiplayer! You need to add a VoxelInvokerComponent to your character");
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
	VOXEL_FUNCTION_COUNTER();
	
	if (!IsCreated())
	{
		FVoxelMessages::Error("Can't destroy world: not created");
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
	return FVoxelWorldGeneratorInit(Seeds, VoxelSize, FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth), MaterialConfig, MaterialCollection, this);
}

FIntBox AVoxelWorld::GetWorldBounds() const
{
	if (bUseCustomWorldBounds)
	{
		return 
			FVoxelUtilities::GetCustomBoundsForDepth<RENDER_CHUNK_SIZE>(
				FIntBox::SafeConstruct(CustomWorldBounds.Min, CustomWorldBounds.Max), 
				OctreeDepth);
	}
	else
	{
		return FVoxelUtilities::GetBoundsFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SetOctreeDepth(int32 NewDepth)
{
	CHECK_IS_NOT_CREATED_VOID();
	OctreeDepth = FVoxelUtilities::ClampDataDepth(NewDepth);
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth);
}

void AVoxelWorld::SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator)
{
	CHECK_IS_NOT_CREATED_VOID();
	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Object;
	WorldGenerator.WorldGeneratorObject = NewGenerator;
}

void AVoxelWorld::SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass)
{
	CHECK_IS_NOT_CREATED_VOID();
	WorldGenerator.Type = EVoxelWorldGeneratorPickerType::Class;
	WorldGenerator.WorldGeneratorClass = NewGeneratorClass;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntVector AVoxelWorld::GlobalToLocal(const FVector& Position, EVoxelWorldCoordinatesRounding Rounding) const
{
	if (RenderType == EVoxelRenderType::Cubic)
	{
		Rounding = EVoxelWorldCoordinatesRounding::RoundDown;
	}

	const FVector LocalPosition = GetTransform().InverseTransformPosition(Position) / VoxelSize;

	FIntVector VoxelPosition;
	switch (Rounding)
	{
	case EVoxelWorldCoordinatesRounding::RoundToNearest: VoxelPosition = FVoxelUtilities::RoundToInt(LocalPosition); break;
	case EVoxelWorldCoordinatesRounding::RoundUp: VoxelPosition = FVoxelUtilities::CeilToInt(LocalPosition); break;
	case EVoxelWorldCoordinatesRounding::RoundDown: VoxelPosition = FVoxelUtilities::FloorToInt(LocalPosition); break;
	default: ensure(false);
	}
	return VoxelPosition - *WorldOffset;
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
	return TArray<FIntVector>(FVoxelUtilities::GetNeighbors(GlobalToLocalFloat(GlobalPosition)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SetOffset(const FIntVector& OffsetInVoxels)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_IS_CREATED_VOID();

	*WorldOffset = OffsetInVoxels;
	Renderer->RecomputeMeshPositions();
}

void AVoxelWorld::AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_IS_CREATED_VOID();

	if (bMoveActor)
	{
		SetActorLocation(GetTransform().TransformPosition(VoxelSize * FVector(OffsetInVoxels)));
	}

	SetOffset(*WorldOffset - OffsetInVoxels);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelMultiplayerInterface* AVoxelWorld::CreateMultiplayerInterfaceInstance()
{
	VOXEL_PRO_ONLY();
}

UVoxelMultiplayerInterface* AVoxelWorld::GetMultiplayerInterfaceInstance() const
{
	VOXEL_PRO_ONLY();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::BeginPlay()
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::BeginPlay();

	UpdateCollisionProfile();

	if (!IsCreated() && bCreateWorldAutomatically)
	{
		// Allow other actors begin play to run before creating the world, if they need to create the global pool
		auto& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimerForNextTick(this, &AVoxelWorld::CreateWorld);
	}
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::EndPlay(EndPlayReason);

	if (IsCreated())
	{
		DestroyWorld();
	}
}

void AVoxelWorld::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	if (GetWorld()->WorldType != EWorldType::Editor && GetWorld()->WorldType != EWorldType::EditorPreview) // We don't want to tick the BP in preview
	{
		Super::Tick(DeltaTime);
	}

	if (IsCreated())
	{
		WorldRoot->TickWorldRoot();
#if WITH_EDITOR
		if (PlayType == EVoxelPlayType::Preview && Data->IsDirty())
		{
			MarkPackageDirty();
		}
#endif
	}
}

void AVoxelWorld::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!IsCreated() || !bEnableCustomWorldRebasing)
	{
		Super::ApplyWorldOffset(InOffset, bWorldShift);
	}
	else
	{
		const FVector RelativeOffset = InOffset / VoxelSize;
		const FIntVector IntegerOffset = FVoxelUtilities::RoundToInt(RelativeOffset);
		const FVector GlobalIntegerOffset = FVector(IntegerOffset) * VoxelSize;
		const FVector Diff = InOffset - GlobalIntegerOffset;

		Super::ApplyWorldOffset(Diff, bWorldShift);

		*WorldOffset += IntegerOffset;

		Renderer->RecomputeMeshPositions();
	}
}

void AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (bIsToggled &&
		!IsCreated() &&
		!GetDefault<UVoxelSettings>()->bDisableAutoPreview &&
		(GetWorld()->WorldType == EWorldType::EditorPreview ||
			GetWorld()->WorldType == EWorldType::Editor ||
			GetWorld()->WorldType == EWorldType::GamePreview))
	{
		CreateInEditor();
	}
#endif
}

void AVoxelWorld::UnregisterAllComponents(bool bForReregister)
{
	if (bDisableComponentUnregister)
	{
		Super::UnregisterAllComponents(true); // Do as if it was a reregister so that proc mesh are ignored
	}
	else
	{
		Super::UnregisterAllComponents(bForReregister);
	}
}

#if WITH_EDITOR
bool AVoxelWorld::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void AVoxelWorld::BeginDestroy()
{
	if (IsCreated())
	{
		DestroyWorld();
	}

	// Forward to BeginDestroy AFTER destroying the world, else the components are invalid
	// Note: when exiting the components are still invalid. This doesn't seem to be really useful, but might as well do it here still
	Super::BeginDestroy();
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

void AVoxelWorld::PostLoad()
{
	Super::PostLoad();
	
	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}

	FVoxelDefaultPool::FixPriorityCategories(PriorityCategories);
	FVoxelDefaultPool::FixPriorityOffsets(PriorityOffsets);

	OctreeDepth = FVoxelUtilities::ClampDataDepth(OctreeDepth);
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth);

	if (int32(UVConfig) >= int32(EVoxelUVConfig::Max))
	{
		UVConfig = EVoxelUVConfig::GlobalUVs;
	}
}

#if WITH_EDITOR

void AVoxelWorld::PreEditChange(UProperty* PropertyThatWillChange)
{
	bDisableComponentUnregister = true;
	Super::PreEditChange(PropertyThatWillChange);
	bDisableComponentUnregister = false;
}

void AVoxelWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	if (!ProcMeshClass)
	{
		ProcMeshClass = UVoxelProceduralMeshComponent::StaticClass();
	}

	if (auto* Property = PropertyChangedEvent.MemberProperty)
	{
		const FName Name = Property->GetFName();
		if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, OctreeDepth))
		{
			OctreeDepth = FVoxelUtilities::ClampDataDepth(OctreeDepth);
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, WorldSizeInVoxel))
		{
			OctreeDepth = FVoxelUtilities::ClampDataDepth(FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(WorldSizeInVoxel));
			WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(OctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, LODToMinDistance))
		{
			if (!LODToMinDistance.Contains("0"))
			{
				LODToMinDistance.Add("0", 10000);
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
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, MaterialsHardness))
		{
			for (auto& It : MaterialsHardness)
			{
				if (!It.Key.IsEmpty())
				{
					It.Key = FString::FromInt(FMath::Clamp(TCString<TCHAR>::Atoi(*It.Key), 0, 255));
				}
				else if (It.Value == 0)
				{
					It.Value = 1;
				}
				It.Value = FMath::Max(It.Value, 0.f);
			}
			MaterialsHardness.KeySort([](const FString& A, const FString& B) { return TCString<TCHAR>::Atoi(*A) < TCString<TCHAR>::Atoi(*B); });
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, SpawnersCollisionDistanceInVoxel))
		{
			SpawnersCollisionDistanceInVoxel = FMath::CeilToInt(SpawnersCollisionDistanceInVoxel / 32.f) * 32;
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, ChunksClustersSize))
		{
			ChunksClustersSize = FMath::Max(ChunksClustersSize, RENDER_CHUNK_SIZE);
			const int32 PowerOf2 = FMath::RoundToInt(FMath::Log2(ChunksClustersSize));
			ChunksClustersSize = 1 << PowerOf2;
			ChunksClustersSize = FMath::Max(ChunksClustersSize, RENDER_CHUNK_SIZE);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, PriorityCategories))
		{
			FVoxelDefaultPool::FixPriorityCategories(PriorityCategories);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, PriorityOffsets))
		{
			FVoxelDefaultPool::FixPriorityOffsets(PriorityOffsets);
		}

		OnPropertyChanged_Interactive.Broadcast();
		
		if (IsCreated() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			OnPropertyChanged.Broadcast();
			
			if (Property->HasMetaData("Recreate"))
			{
				DestroyWorldInternal();
				CreateWorldInternal();
			}
			else if (Property->HasMetaData("RecreateRender"))
			{
				UpdateDynamicLODSettings();
				UpdateDynamicRendererSettings();
				
				LODManager->Destroy();
				LODManager.Reset();
				
				Renderer->Destroy();
				Renderer.Reset();
				
				DestroyVoxelComponents();
				
				Renderer = CreateRenderer();
				LODManager = CreateLODManager();
			}
			else if (Property->HasMetaData("UpdateAll"))
			{
				UpdateDynamicLODSettings();
				GetLODManager().UpdateBounds(FIntBox::Infinite);
			}
			else if (Property->HasMetaData("UpdateLODs"))
			{
				UpdateDynamicLODSettings();
				GetLODManager().ForceLODsUpdate();
			}
			else if (Property->HasMetaData("UpdateRenderer"))
			{
				UpdateDynamicRendererSettings();
				GetRenderer().ApplyNewMaterials();
			}
		}
	}
	
	if (bUseCustomWorldBounds)
	{
		CustomWorldBounds = GetWorldBounds();
	}

	bDisableComponentUnregister = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	bDisableComponentUnregister = false;
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

void AVoxelWorld::OnWorldLoadedCallback()
{
	UE_LOG(LogVoxel, Log, TEXT("%s took %fs to generate"), *GetName(), FPlatformTime::Seconds() - TimeOfCreation);
	OnWorldLoaded.Broadcast();
}

TVoxelSharedRef<FVoxelDebugManager> AVoxelWorld::CreateDebugManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelDebugManager::Create(FVoxelDebugManagerSettings(this, PlayType, Pool.ToSharedRef(), Data.ToSharedRef()));
}

TVoxelSharedRef<FVoxelData> AVoxelWorld::CreateData() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelData::Create(FVoxelDataSettings(this, PlayType), DataOctreeInitialSubdivisionDepth);
}

TVoxelSharedRef<IVoxelPool> AVoxelWorld::CreatePool() const
{
	VOXEL_FUNCTION_COUNTER();

	const auto CreateOwnPool = [&](int32 InNumberOfThreads, bool bInConstantPriorities)
	{
		return FVoxelDefaultPool::Create(
			FMath::Max(1, InNumberOfThreads),
			bInConstantPriorities,
			PriorityCategories,
			PriorityOffsets);
	};
	
	if (PlayType == EVoxelPlayType::Preview)
	{
		return CreateOwnPool(NumberOfThreadsForPreview, true);
	}
	else
	{
		if (bCreateGlobalPool)
		{
			if (!IVoxelPool::IsGlobalVoxelPoolCreated(GetWorld()))
			{
				const auto NewPool = CreateOwnPool(NumberOfThreads, bConstantPriorities);
				IVoxelPool::SetGlobalVoxelPool(GetWorld(), NewPool, GetName());
				return NewPool;
			}
			else
			{
				FVoxelMessages::Warning(FString::Printf(TEXT(
					"CreateGlobalPool = true but global pool is already created! Using existing one, NumberOfThreads will be ignored. "
					"Consider setting CreateGlobalPool to false and calling CreateGlobalVoxelThreadPool at BeginPlay (for instance in your level blueprint). "
					"Creator: %s"), *IVoxelPool::GetGlobalPoolCreator(GetWorld())),
					this);
				return IVoxelPool::GetGlobalPool(GetWorld()).ToSharedRef();
			}
		}
		else
		{
			if (IVoxelPool::IsGlobalVoxelPoolCreated(GetWorld()))
			{
				return IVoxelPool::GetGlobalPool(GetWorld()).ToSharedRef();
			}
			else
			{

				FVoxelMessages::Warning(
						"CreateGlobalPool = false but global pool isn't created! Creating it with default setting NumberOfThreads = 2. "
						"You need to call CreateGlobalVoxelThreadPool at BeginPlay (for instance in your level blueprint).",
					this);
				
				const auto NewPool = CreateOwnPool(NumberOfThreads, bConstantPriorities);
				IVoxelPool::SetGlobalVoxelPool(GetWorld(), NewPool, GetName());
				return NewPool;
			}
		}
	}
}

TVoxelSharedRef<IVoxelRenderer> AVoxelWorld::CreateRenderer() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelDefaultRenderer::Create(FVoxelRendererSettings(
		this,
		PlayType,
		WorldRoot,
		Data.ToSharedRef(),
		Pool.ToSharedRef(),
		ToolRenderingManager.ToSharedRef(),
		DebugManager.ToSharedRef(),
		false));
}

TVoxelSharedRef<IVoxelLODManager> AVoxelWorld::CreateLODManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelDefaultLODManager::Create(
		FVoxelLODSettings(this,
			PlayType,
			Renderer.ToSharedRef(),
			Pool.ToSharedRef()),
		this,
		LODDynamicSettings);
}

TVoxelSharedPtr<FVoxelProcGenManager> AVoxelWorld::CreateProcGenManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelProcGenManager::Create(FVoxelProcGenSettings(this, PlayType));
}

TVoxelSharedPtr<FVoxelToolRenderingManager> AVoxelWorld::CreateToolRenderingManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return MakeVoxelShared<FVoxelToolRenderingManager>();
}


void AVoxelWorld::CreateWorldInternal()
{
	VOXEL_FUNCTION_COUNTER();

	check(!IsCreated());

	UE_LOG(LogVoxel, Log, TEXT("Loading world"));

	bIsCreated = true;
	TimeOfCreation = FPlatformTime::Seconds();

	if (!WorldGenerator.IsValid())
	{
		FVoxelMessages::Error("Invalid World Generator!", this);
	}

	// Setup root
	{
		WorldRoot->CollisionTraceFlag = CollisionTraceFlag;
		WorldRoot->CanCharacterStepUpOn = CanCharacterStepUpOn;
		WorldRoot->SetGenerateOverlapEvents(bGenerateOverlapEvents);
		// Only copy collision data - we don't want to override the physics settings
		WorldRoot->BodyInstance.CopyRuntimeBodyInstancePropertiesFrom(&CollisionPresets);
		WorldRoot->BodyInstance.SetPhysMaterialOverride(PhysMaterialOverride);
		WorldRoot->BodyInstance.bNotifyRigidBodyCollision = bNotifyRigidBodyCollision;
		WorldRoot->BodyInstance.bUseCCD = bUseCCD;
		WorldRoot->RecreatePhysicsState();
	}

	if (PlayType == EVoxelPlayType::Game && WorldRoot->BodyInstance.bSimulatePhysics && CollisionTraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		FVoxelMessages::Error("Simulate physics requires using Simple collisions (either 'Simple And Complex' or 'Use Simple Collision As Complex')", this);
	}

	*WorldOffset = FIntVector::ZeroValue;
	UpdateDynamicLODSettings();
	UpdateDynamicRendererSettings();

	const bool bHasPendingData = PendingData.IsValid();
	if (bHasPendingData)
	{
		Data = MoveTemp(PendingData);
	}
	else
	{
		Data = CreateData();
	}
	Pool = CreatePool();
	DebugManager = CreateDebugManager();
	ProcGenManager = CreateProcGenManager();
	ToolRenderingManager = CreateToolRenderingManager();
	Renderer = CreateRenderer();
	Renderer->OnWorldLoaded.AddUObject(this, &AVoxelWorld::OnWorldLoadedCallback);
	LODManager = CreateLODManager();
	if (SpawnerConfig && (PlayType == EVoxelPlayType::Game || bEnableFoliageInEditor))
	{
		FVoxelMessages::ShowVoxelPluginProError("Spawners are only available in Voxel Plugin Pro", this);
	}
	if (PlayType == EVoxelPlayType::Game && bEnableMultiplayer)
	{
		FVoxelMessages::ShowVoxelPluginProError("Multiplayer is only available in Voxel Plugin Pro", this);
	}

	if (!bHasPendingData)
	{
		// Load if possible
		if (SaveObject)
		{
			LoadFromSaveObject();
			if (!IsCreated()) // if LoadFromSaveObject destroyed the world
			{
				return;
			}
		}

		// Add placeable items AFTER loading
		ApplyPlaceableItems();
	}

	if (PlayType == EVoxelPlayType::Preview)
	{
		UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(false);
	}
}

void AVoxelWorld::DestroyWorldInternal()
{
	VOXEL_FUNCTION_COUNTER();

	check(IsCreated());

#if WITH_EDITOR
	if (PlayType == EVoxelPlayType::Preview)
	{
		SaveData();
	}
#endif

	UE_LOG(LogVoxel, Log, TEXT("Unloading world"));

	bIsCreated = false;
	
	Data.Reset();
	Pool.Reset();
	
	DebugManager->Destroy();
	DebugManager.Reset();
	
	ProcGenManager->Destroy();
	ProcGenManager.Reset();

	Renderer->Destroy();
	Renderer.Reset();
	
	LODManager->Destroy();
	LODManager.Reset();
	

	WorldOffset = MakeVoxelShared<FIntVector>(FIntVector::ZeroValue);

	DestroyVoxelComponents();
}

void AVoxelWorld::DestroyVoxelComponents()
{
	VOXEL_FUNCTION_COUNTER();

	auto Components = GetComponents(); // need a copy as we are modifying it when destroying comps
	for (auto& Component : Components)
	{
		if (IsValid(Component) && Component->HasAnyFlags(RF_Transient) && (Component->IsA<UVoxelProceduralMeshComponent>() || Component->IsA<UHierarchicalInstancedStaticMeshComponent>()))
		{
			Component->DestroyComponent();
		}
	}
}

void AVoxelWorld::LoadFromSaveObject()
{
	VOXEL_FUNCTION_COUNTER();

	check(SaveObject);

	if (SaveObject->Depth == -1)
	{
		// Not saved yet
		return;
	}
	
	FVoxelUncompressedWorldSave Save;
	UVoxelSaveUtilities::DecompressVoxelSave(SaveObject->Save, Save);

	if (Save.GetDepth() == -1)
	{
		FVoxelMessages::Error("Invalid Save Object!", this);
		return;
	}
	if (Save.GetDepth() > Data->Depth)
	{
		UE_LOG(LogVoxel, Warning, TEXT("Save Object depth is bigger than world depth, the save data outside world bounds will be ignored"));
	}

	TArray<FIntBox> BoundsToUpdate;
	if (!Data->LoadFromSave(this, Save, BoundsToUpdate))
	{
		const auto Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			LOCTEXT("ErrorLoading",
				"Some errors occured when loading from the save object. Do you want to continue? This might corrupt the save object\n"
				"Note: always keep your voxel world toggled when renaming assets referenced by it, else the references won't be updated"));

		if (Result != EAppReturnType::Yes)
		{
			Data->ClearData();
			PlayType = EVoxelPlayType::Game; // Hack
			DestroyWorldInternal();
			return;
		}
	}
	LODManager->UpdateBounds(BoundsToUpdate);
}

void AVoxelWorld::ApplyPlaceableItems()
{
}

void AVoxelWorld::UpdateDynamicLODSettings() const
{
	LODDynamicSettings->MinLOD = FVoxelUtilities::ClampChunkDepth(MinLOD);
	LODDynamicSettings->MaxLOD = FVoxelUtilities::ClampChunkDepth(MaxLOD);
	LODDynamicSettings->LODToMinDistance.Reset();
	for (auto& It : LODToMinDistance)
	{
		LODDynamicSettings->LODToMinDistance.Add(TCString<TCHAR>::Atoi(*It.Key), It.Value);
	}
	
	LODDynamicSettings->InvokerDistanceThreshold = InvokerDistanceThreshold;
	
	LODDynamicSettings->ChunksCullingLOD = FVoxelUtilities::ClampChunkDepth(ChunksCullingLOD);

	LODDynamicSettings->bEnableRender = bRenderWorld;
	
	LODDynamicSettings->bEnableCollisions = PlayType == EVoxelPlayType::Game ? bEnableCollisions : true; 
	LODDynamicSettings->bComputeVisibleChunksCollisions = PlayType == EVoxelPlayType::Game ? bComputeVisibleChunksCollisions : true;
	LODDynamicSettings->VisibleChunksCollisionsMaxLOD = PlayType == EVoxelPlayType::Game ? FVoxelUtilities::ClampChunkDepth(VisibleChunksCollisionsMaxLOD) : MAX_WORLD_DEPTH;
	
	LODDynamicSettings->bEnableNavmesh = bEnableNavmesh; // bEnableNavmesh is needed for path previews in editor
	
	LODDynamicSettings->bComputeVisibleChunksNavmesh = bComputeVisibleChunksNavmesh;
	LODDynamicSettings->VisibleChunksNavmeshMaxLOD = FVoxelUtilities::ClampChunkDepth(VisibleChunksNavmeshMaxLOD);

	LODDynamicSettings->bEnableTessellation = bEnableTessellation;
	LODDynamicSettings->TessellationDistance = TessellationDistance;
}

void AVoxelWorld::UpdateDynamicRendererSettings() const
{
	FScopeLock Lock(&RendererDynamicSettings->DynamicSettingsLock);
	
	RendererDynamicSettings->MaterialCollection = MaterialCollection;
	RendererDynamicSettings->VoxelMaterialWithoutTessellation = VoxelMaterial;
	RendererDynamicSettings->VoxelMaterialWithTessellation = TessellatedVoxelMaterial;

	if (MaterialCollection)
	{
		RendererDynamicSettings->bGenerateBlendings = MaterialCollection->bShouldGenerateBlendings;
		RendererDynamicSettings->MaterialCollectionMap = MakeVoxelShared<TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted>>(MaterialCollection->GetBlendedMaterialsMap());
	}
	else
	{
		RendererDynamicSettings->bGenerateBlendings = false;
		RendererDynamicSettings->MaterialCollectionMap = MakeVoxelShared<TMap<FVoxelBlendedMaterialSorted, FVoxelBlendedMaterialUnsorted>>();
	}

	const auto IsTessellated = [](auto* Material)
	{
		UMaterial* BaseMaterial = Material->GetMaterial();
		if (!ensure(BaseMaterial)) return false;
		return BaseMaterial->D3D11TessellationMode != EMaterialTessellationMode::MTM_NoTessellation;
	};
	if (VoxelMaterial && IsTessellated(VoxelMaterial))
	{
		FVoxelMessages::Error("Voxel Material must have tessellation disabled!", this);
		RendererDynamicSettings->VoxelMaterialWithoutTessellation.Reset();
	}
	if (TessellatedVoxelMaterial && !IsTessellated(TessellatedVoxelMaterial))
	{
		FVoxelMessages::Error("Tessellated Voxel Material must have tessellation enabled!", this);
		RendererDynamicSettings->VoxelMaterialWithTessellation.Reset();
	}
}

#if WITH_EDITOR
void AVoxelWorld::Toggle()
{
	MarkPackageDirty();
	if (IsCreated())
	{
		ensure(bIsToggled);
		bIsToggled = false;
		DestroyWorldInternal();
	}
	else
	{
		ensure(!bIsToggled);
		bIsToggled = true;
		CreateInEditor();
		if (!IsCreated()) // Load failed
		{
			bIsToggled = false;
		}
	}
}

void AVoxelWorld::CreateInEditor()
{
	if (!IVoxelWorldEditor::GetVoxelWorldEditor())
	{
		return;
	}
	if (!VoxelWorldEditor)
	{
		UClass* VoxelWorldEditorClass = IVoxelWorldEditor::GetVoxelWorldEditor()->GetVoxelWorldEditorClass();

		if (VoxelWorldEditorClass)
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
	}

	BindEditorDelegates(this);

	if (IsCreated())
	{
		DestroyWorldInternal();
	}
	PlayType = EVoxelPlayType::Preview;
	CreateWorldInternal();
}

void AVoxelWorld::SaveData()
{
	if (IsGarbageCollecting())
	{
		return;
	}
	check(IsCreated());
	if (Data->IsDirty() && GetTransientPackage() != nullptr)
	{
		if (!SaveObject && ensure(IVoxelWorldEditor::GetVoxelWorldEditor()))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NeedToCreateSaveObject", "The voxel world Save Object is null. You need to create one now if you don't want to lose your changes."));
			Modify();
			SaveObject = IVoxelWorldEditor::GetVoxelWorldEditor()->CreateSaveObject();
		}
		if (SaveObject)
		{
			{
				FScopedSlowTask Progress(3);
				Progress.EnterProgressFrame(1.f, LOCTEXT("Rounding", "Rounding voxels"));
				if (GetDefault<UVoxelSettings>()->bRoundBeforeSaving)
				{
					UVoxelDataTools::RoundVoxels(this, FIntBox::Infinite);
				}
				Progress.EnterProgressFrame(1.f, LOCTEXT("Save", "Creating save"));
				FVoxelUncompressedWorldSave Save;
				Data->GetSave(Save);
				Progress.EnterProgressFrame(1.f, LOCTEXT("Compressing", "Compressing save"));
				UVoxelSaveUtilities::CompressVoxelSave(Save, SaveObject->Save);
			}
			
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

inline bool CanLoad(const TVoxelSharedPtr<FVoxelData>& Data)
{
	if (Data->IsDirty())
	{
		const auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("WorldDetailsUnsavedChanges", "There are unsaved changes. Loading will overwrite them. Confirm?"));
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

	if (IsCreated())
	{
		const FNotificationInfo Info(LOCTEXT("SuccessfullyLoaded", "Loaded!"));
		FSlateNotificationManager::Get().AddNotification(Info);
	}
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

	FBufferArchive Archive(true);
	FVoxelCompressedWorldSave CompressedSave;
	UVoxelDataTools::GetCompressedSave(this, CompressedSave);
	CompressedSave.Serialize(Archive);

	if (FFileHelper::SaveArrayToFile(Archive, *Path))
	{
		const FNotificationInfo Info(FText::Format(LOCTEXT("SuccessfullyCreated", "{0} was successfully created"), FText::FromString(Path)));
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

	const FNotificationInfo Info(FText::Format(LOCTEXT("SuccessfullyLoaded", "{0} was successfully loaded"), FText::FromString(Path)));
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
	int32 Year = 0, Month = 0, DayOfWeek = 0, Day = 0, Hour = 0, Min = 0, Sec = 0, MSec = 0;
	FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);
	const FString Sep = TEXT("_");
	const FString Date = FString::FromInt(Year) +
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
	if (IsCreated() && PlayType == EVoxelPlayType::Preview)
	{
		SaveData();
	}
}

void AVoxelWorld::OnPreBeginPIE(bool bIsSimulating)
{
	if (PlayType == EVoxelPlayType::Preview && IsCreated())
	{
		DestroyWorldInternal();
	}
}

void AVoxelWorld::OnEndPIE(bool bIsSimulating)
{
	if (PlayType == EVoxelPlayType::Preview && ensure(!IsCreated()) && bIsToggled)
	{
		CreateInEditor();
	}
}

void AVoxelWorld::OnPrepareToCleanseEditorObject(UObject* Object)
{
	if (PlayType == EVoxelPlayType::Preview && IsCreated())
	{
		DestroyWorldInternal();
	}
}

void AVoxelWorld::OnPreExit()
{
	bIsToggled = false; // Disable saving data
}

void IVoxelWorldEditor::SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor)
{
	check(!VoxelWorldEditor.IsValid());
	VoxelWorldEditor = InVoxelWorldEditor;
}

TSharedPtr<IVoxelWorldEditor> IVoxelWorldEditor::VoxelWorldEditor;
IVoxelEditorDelegatesInterface::FBindEditorDelegates IVoxelEditorDelegatesInterface::BindEditorDelegatesDelegate;
#endif
#undef LOCTEXT_NAMESPACE