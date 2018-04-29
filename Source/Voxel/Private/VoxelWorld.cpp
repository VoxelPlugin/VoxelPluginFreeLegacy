// Copyright 2018 Phyronnaz

#include "VoxelWorld.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "IVoxelRender.h"
#include "Components/CapsuleComponent.h"
#include "VoxelWorldGenerators/FlatWorldGenerator.h"
#include "VoxelInvokerComponent.h"
#include "VoxelWorldEditorInterface.h"
#include "VoxelUtilities.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "VoxelChunksOwner.h"
#include "Octree.h"
#include "VoxelCrashReporter.h"
#include "Engine/World.h"
#include "ConstructorHelpers.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "VoxelProceduralMeshComponent.h"
#include "VoxelWorldGenerators/VoxelShapeWorldGenerators.h"

DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::Tick"), STAT_VoxelWorld_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::ReceiveData"), STAT_VoxelWorld_ReceiveData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::SendData"), STAT_VoxelWorld_SendData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AVoxelWorld::GetIntersection"), STAT_VoxelWorld_GetIntersection, STATGROUP_Voxel);

AVoxelWorld::AVoxelWorld()
	: VoxelWorldEditorClass(nullptr)
	, LOD(9)
	, LODLimit(19)
	, bIsCreated(false)
	, VoxelSize(100)
	, Seed(100)
	, MeshThreadCount(2)
	, CollisionsThreadCount(2)
	, CollisionsUpdateRate(30)
	, LODUpdateRate(15)
	, VoxelWorldEditor(nullptr)
	, bCreateWorldAutomatically(true)
	, ChunksFadeDuration(1)
	, AsyncTasksThreadPool(FQueuedThreadPool::Allocate())
	, bCreateAdditionalVerticesForMaterialsTransitions(true)
	, bEnableNormals(true)
	, MaxCollisionsLOD(3)
{
	PrimaryActorTick.bCanEverTick = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

#if WITH_EDITORONLY_DATA
	auto SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTextureObject;
		FName ID_Voxel;
		FText NAME_Voxel;
		FConstructorStatics()
			: SpriteTextureObject(TEXT("/Engine/EditorResources/S_Terrain"))
			, ID_Voxel(TEXT("VoxelWorld"))
			, NAME_Voxel(NSLOCTEXT("VoxelCategory", "Voxel World", "Voxel World"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	if (SpriteComponent)
	{
		SpriteComponent->Sprite = ConstructorStatics.SpriteTextureObject.Get();
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Voxel;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Voxel;
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->bReceivesDecals = false;
	}
#endif // WITH_EDITORONLY_DATA

	AsyncTasksThreadPool->Create(1, 1024 * 1024);

}

AVoxelWorld::~AVoxelWorld()
{
	delete AsyncTasksThreadPool;
}


void AVoxelWorld::CreateWorld()
{
	if (!IsCreated())
	{
		CreateWorldInternal();
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Can't create world: already created"));
	}
}

void AVoxelWorld::DestroyWorld()
{
	if (IsCreated())
	{
		DestroyWorldInternal();
		for (auto& Component : GetComponents())
		{
			auto Mesh = Cast<UVoxelProceduralMeshComponent>(Component);
			if (Mesh)
			{
				Mesh->DestroyComponent();
			}
		}
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Can't destroy world: not created"));
	}
}

void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	FVoxelCrashReporter::bIgnoreMessages = false;

	if (!IsCreated() && bCreateWorldAutomatically)
	{
		CreateWorldInternal();
	}
}

void AVoxelWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Make sure all async tasks are ended
	AsyncTasksThreadPool->Destroy();

	Render.Reset();
	Data.Reset();

	// Make sure all physics threads are ended (maybe?). Might reduce crashes
	FPlatformProcess::Sleep(0.1f);
}

