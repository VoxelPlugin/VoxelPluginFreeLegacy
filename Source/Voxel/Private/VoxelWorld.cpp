// Copyright 2021 Phyronnaz

#include "VoxelWorld.h"

#include "VoxelPool.h"
#include "VoxelMessages.h"
#include "VoxelSettings.h"
#include "VoxelEditorDelegates.h"
#include "VoxelWorldRootComponent.h"

#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelData/VoxelSaveUtilities.h"

#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"

#include "VoxelMultiplayer/VoxelMultiplayerTcp.h"

#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelPlaceableItems/Actors/VoxelAssetActor.h"
#include "VoxelPlaceableItems/Actors/VoxelDisableEditsBox.h"
#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActor.h"
#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActorHelper.h"

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelTexturePool.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"

#include "VoxelFoliageInterface.h"

#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Serialization/BufferArchive.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/PlatformFilemanager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Components/BillboardComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void AVoxelWorld::FGameThreadTasks::Flush()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	TFunction<void()> Task;
	while (Tasks.Dequeue(Task))
	{
		Task();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AVoxelWorld::AVoxelWorld()
{
	MultiplayerInterface = UVoxelMultiplayerTcpInterface::StaticClass();
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bHighPriority = true;

	WorldRoot = CreateDefaultSubobject<UVoxelWorldRootComponent>("Root");
	LineBatchComponent = CreateDefaultSubobject<UVoxelLineBatchComponent>("LineBatchComponent");
	
	RootComponent = WorldRoot;

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

	// Automatically refresh material on property change/material recompile
	const auto RefreshMaterial = [=](UMaterialInterface* Material)
	{
		if (!Material || !bAutomaticallyRefreshMaterials || !IsCreated())
		{
			return;
		}

		bool bUsed = false;
		if (MaterialConfig == EVoxelMaterialConfig::RGB)
		{
			if (VoxelMaterial == Material)
			{
				bUsed = true;
			}
			else
			{
				for (auto& LODMaterial : LODMaterials)
				{
					if (LODMaterial.Material == Material)
					{
						bUsed = true;
						break;
					}
				}
			}
		}
		else
		{
			if (MaterialCollection && 
				MaterialCollection->GetMaterialIndex(Material->GetFName()) != -1)
			{
				bUsed = true;
			}
			else
			{
				for (auto& LODMaterialCollection : LODMaterialCollections)
				{
					if (LODMaterialCollection.MaterialCollection && 
						LODMaterialCollection.MaterialCollection->GetMaterialIndex(Material->GetFName()) != -1)
					{
						bUsed = true;
						break;
					}
				}
			}
		}

		if (bUsed)
		{
			UVoxelBlueprintLibrary::ApplyNewMaterials(this);
		}
	};
	UMaterial::OnMaterialCompilationFinished().AddWeakLambda(this, RefreshMaterial);

	const auto TryRefreshMaterials = [=](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (!bAutomaticallyRefreshMaterials)
		{
			return;
		}
		
		RefreshMaterial(Cast<UMaterialInterface>(Object));

		if (MaterialConfig != EVoxelMaterialConfig::RGB && Object->IsA<UVoxelMaterialCollectionBase>())
		{
			bool bUsed = false;
			if (MaterialCollection == Object)
			{
				bUsed = true;
			}
			else
			{
				for (auto& It : LODMaterialCollections)
				{
					if (It.MaterialCollection == Object)
					{
						bUsed = true;
						break;
					}
				}
			}

			if (auto* MaterialCollectionInstance = Cast<UVoxelInstancedMaterialCollectionInstance>(MaterialCollection))
			{
				if (MaterialCollectionInstance->LayersSource == Object)
				{
					bUsed = true;
				}
			}

			if (bUsed)
			{
				UVoxelBlueprintLibrary::ApplyNewMaterials(this);

				if (PropertyChangedEvent.MemberProperty && 
					PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelInstancedMaterialCollection, MaxMaterialsToBlendAtOnce))
				{
					// Need to recompute the chunks if MaxMaterialsToBlendAtOnce changed, as they are built with the wrong number of indices
					UVoxelBlueprintLibrary::UpdateAll(this);
				}
			}
		}
	};
	
	const auto TryRefreshFoliage = [=](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
	};
		
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddWeakLambda(this, [=](UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive)
		{
			return;
		}
		
		if (!Object || !IsCreated())
		{
			return;
		}

		TryRefreshMaterials(Object, PropertyChangedEvent);
		TryRefreshFoliage(Object, PropertyChangedEvent);
	});

	FVoxelEditorDelegates::OnVoxelGraphUpdated.AddWeakLambda(this, [=](UVoxelGenerator* Object)
	{
		if (!Object || !IsCreated())
		{
			return;
		}
		
		FPropertyChangedEvent PropertyChangedEvent(nullptr);
		TryRefreshFoliage(Object, PropertyChangedEvent);
	});
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::CreateWorld(FVoxelWorldCreateInfo Info)
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
		CreateInEditor(Info);
		return;
	}
