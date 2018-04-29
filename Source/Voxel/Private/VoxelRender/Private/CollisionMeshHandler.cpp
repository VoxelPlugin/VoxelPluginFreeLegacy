// Copyright 2018 Phyronnaz

#include "CollisionMeshHandler.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "VoxelInvokerComponent.h"
#include "VoxelWorld.h"
#include "VoxelPolygonizerForCollisions.h"
#include "Async.h"
#include "VoxelCrashReporter.h"

DECLARE_CYCLE_STAT(TEXT("FCollisionMeshHandler::StartTasksTick"), STAT_FCollisionMeshHandler_StartTasksTick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FCollisionMeshHandler::EndTasksTick"), STAT_FCollisionMeshHandler_EndTasksTick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FCollisionMeshHandler::EndTasksTick.EnsureCompletion"), STAT_FCollisionMeshHandler_EndTasksTick_EnsureCompletion, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FCollisionMeshHandler::UpdateInBox"), STAT_FCollisionMeshHandler_UpdateInBox, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FCollisionMeshHandler::Update"), STAT_FCollisionMeshHandler_Update, STATGROUP_Voxel);

FAsyncCollisionTask::FAsyncCollisionTask(FVoxelData* Data, FIntVector ChunkPosition, bool bEnableRender)
	: Data(Data)
	, ChunkPosition(ChunkPosition)
	, bEnableRender(bEnableRender)
{

}

void FAsyncCollisionTask::DoWork()
{
	FVoxelPolygonizerForCollisions* Poly = new FVoxelPolygonizerForCollisions(Data, ChunkPosition, bEnableRender);
	bool bSuccess = Poly->CreateSection(Section);
	if (!bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
		Section.Reset();
	}
	delete Poly;
}


//////////////////////////////////////////////////////////////////////////


FCollisionMeshHandler::FCollisionMeshHandler(TWeakObjectPtr<UVoxelInvokerComponent> const Invoker, AVoxelWorld* const World, AActor* const ChunksOwner, FQueuedThreadPool* const Pool)
	: Invoker(Invoker)
	, World(World)
	, Pool(Pool)
{
	check(Invoker.IsValid());
	CurrentCenter = World->GlobalToLocal(Invoker.Get()->GetOwner()->GetActorLocation());

	for (int X = 0; X < 2; X++)
	{
		for (int Y = 0; Y < 2; Y++)
		{
			for (int Z = 0; Z < 2; Z++)
			{
				UVoxelProceduralMeshComponent* Chunk = NewObject<UVoxelProceduralMeshComponent>(ChunksOwner, NAME_None, RF_Transient);
				Chunk->bUseAsyncCooking = true;
				Chunk->SetupAttachment(World->GetRootComponent(), NAME_None);
				Chunk->RegisterComponent();
				Chunk->SetWorldScale3D(FVector::OneVector * World->GetVoxelSize());
				Chunk->SetWorldLocation(World->LocalToGlobal(CurrentCenter + FIntVector(X - 1, Y - 1, Z - 1) * CHUNKSIZE_FC));
				Chunk->BodyInstance = World->GetCollisionPresets();
				Components[X][Y][Z] = Chunk;
				Tasks[X][Y][Z] = nullptr;
				Update(X, Y, Z);
			}
		}
	}
	EndTasksTick();
}

FCollisionMeshHandler::~FCollisionMeshHandler()
{
	for (int X = 0; X < 2; X++)
	{
		for (int Y = 0; Y < 2; Y++)
		{
			for (int Z = 0; Z < 2; Z++)
			{
				if (Tasks[X][Y][Z])
				{
					if (!Tasks[X][Y][Z]->Cancel())
					{
						Tasks[X][Y][Z]->EnsureCompletion();
					}
					delete Tasks[X][Y][Z];
				}
			}
		}
	}
}