void AVoxelWorld::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_Tick);

	Super::Tick(DeltaTime);
	
	if (GetActorScale3D() != FVector::OneVector)
	{
		SetActorScale3D(FVector::OneVector);
	}
	if (GetActorRotation() != FRotator::ZeroRotator)
	{
		SetActorRotation(FRotator::ZeroRotator);
	}

	if (IsCreated())
	{
		Render->Tick(DeltaTime);

		if (GetWorld()->WorldType == EWorldType::Editor)
		{
			ChunksOwner->SetActorTransform(GetActorTransform());
		}

	}
	
}

#if WITH_EDITOR
bool AVoxelWorld::ShouldTickIfViewportsOnly() const
{
	return true;
}

bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	return (!bIsCreated || InProperty->GetNameCPP() == TEXT("SaveObject")) && Super::CanEditChange(InProperty);
}

void AVoxelWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	WorldSizeInVoxel = CHUNK_SIZE * (1 << LOD);
}

#endif

float AVoxelWorld::GetValue(const FIntVector& Position) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetValue called but the world isn't created"));
		return 0;
	}

	if (IsInWorld(Position))
	{
		FVoxelMaterial Material;
		float Value;

		auto Octrees = Data->BeginGet(FIntBox(Position));
		Data->GetValueAndMaterial(Position, Value, Material);
		Data->EndGet(Octrees);

		return Value;
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Get value: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 0;
	}
}

FVoxelMaterial AVoxelWorld::GetMaterial(const FIntVector& Position) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetMaterial called but the world isn't created"));
		return FVoxelMaterial();
	}

	if (IsInWorld(Position))
	{
		FVoxelMaterial Material;
		float Value;

		auto Octrees = Data->BeginGet(FIntBox(Position));
		Data->GetValueAndMaterial(Position, Value, Material);
		Data->EndGet(Octrees);

		return Material;
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Get material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return FVoxelMaterial();
	}
}

void AVoxelWorld::SetValue(const FIntVector& Position, float Value)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("SetValue called but the world isn't created"));
		return;
	}

	if (IsInWorld(Position))
	{
		auto Octrees = Data->BeginSet(FIntBox(Position));
		Data->SetValue(Position, Value);
		Data->EndSet(Octrees);
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Get material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void AVoxelWorld::SetMaterial(const FIntVector& Position, const FVoxelMaterial& Material)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("SetMaterial called but the world isn't created"));
		return;
	}
	if (IsInWorld(Position))
	{
		auto Octrees = Data->BeginSet(FIntBox(Position));
		Data->SetMaterial(Position, Material);
		Data->EndSet(Octrees);
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("Set material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}


bool AVoxelWorld::IsInside(const FVector& Position)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("IsInside called but the world isn't created"));
		return false;
	}
	for (auto P : GetNeighboringPositions(Position))
	{
		if (GetValue(P) <= 0)
		{
			return true;
		}
	}
	return false;
}

void AVoxelWorld::GetSave(FVoxelWorldSave& OutSave) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetSave called but the world isn't created"));
		return;
	}
	Data->GetSave(OutSave);
}

void AVoxelWorld::LoadFromSave(const FVoxelWorldSave& Save, bool bReset)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("LoadFromSave called but the world isn't created"));
		return;
	}
	if (Save.LOD == LOD)
	{
		TArray<FIntVector> ModifiedPositions;
		Data->LoadFromSaveAndGetModifiedPositions(Save, ModifiedPositions, bReset);
		for (auto Position : ModifiedPositions)
		{
			if (IsInWorld(Position))
			{
				UpdateChunksAtPosition(Position);
			}
		}
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("LoadFromSave: Current Depth is %d while Save one is %d"), LOD, Save.LOD);
	}
}


AVoxelWorldEditorInterface* AVoxelWorld::GetVoxelWorldEditor() const
{
	return VoxelWorldEditor;
}

FVoxelData* AVoxelWorld::GetData() const
{
	return Data.Get();
}

FVoxelWorldGeneratorInstance* AVoxelWorld::GetWorldGenerator() const
{
	return InstancedWorldGenerator.Get();
}

int AVoxelWorld::GetSeed() const
{
	return Seed;
}

void AVoxelWorld::SetSeed(int InSeed)
{
	if (IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("Can't set seed when created"));
	}
	else
	{
		Seed = InSeed;
	}
}

