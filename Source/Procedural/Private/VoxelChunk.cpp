// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "ChunkOctree.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"
#include "DrawDebugHelpers.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
AVoxelChunk::AVoxelChunk() : bNeedSectionUpdate(false), Task(nullptr), bNeedDeletion(false), bAdjacentChunksNeedUpdate(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;
}

AVoxelChunk::~AVoxelChunk()
{

}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();
}

void AVoxelChunk::Tick(float DeltaTime)
{
	if (bNeedSectionUpdate && Task != nullptr && Task->IsDone())
	{
		SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);
		bNeedSectionUpdate = false;
		PrimaryMesh->SetProcMeshSection(0, Section);
		delete Task;
		Task = nullptr;
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
}

void AVoxelChunk::Init(FIntVector position, int depth, AVoxelWorld* world)
{
	check(world);

	Position = position;
	Depth = depth;
	World = world;

	FString name = FString::FromInt(position.X) + ", " + FString::FromInt(position.Y) + ", " + FString::FromInt(position.Z);
	FVector relativeLocation = (FVector)position;

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorLabel(name);
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);

	// Configure primary mesh
	PrimaryMesh->SetMaterial(0, world->VoxelMaterial);
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	// Update adjacents
	bAdjacentChunksNeedUpdate = true;
}

void AVoxelChunk::Update(bool async)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	// Make sure we've ticked
	Tick(0);

	if (Task == nullptr || Task->IsDone())
	{
		if (Task != nullptr)
		{
			// Should not be necessary
			Task->EnsureCompletion();
			delete Task;
		}

		for (int i = 0; i < 6; i++)
		{
			if (Depth == 0)
			{
				ChunkHasHigherRes[i] = false;
			}
			else
			{
				TransitionDirection Direction = (TransitionDirection)i;
				ChunkHasHigherRes[i] = (GetChunk(Direction) != nullptr) && (GetChunk(Direction)->GetDepth() > Depth);
			}
		}

		Task = new FAsyncTask<VoxelThread>(this);
		if (async)
		{
			Task->StartBackgroundTask(World->ThreadPool);
		}
		else
		{
			Task->StartSynchronousTask();
		}
		bNeedSectionUpdate = true;
	}
}

void AVoxelChunk::BasicUpdate()
{
	int Width = 16 << Depth;
	for (int i = 0; i < 6; i++)
	{
		TransitionDirection Direction = (TransitionDirection)i;
		bool bHigherRes = (GetChunk(Direction) != nullptr) && (GetChunk(Direction)->GetDepth() > Depth);
		if (ChunkHasHigherRes[i] != bHigherRes)
		{
			if (Task == nullptr || Task->IsDone())
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

int AVoxelChunk::GetDepth()
{
	return Depth;
}

signed char AVoxelChunk::GetValue(int x, int y, int z)
{
	return World->GetValue(Position + FIntVector(x, y, z));
}

FColor AVoxelChunk::GetColor(int x, int y, int z)
{
	return World->GetColor(Position + FIntVector(x, y, z));
}

AVoxelChunk* AVoxelChunk::GetChunk(TransitionDirection direction)
{
	int Width = 16 << Depth;
	TArray<FIntVector> L = { FIntVector(-Width, 0, 0) , FIntVector(Width, 0, 0) , FIntVector(0, -Width, 0), FIntVector(0, Width, 0) , FIntVector(0, 0, -Width) , FIntVector(0, 0, Width) };

	FIntVector P = Position + FIntVector(Width / 2, Width / 2, Width / 2) + L[direction];
	if (World->IsInWorld(P))
	{
		return World->GetChunkAt(P);
	}
	else
	{
		return nullptr;
	}
}