#endif
	
	PlayType = EVoxelPlayType::Game;
	CreateWorldInternal(Info);
	if (bUseCameraIfNoInvokersFound && UVoxelInvokerComponentBase::GetInvokers(this).Num() == 0)
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

			LOG_VOXEL(Log, TEXT("No Voxel Invoker found, using camera as invoker"));
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

FVoxelIntBox AVoxelWorld::GetWorldBounds() const
{
	return FVoxelRuntimeSettings::GetWorldBounds(bUseCustomWorldBounds, CustomWorldBounds, RenderOctreeDepth);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AVoxelWorld::SetGeneratorObject(UVoxelGenerator* NewGenerator)
{
	Generator = NewGenerator;
}

void AVoxelWorld::SetGeneratorClass(TSubclassOf<UVoxelGenerator> NewGeneratorClass)
{
	Generator = NewGeneratorClass.Get();
}

void AVoxelWorld::SetRenderOctreeDepth(int32 NewDepth)
{
	RenderOctreeDepth = FMath::Max(1, FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(NewDepth));
	WorldSizeInVoxel = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(RenderOctreeDepth);
}

void AVoxelWorld::SetWorldSize(int32 NewWorldSizeInVoxels)
{
	SetWorldSize(uint32(FMath::Max(0, NewWorldSizeInVoxels)));
}
void AVoxelWorld::SetWorldSize(uint32 NewWorldSizeInVoxels)
{
	SetRenderOctreeDepth(FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(NewWorldSizeInVoxels));
}

TArray<FIntVector> AVoxelWorld::GetNeighboringPositions(const FVector& GlobalPosition) const
{
	return TArray<FIntVector>(FVoxelUtilities::GetNeighbors(GlobalToLocalFloat(GlobalPosition)));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelGeneratorCache* AVoxelWorld::GetGeneratorCache() const
{
	CHECK_VOXELWORLD_IS_CREATED_IMPL(this, nullptr);
	
	if (!GeneratorCache)
	{
		GeneratorCache = NewObject<UVoxelGeneratorCache>();
		GeneratorCache->GeneratorCache = GetSubsystemChecked<FVoxelGeneratorCache>().AsShared();
	}
	return GeneratorCache;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelGeneratorInit AVoxelWorld::GetGeneratorInit() const
{
	return FVoxelGeneratorInit(
		VoxelSize,
		FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(RenderOctreeDepth),
		RenderType,
		MaterialConfig,
		MaterialCollection,
		this);
}

UVoxelMultiplayerInterface* AVoxelWorld::CreateMultiplayerInterfaceInstance()
{
	FVoxelMessages::Info(FUNCTION_ERROR("Multiplayer with TCP require Voxel Plugin Pro"));
	return nullptr;
}

UVoxelMultiplayerInterface* AVoxelWorld::GetMultiplayerInterfaceInstance() const
{
	FVoxelMessages::Info(FUNCTION_ERROR("Multiplayer with TCP require Voxel Plugin Pro"));
	return nullptr;
}

void AVoxelWorld::SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse NewResponse)
{
	VOXEL_FUNCTION_COUNTER();

	CollisionPresets.SetResponseToChannel(Channel, NewResponse);

	if (IsCreated())
	{
		GetWorldRoot().SetCollisionResponseToChannel(Channel, NewResponse);

		GetSubsystemChecked<IVoxelRenderer>().ApplyToAllMeshes([&](UVoxelProceduralMeshComponent& MeshComponent)
		{
			MeshComponent.SetCollisionResponseToChannel(Channel, NewResponse);
		});
	}
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
		const FVoxelDoubleTransform NewTransform = GetVoxelTransform();
		if (bUseAbsoluteTransforms && LastTransform != NewTransform)
		{
			LastTransform = NewTransform;
			UVoxelBlueprintLibrary::RecomputeComponentPositions(this);
		}
		
		if (bRegenerateFoliageOnNextFrame)
		{
			UVoxelBlueprintLibrary::RecreateSpawners(this);
		}
		WorldRoot->TickWorldRoot();
		GameThreadTasks->Flush();
#if WITH_EDITOR
		if (PlayType == EVoxelPlayType::Preview && GetSubsystemChecked<FVoxelData>().IsDirty())
		{
			MarkPackageDirty();
		}
#endif
	}

	bRegenerateFoliageOnNextFrame = false;
}

void AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (bIsToggled &&
		!IsCreated() &&
		!HasAnyFlags(RF_ClassDefaultObject) &&
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
#if WITH_EDITOR
	if (bDisableComponentUnregister)
	{
		Super::UnregisterAllComponents(true); // Do as if it was a reregister so that proc mesh are ignored
	}
	else
#endif
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

	FVoxelPool::FixPriorityCategories(PriorityCategories);
	FVoxelPool::FixPriorityOffsets(PriorityOffsets);

	SetRenderOctreeDepth(RenderOctreeDepth);

	if (int32(UVConfig) >= int32(EVoxelUVConfig::Max))
	{
		UVConfig = EVoxelUVConfig::GlobalUVs;
	}

	if (NumberOfThreads_DEPRECATED != 0)
	{
		FVoxelMessages::Warning(FString::Printf(
			TEXT(
				"NumberOfThreads is now set globally in the project settings instead of per voxel world. "
				"The value on the voxel world (%d) will be ignored."), NumberOfThreads_DEPRECATED), this);
	}
	if (SpawnerConfig_DEPRECATED)
	{
		FVoxelEditorDelegates::OnMigrateLegacySpawners.Broadcast(this);
	}
}

#if WITH_EDITOR

void AVoxelWorld::PreEditChange(FProperty* PropertyThatWillChange)
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

	if (auto* Property = PropertyChangedEvent.Property)
	{
		const FName Name = Property->GetFName();
		if (Name == GET_MEMBER_NAME_STATIC(FVoxelLODMaterialsBase, StartLOD))
		{
			for (auto& It : LODMaterials)
			{
				It.EndLOD = FMath::Max(It.StartLOD, It.EndLOD);
			}
			for (auto& It : LODMaterialCollections)
			{
				It.EndLOD = FMath::Max(It.StartLOD, It.EndLOD);
			}
		}
		else if (Name == GET_MEMBER_NAME_STATIC(FVoxelLODMaterialsBase, EndLOD))
		{
			for (auto& It : LODMaterials)
			{
				It.StartLOD = FMath::Min(It.StartLOD, It.EndLOD);
			}
			for (auto& It : LODMaterialCollections)
			{
				It.StartLOD = FMath::Min(It.StartLOD, It.EndLOD);
			}
		}
	}
	
	if (auto* Property = PropertyChangedEvent.MemberProperty)
	{
		const FName Name = Property->GetFName();
		if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, RenderOctreeDepth))
		{
			SetRenderOctreeDepth(RenderOctreeDepth);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, WorldSizeInVoxel))
		{
			SetWorldSize(WorldSizeInVoxel);
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
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, FoliageCollisionDistanceInVoxel))
		{
			FoliageCollisionDistanceInVoxel = FMath::CeilToInt(FoliageCollisionDistanceInVoxel / 32.f) * 32;
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
			FVoxelPool::FixPriorityCategories(PriorityCategories);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, PriorityOffsets))
		{
			FVoxelPool::FixPriorityOffsets(PriorityOffsets);
		}
		else if (Name == GET_MEMBER_NAME_STATIC(AVoxelWorld, MaterialConfig))
		{
			if (IsCreated() && UVoxelBlueprintLibrary::HasMaterialData(this))
			{
				const auto Result = FMessageDialog::Open(
					EAppMsgType::YesNo,
					VOXEL_LOCTEXT("Changing the material config with existing material data! Do you want to clear that data (recommended)?"));
				
				if (Result == EAppReturnType::Yes)
				{
					UVoxelBlueprintLibrary::ClearMaterialData(this);
					GetSubsystemChecked<FVoxelData>().MarkAsDirty();
				}
			}
		}

		OnPropertyChanged_Interactive.Broadcast();
		
		if (IsCreated() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
		{
			OnPropertyChanged.Broadcast();
			
			if (Property->HasMetaData("Recreate"))
			{
				UVoxelBlueprintLibrary::Recreate(this, true);
			}
			else if (Property->HasMetaData("RecreateRender"))
			{
				RecreateRender();
			}
			else if (Property->HasMetaData("RecreateFoliage"))
			{
				RecreateFoliage();
			}
			else if (Property->HasMetaData("UpdateAll"))
			{
				GetRuntime().DynamicSettings->SetFromRuntime(*this);
				GetSubsystemChecked<IVoxelLODManager>().UpdateBounds(FVoxelIntBox::Infinite);
			}
			else if (Property->HasMetaData("UpdateLODs"))
			{
				GetRuntime().DynamicSettings->SetFromRuntime(*this);
				GetSubsystemChecked<IVoxelLODManager>().ForceLODsUpdate();
			}
			else if (Property->HasMetaData("UpdateRenderer"))
			{
				GetRuntime().DynamicSettings->SetFromRuntime(*this);
				GetSubsystemChecked<IVoxelRenderer>().ApplyNewMaterials();
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
	LOG_VOXEL(Log, TEXT("%s took %fs to generate"), *GetName(), FPlatformTime::Seconds() - TimeOfCreation);
	bIsLoaded = true;

	if (bSimulatePhysicsOnceLoaded)
	{
		WorldRoot->BodyInstance.SetInstanceSimulatePhysics(true);
	}

	OnWorldLoaded.Broadcast();
}

void AVoxelWorld::CreateWorldInternal(const FVoxelWorldCreateInfo& Info)
{
	VOXEL_FUNCTION_COUNTER();

	check(!IsCreated());

	LOG_VOXEL(Log, TEXT("Loading world"));

	bIsLoaded = false;
	TimeOfCreation = FPlatformTime::Seconds();
	LastTransform = GetVoxelTransform();

	if (!Generator.IsValid())
	{
		FVoxelMessages::Error("Invalid generator!", this);
	}

	// Setup root
	ApplyCollisionSettingsToRoot();
	
	if (PlayType == EVoxelPlayType::Game && WorldRoot->BodyInstance.bSimulatePhysics && CollisionTraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		FVoxelMessages::Error("Simulate physics requires using Simple collisions (either 'Simple And Complex' or 'Use Simple Collision As Complex')", this);
	}
	bSimulatePhysicsOnceLoaded = WorldRoot->BodyInstance.bSimulatePhysics;
	WorldRoot->BodyInstance.bSimulatePhysics = false;
	
	GetLineBatchComponent().Flush();

	ensure(!GameThreadTasks);
	GameThreadTasks = MakeVoxelShared<FGameThreadTasks>();

	{
		FVoxelRuntimeSettings RuntimeSettings = GetRuntimeSettings();
		
		if (Info.bOverrideData)
		{
			ensure(!(Info.DataOverride && Info.DataOverride_Raw));
			if (Info.DataOverride)
			{
				if (Info.DataOverride->IsCreated())
				{
					RuntimeSettings.DataOverride = Info.DataOverride->GetSubsystemChecked<FVoxelData>().AsShared();
				}
				else
				{
					FVoxelMessages::Warning(FUNCTION_ERROR("Info.DataOverride is not created! Can't copy data from it."), this);
				}
			}
			else if (Info.DataOverride_Raw)
			{
				RuntimeSettings.DataOverride = Info.DataOverride_Raw;
			}
			else
			{
				FVoxelMessages::Warning(FUNCTION_ERROR("Info.bOverrideData is true, but DataOverride is null!"), this);
			}
		}

		ensure(!Runtime);
		Runtime = FVoxelRuntime::Create(RuntimeSettings);
	}

	Runtime->DynamicSettings->SetFromRuntime(*this);
	if (PlayType == EVoxelPlayType::Preview)
	{
		Runtime->DynamicSettings->ConfigurePreview();
	}
	
	FVoxelRuntimeData& RuntimeData = *Runtime->RuntimeData;
	RuntimeData.OnWorldLoaded.AddUObject(this, &AVoxelWorld::OnWorldLoadedCallback);
	RuntimeData.OnMaxFoliageInstancesReached.AddWeakLambda(this, [this]()
	{
		OnMaxFoliageInstancesReached.Broadcast();
	});
	
	if (FoliageCollections.Num() > 0)
	{
		FVoxelMessages::Info("FoliageCollections are only available in Voxel Plugin Pro", this);
	}
	if (bEnableMultiplayer)
	{
		FVoxelMessages::Info("TCP Multiplayer is only available in Voxel Plugin Pro", this);
	}

	if (Info.bOverrideData && Info.bOverrideSave)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Cannot use Info.bOverrideSave if Info.bOverrideData is true!"), this);
	}

	if (!Info.bOverrideData)
	{
		// Load if possible
		if (Info.bOverrideSave)
		{
			UVoxelDataTools::LoadFromSave(this, Info.SaveOverride);
		}
		else if (SaveObject)
		{
			LoadFromSaveObject();
			if (!IsCreated()) // if LoadFromSaveObject destroyed the world
			{
				return;
			}
		}

		// Add placeable items AFTER loading
		if (!PlaceableItemManager)
		{
			// This lets objects adds new items without having to specify a custom class
			PlaceableItemManager = NewObject<UVoxelPlaceableItemManager>();
		}
		PlaceableItemManager->SetGeneratorCache(GetSubsystemChecked<FVoxelGeneratorCache>().AsShared());
		PlaceableItemManager->Clear();
		PlaceableItemManager->Generate();
		PlaceableItemManager->ApplyToData(GetSubsystemChecked<FVoxelData>());
		PlaceableItemManager->DrawDebug(*this, GetLineBatchComponent());

		// Do that after Clear/Generate
		ApplyPlaceableItems();

		// Let events add other items
		OnGenerateWorld.Broadcast();

		ensure(!PlaceableItemActorHelper);
		PlaceableItemActorHelper = NewObject<UVoxelPlaceableItemActorHelper>(this);
		PlaceableItemActorHelper->Initialize();
	}

	UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(this, false);
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

	LOG_VOXEL(Log, TEXT("Unloading world"));

	bIsLoaded = false;

	DebugTextures.Reset();

	Runtime->Destroy();
	Runtime.Reset();

	GameThreadTasks->Flush();
	GameThreadTasks.Reset();

	if (GeneratorCache) // Could be null, lazily created
	{
		// Reset to avoid keeping instances alive
		GeneratorCache->GeneratorCache.Reset();
		GeneratorCache->MarkPendingKill();
		GeneratorCache = nullptr;
	}

	if (PlaceableItemManager)
	{
		// Reset to avoid keeping instances alive
		PlaceableItemManager->ResetGeneratorCache();
	}
	
	if (PlaceableItemActorHelper)
	{
		PlaceableItemActorHelper->MarkPendingKill();
		PlaceableItemActorHelper = nullptr;
	}

	DestroyVoxelComponents();

	if (LineBatchComponent)
	{
		// Note: components might be destroyed if called in BeginDestroy (eg if actor deleted in the editor)
		LineBatchComponent->Flush();
	}
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
	
	if (Save.Const().GetDepth() == -1)
	{
		FVoxelMessages::Error("Invalid Save Object!", this);
		return;
	}
	if (Save.Const().GetDepth() > GetSubsystemChecked<FVoxelData>().Depth)
	{
		LOG_VOXEL(Warning, TEXT("Save Object depth is bigger than world depth, the save data outside world bounds will be ignored"));
	}

	if (!UVoxelDataTools::LoadFromSave(this, Save))
	{
		const auto Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			VOXEL_LOCTEXT("Some errors occured when loading from the save object. Do you want to continue? This might corrupt the save object"));

		if (Result != EAppReturnType::Yes)
		{
			GetSubsystemChecked<FVoxelData>().ClearData();
			PlayType = EVoxelPlayType::Game; // Hack
			DestroyWorldInternal();
		}
	}
}