uint8 AVoxelWorld::GetLOD() const
{
	return LOD;
}

void AVoxelWorld::SetVoxelMaterial(UMaterialInterface* NewMaterial)
{
	auto DynamicInstance = Cast<UMaterialInstanceDynamic>(NewMaterial);
	if (DynamicInstance)
	{
		VoxelMaterial = DynamicInstance->Parent;
		VoxelMaterialInstance = DynamicInstance;
	}
	else
	{
		VoxelMaterial = NewMaterial;
		VoxelMaterialInstance = UMaterialInstanceDynamic::Create(NewMaterial, this);
	}

	if (IsCreated())
	{
		for (auto& Component : GetComponents())
		{
			auto Mesh = Cast<UVoxelProceduralMeshComponent>(Component);
			if (Mesh)
			{
				for (int Index = 0; Index < Mesh->GetNumSections(); Index++)
				{
					UMaterialInstanceDynamic* NewMat = UMaterialInstanceDynamic::Create(VoxelMaterial, this);
					NewMat->CopyInterpParameters(VoxelMaterialInstance);
					NewMat->SetScalarParameterValue(FName(TEXT("EndTime")), 0); // Needed for first init, as 1e10 is too big
					NewMat->SetScalarParameterValue(FName(TEXT("FadeDuration")), GetChunksFadeDuration());
					NewMat->SetScalarParameterValue(FName(TEXT("StartTime")), -GetChunksFadeDuration());
					NewMat->SetScalarParameterValue(FName(TEXT("EndTime")), 1e10);
					Mesh->SetMaterial(Index, NewMat);
				}
			}
		}
	}
}

void AVoxelWorld::SetLOD(uint8 NewLOD)
{
	if (!IsCreated())
	{
		LOD = NewLOD;
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("SetLOD: World is created"));
	}
}

void AVoxelWorld::SetWorldGenerator(UVoxelWorldGenerator* NewGenerator)
{
	if (!NewGenerator)
	{
		UE_LOG(LogVoxel, Error, TEXT("SetWorldGenerator: NewGenerator is NULL"));
		return;
	}


	WorldGenerator.UseClassOrObject = EVoxelWorldGeneratorClassOrObject::Object;
	WorldGenerator.WorldGeneratorObject = NewGenerator;

	if (IsCreated())
	{
		InstancedWorldGenerator = NewGenerator->GetWorldGenerator();
		InstancedWorldGenerator->SetVoxelWorld(this);
		Data->SetWorldGenerator(InstancedWorldGenerator.ToSharedRef());
		Data->DiscardValuesByPredicate([](const FIntBox& Box) { return -1; });
	}	
}

void AVoxelWorld::SwapWorldGeneratorSphere(UVoxelWorldGenerator* NewGenerator, const FIntVector& Center, float Radius)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("SwapWorldGeneratorSphere: can't swap if not created"));
	}
	else
	{
		const FIntVector P(FMath::CeilToInt(Radius));
		const FIntBox SphereBox(Center - P, Center + P + FIntVector(1, 1, 1));

		auto IsInside = [&](FIntVector& V) { return (V - Center).Size() < Radius; };

		auto OldGeneratorInstance = InstancedWorldGenerator.ToSharedRef();
		auto NewGeneratorInstance = NewGenerator->GetWorldGenerator();
		NewGeneratorInstance->SetVoxelWorld(this);

		InstancedWorldGenerator = MakeShared<FVoxelSphereShapeWorldGenerator>(NewGeneratorInstance, OldGeneratorInstance, Center, Radius);

		Data->SetWorldGenerator(InstancedWorldGenerator.ToSharedRef());
		Data->DiscardValuesByPredicate(
			[&](const FIntBox& Box)
		{
			bool bAllInside = true;
			bool bAllOutside = true;
			for (auto& Corner : Box.GetCorners())
			{
				bool bInside = IsInside(Corner);
				bAllInside = bAllInside && bInside;
				bAllOutside = bAllOutside && !bInside;
			}
			if (bAllInside)
			{
				return 1;
			}
			else if (bAllOutside && (FMath::Max(SphereBox.Size().GetMin(), 1.f) > Box.Size().GetMax() || !SphereBox.Intersect(Box)))
			{
				return -1;
			}
			else
			{
				return 0;
			}
		});
	}
}

