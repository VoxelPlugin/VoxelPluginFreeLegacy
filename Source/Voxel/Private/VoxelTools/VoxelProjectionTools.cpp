// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelDataAccelerator.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

struct FHitsBuilder
{
	struct FPlanePosition
	{
		FVector2D PlanePosition;
		float DistanceSquared;
		FHitResult Hit;
	};
	TMap<FIntVector, FPlanePosition> PlanePositions;

	TArray<FVoxelProjectionHit> GetHits() const
	{
		TArray<FVoxelProjectionHit> Result;
		Result.Reserve(PlanePositions.Num());
		for (auto& It : PlanePositions)
		{
			Result.Add(FVoxelProjectionHit{ It.Key, It.Value.PlanePosition, It.Value.Hit });
		}
		return Result;
	}

	inline void Add(AVoxelWorld* World, const FHitResult& Hit, const FVector2D& PlanePosition)
	{
		const FVoxelVector LocalPosition = World->GlobalToLocalFloat(Hit.ImpactPoint);
		for (auto& Point : FVoxelUtilities::GetNeighbors(LocalPosition))
		{
			const float DistanceSquared = (Point - LocalPosition).SizeSquared();
			auto* const Existing = PlanePositions.Find(Point);
			if (Existing)
			{
				if (Existing->DistanceSquared > DistanceSquared)
				{
					Existing->PlanePosition = PlanePosition;
					Existing->DistanceSquared = DistanceSquared;
					Existing->Hit = Hit;
				}
			}
			else
			{
				PlanePositions.Add(Point, { PlanePosition, DistanceSquared, Hit });
			}
		}
	}
};

class FAsyncLinetracesLatentAction : public FPendingLatentAction
{
public:
	const FName ExecutionFunction;
	const int32 OutputLink;
	const FWeakObjectPtr CallbackTarget;
	TArray<FVoxelProjectionHit>* const OutHits;
	const TWeakObjectPtr<AVoxelWorld> VoxelWorld;
	const TWeakObjectPtr<UWorld> World;
	const FVoxelLineTraceParameters Parameters;
	
	FHitsBuilder Builder;
	uint32 NumTraces = 0;
	uint32 NumCompletedTraces = 0;

	struct FLocalTraceData
	{
		FVector Start;
		FVector End;
		FVector2D PlanePosition;
	};
	TMap<FTraceHandle, FLocalTraceData> TracesLocalData;

	struct FManualWeakRef
	{
		FAsyncLinetracesLatentAction& Ptr;
		
		void TraceDone(const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
		{
			Ptr.TraceDone(TraceHandle, TraceData);
		}
	};
	const TSharedRef<FManualWeakRef> WeakRef = MakeShareable(new FManualWeakRef{ *this });

	template<typename T>
	FAsyncLinetracesLatentAction(
		const FLatentActionInfo& LatentInfo,
		TArray<FVoxelProjectionHit>* OutHits,
		AVoxelWorld* VoxelWorld, 
		FVoxelLineTraceParameters Parameters,
		T GenerateRaysLambda)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, OutHits(OutHits)
		, VoxelWorld(VoxelWorld)
		, World(VoxelWorld->GetWorld())
		, Parameters(Parameters)
	{
		check(VoxelWorld);

		const FCollisionQueryParams Params = Parameters.GetParams();
		const FCollisionResponseContainer ResponseContainer = Parameters.GetResponseContainer();

		FTraceDelegate TraceDelegate;
		TraceDelegate.BindSP(WeakRef, &FManualWeakRef::TraceDone);

		GenerateRaysLambda([&](const FVector& Start, const FVector& End, const FVector2D& Position)
			{
				VOXEL_SCOPE_COUNTER("Start Async Linetrace");
				const FTraceHandle Handle = World->AsyncLineTraceByChannel(
					EAsyncTraceType::Single, 
					Start, 
					End, 
					Parameters.CollisionChannel, 
					Params, 
					ResponseContainer, 
					&TraceDelegate);
				TracesLocalData.Add(Handle, { Start, End, Position });
				NumTraces++;
			});
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		const bool bFinished = NumCompletedTraces == NumTraces || !VoxelWorld.IsValid();
		if (bFinished)
		{
			*OutHits = Builder.GetHits();
		}
		Response.FinishAndTriggerIf(bFinished, ExecutionFunction, OutputLink, CallbackTarget);
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		return FString::Printf(TEXT("Trace %d/%d"), NumCompletedTraces, NumTraces);
	}
#endif

