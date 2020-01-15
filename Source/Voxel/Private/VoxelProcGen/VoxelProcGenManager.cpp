// Copyright 2020 Phyronnaz

#include "VoxelProcGen/VoxelProcGenManager.h"
#include "VoxelComponents/VoxelInvokerComponent.h"
#include "VoxelWorld.h"
#include "VoxelGlobals.h"

DECLARE_MEMORY_STAT(TEXT("Proc Gen - Events Memory"), STAT_ProcGenManager_EventsMemory, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Proc Gen - Num events"), STAT_ProcGenManager_NumEvents, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Proc Gen - Num active or generated chunks"), STAT_ProcGenManager_NumActiveOrGeneratedChunks, STATGROUP_VoxelMemory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelProcGenSettings::FVoxelProcGenSettings(const AVoxelWorld* InWorld, EVoxelPlayType PlayType)
	: UpdateRate(FMath::Max(SMALL_NUMBER, InWorld->GenerationEventsTickRate))
	, VoxelWorldInterface(InWorld)
	, World(InWorld->GetWorld())
	, WorldBounds(InWorld->GetWorldBounds())
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelProcGenManager> FVoxelProcGenManager::Create(const FVoxelProcGenSettings& Settings)
{
	TVoxelSharedRef<FVoxelProcGenManager> Manager = MakeShareable(new FVoxelProcGenManager(Settings));
	return Manager;
}

void FVoxelProcGenManager::Destroy()
{
	StopTicking();
}

FVoxelProcGenManager::FVoxelProcGenManager(const FVoxelProcGenSettings& Settings)
	: Settings(Settings)
{

}