UMaterialInstanceDynamic* AVoxelWorld::GetVoxelMaterialDynamicInstance()
{
	if (!VoxelMaterialInstance && VoxelMaterial)
	{
		VoxelMaterialInstance = UMaterialInstanceDynamic::Create(VoxelMaterial, this);
	}
	return VoxelMaterialInstance;
}

UMaterialInterface* AVoxelWorld::GetVoxelMaterial() const
{
	return VoxelMaterial;
}

bool AVoxelWorld::GetEnableNormals() const
{
	return bEnableNormals;
}

int AVoxelWorld::GetLODLimit() const
{
	return LODLimit;
}

const FBodyInstance& AVoxelWorld::GetCollisionPresets() const
{
	return CollisionPresets;
}

UVoxelWorldSaveObject* AVoxelWorld::GetSaveObject() const
{
	return SaveObject;
}


int AVoxelWorld::GetMaxCollisionsLOD() const
{
	return MaxCollisionsLOD;
}

bool AVoxelWorld::GetDebugCollisions() const
{
	return bDebugCollisions;
}

float AVoxelWorld::GetCollisionsUpdateRate() const
{
	return CollisionsUpdateRate;
}

float AVoxelWorld::GetLODUpdateRate() const
{
	return LODUpdateRate;
}

float AVoxelWorld::GetChunksFadeDuration() const
{
	return ChunksFadeDuration;
}

int AVoxelWorld::GetCollisionsThreadCount() const
{
	return CollisionsThreadCount;
}

int AVoxelWorld::GetMeshThreadCount() const
{
	return MeshThreadCount;
}

FQueuedThreadPool* AVoxelWorld::GetAsyncTasksThreadPool() const
{
	return AsyncTasksThreadPool;
}

bool AVoxelWorld::GetCreateAdditionalVerticesForMaterialsTransitions() const
{
	return bCreateAdditionalVerticesForMaterialsTransitions;
}

FIntVector AVoxelWorld::GlobalToLocal(const FVector& Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position) / GetVoxelSize();
	return FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
}

FVector AVoxelWorld::GlobalToLocalFloat(const FVector& Position) const
{
	return GetTransform().InverseTransformPosition(Position) / GetVoxelSize();
}

FVector AVoxelWorld::LocalToGlobal(const FIntVector& Position) const
{
	return LocalToGlobalFloat((FVector)Position);
}

FVector AVoxelWorld::LocalToGlobalFloat(const FVector& Position) const
{
	return GetTransform().TransformPosition(GetVoxelSize() * Position);
}

TArray<FIntVector> AVoxelWorld::GetNeighboringPositions(const FVector& GlobalPosition) const
{
	FVector P = GetTransform().InverseTransformPosition(GlobalPosition) / GetVoxelSize();
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

void AVoxelWorld::UpdateChunksAtPosition(const FIntVector& Position)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("UpdateChunksAtPosition called but the world isn't created"));
		return;
	}
	Render->UpdateBox(FIntBox(Position));
}

void AVoxelWorld::UpdateChunksOverlappingBox(const FIntBox& Box)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("UpdateChunksOverlappingBox called but the world isn't created"));
		return;
	}
	Render->UpdateBox(Box);
}

void AVoxelWorld::UpdateAll()
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("UpdateAll called but the world isn't created"));
		return;
	}
	Render->UpdateBox(FIntBox::Infinite());
}

void AVoxelWorld::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	check(IsCreated());
	if (Invoker.IsValid())
	{
		Render->AddInvoker(Invoker);
		Invokers.Add(Invoker);
	}
}