	void TraceDone(const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
	{
		VOXEL_SCOPE_COUNTER("Trace Done");

		NumCompletedTraces++;

		ensure(TraceData.OutHits.Num() <= 1);

		if (!VoxelWorld.IsValid())
		{
			return;
		}

		const auto LocalData = TracesLocalData.FindChecked(TraceHandle);

		bool bHit = false;
		FHitResult OutHit;
		for (auto& Hit : TraceData.OutHits)
		{
			if (Hit.Actor == VoxelWorld)
			{
				bHit = true;
				OutHit = Hit;
				Builder.Add(VoxelWorld.Get(), Hit, LocalData.PlanePosition);
				break;
			}
		}

		if (ensure(World.IsValid()))
		{
			Parameters.DrawDebug(World.Get(), LocalData.Start, LocalData.End, bHit, OutHit);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FCollisionQueryParams FVoxelLineTraceParameters::GetParams() const
{
	FCollisionQueryParams Params(STATIC_FNAME("FindProjectionVoxels"), SCENE_QUERY_STAT_ONLY(FindProjectionVoxels), true);
	Params.AddIgnoredActors(ActorsToIgnore);
	return Params;
}

FCollisionResponseContainer FVoxelLineTraceParameters::GetResponseContainer() const
{
	FCollisionResponseContainer ResponseContainer;
	for (auto& CollisionChannelToIgnore : CollisionChannelsToIgnore)
	{
		ResponseContainer.SetResponse(CollisionChannelToIgnore, ECollisionResponse::ECR_Ignore);
	}
	return ResponseContainer;
}

void FVoxelLineTraceParameters::DrawDebug(const UWorld* World, const FVector& Start, const FVector& End, bool bHit, const FHitResult& OutHit) const
{
#if ENABLE_DRAW_DEBUG
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		// @fixme, draw line with thickness = 2.f?
		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugLine(World, Start, OutHit.ImpactPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, OutHit.ImpactPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			static const float KISMET_TRACE_DEBUG_IMPACTPOINT_SIZE = 16.f;
			DrawDebugPoint(World, OutHit.ImpactPoint, KISMET_TRACE_DEBUG_IMPACTPOINT_SIZE, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelLineTraceParameters UVoxelProjectionTools::MakeVoxelLineTraceParameters(
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannelsToIgnore, 
	TArray<AActor*> ActorsToIgnore, 
	TEnumAsByte<ECollisionChannel> CollisionChannel, 
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType, 
	FLinearColor TraceColor, 
	FLinearColor TraceHitColor, 
	float DrawTime)
{
	return
	{
		CollisionChannel,
		CollisionChannelsToIgnore,
		ActorsToIgnore,
		DrawDebugType,
		TraceColor,
		TraceHitColor,
		DrawTime
	};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelProjectionTools::FindProjectionVoxels(
	TArray<FVoxelProjectionHit>& Hits, 
	AVoxelWorld* World, 
	FVoxelLineTraceParameters Parameters, 
	FVector Position, 
	FVector Direction, 
	float Radius,
	EVoxelProjectionShape Shape,
	float NumRays, 
	float MaxDistance)
{
	VOXEL_FUNCTION_COUNTER();
	
	Hits.Reset();
	
	CHECK_VOXELWORLD_IS_CREATED();

	if (!Direction.Normalize())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Direction!"));
		return 0;
	}

	UWorld* const WorldPtr = World->GetWorld();
	const FCollisionQueryParams Params = Parameters.GetParams();
	const FCollisionResponseContainer ResponseContainer = Parameters.GetResponseContainer();
	FHitsBuilder Builder;

	const auto Lambda = [&](const FVector& Start, const FVector& End, const FVector2D& PlanePosition)
	{
		VOXEL_SCOPE_COUNTER("Linetrace");
		FHitResult OutHit;
		const bool bHit = WorldPtr->LineTraceSingleByChannel(
			OutHit,
			Start,
			End,
			Parameters.CollisionChannel,
			Params,
			ResponseContainer);
		Parameters.DrawDebug(WorldPtr, Start, End, bHit, OutHit);
		if (bHit)
		{
			Builder.Add(World, OutHit, PlanePosition);
		}
	};
	
	const int32 NumTraced = GenerateRays(Position, Direction, Radius, Shape, NumRays, MaxDistance, Lambda);
	
	Hits = Builder.GetHits();

	return NumTraced;
}

int32 UVoxelProjectionTools::FindProjectionVoxelsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FVoxelProjectionHit>& Hits,
	AVoxelWorld* World,
	FVoxelLineTraceParameters Parameters, 
	FVector Position, 
	FVector Direction, 
	float Radius,
	EVoxelProjectionShape Shape,
	float NumRays, 
	float MaxDistance,
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	if (!Direction.Normalize())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Direction!"));
		return 0;
	}

	int32 NumTraced = 0;
	const auto Lambda = [&]()
	{
		return new FAsyncLinetracesLatentAction(LatentInfo, &Hits, World, Parameters, [&](auto In)
			{
				NumTraced = GenerateRays(Position, Direction, Radius, Shape, NumRays, MaxDistance, In);
			});
	};
	FVoxelToolHelpers::StartLatentAction(
		WorldContextObject,
		LatentInfo,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		Lambda);

	return NumTraced;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FIntVector> UVoxelProjectionTools::GetHitsPositions(const TArray<FVoxelProjectionHit>& Hits)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Hits.Num());
	TArray<FIntVector> Voxels;
	Voxels.Reserve(Hits.Num());
	for (auto& Hit : Hits)
	{
		Voxels.Add(Hit.VoxelPosition);
	}
	return Voxels;
}

FVector UVoxelProjectionTools::GetHitsAverageNormal(const TArray<FVoxelProjectionHit>& Hits)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Hits.Num());
	if (Hits.Num() == 0)
	{
		return FVector::UpVector;
	}
	FVector N = Hits[0].Hit.ImpactNormal;
	for (int32 Index = 1; Index < Hits.Num(); ++Index)
	{
		N += Hits[Index].Hit.ImpactNormal;
	}
	if (!ensure(!FMath::IsNaN(N.X + N.Y + N.Z)))
	{
		return FVector::UpVector;
	}
	return N.GetSafeNormal();
}

FVector UVoxelProjectionTools::GetHitsAveragePosition(const TArray<FVoxelProjectionHit>& Hits)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Hits.Num());
	if (Hits.Num() == 0)
	{
		return FVector::ZeroVector;
	}
	FVector Position = Hits[0].Hit.ImpactPoint;
	for (int32 Index = 1; Index < Hits.Num(); ++Index)
	{
		Position += Hits[Index].Hit.ImpactPoint;
	}
	if (!ensure(!FMath::IsNaN(Position.X + Position.Y + Position.Z)))
	{
		return FVector::UpVector;
	}
	return Position / Hits.Num();
}

FVoxelSurfaceEditsVoxels UVoxelProjectionTools::CreateSurfaceVoxelsFromHits(const TArray<FVoxelProjectionHit>& Hits)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Hits.Num());

	TArray<FVoxelSurfaceEditsVoxelBase> Voxels;
	Voxels.Reserve(Hits.Num());
	for (auto& Hit : Hits)
	{
		Voxels.Emplace(Hit.VoxelPosition, Hit.Hit.Normal, 0.f);
	}

	FVoxelSurfaceEditsVoxels EditsVoxels;
	EditsVoxels.Info.bHasValues = false;
	EditsVoxels.Info.bHasExactDistanceField = false;
	EditsVoxels.Info.bHasNormals = true;
	EditsVoxels.Info.bIs2D = false;
	
	EditsVoxels.Voxels = MakeVoxelSharedCopy(MoveTemp(Voxels));

	return EditsVoxels;
}