void AVoxelWorld::ApplyPlaceableItems()
{
	VOXEL_FUNCTION_COUNTER();

	TArray<AVoxelPlaceableItemActor*> PlaceableItemActors;
	for (TActorIterator<AVoxelPlaceableItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		auto* Actor = *ActorItr;
		if (Actor->IsA<AVoxelAssetActor>() && !bMergeAssetActors)
		{
			continue;
		}
		if (Actor->IsA<AVoxelDisableEditsBox>() && !bMergeDisableEditsBoxes)
		{
			continue;
		}
		if (Actor->bOnlyImportIntoPreviewWorld && Actor->PreviewWorld != this)
		{
			continue;
		}
		PlaceableItemActors.Add(Actor);
	}
	
	TGuardValue<bool> AllowScriptsInEditor(GAllowActorScriptExecutionInEditor, true);

	TMap<AVoxelPlaceableItemActor*, int32> Priorities;
	for (auto* It : PlaceableItemActors)
	{
		Priorities.Add(It, It->K2_GetPriority());
	}
	PlaceableItemActors.Sort([&](auto& ActorA, auto& ActorB) { return Priorities[&ActorA] < Priorities[&ActorB]; });

	for (auto* PlaceableItemActor : PlaceableItemActors)
	{
		PlaceableItemActor->K2_AddItemToWorld(this);
	}
}