void AVoxelWorld::CreateWorldInternal(AActor* InChunksOwner)
{
	check(!IsCreated());

	UE_LOG(LogVoxel, Warning, TEXT("Loading world"));
	
	SetActorScale3D(FVector::OneVector);
	SetActorRotation(FRotator::ZeroRotator.Quaternion());

	check(!Data.IsValid());
	check(!Render.IsValid());

	InstancedWorldGenerator = WorldGenerator.GetWorldGenerator();
	InstancedWorldGenerator->SetVoxelWorld(this);

	// Create Data
	Data = MakeShareable(new FVoxelData(LOD, InstancedWorldGenerator.ToSharedRef(), false, bEnableUndoRedo));

#if DO_CHECK
	FVoxelUtilities::TestRLE();
#endif

	// Create Render
	if (!InChunksOwner)
	{
		InChunksOwner = this;
	}
	Render = FVoxelRenderFactory::GetVoxelRender(RenderType, this, InChunksOwner);

	
	
	bIsCreated = true;

	// Load if possible
	if (SaveObject)
	{
		LoadFromSave(SaveObject->Save);
	}
}

void AVoxelWorld::DestroyWorldInternal()
{
	check(IsCreated());

	UE_LOG(LogVoxel, Warning, TEXT("Unloading world"));

	check(Render.IsValid());
	check(Data.IsValid());

	Render.Reset();
	Data.Reset(); // Data must be deleted AFTER Render

	bIsCreated = false;
}

void AVoxelWorld::CreateInEditor()
{
	if (VoxelWorldEditorClass)
	{
		// Create/Find VoxelWorldEditor
		{
			VoxelWorldEditor = nullptr;

			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), VoxelWorldEditorClass, FoundActors);

			for (auto Actor : FoundActors)
			{
				auto VoxelWorldEditorActor = Cast<AVoxelWorldEditorInterface>(Actor);
				if (VoxelWorldEditorActor)
				{
					VoxelWorldEditor = VoxelWorldEditorActor;
					break;
				}
			}
			if (!VoxelWorldEditor)
			{
				// else spawn
				FActorSpawnParameters Transient = FActorSpawnParameters();
				Transient.ObjectFlags = RF_Transient;
				VoxelWorldEditor = GetWorld()->SpawnActor<AVoxelWorldEditorInterface>(VoxelWorldEditorClass, Transient);
			}
		}
		VoxelWorldEditor->Init(this);


		if (IsCreated())
		{
			DestroyWorldInternal();
		}

		const bool bEnableUndoRedoSave = bEnableUndoRedo;
		bEnableUndoRedo = true;

		// Create/Find ChunksOwner
		{
			ChunksOwner = nullptr;

			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelChunksOwner::StaticClass(), FoundActors);

			for (auto Actor : FoundActors)
			{
				auto ChunksOwnerActor = Cast<AVoxelChunksOwner>(Actor);
				if (ChunksOwnerActor)
				{
					ChunksOwner = ChunksOwnerActor;
					break;
				}
			}
			if (!ChunksOwner)
			{
				// else spawn
				FActorSpawnParameters Transient = FActorSpawnParameters();
				Transient.ObjectFlags = RF_Transient;
				ChunksOwner = GetWorld()->SpawnActor<AVoxelChunksOwner>(AVoxelChunksOwner::StaticClass(), Transient);
			}

			ChunksOwner->World = this;
		}

		CreateWorldInternal(ChunksOwner);

		bEnableUndoRedo = bEnableUndoRedoSave;

		AddInvoker(VoxelWorldEditor->GetInvoker());

		UpdateAll();
	}
}

void AVoxelWorld::DestroyInEditor()
{
	if (IsCreated())
	{
		DestroyWorldInternal();
		ChunksOwner->Destroy();
	}
}

bool AVoxelWorld::IsCreated() const
{
	return bIsCreated;
}

int AVoxelWorld::GetLODAt(const FIntVector& Position) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetLODAt called but the world isn't created"));
		return 0;
	}
	if (IsInWorld(Position))
	{
		return Render->GetLODAtPosition(Position);
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("GetDepthAt: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 0;
	}
}

