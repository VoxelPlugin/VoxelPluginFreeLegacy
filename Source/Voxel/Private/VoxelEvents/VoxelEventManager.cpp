// Copyright 2020 Phyronnaz

#include "VoxelEvents/VoxelEventManager.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelWorld.h"
#include "VoxelMinimal.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Num Voxel Events"), STAT_NumVoxelEvents, STATGROUP_VoxelCounters);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Event Manager - Num active or generated chunks"), STAT_VoxelEventManager_NumActiveOrGeneratedChunks, STATGROUP_VoxelCounters);

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelEventsMemory);

static TAutoConsoleVariable<int32> CVarShowEventsBounds(
	TEXT("voxel.events.ShowBounds"),
	0,
	TEXT("If true, will show event updates bounds"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelEventManagerSettings::FVoxelEventManagerSettings(const AVoxelWorld* InWorld, EVoxelPlayType PlayType)
	: UpdateRate(FMath::Max(SMALL_NUMBER, InWorld->EventsTickRate))
	, VoxelWorldInterface(InWorld)
	, World(InWorld->GetWorld())
	, WorldBounds(InWorld->GetWorldBounds())
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelEventManager> FVoxelEventManager::Create(const FVoxelEventManagerSettings& Settings)
{
	TVoxelSharedRef<FVoxelEventManager> Manager = MakeShareable(new FVoxelEventManager(Settings));
	UVoxelInvokerComponentBase::OnForceRefreshInvokers.AddThreadSafeSP(Manager, &FVoxelEventManager::ClearOldInvokerComponents);
	return Manager;
}

void FVoxelEventManager::Destroy()
{
	StopTicking();
}

FVoxelEventManager::FVoxelEventManager(const FVoxelEventManagerSettings& Settings)
	: Settings(Settings)
{

}

FVoxelEventManager::~FVoxelEventManager()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelEventsMemory, EventsAllocatedSize);
	DEC_DWORD_STAT_BY(STAT_NumVoxelEvents, NumEvents);
	DEC_DWORD_STAT_BY(STAT_VoxelEventManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelEventHandle FVoxelEventManager::BindEvent(
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 DistanceInChunks,
	const FChunkDelegate& OnActivate,
	const FChunkDelegate& OnDeactivate,
	EVoxelEventFlags::Type Flags)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!ensureAlways(OnActivate.IsBound() || OnDeactivate.IsBound()))
	{
		return {};
	}

	const FEventKey EventKey{ ChunkSize, DistanceInChunks, Flags };

	auto& EventInfo = Events.FindOrAdd(EventKey);
	if (!EventInfo.IsValid())
	{
		EventInfo = MakeUnique<FEventInfo>(ChunkSize, DistanceInChunks, Flags);
	}

	FVoxelEventHandle Handle;
	Handle.OnActivateHandle = EventInfo->OnActivate.Add(OnActivate);
	Handle.OnDeactivateHandle = EventInfo->OnDeactivate.Add(OnDeactivate);
	Handle.ChunkSize = ChunkSize;
	Handle.DistanceInChunks = DistanceInChunks;
	Handle.Flags = Flags;

	if (bFireExistingOnes)
	{
		IterateActiveChunks(ChunkSize, DistanceInChunks, Flags, [&](auto Bounds)
		{
			OnActivate.ExecuteIfBound(Bounds);
		});
	}

	// Force update
	ClearOldInvokerComponents();

	return Handle;
}

FVoxelEventHandle FVoxelEventManager::BindGenerationEvent(
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 DistanceInChunks,
	const FChunkDelegate& OnGenerate,
	EVoxelEventFlags::Type Flags)
{
	return BindEvent(
		bFireExistingOnes,
		ChunkSize,
		DistanceInChunks,
		OnGenerate,
		FChunkDelegate(),
		EVoxelEventFlags::Type(Flags | EVoxelEventFlags::GenerationEvent));
}