void FCollisionMeshHandler::StartTasksTick()
{
	SCOPE_CYCLE_COUNTER(STAT_FCollisionMeshHandler_StartTasksTick);

	if (!Invoker.IsValid())
	{
		UE_LOG(LogVoxel, Error, TEXT("Invalid invoker"));
		return;
	}
	const FIntVector NewPosition = World->GlobalToLocal(Invoker.Get()->GetOwner()->GetActorLocation());
	const FIntVector Delta = NewPosition - CurrentCenter;

	if (Delta.X > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[0][0][0];
		tmp[0][1] = Components[0][0][1];
		tmp[1][0] = Components[0][1][0];
		tmp[1][1] = Components[0][1][1];

		Components[0][0][0] = Components[1][0][0];
		Components[0][0][1] = Components[1][0][1];
		Components[0][1][0] = Components[1][1][0];
		Components[0][1][1] = Components[1][1][1];

		Components[1][0][0] = tmp[0][0];
		Components[1][0][1] = tmp[0][1];
		Components[1][1][0] = tmp[1][0];
		Components[1][1][1] = tmp[1][1];

		CurrentCenter += FIntVector(CHUNKSIZE_FC, 0, 0);

		ChunksToUpdate.Add(FIntVector(true, false, false));
		ChunksToUpdate.Add(FIntVector(true, true, false));
		ChunksToUpdate.Add(FIntVector(true, false, true));
		ChunksToUpdate.Add(FIntVector(true, true, true));
	}
	else if (-Delta.X > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[1][0][0];
		tmp[0][1] = Components[1][0][1];
		tmp[1][0] = Components[1][1][0];
		tmp[1][1] = Components[1][1][1];

		Components[1][0][0] = Components[0][0][0];
		Components[1][0][1] = Components[0][0][1];
		Components[1][1][0] = Components[0][1][0];
		Components[1][1][1] = Components[0][1][1];

		Components[0][0][0] = tmp[0][0];
		Components[0][0][1] = tmp[0][1];
		Components[0][1][0] = tmp[1][0];
		Components[0][1][1] = tmp[1][1];

		CurrentCenter -= FIntVector(CHUNKSIZE_FC, 0, 0);

		ChunksToUpdate.Add(FIntVector(false, false, false));
		ChunksToUpdate.Add(FIntVector(false, true, false));
		ChunksToUpdate.Add(FIntVector(false, false, true));
		ChunksToUpdate.Add(FIntVector(false, true, true));
	}
	if (Delta.Y > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[0][0][0];
		tmp[0][1] = Components[0][0][1];
		tmp[1][0] = Components[1][0][0];
		tmp[1][1] = Components[1][0][1];

		Components[0][0][0] = Components[0][1][0];
		Components[0][0][1] = Components[0][1][1];
		Components[1][0][0] = Components[1][1][0];
		Components[1][0][1] = Components[1][1][1];

		Components[0][1][0] = tmp[0][0];
		Components[0][1][1] = tmp[0][1];
		Components[1][1][0] = tmp[1][0];
		Components[1][1][1] = tmp[1][1];

		CurrentCenter += FIntVector(0, CHUNKSIZE_FC, 0);

		ChunksToUpdate.Add(FIntVector(false, true, false));
		ChunksToUpdate.Add(FIntVector(true, true, false));
		ChunksToUpdate.Add(FIntVector(false, true, true));
		ChunksToUpdate.Add(FIntVector(true, true, true));
	}
	else if (-Delta.Y > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[0][1][0];
		tmp[0][1] = Components[0][1][1];
		tmp[1][0] = Components[1][1][0];
		tmp[1][1] = Components[1][1][1];

		Components[0][1][0] = Components[0][0][0];
		Components[0][1][1] = Components[0][0][1];
		Components[1][1][0] = Components[1][0][0];
		Components[1][1][1] = Components[1][0][1];

		Components[0][0][0] = tmp[0][0];
		Components[0][0][1] = tmp[0][1];
		Components[1][0][0] = tmp[1][0];
		Components[1][0][1] = tmp[1][1];

		CurrentCenter -= FIntVector(0, CHUNKSIZE_FC, 0);

		ChunksToUpdate.Add(FIntVector(false, false, false));
		ChunksToUpdate.Add(FIntVector(true, false, false));
		ChunksToUpdate.Add(FIntVector(false, false, true));
		ChunksToUpdate.Add(FIntVector(true, false, true));
	}
	if (Delta.Z > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[0][0][0];
		tmp[0][1] = Components[0][1][0];
		tmp[1][0] = Components[1][0][0];
		tmp[1][1] = Components[1][1][0];

		Components[0][0][0] = Components[0][0][1];
		Components[0][1][0] = Components[0][1][1];
		Components[1][0][0] = Components[1][0][1];
		Components[1][1][0] = Components[1][1][1];

		Components[0][0][1] = tmp[0][0];
		Components[0][1][1] = tmp[0][1];
		Components[1][0][1] = tmp[1][0];
		Components[1][1][1] = tmp[1][1];

		CurrentCenter += FIntVector(0, 0, CHUNKSIZE_FC);

		ChunksToUpdate.Add(FIntVector(false, false, true));
		ChunksToUpdate.Add(FIntVector(true, false, true));
		ChunksToUpdate.Add(FIntVector(false, true, true));
		ChunksToUpdate.Add(FIntVector(true, true, true));
	}
	else if (-Delta.Z > CHUNKSIZE_FC / 2)
	{
		UVoxelProceduralMeshComponent* tmp[2][2];

		tmp[0][0] = Components[0][0][1];
		tmp[0][1] = Components[0][1][1];
		tmp[1][0] = Components[1][0][1];
		tmp[1][1] = Components[1][1][1];

		Components[0][0][1] = Components[0][0][0];
		Components[0][1][1] = Components[0][1][0];
		Components[1][0][1] = Components[1][0][0];
		Components[1][1][1] = Components[1][1][0];

		Components[0][0][0] = tmp[0][0];
		Components[0][1][0] = tmp[0][1];
		Components[1][0][0] = tmp[1][0];
		Components[1][1][0] = tmp[1][1];

		CurrentCenter -= FIntVector(0, 0, CHUNKSIZE_FC);

		ChunksToUpdate.Add(FIntVector(false, false, false));
		ChunksToUpdate.Add(FIntVector(true, false, false));
		ChunksToUpdate.Add(FIntVector(false, true, false));
		ChunksToUpdate.Add(FIntVector(true, true, false));
	}

	for (auto P : ChunksToUpdate)
	{
		Update(P.X, P.Y, P.Z);
	}
	ChunksToUpdate.Reset();
}