void AVoxelWorld::ApplyCollisionSettingsToRoot() const
{
	VOXEL_FUNCTION_COUNTER();
	
	WorldRoot->CollisionTraceFlag = CollisionTraceFlag;
	WorldRoot->CanCharacterStepUpOn = CanCharacterStepUpOn;
	WorldRoot->SetGenerateOverlapEvents(bGenerateOverlapEvents);
	// Only copy collision data - we don't want to override the physics settings
	WorldRoot->BodyInstance.CopyRuntimeBodyInstancePropertiesFrom(&CollisionPresets);
	WorldRoot->BodyInstance.SetObjectType(CollisionPresets.GetObjectType());
	WorldRoot->BodyInstance.SetPhysMaterialOverride(PhysMaterialOverride);
	WorldRoot->BodyInstance.bNotifyRigidBodyCollision = bNotifyRigidBodyCollision;
	WorldRoot->BodyInstance.bUseCCD = bUseCCD;
	WorldRoot->BodyInstance.COMNudge = CollisionPresets.COMNudge;
	WorldRoot->RecreatePhysicsState();
}

void AVoxelWorld::RecreateRender()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsCreated());

	Runtime->DynamicSettings->SetFromRuntime(*this);

	Runtime->RecreateSubsystems(EVoxelSubsystemFlags::RecreateRender | EVoxelSubsystemFlags::RecreateFoliage, GetRuntimeSettings());
	
	DestroyVoxelComponents();
	DebugTextures.Reset();
}

