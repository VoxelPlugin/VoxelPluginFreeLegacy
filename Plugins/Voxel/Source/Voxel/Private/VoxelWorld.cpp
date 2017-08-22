// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"
#include <forward_list>

DEFINE_LOG_CATEGORY(VoxelLog)
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateAll"), STAT_UpdateAll, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ ApplyQueuedUpdates"), STAT_ApplyQueuedUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Add"), STAT_Add, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Remove"), STAT_Remove, STATGROUP_Voxel);

// Sets default values
AVoxelWorld::AVoxelWorld() : bNotCreated(true), Quality(0.75f), DeletionDelay(0.1f), Depth(10), HighResolutionDistanceOffset(25), bRebuildBorders(true), PlayerCamera(nullptr), bAutoFindCamera(true), bAutoUpdateCameraPosition(true)
{
	PrimaryActorTick.bCanEverTick = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);

	WorldGenerator = TSubclassOf<UVoxelWorldGenerator>(UVoxelWorldGenerator::StaticClass());
}


void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	bNotCreated = false;

	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(8);

	WorldGeneratorInstance = WorldGenerator.GetDefaultObject();
	if (WorldGeneratorInstance == nullptr)
	{
		NewObject<UVoxelWorldGenerator>(WorldGeneratorInstance);
	}

	Data = MakeShareable(new VoxelData(Depth, WorldGeneratorInstance));
	MainOctree = MakeShareable(new ChunkOctree(FIntVector::ZeroValue, Depth));

	UpdateCameraPosition(FVector::ZeroVector);
}

void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoFindCamera)
	{
		if (PlayerCamera == nullptr)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCameraManager::StaticClass(), FoundActors);

			if (FoundActors.Num() == 0)
			{
				UE_LOG(VoxelLog, Warning, TEXT("No camera found"));
			}
			else if (FoundActors.Num() == 1)
			{
				PlayerCamera = (APlayerCameraManager*)FoundActors[0];
			}
			else
			{
				UE_LOG(VoxelLog, Warning, TEXT("More than one camera found"));
			}
		}
	}

	if (bAutoUpdateCameraPosition && PlayerCamera != nullptr)
	{
		UpdateCameraPosition(PlayerCamera->GetTransform().GetLocation());
	}
}

AVoxelChunk* AVoxelWorld::GetChunkAt(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		TSharedPtr<ChunkOctree> Chunk = MainOctree->GetChunk(Position).Pin();
		check(Chunk.IsValid());
		return Chunk->GetVoxelChunk();
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Error: Cannot GetChunkAt (%d, %d, %d): Not in world"), Position.X, Position.Y, Position.Z);
		return nullptr;
	}
}

void AVoxelWorld::UpdateCameraPosition(FVector Position)
{
	// Reset to avoid references to destroyed chunks
	QueuedChunks.Reset();
	// Recreate octree
	MainOctree->CreateTree(this, Position);
	// Apply updates added when recreating octree
	ApplyQueuedUpdates(true);
}


float AVoxelWorld::GetValue(FIntVector Position) const
{
	return Data->GetValue(Position);
}

FColor AVoxelWorld::GetColor(FIntVector Position) const
{
	return Data->GetColor(Position);
}

void AVoxelWorld::SetValue(FIntVector Position, float Value) const
{
	Data->SetValue(Position, Value);
}

void AVoxelWorld::SetColor(FIntVector Position, FColor Color) const
{
	Data->SetColor(Position, Color);
}

TArray<FVoxelChunkSaveStruct> AVoxelWorld::GetSaveArray() const
{
	return Data->GetSaveArray();
}

void AVoxelWorld::LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray) const
{
	Data->LoadFromArray(SaveArray);
}


FIntVector AVoxelWorld::GlobalToLocal(FVector Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position);
	return FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
}

void AVoxelWorld::Add(FIntVector Position, int Strength) const
{
	SCOPE_CYCLE_COUNTER(STAT_Add);
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Data->GetValue(Position) - Strength);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
	}
}

void AVoxelWorld::Remove(FIntVector Position, int Strength) const
{
	SCOPE_CYCLE_COUNTER(STAT_Remove);
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Data->GetValue(Position) + Strength);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
	}
}


void AVoxelWorld::Update(FIntVector Position, bool bAsync)
{
	if (QueuedChunks.Num() != 0)
	{
		UE_LOG(VoxelLog, Warning, TEXT("Update called but there are still chunks in queue"));
	}
	QueueUpdate(Position);
	ApplyQueuedUpdates(bAsync);
}

void AVoxelWorld::QueueUpdate(FIntVector Position)
{
	int X = Position.X + Size() / 2;
	int Y = Position.Y + Size() / 2;
	int Z = Position.Z + Size() / 2;

	bool bXIsAtBorder = (X % 16 == 0) && (X != 0);
	bool bYIsAtBorder = (Y % 16 == 0) && (Y != 0);
	bool bZIsAtBorder = (Z % 16 == 0) && (Z != 0);

	QueueUpdate(MainOctree->GetChunk(Position));

	if (bXIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 0, 0)));
	}
	if (bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 8, 0)));
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 8, 0)));
	}
	if (bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 0, 8)));
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 0, 8)));
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 8, 8)));
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 8, 8)));
	}
}

void AVoxelWorld::QueueUpdate(TWeakPtr<ChunkOctree> Chunk)
{
	QueuedChunks.Add(Chunk);
}

void AVoxelWorld::ApplyQueuedUpdates(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyQueuedUpdates);
	//UE_LOG(VoxelLog, Log, TEXT("Updating %d chunks"), QueuedChunks.Num());

	for (auto& Chunk : QueuedChunks)
	{
		TSharedPtr<ChunkOctree> LockedObserver(Chunk.Pin());

		if (LockedObserver.IsValid())
		{
			LockedObserver->Update(bAsync);
		}
		else
		{
			UE_LOG(VoxelLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	QueuedChunks.Reset();
}

void AVoxelWorld::UpdateAll(bool bAsync) const
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAll);
	MainOctree->Update(bAsync);
}


#if WITH_EDITOR
bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth) || InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial))
		return ParentVal && bNotCreated;
	else
		return ParentVal;
}
#endif



bool AVoxelWorld::IsInWorld(FIntVector Position) const
{
	return Data->IsInWorld(Position);
}

int AVoxelWorld::Size() const
{
	return Data->Size();
}

float AVoxelWorld::GetDeletionDelay() const
{
	return DeletionDelay;
}

float AVoxelWorld::GetQuality() const
{
	return Quality;
}

float AVoxelWorld::GetHighResolutionDistanceOffset() const
{
	return HighResolutionDistanceOffset;
}

bool AVoxelWorld::GetRebuildBorders() const
{
	return bRebuildBorders;
}

TSharedPtr<ChunkOctree> AVoxelWorld::GetChunkOctree() const
{
	return TSharedPtr<ChunkOctree>(MainOctree);
}

TSharedPtr<ValueOctree> AVoxelWorld::GetValueOctree() const
{
	return Data->GetValueOctree();
}