void FVoxelEventManager::UnbindEvent(FVoxelEventHandle Handle)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!ensure(Handle.IsValid()))
	{
		return;
	}

	const FEventKey EventKey{ Handle.ChunkSize, Handle.DistanceInChunks, Handle.Flags };
	auto* EventPtr = Events.Find(EventKey);
	if (!ensure(EventPtr))
	{
		return;
	}
	auto& Event = **EventPtr;
	Event.OnActivate.Remove(Handle.OnActivateHandle);
	Event.OnDeactivate.Remove(Handle.OnDeactivateHandle);

	if (!Event.OnActivate.IsBound() && !Event.OnDeactivate.IsBound())
	{
		Events.Remove(EventKey);
		EventsInvokers.Remove(EventKey);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEventManager::Tick(float DeltaTime)
{
	VOXEL_FUNCTION_COUNTER();

	const double Time = FPlatformTime::Seconds();
	if (Time - LastUpdateTime > 1. / Settings.UpdateRate)
	{
		LastUpdateTime = Time;
		Update();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEventManager::Update()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Settings.VoxelWorldInterface.IsValid()) return;

	TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> NewInvokerComponents = UVoxelInvokerComponentBase::GetInvokers(Settings.World.Get());
	NewInvokerComponents.RemoveAllSwap([](auto& Invoker) { return !Invoker->bUseForEvents; });
	NewInvokerComponents.Sort([](auto& A, auto& B) { return A.Get() < B.Get(); });

	if (NewInvokerComponents.Num() == 0) return;

	const auto GetInvokerPosition = [&](auto& Invoker) { return Invoker->GetInvokerVoxelPosition(Settings.VoxelWorldInterface.Get()); };

	TArray<TPair<FEventKey, TArray<FIntVector>>> InvokersToUpdate;
	if (NewInvokerComponents != OldInvokerComponents)
	{
		OldInvokerComponents = NewInvokerComponents;
		EventsInvokers.Reset();
		for (auto& EventsIt : Events)
		{
			const FEventKey EventKey = EventsIt.Key;
			const auto& EventInfo = *EventsIt.Value;

			if (!EventInfo.IsBound()) continue;
			
			check(!EventsInvokers.Contains(EventKey));
			auto& EventInvokers = EventsInvokers.Add(EventKey);
			EventInvokers.Reserve(NewInvokerComponents.Num());
			
			TArray<FIntVector> Positions;
			for (auto& InvokerComponent : NewInvokerComponents)
			{
				if ((EventInfo.Flags & EVoxelEventFlags::LocalInvokerOnly) && !InvokerComponent->IsLocalInvoker())
				{
					continue;
				}
				
				const FIntVector Position = GetInvokerPosition(InvokerComponent);
				EventInvokers.Emplace(EventInfo.ChunkSize, Position, InvokerComponent);
				Positions.Add(Position);
			}

			InvokersToUpdate.Emplace(EventKey, MoveTemp(Positions));
		}
	}
	else
	{
		for (auto& EventInvokerIt : EventsInvokers)
		{
			const FEventKey EventKey = EventInvokerIt.Key;
			auto& EventInvokers = EventInvokerIt.Value;

			if (!Events[EventKey]->IsBound()) continue;

			// First check if some need to update
			bool bUpdate = false;
			for (auto& EventInvoker : EventInvokers)
			{
				const FIntVector Position = GetInvokerPosition(EventInvoker.InvokerComponent);
				const uint64 DistanceSquared = FVoxelUtilities::SquaredSize(EventInvoker.Position - Position);
				if (DistanceSquared > FMath::Square(EventInvoker.ChunkSize / 4.f)) // Heuristic
				{
					bUpdate = true;
					break;
				}
			}
			if (!bUpdate) continue;

			// If true iterate all to have all the positions

			TArray<FIntVector> Positions;
			for (auto& EventInvoker : EventInvokers)
			{
				const FIntVector Position = GetInvokerPosition(EventInvoker.InvokerComponent);
				EventInvoker.Position = Position;
				Positions.Add(Position);
			}

			InvokersToUpdate.Emplace(EventKey, MoveTemp(Positions));
		}
	}
	if (InvokersToUpdate.Num() > 0)
	{
		UpdateInvokers(InvokersToUpdate);
	}
}

void FVoxelEventManager::UpdateInvokers(const TArray<TPair<FEventKey, TArray<FIntVector>>>& InvokersToUpdate)
{
	VOXEL_FUNCTION_COUNTER();

	const bool bDebug = CVarShowEventsBounds.GetValueOnGameThread() != 0;

	for (auto& It : InvokersToUpdate)
	{
		const FEventKey EventKey = It.Key;
		const auto& InvokerPositions = It.Value;
		auto& EventInfo = *Events[EventKey];

		const auto GetChunkBounds = [&](const FIntVector& Chunk)
		{
			return FVoxelIntBox(Chunk * EventInfo.ChunkSize, (Chunk + 1) * EventInfo.ChunkSize);
		};

		TSet<FIntVector> NewActiveChunks;
		{
			VOXEL_SCOPE_COUNTER("Find NewActiveChunks");

			// NOTE: This part could be multi threaded
			
			// Iterate every position and add all chunks within activation radius
			for (const FIntVector& InvokerPosition : InvokerPositions)
			{
				const FIntVector MinChunkPosition = FVoxelUtilities::DivideFloor(InvokerPosition - EventInfo.ChunkSize * EventInfo.DistanceInChunks, EventInfo.ChunkSize);
				// Max is exclusive, since this is the coordinate of the Bounds.Min of the chunk
				const FIntVector MaxChunkPosition = FVoxelUtilities::DivideCeil(InvokerPosition + EventInfo.ChunkSize * EventInfo.DistanceInChunks, EventInfo.ChunkSize);

				if (!Settings.WorldBounds.Intersect(FVoxelIntBox(MinChunkPosition, MaxChunkPosition)))
				{
					continue;
				}

				const uint64 SquaredDistanceInVoxels = FMath::Square(EventInfo.DistanceInChunks * EventInfo.ChunkSize);

				for (int32 X = MinChunkPosition.X; X < MaxChunkPosition.X; X++)
				{
					for (int32 Y = MinChunkPosition.Y; Y < MaxChunkPosition.Y; Y++)
					{
						for (int32 Z = MinChunkPosition.Z; Z < MaxChunkPosition.Z; Z++)
						{
							const FIntVector Chunk = FIntVector(X, Y, Z);
							const FVoxelIntBox ChunkBounds = GetChunkBounds(Chunk);
							
							if (ChunkBounds.ComputeSquaredDistanceFromBoxToPoint(InvokerPosition) <= SquaredDistanceInVoxels &&
								ChunkBounds.Intersect(Settings.WorldBounds))
							{
								NewActiveChunks.Add(Chunk);
							}
						}
					}
				}
			}
		}

		{
			VOXEL_SCOPE_COUNTER("Fire Delegates");

			if (EventInfo.Flags & EVoxelEventFlags::GenerationEvent)
			{
				// Generation event: trigger all chunks not already triggered
				ensure(!EventInfo.OnDeactivate.IsBound());
				if (ensure(EventInfo.OnActivate.IsBound()))
				{
					for (auto& Chunk : NewActiveChunks)
					{
						bool bAlreadyInSet;
						EventInfo.ActiveOrGeneratedChunks.Add(Chunk, &bAlreadyInSet);
						if (!bAlreadyInSet)
						{
							const FVoxelIntBox Bounds = GetChunkBounds(Chunk);
							EventInfo.OnActivate.Broadcast(Bounds);

							if (bDebug)
							{
								UVoxelDebugUtilities::DrawDebugIntBox(Settings.VoxelWorldInterface.Get(), Bounds, 1.f, 0, FColor::Yellow);
							}
						}
					}
				}
			}
			else
			{
				// Normal event: activate new chunks, deactivate old chunks
				if (EventInfo.OnActivate.IsBound())
				{
					for (auto& Chunk : NewActiveChunks)
					{
						if (!EventInfo.ActiveOrGeneratedChunks.Contains(Chunk))
						{
							const FVoxelIntBox Bounds = GetChunkBounds(Chunk);
							EventInfo.OnActivate.Broadcast(Bounds);

							if (bDebug)
							{
								UVoxelDebugUtilities::DrawDebugIntBox(Settings.VoxelWorldInterface.Get(), Bounds, 1.f, 0, FColor::Blue);
							}
						}
					}
				}
				if (EventInfo.OnDeactivate.IsBound())
				{
					for (auto& Chunk : EventInfo.ActiveOrGeneratedChunks)
					{
						if (!NewActiveChunks.Contains(Chunk))
						{
							const FVoxelIntBox Bounds = GetChunkBounds(Chunk);
							EventInfo.OnDeactivate.Broadcast(Bounds);

							if (bDebug)
							{
								UVoxelDebugUtilities::DrawDebugIntBox(Settings.VoxelWorldInterface.Get(), Bounds, 1.f, 0, FColor::Red);
							}
						}
					}
				}
				EventInfo.ActiveOrGeneratedChunks = NewActiveChunks;
			}
		}
	}

	UpdateEventsAllocatedSize();
}

void FVoxelEventManager::ClearOldInvokerComponents()
{
	OldInvokerComponents.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEventManager::UpdateEventsAllocatedSize()
{
	VOXEL_FUNCTION_COUNTER();
	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelEventsMemory, EventsAllocatedSize);
	DEC_DWORD_STAT_BY(STAT_VoxelEventManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);
	EventsAllocatedSize = 0;
	NumActiveOrGeneratedChunks = 0;

	EventsAllocatedSize += Events.GetAllocatedSize();
	for (auto& It : Events)
	{
		NumActiveOrGeneratedChunks += It.Value->ActiveOrGeneratedChunks.Num();
		EventsAllocatedSize += It.Value->GetAllocatedSize();
	}

	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelEventsMemory, EventsAllocatedSize);
	INC_DWORD_STAT_BY(STAT_VoxelEventManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);

	DEC_DWORD_STAT_BY(STAT_NumVoxelEvents, NumEvents);
	NumEvents = Events.Num();
	INC_DWORD_STAT_BY(STAT_NumVoxelEvents, NumEvents);
}