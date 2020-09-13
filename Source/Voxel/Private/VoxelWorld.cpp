// Copyright 2020 Phyronnaz

#include "VoxelWorld.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorCache.h"
#include "IVoxelPool.h"
#include "VoxelSettings.h"
#include "VoxelDefaultPool.h"
#include "VoxelWorldRootComponent.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelRender/LODManager/VoxelDefaultLODManager.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelRender/MaterialCollections/VoxelInstancedMaterialCollection.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelMultiplayer/VoxelMultiplayerTcp.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelPlaceableItems/Actors/VoxelPlaceableItemActor.h"
#include "VoxelPlaceableItems/Actors/VoxelDataItemActor.h"
#include "VoxelPlaceableItems/Actors/VoxelAssetActor.h"
#include "VoxelPlaceableItems/Actors/VoxelDisableEditsBox.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelEvents/VoxelEventManager.h"
#include "VoxelMessages.h"
#include "VoxelFeedbackContext.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/NetConnection.h"

#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
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
	: LODDynamicSettings(MakeVoxelShared<FVoxelLODDynamicSettings>())
	, RendererDynamicSettings(MakeVoxelShared<FVoxelRendererDynamicSettings>())
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
#endif
}

AVoxelWorld::~AVoxelWorld()
{

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
	if (bUseCameraIfNoInvokersFound && UVoxelInvokerComponentBase::GetInvokers(GetWorld()).Num() == 0)
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

FVoxelWorldGeneratorInit AVoxelWorld::GetInitStruct() const
{
	return FVoxelWorldGeneratorInit(
		Seeds,
		VoxelSize,
		FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(RenderOctreeDepth),
		RenderType,
		MaterialConfig,
		MaterialCollection,
		this);
}

FVoxelIntBox AVoxelWorld::GetWorldBounds() const
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

void AVoxelWorld::SetWorldGeneratorObject(UVoxelWorldGenerator* NewGenerator)
{
	WorldGenerator = NewGenerator;
}

void AVoxelWorld::SetWorldGeneratorClass(TSubclassOf<UVoxelWorldGenerator> NewGeneratorClass)
{
	WorldGenerator = NewGeneratorClass.Get();
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

FVector AVoxelWorld::GlobalToLocalFloatBP(const FVector& Position) const
{
	return GlobalToLocalFloat(Position).ToFloat();
}

FVoxelVector AVoxelWorld::GlobalToLocalFloat(const FVector& Position) const
{
	return GetTransform().InverseTransformPosition(Position) / VoxelSize - FVoxelVector(*WorldOffset);
}

FVector AVoxelWorld::LocalToGlobal(const FIntVector& Position) const
{
	return GetTransform().TransformPosition(VoxelSize * FVector(Position + *WorldOffset));
}

FVector AVoxelWorld::LocalToGlobalFloatBP(const FVector& Position) const
{
	return LocalToGlobalFloat(Position);
}

FVector AVoxelWorld::LocalToGlobalFloat(const FVoxelVector& Position) const
{
	return GetTransform().TransformPosition((VoxelSize * (Position + *WorldOffset)).ToFloat());
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
	CHECK_VOXELWORLD_IS_CREATED_IMPL(this,);

	*WorldOffset = OffsetInVoxels;
	Renderer->RecomputeMeshPositions();
}

void AVoxelWorld::AddOffset(const FIntVector& OffsetInVoxels, bool bMoveActor)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_IMPL(this,);

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
		
		Renderer->ApplyToAllMeshes([&](UVoxelProceduralMeshComponent& MeshComponent)
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
		WorldRoot->TickWorldRoot();
		GameThreadTasks->Flush();
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

	FVoxelDefaultPool::FixPriorityCategories(PriorityCategories);
	FVoxelDefaultPool::FixPriorityOffsets(PriorityOffsets);

	SetRenderOctreeDepth(RenderOctreeDepth);

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
					GetData().MarkAsDirty();
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
			else if (Property->HasMetaData("RecreateSpawners"))
			{
				RecreateSpawners();
			}
			else if (Property->HasMetaData("UpdateAll"))
			{
				UpdateDynamicLODSettings();
				GetLODManager().UpdateBounds(FVoxelIntBox::Infinite);
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
	LOG_VOXEL(Log, TEXT("%s took %fs to generate"), *GetName(), FPlatformTime::Seconds() - TimeOfCreation);
	bIsLoaded = true;
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
			const auto ExistingPool = IVoxelPool::GetPoolForWorld(GetWorld());
			if (!ExistingPool.IsValid())
			{
				const auto NewPool = CreateOwnPool(NumberOfThreads, bConstantPriorities);
				IVoxelPool::SetWorldPool(GetWorld(), NewPool, GetName());
				return NewPool;
			}
			else
			{
				FVoxelMessages::Warning(
					"CreateGlobalPool = true but global or world pool is already created! Using existing one, NumberOfThreads will be ignored.\n"
					"Consider setting CreateGlobalPool to false and calling CreateWorldVoxelThreadPool at BeginPlay (for instance in your level blueprint).",
					this);
				return ExistingPool.ToSharedRef();
			}
		}
		else
		{
			const auto ExistingPool = IVoxelPool::GetPoolForWorld(GetWorld());
			if (ExistingPool.IsValid())
			{
				return ExistingPool.ToSharedRef();
			}
			else
			{
				FVoxelMessages::Warning(
						"CreateGlobalPool = false but global pool isn't created! Creating it with default setting NumberOfThreads = 2. "
						"You need to call CreateWorldVoxelThreadPool at BeginPlay (for instance in your level blueprint).",
					this);
				
				const auto NewPool = CreateOwnPool(NumberOfThreads, bConstantPriorities);
				IVoxelPool::SetWorldPool(GetWorld(), NewPool, GetName());
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

TVoxelSharedPtr<FVoxelEventManager> AVoxelWorld::CreateEventManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return FVoxelEventManager::Create(FVoxelEventManagerSettings(this, PlayType));
}

TVoxelSharedPtr<FVoxelToolRenderingManager> AVoxelWorld::CreateToolRenderingManager() const
{
	VOXEL_FUNCTION_COUNTER();
	return MakeVoxelShared<FVoxelToolRenderingManager>();
}


void AVoxelWorld::CreateWorldInternal(const FVoxelWorldCreateInfo& Info)
{
	VOXEL_FUNCTION_COUNTER();

	check(!IsCreated());

	LOG_VOXEL(Log, TEXT("Loading world"));

	bIsCreated = true;
	bIsLoaded = false;
	TimeOfCreation = FPlatformTime::Seconds();

	if (!WorldGenerator.IsValid())
	{
		FVoxelMessages::Error("Invalid World Generator!", this);
	}

	// Setup root
	ApplyCollisionSettingsToRoot();
	
	if (PlayType == EVoxelPlayType::Game && WorldRoot->BodyInstance.bSimulatePhysics && CollisionTraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple)
	{
		FVoxelMessages::Error("Simulate physics requires using Simple collisions (either 'Simple And Complex' or 'Use Simple Collision As Complex')", this);
	}

	GetLineBatchComponent().Flush();

	*WorldOffset = FIntVector::ZeroValue;
	UpdateDynamicLODSettings();
	UpdateDynamicRendererSettings();

	WorldGeneratorCache = MakeVoxelShared<FVoxelWorldGeneratorCache>(GetInitStruct());
	GameThreadTasks = MakeVoxelShared<FGameThreadTasks>();

	if (Info.bOverrideData)
	{
		ensure(!(Info.DataOverride && Info.DataOverride_Raw));
		if (Info.DataOverride)
		{
			if (Info.DataOverride->IsCreated())
			{
				Data = Info.DataOverride->GetDataSharedPtr();
			}
			else
			{
				FVoxelMessages::Warning(FUNCTION_ERROR("Info.DataOverride is not created! Can't copy data from it."), this);
			}
		}
		else if (Info.DataOverride_Raw)
		{
			Data = Info.DataOverride_Raw;
		}
		else
		{
			FVoxelMessages::Warning(FUNCTION_ERROR("Info.bOverrideData is true, but DataOverride is null!"), this);
		}
	}
	if (!Data)
	{
		Data = CreateData();
	}
	Pool = CreatePool();
	DebugManager = CreateDebugManager();
	EventManager = CreateEventManager();
	ToolRenderingManager = CreateToolRenderingManager();
	Renderer = CreateRenderer();
	Renderer->OnWorldLoaded.AddUObject(this, &AVoxelWorld::OnWorldLoadedCallback);
	LODManager = CreateLODManager();

	if (SpawnerConfig)
	{
		FVoxelMessages::Info("Spawners are only available in Voxel Plugin Pro", this);
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
		PlaceableItemManager->Clear();
		PlaceableItemManager->Generate();

		// Do that after Clear/Generate
		ApplyPlaceableItems();

		// Let events add other items
		OnGenerateWorld.Broadcast();
		
		using FItemInfo = FVoxelDataItemConstructionInfo;
		using FItemPtr = TVoxelWeakPtr<const TVoxelDataItemWrapper<FVoxelDataItem>>;
		
		TMap<AVoxelDataItemActor*, TArray<FItemInfo>> ActorsToItemInfos;
		for (TActorIterator<AVoxelDataItemActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			auto* Actor = *ActorItr;

			auto& ItemInfos = PlaceableItemManager->GetDataItemInfos();
			const int32 PreviousNum = ItemInfos.Num();
			Actor->CallAddItemToWorld(this);
			const int32 NewNum = ItemInfos.Num();

			auto& ActorItemInfos = ActorsToItemInfos.FindOrAdd(Actor);
			for (int32 Index = PreviousNum; Index < NewNum; Index++)
			{
				ActorItemInfos.Add(ItemInfos[Index]);
			}
		}

		TMap<FItemInfo, FItemPtr> GlobalItemInfoToItemPtr;
		PlaceableItemManager->ApplyToData(*Data, *WorldGeneratorCache, &GlobalItemInfoToItemPtr);

		for (auto& ActorIt : ActorsToItemInfos)
		{
			AVoxelDataItemActor* const Actor = ActorIt.Key;
			TArray<FItemInfo>& ActorItemInfos = ActorIt.Value;
			if (ActorItemInfos.Num() == 0)
			{
				continue;
			}
			
			const auto ItemInfoToItemPtr = MakeShared<TMap<FItemInfo, FItemPtr>>();
			for (auto& ItemInfo : ActorItemInfos)
			{
				FItemPtr* ItemPtrPtr = GlobalItemInfoToItemPtr.Find(ItemInfo);
				if (ensure(ItemPtrPtr) && ensure(ItemPtrPtr->IsValid()))
				{
					ItemInfoToItemPtr->Add(ItemInfo, *ItemPtrPtr);
				}
			}

			Actor->OnRefresh.RemoveAll(this);
			Actor->OnRefresh.AddWeakLambda(this, [this, ItemInfoToItemPtr, Actor]()
			{
				if (!IsCreated() || !ensure(PlaceableItemManager))
				{
					return;
				}
				
				TArray<FVoxelIntBox> BoundsToUpdate;
				{
					// Find which items to add/remove
					TSet<FItemInfo> ItemInfosToAdd;
					TSet<FItemInfo> ItemInfosToRemove;
					{
						PlaceableItemManager->Clear();
						Actor->CallAddItemToWorld(this);

						TSet<FItemInfo> PreviousItemInfos;
						TSet<FItemInfo> NewItemInfos;

						for (auto& It : *ItemInfoToItemPtr)
						{
							PreviousItemInfos.Add(It.Key);
						}
						NewItemInfos.Append(PlaceableItemManager->GetDataItemInfos());

						ItemInfosToAdd = NewItemInfos.Difference(PreviousItemInfos);
						ItemInfosToRemove = PreviousItemInfos.Difference(NewItemInfos);
					}

					// Remove the items that aren't here anymore
					for (const auto& ItemInfoToRemove : ItemInfosToRemove)
					{
						FItemPtr ItemPtr;
						if (!ensure(ItemInfoToItemPtr->RemoveAndCopyValue(ItemInfoToRemove, ItemPtr)))
						{
							continue;
						}
						
						BoundsToUpdate.Add(ItemInfoToRemove.Bounds);

						FVoxelWriteScopeLock Lock(*Data, ItemInfoToRemove.Bounds, FUNCTION_FNAME);
						FString Error;
						if (!ensure(Data->RemoveItem(ItemPtr, Error)))
						{
							LOG_VOXEL(Error, TEXT("Failed to remove data item for %s: %s"), *Actor->GetName(), *Error);
						}
					}
					
					// Add the new ones
					{
						PlaceableItemManager->Clear();

						for (auto& ItemInfoToAdd : ItemInfosToAdd)
						{
							PlaceableItemManager->AddDataItem(ItemInfoToAdd);
						}

						TMap<FItemInfo, FItemPtr> NewItemInfoToPtr;
						PlaceableItemManager->ApplyToData(*Data, *WorldGeneratorCache, &NewItemInfoToPtr);

						for (auto& NewIt : NewItemInfoToPtr)
						{
							const FItemInfo& ItemInfo = NewIt.Key;
							const FItemPtr& ItemPtr = NewIt.Value;
							ensure(ItemPtr.IsValid());

							BoundsToUpdate.Add(ItemInfo.Bounds);
							ItemInfoToItemPtr->Add(ItemInfo, ItemPtr);
						}
					}
				}

				if (BoundsToUpdate.Num() > 0)
				{
					LODManager->UpdateBounds(BoundsToUpdate);
					
					if (!Data->IsCurrentFrameEmpty())
					{
						// Save the frame for the eventual asset item merge/remove edits
						// Dummy frame, doesn't really store anything interesting
						Data->SaveFrame(FVoxelIntBox(BoundsToUpdate));
					}


#if WITH_EDITOR
					IVoxelWorldEditor::GetVoxelWorldEditor()->RegisterTransaction(this, "Applying voxel data item");
#endif
				}
					
				PlaceableItemManager->DrawDebug(*this, GetLineBatchComponent());
			});
		}
		
		PlaceableItemManager->DrawDebug(*this, GetLineBatchComponent());
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

	LOG_VOXEL(Log, TEXT("Unloading world"));

	bIsCreated = false;
	bIsLoaded = false;
	
	Data.Reset();
	Pool.Reset();
	
	DebugManager->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), DebugManager);
	
	EventManager->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), EventManager);

	Renderer->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), Renderer);
	
	LODManager->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), LODManager);
	

	WorldOffset = MakeVoxelShared<FIntVector>(FIntVector::ZeroValue);

	GameThreadTasks->Flush();
	GameThreadTasks.Reset();

	// Clear world generator cache to avoid keeping instances alive
	WorldGeneratorCache.Reset();

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
	if (Save.Const().GetDepth() > Data->Depth)
	{
		LOG_VOXEL(Warning, TEXT("Save Object depth is bigger than world depth, the save data outside world bounds will be ignored"));
	}

	if (!UVoxelDataTools::LoadFromSave(this, Save))
	{
		const auto Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			VOXEL_LOCTEXT("Some errors occured when loading from the save object. Do you want to continue? This might corrupt the save object\n"
				"Note: always keep your voxel world toggled when renaming assets referenced by it, else the references won't be updated"));

		if (Result != EAppReturnType::Yes)
		{
			Data->ClearData();
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

void AVoxelWorld::UpdateDynamicLODSettings() const
{
	LODDynamicSettings->MinLOD = FVoxelUtilities::ClampMesherDepth(MinLOD);
	LODDynamicSettings->MaxLOD = FVoxelUtilities::ClampMesherDepth(MaxLOD);
	
	LODDynamicSettings->InvokerDistanceThreshold = InvokerDistanceThreshold;
	
	LODDynamicSettings->ChunksCullingLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(ChunksCullingLOD);

	LODDynamicSettings->bEnableRender = bRenderWorld;
	
	LODDynamicSettings->bEnableCollisions = PlayType == EVoxelPlayType::Game ? bEnableCollisions : true; 
	LODDynamicSettings->bComputeVisibleChunksCollisions = PlayType == EVoxelPlayType::Game ? bComputeVisibleChunksCollisions : true;
	LODDynamicSettings->VisibleChunksCollisionsMaxLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(PlayType == EVoxelPlayType::Game ? VisibleChunksCollisionsMaxLOD : 32);
	
	LODDynamicSettings->bEnableNavmesh = bEnableNavmesh; // bEnableNavmesh is needed for path previews in editor
	
	LODDynamicSettings->bComputeVisibleChunksNavmesh = bComputeVisibleChunksNavmesh;
	LODDynamicSettings->VisibleChunksNavmeshMaxLOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(VisibleChunksNavmeshMaxLOD);
}

void AVoxelWorld::UpdateDynamicRendererSettings() const
{
	VOXEL_FUNCTION_COUNTER();

	TArray<UVoxelMaterialCollectionBase*> MaterialCollectionsToInitialize;
	for (int32 LOD = 0; LOD < 32; LOD++)
	{
		auto& LODData = RendererDynamicSettings->LODData[LOD];

		// Copy materials
		LODData.Material = nullptr;
		for (auto& It : LODMaterials)
		{
			if (It.StartLOD <= LOD && LOD <= It.EndLOD)
			{
				if (LODData.Material.IsValid())
				{
					FVoxelMessages::Warning(FString::Printf(TEXT("Multiple materials are assigned to LOD %d!"), LOD), this);
				}
				LODData.Material = It.Material;
			}
		}
		if (!LODData.Material.IsValid())
		{
			LODData.Material = VoxelMaterial;
		}

		// Copy material collection
		LODData.MaterialCollection = nullptr;
		for (auto& It : LODMaterialCollections)
		{
			if (It.StartLOD <= LOD && LOD <= It.EndLOD)
			{
				if (LODData.MaterialCollection.IsValid())
				{
					FVoxelMessages::Warning(FString::Printf(TEXT("Multiple material collections are assigned to LOD %d!"), LOD), this);
				}
				LODData.MaterialCollection = It.MaterialCollection;
			}
		}
		if (!LODData.MaterialCollection.IsValid())
		{
			LODData.MaterialCollection = MaterialCollection;
		}

		// Set MaxMaterialIndices
		if (auto* Collection = LODData.MaterialCollection.Get())
		{
			LODData.MaxMaterialIndices.Set(FMath::Max(Collection->GetMaxMaterialIndices(), 1));
			MaterialCollectionsToInitialize.AddUnique(Collection);
		}
		else
		{
			LODData.MaxMaterialIndices.Set(1);
		}
	}

	// Initialize all used collections
	for (auto* Collection : MaterialCollectionsToInitialize)
	{
		Collection->InitializeCollection();
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
	WorldRoot->RecreatePhysicsState();
}

void AVoxelWorld::RecreateRender()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsCreated());
	
	UpdateDynamicLODSettings();
	UpdateDynamicRendererSettings();

	
	LODManager->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), LODManager);

	Renderer->Destroy();
	FVoxelUtilities::DeleteTickable(GetWorld(), Renderer);

	DestroyVoxelComponents();

	Renderer = CreateRenderer();
	LODManager = CreateLODManager();
	
}

