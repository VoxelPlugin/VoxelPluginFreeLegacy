// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunk.h"
#include "ChunkOctree.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
AVoxelChunk::AVoxelChunk() : bNeedSectionUpdate(false), Task(nullptr), bNeedDeletion(false), bAdjacentChunksNeedUpdate(false), World(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;

	ChunkHasHigherRes.SetNum(6);
}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();
}

void AVoxelChunk::Tick(float DeltaTime)
{
	if (!World)
	{
		Destroy(this);
		return;
	}
	if (bNeedSectionUpdate && Task != nullptr && Task->IsDone())
	{
		bNeedSectionUpdate = false;
		UpdateSection();
	}
	if (bAdjacentChunksNeedUpdate && World->GetRebuildBorders())
	{
		bAdjacentChunksNeedUpdate = false;
		for (int i = 0; i < 6; i++)
		{
			AVoxelChunk* Chunk = GetChunk((TransitionDirection)i);
			if (Chunk != nullptr)
			{
				Chunk->BasicUpdate();
			}
		}
	}
	if (bNeedDeletion)
	{
		TimeUntilDeletion -= DeltaTime;
		if (TimeUntilDeletion < 0)
		{
			Delete();
		}
	}
	if (bUpdate)
	{
		bUpdate = false;
		Update(false);
	}
}

#if WITH_EDITOR
bool AVoxelChunk::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif //WITH_EDITOR

void AVoxelChunk::Init(FIntVector NewPosition, int NewDepth, AVoxelWorld* NewWorld)
{
	check(NewWorld);

	Position = NewPosition;
	Depth = NewDepth;
	World = NewWorld;

	FString Name = FString::FromInt(NewPosition.X) + ", " + FString::FromInt(NewPosition.Y) + ", " + FString::FromInt(NewPosition.Z);
	FVector RelativeLocation = (FVector)NewPosition;

	this->AttachToActor(NewWorld, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
#if WITH_EDITOR
	this->SetActorLabel(Name);
#endif
	this->SetActorRelativeLocation(RelativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);

	// Configure primary mesh
	PrimaryMesh->SetMaterial(0, NewWorld->VoxelMaterial);
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	// Update adjacents
	bAdjacentChunksNeedUpdate = true;
}

void AVoxelChunk::Update(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	// Make sure we've ticked
	Tick(0);

	if (Task == nullptr)
	{
		for (int i = 0; i < 6; i++)
		{
			if (Depth == 0)
			{
				ChunkHasHigherRes[i] = false;
			}
			else
			{
				TransitionDirection Direction = (TransitionDirection)i;
				ChunkHasHigherRes[i] = (GetChunk(Direction) != nullptr) && (GetChunk(Direction)->GetDepth() < Depth);
			}
		}

		Task = new FAsyncTask<VoxelThread>(this);
		if (bAsync)
		{
			Task->StartBackgroundTask(World->ThreadPool);
			bNeedSectionUpdate = true;
		}
		else
		{
			Task->StartSynchronousTask();
			// Avoid holes
			UpdateSection();
		}
	}
	//else
	//{
		//UE_LOG(VoxelLog, Warning, TEXT("Update called when already updating"));
	//}
}

void AVoxelChunk::BasicUpdate()
{
	for (int i = 0; i < 6; i++)
	{
		TransitionDirection Direction = (TransitionDirection)i;
		bool bHigherRes = (GetChunk(Direction) != nullptr) && (GetChunk(Direction)->GetDepth() < Depth);
		if (ChunkHasHigherRes[i] != bHigherRes)
		{
			if (Task == nullptr)
			{
				Update(true);
				return;
			}
			else
			{
				ChunkHasHigherRes[i] = bHigherRes;
			}
		}
	}
}

void AVoxelChunk::Unload()
{
	bNeedDeletion = true;
	TimeUntilDeletion = World->GetDeletionDelay();
	bAdjacentChunksNeedUpdate = true;
}

void AVoxelChunk::Delete()
{
	if (Task != nullptr)
	{
		Task->EnsureCompletion();
		delete Task;
	}
	if (this->IsValidLowLevel() && !this->IsPendingKill())
	{
		this->Destroy();
	}
}

void AVoxelChunk::UpdateSection()
{
	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);
	Task->EnsureCompletion();
	PrimaryMesh->SetProcMeshSection(0, Task->GetTask().Section);
	delete Task;
	Task = nullptr;
}

int AVoxelChunk::GetDepth() const
{
	return Depth;
}

int AVoxelChunk::Width() const
{
	return 16 << Depth;
}

float AVoxelChunk::GetValue(int x, int y, int z) const
{
	return World->GetValue(Position + FIntVector(x, y, z));
}

FColor AVoxelChunk::GetColor(int x, int y, int z) const
{
	return World->GetColor(Position + FIntVector(x, y, z));
}

AVoxelChunk* AVoxelChunk::GetChunk(TransitionDirection Direction) const
{
	TArray<FIntVector> L = { FIntVector(-Width(), 0, 0) , FIntVector(Width(), 0, 0) , FIntVector(0, -Width(), 0), FIntVector(0, Width(), 0) , FIntVector(0, 0, -Width()) , FIntVector(0, 0, Width()) };

	FIntVector P = Position + FIntVector(Width() / 2, Width() / 2, Width() / 2) + L[Direction];
	if (World->IsInWorld(P))
	{
		return World->GetChunkAt(P);
	}
	else
	{
		return nullptr;
	}
}