void FCollisionMeshHandler::EndTasksTick()
{
	SCOPE_CYCLE_COUNTER(STAT_FCollisionMeshHandler_EndTasksTick);

	for (int X = 0; X < 2; X++)
	{
		for (int Y = 0; Y < 2; Y++)
		{
			for (int Z = 0; Z < 2; Z++)
			{
				auto& Task = Tasks[X][Y][Z];
				auto& Component = Components[X][Y][Z];
				if (Task)
				{
					{
						SCOPE_CYCLE_COUNTER(STAT_FCollisionMeshHandler_EndTasksTick_EnsureCompletion);
						Task->EnsureCompletion();
					}
					Component->SetProcMeshSection(0, Task->GetTask().Section);
					Component->SetWorldLocation(World->LocalToGlobal(Task->GetTask().ChunkPosition), false, nullptr, ETeleportType::TeleportPhysics);
					delete Task;
					Task = nullptr;
				}
			}
		}
	}
}
bool FCollisionMeshHandler::IsValid()
{
	return Invoker.IsValid();
};

void  FCollisionMeshHandler::Destroy()
{
	for (int X = 0; X < 2; X++)
	{
		for (int Y = 0; Y < 2; Y++)
		{
			for (int Z = 0; Z < 2; Z++)
			{
				Components[X][Y][Z]->DestroyComponent();
			}
		}
	}
}

void  FCollisionMeshHandler::UpdateInBox(const FIntBox& Box)
{
	SCOPE_CYCLE_COUNTER(STAT_FCollisionMeshHandler_UpdateInBox);

	for (int X = 0; X < 2; X++)
	{
		for (int Y = 0; Y < 2; Y++)
		{
			for (int Z = 0; Z < 2; Z++)
			{
				FIntVector P(X - 1, Y - 1, Z - 1);
				FIntBox CurrentBox(CurrentCenter + P * CHUNKSIZE_FC, CurrentCenter + P * CHUNKSIZE_FC + FIntVector(CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1));
				if (CurrentBox.Intersect(Box))
				{
					ChunksToUpdate.Add(FIntVector(X, Y, Z));
				}
			}
		}
	}
}

void  FCollisionMeshHandler::Update(const int X, const int Y, const int Z)
{
	SCOPE_CYCLE_COUNTER(STAT_FCollisionMeshHandler_Update);

	check(0 <= X && X < 2);
	check(0 <= Y && Y < 2);
	check(0 <= Z && Z < 2);

	const FIntVector ChunkPosition = CurrentCenter + FIntVector(X - 1, Y - 1, Z - 1) * CHUNKSIZE_FC;

	auto& Task = Tasks[X][Y][Z];
	auto& Component = Components[X][Y][Z];
	
	FIntVector P(X - 1, Y - 1, Z - 1);
	FIntBox Bounds(CurrentCenter + P * CHUNKSIZE_FC, CurrentCenter + P * CHUNKSIZE_FC + FIntVector(CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1, CHUNKSIZE_FC + 1));

	if (World->GetBounds().Intersect(Bounds))
	{
		check(!Task);
		Task = new FAsyncTask<FAsyncCollisionTask>(World->GetData(), ChunkPosition, World->GetDebugCollisions());
		Task->StartBackgroundTask(Pool);
	}
	else
	{
		if (Component->GetProcMeshSection(0) && Component->GetProcMeshSection(0)->ProcIndexBuffer.Num() > 0)
		{
			Component->SetProcMeshSection(0, FVoxelProcMeshSection());
		}
	}
}