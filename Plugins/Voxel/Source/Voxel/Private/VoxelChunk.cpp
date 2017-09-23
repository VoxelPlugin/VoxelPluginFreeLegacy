// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunk.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"
#include "Misc/IQueuedWork.h"
#include "AI/Navigation/NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine.h"
#include "Camera/PlayerCameraManager.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
AVoxelChunk::AVoxelChunk() : bNeedSectionUpdate(false), Task(nullptr), bNeedDeletion(false), bAdjacentChunksNeedUpdate(false), World(nullptr), bIsUsed(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RootComponent = PrimaryMesh;

	DebugLineBatch = CreateDefaultSubobject<ULineBatchComponent>(FName("LineBatch"));

	InstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("InstancedMesh"));

	ChunkHasHigherRes.SetNum(6);
}

void AVoxelChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsUsed)
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

	// Set material
	PrimaryMesh->SetMaterial(0, NewWorld->VoxelMaterial);

	// Force world update before this
	AddTickPrerequisiteActor(World);

	// Update adjacent
	bAdjacentChunksNeedUpdate = true;

	// Grass
	InstancedMesh->SetStaticMesh(World->GrassMesh);

	// Set as used
	bIsUsed = true;

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

	check(bIsUsed);

	// Make sure we've ticked
	Tick(0);

	if (Task == nullptr)
	{
		// Update ChunkHasHigherRes
		for (int i = 0; i < 6; i++)
		{
			if (Depth == 0)
			{
				ChunkHasHigherRes[i] = false;
			}
			else
			{
				auto Direction = (TransitionDirection)i;
				ChunkHasHigherRes[i] = (GetChunk(Direction) != nullptr) && (GetChunk(Direction)->GetDepth() < Depth);
			}
		}

		Task = new VoxelThread(this);
		if (bAsync)
		{
			World->GetThreadPool()->AddQueuedWork(Task);
			bNeedSectionUpdate = true;
		}
		else
		{
			Task->DoThreadedWork();
			// Update immediately to avoid holes
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

	// If task if queued, remove it
	if (Task != nullptr)
	{
		World->GetThreadPool()->RetractQueuedWork(Task);
		delete Task;
		Task = nullptr;
	}
}

void AVoxelChunk::Delete()
{	// If task if queued, remove it
	if (Task != nullptr)
	{
		World->GetThreadPool()->RetractQueuedWork(Task);
		delete Task;
		Task = nullptr;
	}

	// Reset mesh & position & clear lines
	PrimaryMesh->SetProcMeshSection(0, FProcMeshSection());
	DebugLineBatch->Flush();
	SetActorLocation(FVector(0, 0, 0));
#if WITH_EDITOR
	SetActorLabel("PoolChunk");
#endif // WITH_EDITOR


	// Add to pool
	check(World);
	World->AddChunkToPool(this);


	// Reset variables
	bNeedSectionUpdate = false;
	bNeedDeletion = false;
	bAdjacentChunksNeedUpdate = false;
	World = nullptr;
	bIsUsed = false;
}

void AVoxelChunk::UpdateSection()
{
	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);
	check(Task->IsDone());

	InstancedMesh->ClearInstances();

	if (Depth == 0)
	{
		auto Section = Task->GetSection();

		if (PlayerCamera == nullptr)
		{
			// Find camera

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

		if (PlayerCamera)
		{
			for (int Index = 0; Index < Section.ProcIndexBuffer.Num(); Index += 3)
			{
				FVector A = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index]].Position;
				FVector B = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 1]].Position;
				FVector C = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 2]].Position;

				if (FVector::Distance(GetTransform().TransformPosition(A), PlayerCamera->GetTransform().GetLocation()) < World->GrassRenderDistance)
				{
					FVector X = B - A;
					FVector Y = C - A;

					const float SizeX = X.Size();
					const float SizeY = Y.Size();

					X.Normalize();
					Y.Normalize();

					for (int i = 0; i < World->GrassDensity; i++)
					{
						float CoordX = FMath::RandRange(0.f, SizeY);
						float CoordY = FMath::RandRange(0.f, SizeX);

						if (SizeY - CoordX * SizeY / SizeX < CoordY)
						{
							CoordX = SizeX - CoordX;
							CoordY = SizeY - CoordY;
						}

						FVector P = A + X * CoordX + Y * CoordY;
						InstancedMesh->AddInstance(FTransform(FRotator::ZeroRotator, GetTransform().TransformPosition(P)));
						//DrawDebugPoint(GetWorld(), GetTransform().TransformPosition(P), 2, FColor::Red, false, 1000, 0);
					}
				}
			}
		}
	}


	PrimaryMesh->SetProcMeshSection(0, Task->GetSection());
	delete Task;
	Task = nullptr;

	UNavigationSystem::UpdateComponentInNavOctree(*PrimaryMesh);
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