void AVoxelWorld::RecreateSpawners()
{
	if (SpawnerConfig)
	{
		FVoxelMessages::Info("Spawners are only available in Voxel Plugin Pro", this);
	}
}

void AVoxelWorld::RecreateAll(const FVoxelWorldCreateInfo& Info)
{
	check(IsCreated());

	DestroyWorldInternal();
	CreateWorldInternal(Info);
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
	if (Data->IsDirty() && GetTransientPackage() != nullptr)
	{
		if (!SaveObject && ensure(IVoxelWorldEditor::GetVoxelWorldEditor()))
		{
			const EAppReturnType::Type Result = FMessageDialog::Open(
				EAppMsgType::YesNo,
				VOXEL_LOCTEXT("The voxel world Save Object is null. You need to create one now if you don't want to lose your changes.\n\nSave changes?"));
			
			if (Result == EAppReturnType::No)
			{
				// Clear dirty flag so we don't get more annoying popups
				Data->ClearDirtyFlag();
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

			Data->ClearDirtyFlag();
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

inline bool CanLoad(const TVoxelSharedPtr<FVoxelData>& Data)
{
	if (Data->IsDirty())
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
	if (!CanLoad(Data))
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
	if (!CanLoad(Data))
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

	if (auto* CastedObject = Cast<UMaterialInterface>(Object))
	{
		MarkPackageDirty();
		
		VoxelMaterial = CastedObject;
		MaterialConfig = EVoxelMaterialConfig::RGB;
		RecreateRender();
	}
}

void IVoxelWorldEditor::SetVoxelWorldEditor(TSharedPtr<IVoxelWorldEditor> InVoxelWorldEditor)
{
	check(!VoxelWorldEditor.IsValid());
	VoxelWorldEditor = InVoxelWorldEditor;
}

TSharedPtr<IVoxelWorldEditor> IVoxelWorldEditor::VoxelWorldEditor;
IVoxelEditorDelegatesInterface::FBindEditorDelegates IVoxelEditorDelegatesInterface::BindEditorDelegatesDelegate;
#endif