void AVoxelWorld::RecreateFoliage()
{
	Runtime->RecreateSubsystems(EVoxelSubsystemFlags::RecreateFoliage, GetRuntimeSettings());
}

void AVoxelWorld::RecreateAll(const FVoxelWorldCreateInfo& Info)
{
	check(IsCreated());

	DestroyWorldInternal();
	CreateWorldInternal(Info);
}

FVoxelRuntimeSettings AVoxelWorld::GetRuntimeSettings() const
{
	FVoxelRuntimeSettings RuntimeSettings;
	RuntimeSettings.SetFromRuntime(*this);

	if (PlayType == EVoxelPlayType::Preview)
	{
		RuntimeSettings.ConfigurePreview();
	}

	return RuntimeSettings;
}

#if WITH_EDITOR
void AVoxelWorld::Toggle()
{
	MarkPackageDirty();
	if (IsCreated())
	{
		// ensure(bIsToggled);
		bIsToggled = false;
		DestroyWorldInternal();
	}
	else
	{
		// ensure(!bIsToggled);
		bIsToggled = true;
		CreateInEditor();
		if (!IsCreated()) // Load failed
		{
			bIsToggled = false;
		}
	}
}

void AVoxelWorld::CreateInEditor(const FVoxelWorldCreateInfo& Info)
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
	CreateWorldInternal(Info);
}