FVoxelProcGenManager::~FVoxelProcGenManager()
{
	DEC_DWORD_STAT_BY(STAT_ProcGenManager_EventsMemory, EventsAllocatedSize);
	DEC_DWORD_STAT_BY(STAT_ProcGenManager_NumEvents, NumEvents);
	DEC_DWORD_STAT_BY(STAT_ProcGenManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelProcGenEventHandle FVoxelProcGenManager::BindEvent(
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 DistanceInChunks,
	const FChunkDelegate& OnActivate,
	const FChunkDelegate& OnDeactivate,
	EVoxelProcGenFlags::Type Flags)
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

	FVoxelProcGenEventHandle Handle;
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
	OldInvokerComponents.Reset();

	return Handle;
}

FVoxelProcGenEventHandle FVoxelProcGenManager::BindGenerationEvent(
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 DistanceInChunks,
	const FChunkDelegate& OnGenerate,
	EVoxelProcGenFlags::Type Flags)
{
	return BindEvent(
		bFireExistingOnes,
		ChunkSize,
		DistanceInChunks,
		OnGenerate,
		FChunkDelegate(),
		EVoxelProcGenFlags::Type(Flags & EVoxelProcGenFlags::GenerationEvent));
}

void FVoxelProcGenManager::UnbindEvent(FVoxelProcGenEventHandle Handle)
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

void FVoxelProcGenManager::Tick(float DeltaTime)
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

void FVoxelProcGenManager::Update()
{
	VOXEL_FUNCTION_COUNTER();

	if (!Settings.VoxelWorldInterface.IsValid()) return;

	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> NewInvokerComponents = UVoxelInvokerComponent::GetInvokers(Settings.World.Get());
	NewInvokerComponents.Sort([](auto& A, auto& B) { return A.Get() < B.Get(); });

	const auto GetInvokerPosition = [&](auto& Invoker) { return Settings.VoxelWorldInterface->GlobalToLocalFloat(Invoker->GetPosition()); };

	TArray<TPair<FEventKey, TArray<FVector>>> InvokersToUpdate;
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
			
			TArray<FVector> Positions;
			for (auto& InvokerComponent : NewInvokerComponents)
			{
				if ((EventInfo.Flags & EVoxelProcGenFlags::LocalInvokerOnly) && !InvokerComponent->IsLocalInvoker())
				{
					continue;
				}
				
				const FVector Position = GetInvokerPosition(InvokerComponent);
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
				const FVector Position = GetInvokerPosition(EventInvoker.InvokerComponent);
				const float DistanceSquared = (EventInvoker.Position - Position).SizeSquared();
				if (DistanceSquared > FMath::Square(EventInvoker.ChunkSize / 4.f)) // Heuristic
				{
					bUpdate = true;
					break;
				}
			}
			if (!bUpdate) continue;

			// If true iterate all to have all the positions

			TArray<FVector> Positions;
			for (auto& EventInvoker : EventInvokers)
			{
				const FVector Position = GetInvokerPosition(EventInvoker.InvokerComponent);
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

void FVoxelProcGenManager::UpdateInvokers(const TArray<TPair<FEventKey, TArray<FVector>>>& InvokersToUpdate)
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& It : InvokersToUpdate)
	{
		const FEventKey EventKey = It.Key;
		const auto& Positions = It.Value;
		auto& EventInfo = *Events[EventKey];

		const auto GetChunkBounds = [&](const FIntVector& Chunk)
		{
			return FIntBox(Chunk * EventInfo.ChunkSize, (Chunk + 1) * EventInfo.ChunkSize);
		};

		TSet<FIntVector> NewActiveChunks;
		
		// Iterate every position and add all chunks within activation radius
		for (auto& Position : Positions)
		{
			const int32 Distance = EventInfo.DistanceInChunks;
			const FIntVector BaseChunkPosition = FVoxelUtilities::RoundToInt(Position / EventInfo.ChunkSize);

			if (!Settings.WorldBounds.Intersect(FIntBox(
				(BaseChunkPosition - Distance) * EventInfo.ChunkSize,
				(BaseChunkPosition + Distance + 1) * EventInfo.ChunkSize)))
			{
				continue;
			}

			for (int32 X = -Distance; X <= Distance; X++)
			{
				const float SquaredXF = FMath::Square(X / float(Distance));
				const float YF = Distance * FMath::Sqrt(FMath::Max(0.f, 1.f - SquaredXF));
				for (int32 Y = FMath::FloorToInt(-YF); Y <= FMath::CeilToInt(YF); Y++)
				{
					const float SquaredYF = FMath::Square(Y / float(Distance));
					const float ZF = Distance * FMath::Sqrt(FMath::Max(0.f, 1.f - SquaredXF - SquaredYF));
					for (int32 Z = FMath::FloorToInt(-ZF); Z <= FMath::CeilToInt(ZF); Z++)
					{
						const FIntVector Chunk = FIntVector(X, Y, Z) + BaseChunkPosition;
						if (GetChunkBounds(Chunk).Intersect(Settings.WorldBounds))
						{
							NewActiveChunks.Add(Chunk);
						}
					}
				}
			}
		}

		{
			VOXEL_SCOPE_COUNTER("Fire Delegates");

			if (EventInfo.Flags & EVoxelProcGenFlags::GenerationEvent)
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
							EventInfo.OnActivate.Broadcast(GetChunkBounds(Chunk));
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
							EventInfo.OnActivate.Broadcast(GetChunkBounds(Chunk));
						}
					}
				}
				if (EventInfo.OnDeactivate.IsBound())
				{
					for (auto& Chunk : EventInfo.ActiveOrGeneratedChunks)
					{
						if (!NewActiveChunks.Contains(Chunk))
						{
							EventInfo.OnDeactivate.Broadcast(GetChunkBounds(Chunk));
						}
					}
				}
				EventInfo.ActiveOrGeneratedChunks = NewActiveChunks;
			}
		}
	}

	UpdateEventsAllocatedSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelProcGenManager::UpdateEventsAllocatedSize()
{
	VOXEL_FUNCTION_COUNTER();
	
	DEC_DWORD_STAT_BY(STAT_ProcGenManager_EventsMemory, EventsAllocatedSize);
	DEC_DWORD_STAT_BY(STAT_ProcGenManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);
	EventsAllocatedSize = 0;
	NumActiveOrGeneratedChunks = 0;

	EventsAllocatedSize += Events.GetAllocatedSize();
	for (auto& It : Events)
	{
		NumActiveOrGeneratedChunks += It.Value->ActiveOrGeneratedChunks.Num();
		EventsAllocatedSize += It.Value->GetAllocatedSize();
	}

	INC_DWORD_STAT_BY(STAT_ProcGenManager_EventsMemory, EventsAllocatedSize);
	INC_DWORD_STAT_BY(STAT_ProcGenManager_NumActiveOrGeneratedChunks, NumActiveOrGeneratedChunks);

	DEC_DWORD_STAT_BY(STAT_ProcGenManager_NumEvents, NumEvents);
	NumEvents = Events.Num();
	INC_DWORD_STAT_BY(STAT_ProcGenManager_NumEvents, NumEvents);
}