FVoxelSurfaceEditsVoxels UVoxelProjectionTools::CreateSurfaceVoxelsFromHitsWithExactValues(AVoxelWorld* World, const TArray<FVoxelProjectionHit>& Hits)
{
	CHECK_VOXELWORLD_IS_CREATED();
	VOXEL_TOOL_FUNCTION_COUNTER(Hits.Num());

	if (Hits.Num() == 0)
	{
		return {};
	}
	
	TArray<FVoxelSurfaceEditsVoxelBase> Voxels;
	Voxels.Reserve(Hits.Num());

	FVoxelIntBox Bounds = FVoxelIntBox(Hits[0].VoxelPosition);
	for (int32 Index = 1; Index < Hits.Num(); Index++)
	{
		Bounds = Bounds + Hits[Index].VoxelPosition;
	}

	auto& Data = World->GetData();
	FVoxelReadScopeLock Lock(Data, Bounds, FUNCTION_FNAME);
	const FVoxelConstDataAccelerator Accelerator(Data, Bounds);

	for (auto& Hit : Hits)
	{
		Voxels.Emplace(Hit.VoxelPosition, Hit.Hit.Normal, Accelerator.GetValue(Hit.VoxelPosition, 0).ToFloat());
	}

	FVoxelSurfaceEditsVoxels EditsVoxels;
	EditsVoxels.Info.bHasValues = true;
	EditsVoxels.Info.bHasExactDistanceField = false;
	EditsVoxels.Info.bHasNormals = true;
	EditsVoxels.Info.bIs2D = false;
	
	EditsVoxels.Voxels = MakeVoxelSharedCopy(MoveTemp(Voxels));

	return EditsVoxels;
}