void AVoxelWorld::SaveData()
{
	if (IsGarbageCollecting())
	{
		return;
	}
	check(IsCreated());
	if (GetSubsystemChecked<FVoxelData>().IsDirty() && GetTransientPackage() != nullptr)
	{
		if (!SaveObject && ensure(IVoxelWorldEditor::GetVoxelWorldEditor()))
		{
			const EAppReturnType::Type Result = FMessageDialog::Open(
				EAppMsgType::YesNo,
				VOXEL_LOCTEXT("The voxel world Save Object is null. You need to create one now if you don't want to lose your changes.\n\nSave changes?"));
			
			if (Result == EAppReturnType::No)
			{
				// Clear dirty flag so we don't get more annoying popups
				GetSubsystemChecked<FVoxelData>().ClearDirtyFlag();
				return;
			}
			
			Modify();
			SaveObject = IVoxelWorldEditor::GetVoxelWorldEditor()->CreateSaveObject();
		}
		if (SaveObject)
		{
			{
				FVoxelScopedSlowTask Progress(3);
				Progress.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Rounding voxels"));
				if (GetDefault<UVoxelSettings>()->bRoundBeforeSaving)
				{
					UVoxelDataTools::RoundVoxels(this, FVoxelIntBox::Infinite);
				}

				FVoxelUncompressedWorldSave Save;

				Progress.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Creating save"));
				UVoxelDataTools::GetSave(this, Save);
				
				Progress.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Compressing save"));
				UVoxelSaveUtilities::CompressVoxelSave(Save, SaveObject->Save);
			}
			
			SaveObject->CopyDepthFromSave();
			SaveObject->MarkPackageDirty();

			FNotificationInfo Info = FNotificationInfo(VOXEL_LOCTEXT("Voxel world saved!"));
			Info.CheckBoxState = ECheckBoxState::Checked;
			FSlateNotificationManager::Get().AddNotification(Info);

			GetSubsystemChecked<FVoxelData>().ClearDirtyFlag();
		}
		else
		{
			FNotificationInfo Info = FNotificationInfo(VOXEL_LOCTEXT("Voxel world not saved: no Save Object!"));
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

inline bool CanLoad(const FVoxelData& Data)
{
	if (Data.IsDirty())
	{
		const auto Result = FMessageDialog::Open(EAppMsgType::YesNoCancel, VOXEL_LOCTEXT("There are unsaved changes. Loading will overwrite them. Confirm?"));
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
	if (!CanLoad(GetSubsystemChecked<FVoxelData>()))
	{
		return;
	}

	LoadFromSaveObject();

	if (IsCreated())
	{
		const FNotificationInfo Info(VOXEL_LOCTEXT("Loaded!"));
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}

bool AVoxelWorld::SaveToFile(const FString& Path, FText& Error)
{
	check(IsCreated());

	if (Path.IsEmpty())
	{
		Error = VOXEL_LOCTEXT("Empty Save File Path");
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
		const FNotificationInfo Info(FText::Format(VOXEL_LOCTEXT("{0} was successfully created"), FText::FromString(Path)));
		FSlateNotificationManager::Get().AddNotification(Info);

		if (CompressedSave.Objects.Num() > 0)
		{
			const FNotificationInfo OtherInfo(FText::Format(VOXEL_LOCTEXT("The voxel data has {0} placeable items (eg, data assets)! These will not be saved"), CompressedSave.Objects.Num()));
			FSlateNotificationManager::Get().AddNotification(OtherInfo);
		}
		
		return true;
	}
	else
	{
		Error = FText::Format(VOXEL_LOCTEXT("Error when creating {0}"), FText::FromString(Path));
		return false;
	}
}

bool AVoxelWorld::LoadFromFile(const FString& Path, FText& Error)
{
	if (!CanLoad(GetSubsystemChecked<FVoxelData>()))
	{
		Error = VOXEL_LOCTEXT("Canceled");
		return false;
	}

	TArray<uint8> Array;
	if (!FFileHelper::LoadFileToArray(Array, *Path))
	{
		Error = FText::Format(VOXEL_LOCTEXT("Error when reading {0}"), FText::FromString(Path));
		return false;
	}

	FMemoryReader Reader(Array);
	FVoxelCompressedWorldSave CompressedSave;
	CompressedSave.Serialize(Reader);
	UVoxelDataTools::LoadFromCompressedSave(this, CompressedSave);

	const FNotificationInfo Info(FText::Format(VOXEL_LOCTEXT("{0} was successfully loaded"), FText::FromString(Path)));
	FSlateNotificationManager::Get().AddNotification(Info);
	return true;
}

FString AVoxelWorld::GetDefaultFilePath() const
{
	FString Path = SaveFilePath;
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

void AVoxelWorld::OnApplyObjectToActor(UObject* Object, AActor* Actor)
{
	if (Actor != this)
	{
		return;
	}

	auto* Material = Cast<UMaterialInterface>(Object);
	if (!Material || Material->GetMaterial()->MaterialDomain != MD_Surface)
	{
		return;
	}
	
	VoxelMaterial = Material;
	MaterialConfig = EVoxelMaterialConfig::RGB;
	RecreateRender();
	MarkPackageDirty();
}

void IVoxelWorldEditor::SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor)
{
	check(!VoxelWorldEditor.IsValid());
	VoxelWorldEditor = InVoxelWorldEditor;
}

TSharedPtr<IVoxelWorldEditor> IVoxelWorldEditor::VoxelWorldEditor;
IVoxelEditorDelegatesInterface::FBindEditorDelegates IVoxelEditorDelegatesInterface::BindEditorDelegatesDelegate;
#endif