bool AVoxelWorld::IsInWorld(const FIntVector& Position) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("IsInWorld called but the world isn't created"));
		return false;
	}
	return Data->IsInWorld(Position);
}

bool AVoxelWorld::GetIntersectionBP(const FIntVector& Start, const FIntVector& End, FVector& GlobalPosition, FIntVector& VoxelPosition)
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetIntersection called but the world isn't created"));
		return false;
	}
	return GetIntersection(Start, End, GlobalPosition, VoxelPosition);
}

bool AVoxelWorld::GetIntersection(const FIntVector& Start, const FIntVector& End, FVector& OutGlobalPosition, FIntVector& OutVoxelPosition) const
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelWorld_GetIntersection);

	FIntVector Diff = End - Start;
	if (Diff.X != 0)
	{
		if (Diff.Y != 0 || Diff.Z != 0)
		{
			UE_LOG(LogVoxel, Error, TEXT("GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}
	else if (Diff.Y != 0)
	{
		if (Diff.X != 0 || Diff.Z != 0)
		{
			UE_LOG(LogVoxel, Error, TEXT("GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}
	else if (Diff.Z != 0)
	{
		if (Diff.X != 0 || Diff.Y != 0)
		{
			UE_LOG(LogVoxel, Error, TEXT("GetIntersection: Start and end should have 2 common coordinates"));
			return false;
		}
	}

	FIntVector RealStart(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y), FMath::Min(Start.Z, End.Z));
	FIntVector RealEnd(FMath::Max(Start.X, End.X) + 1, FMath::Max(Start.Y, End.Y) + 1, FMath::Max(Start.Z, End.Z) + 1);

	bool bFound = false;
	auto Octrees = Data->BeginGet(FIntBox(RealStart, RealEnd));
	float OldValue = Data->GetValue(RealStart.X, RealStart.Y, RealStart.Z);
	FIntVector OldPosition = RealStart;
	for (int X = RealStart.X; X < RealEnd.X; X++)
	{
		for (int Y = RealStart.Y; Y < RealEnd.Y; Y++)
		{
			for (int Z = RealStart.Z; Z < RealEnd.Z; Z++)
			{
				if (UNLIKELY(!Data->IsInWorld(X, Y, Z))) 
				{
					UE_LOG(LogVoxel, Error, TEXT("GetIntersection: Out of world!"));
					return false;
				}

				float Value = Data->GetValue(X, Y, Z);
				FIntVector Position(X, Y, Z);

				if (!FVoxelUtilities::HaveSameSign(OldValue, Value))
				{
					check(OldValue - Value != 0);
					const float t = OldValue / (OldValue - Value);

					FVector Q = t * static_cast<FVector>(Position) + (1 - t) * static_cast<FVector>(OldPosition);
					OutGlobalPosition = LocalToGlobalFloat(Q);
					OutVoxelPosition = Position;
					bFound = true;
				}

				OldValue = Value;
				OldPosition = Position;

				if (bFound)
				{
					break;
				}
			}
			if (bFound)
			{
				break;
			}
		}
		if (bFound)
		{
			break;
		}
	}
	Data->EndGet(Octrees);

	return bFound;
}

FVector AVoxelWorld::GetNormal(const FIntVector& Position) const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetNormal called but the world isn't created"));
		return FVector::ZeroVector;
	}

	int X = Position.X;
	int Y = Position.Y;
	int Z = Position.Z;

	auto Octrees = Data->BeginGet(FIntBox(FIntVector(Position.X - 1, Position.Y - 1, Position.Z - 1), FIntVector(Position.X + 2, Position.Y + 2, Position.Z + 2)));
	FVector Gradient = Data->GetGradient(Position);
	Data->EndGet(Octrees);

	return Gradient;
}

float AVoxelWorld::GetVoxelSize() const
{
	return VoxelSize;
}

FIntBox AVoxelWorld::GetBounds() const
{
	if (!IsCreated())
	{
		UE_LOG(LogVoxel, Error, TEXT("GetBounds called but the world isn't created"));
		return FIntBox();
	}
	return Data->GetBounds();
}