// Copyright 2018 Phyronnaz

#include "VoxelTools/VoxelCraterTools.h"
#include "VoxelData/VoxelData.h"
#include "Async/Async.h"
#include "VoxelTools/VoxelTools.h"
#include "VoxelPoolManager.h"
#include "Engine/Engine.h"

template<bool bUseSampleRate>
inline void AddCraterImpl(FVoxelData* Data, const FIntVector& Position, float Radius, const FastNoise& Noise, float SampleRate, FIntBox& OutBounds, TArray<FIntVector>& OutPositions)
{
	FIntVector R(FMath::CeilToInt(Radius) + 3);
	const FIntBox Bounds(Position - R, Position + R);

	OutBounds = Bounds;

	FVoxelScopeSetLock Lock(Data, Bounds);

	Data->SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& OldValue)
	{
		float Distance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).Size();

		if (Distance <= Radius + 2)
		{
			float WhiteNoise = 0;
			if (FMath::Abs(Distance - Radius) < 2)
			{
				WhiteNoise = Noise.GetWhiteNoise(X / Distance, Y / Distance, Z / Distance);
			}

			FVoxelValue NewValue = FMath::Clamp(Radius - Distance + WhiteNoise, -2.f, 2.f) / 2;

			if (NewValue.IsEmpty() || FVoxelUtilities::HaveSameSign(OldValue, NewValue))
			{
				if (bUseSampleRate && !OldValue.IsEmpty() && (SampleRate >= 1 || FMath::FRand() < SampleRate))
				{
					OutPositions.Emplace(X, Y, Z);
				}
				OldValue = NewValue;
			}
		}
	});
}

void FVoxelAddCraterAsyncWork::DoWork()
{
	if (SampleRate > 0)
	{
		AddCraterImpl<true>(Data.Get(), Position, Radius, Noise, SampleRate, Bounds, Positions);
	}
	else
	{
		AddCraterImpl<false>(Data.Get(), Position, Radius, Noise, SampleRate, Bounds, Positions);
	}
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelCraterTools::AddCrater(AVoxelWorld* World, FIntVector Position, float Radius, float SampleRate, TArray<FIntVector>& Positions)
{
	CHECK_WORLD_VOXELTOOLS(AddCrater);
	
	Positions.Reset();

	FIntBox Bounds;
	FastNoise Noise;
	if (SampleRate > 0)
	{
		AddCraterImpl<true>(World->GetData(), Position, Radius, Noise, SampleRate, Bounds, Positions);
	}
	else
	{
		AddCraterImpl<false>(World->GetData(), Position, Radius, Noise, SampleRate, Bounds, Positions);
	}
	World->UpdateChunksOverlappingBox(Bounds, true);
}

void UVoxelCraterTools::AddCraterAsync(UObject* WorldContextObject, AVoxelWorld* World, FIntVector Position, float Radius, float SampleRate, TArray<FIntVector>& Positions, FLatentActionInfo LatentInfo)
{
	CHECK_WORLD_VOXELTOOLS(AddCraterAsync, );
	
	auto* PositionsPtr = &Positions;

	if (UWorld* ObjectWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = ObjectWorld->GetLatentActionManager();
		if (!LatentActionManager.FindExistingAction<FVoxelLatentAction<FVoxelAddCraterAsyncWork>>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			TSharedPtr<FVoxelAddCraterAsyncWork> Task = MakeShared<FVoxelAddCraterAsyncWork>(World, Position, Radius, SampleRate);
			auto* Pool = World->GetPool()->AsyncTasksPool;
			Pool->AddQueuedWork(Task.Get());
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FVoxelLatentAction<FVoxelAddCraterAsyncWork>(Task, LatentInfo, "AddCrater: Waiting for completion",
				[=](FVoxelAddCraterAsyncWork& Work)
				{
					*PositionsPtr = Work.Positions;
					Work.World->UpdateChunksOverlappingBox(Work.Bounds, true);
				}
			));
		}
	}
}
