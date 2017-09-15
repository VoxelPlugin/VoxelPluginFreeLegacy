// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunk.h"
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

	DebugLineBatch = CreateDefaultSubobject<ULineBatchComponent>(FName("LineBatch"));

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

	// Update adjacent
	bAdjacentChunksNeedUpdate = true;

	// Debug
	if (World->bDrawChunksBorders)
	{
		int w = Width();

		TArray<FBatchedLine> Lines;
		TArray<FIntVector> Starts = { FIntVector(0, 0, 0), FIntVector(w, 0, 0), FIntVector(w, w, 0), FIntVector(0, w, 0),
									  FIntVector(0, 0, 0), FIntVector(w, 0, 0), FIntVector(0, w, 0), FIntVector(w, w, 0),
									  FIntVector(0, 0, w), FIntVector(w, 0, w), FIntVector(w, w, w), FIntVector(0, w, w) };

		TArray<FIntVector> Ends = { FIntVector(w, 0, 0), FIntVector(w, w, 0), FIntVector(0, w, 0), FIntVector(0, 0, 0),
									FIntVector(0, 0, w), FIntVector(w, 0, w), FIntVector(0, w, w), FIntVector(w, w, w),
									FIntVector(w, 0, w), FIntVector(w, w, w), FIntVector(0, w, w), FIntVector(0, 0, w) };

		for (int i = 0; i < Starts.Num(); i++)
		{
			FVector Start = World->GetTransform().TransformPosition((FVector)(Position + Starts[i]));
			FVector End = World->GetTransform().TransformPosition((FVector)(Position + Ends[i]));
			Lines.Add(FBatchedLine(Start, End, FColor::Red, -1, Depth * Depth * Depth * Depth + 10, 0));
		}
		DebugLineBatch->DrawLines(Lines);
	}
}

void AVoxelChunk::Update(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	// Make sure we've ticked
	Tick(0);

	bAsync = false;

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
			Task->StartBackgroundTask(World->GetThreadPool());
			bNeedSectionUpdate = true;
		}
		else
		{
			Task->StartSynchronousTask();
			// Avoid holes
			UpdateSection();
		}
	}
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
	DebugLineBatch->Flush();
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
