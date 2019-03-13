// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelProjectionTools.h"
#include "DrawDebugHelpers.h"
#include "VoxelTools/VoxelTools.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"

#include "Engine/Engine.h"
#include "Curves/CurveFloat.h"
#include "VoxelToolsHelpers.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::SetValueProjection"), STAT_UVoxelProjectionTools_SetValueProjection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::SetMaterialProjection"), STAT_UVoxelProjectionTools_SetMaterialProjection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::TraceDone"), STAT_UVoxelProjectionTools_TraceDone, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::FindHitVoxelsForRaycasts"), STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::FindHitVoxelsForRaycasts::StartAsyncTrace"), STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts_StartAsyncTrace, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProjectionTools::FindHitVoxelsForRaycasts::StartTrace"), STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts_StartTrace, STATGROUP_Voxel);

TMap<FTraceHandle, UVoxelProjectionTools::FTraceDetails> UVoxelProjectionTools::TraceMap;
FTraceDelegate UVoxelProjectionTools::TraceDelegate;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void ProcessTraceResult(FVoxelProjectionEditWork& Work, const FHitResult& Hit, float Distance)
{
	if (Work.bShowRaycasts)
	{
		DrawDebugLine(Work.World->GetWorld(), Hit.TraceStart, Hit.TraceEnd, FColor::Magenta, false, 1);
	}

	TArray<FIntVector> Points;
	if(Work.World->RenderType == EVoxelRenderType::Cubic)
	{
		// Make sure we're inside the cube
		FVector HitPoint = Hit.ImpactPoint + Hit.Normal * Work.World->VoxelSize / 2 * (Work.bAdd ? 1 : -1);
		Points.Add(Work.World->GlobalToLocal(HitPoint));
	}
	else
	{
		Points = Work.World->GetNeighboringPositions(Hit.ImpactPoint);
	}

	for (auto& Point : Points)
	{
		if (!Work.AddedPoints.Contains(Point) && Work.World->GetData().IsInWorld(Point))
		{
			Work.AddedPoints.Add(Point);
			Work.HitVoxels.Emplace(Point, Distance);
			if (Work.Bounds.Size().GetMax() == 0)
			{
				Work.Bounds = FIntBox(Point);
			}
			else
			{
				Work.Bounds += Point;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProjectionTools::TraceDone(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_TraceDone);

	FTraceDetails TraceDetails;
	if (!TraceMap.RemoveAndCopyValue(TraceHandle, TraceDetails)) { return; }

	auto& Work = *TraceDetails.Work;
	Work.TracesCompleted++;

	AVoxelWorld* World = Work.World.Get();
	if (!World) { return; }

	FHitResult Hit;
	{
		bool bHitIsSet = false;
		for (auto& OutHit : TraceDatum.OutHits)
		{
			if (OutHit.Actor == World)
			{
				Hit = OutHit;
				bHitIsSet = true;
				break;
			}
		}
		if (!bHitIsSet) { return; }
	}
	ProcessTraceResult(Work, Hit, TraceDetails.Distance);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<bool bAsync>
void UVoxelProjectionTools::FindHitVoxelsForRaycasts(FVoxelProjectionEditWork& Work, FVector Position, FVector Normal, float Radius, float ToolHeight, float EditDistance, float StepInVoxel)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts);

	auto World = Work.World;
	const FVector ToolPosition = Position + Normal * ToolHeight;

	/**
	* Create a 2D basis from (Tangent, Bitangent)
	*/
	FVector Tangent;
	FVector Bitangent;
	{
		// Compute tangent
		// N dot T = 0
		// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
		// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
		if (Normal.Z != 0)
		{
			Tangent.X = 1;
			Tangent.Y = 1;
			Tangent.Z = -1 / Normal.Z * (Normal.X + Normal.Y);
			Tangent.Normalize();
		}
		else
		{
			Tangent = FVector(0, 0, 1);
		}

		// Compute bitangent
		Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();
	}

	const float Step = StepInVoxel * World->VoxelSize;
	const int Count = FMath::CeilToInt(Radius / Step);
	const ECollisionChannel CollisionChannel = World->CollisionPresets.GetObjectType();

	for (int I = -Count; I <= Count; I++)
	{
		for (int J = -Count; J <= Count; J++)
		{
			float X = I * Step;
			float Y = J * Step;
			float Distance = FVector2D(X, Y).Size();

			if (Distance >= Radius)
			{
				continue;
			}

			//DrawDebugPoint(World->GetWorld(), Position + (Tangent * X + Bitangent * Y), 10, FColor::Red, false, World->GetWorld()->GetDeltaSeconds() * 2);

			// Use 2D basis
			FVector Start = ToolPosition + (Tangent * X + Bitangent * Y);
			FVector End = Start - Normal * EditDistance;
			if (bAsync)
			{
				FTraceHandle Handle;
				{
					SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts_StartAsyncTrace);
					Handle = World->GetWorld()->AsyncLineTraceByChannel(EAsyncTraceType::Single, Start, End, CollisionChannel, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam, &TraceDelegate);
				}
				FTraceDetails TraceDetails{ &Work, Distance };
				TraceMap.Add(Handle, TraceDetails);
				Work.TotalTraces++;
			}
			else
			{
				FHitResult Hit;
				{
					SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_FindHitVoxelsForRaycasts_StartTrace);
					World->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, CollisionChannel);
				}
				if (Hit.Actor == World)
				{
					ProcessTraceResult(Work, Hit, Distance);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void SetValueProjectionHelper(
	FVoxelProjectionEditWork& Work,
	float Radius,
	float LockTimeoutInSeconds,
	float Strength, 
	UCurveFloat* StrengthCurve, 
	TArray<FModifiedVoxelValue>& ModifiedVoxels,
	EBlueprintSuccess& Branches, 
	EFailReason& FailReason,
	FString& LockerName)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_SetValueProjection);
	
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;

	if (Work.HitVoxels.Num() == 0) { return; }

	AVoxelWorld* World = Work.World.Get();
	if (!World) { return; }

	FVoxelData& Data = World->GetData();
	{
		FVoxelReadWriteScopeTryLock Lock(Data, Work.Bounds, LockTimeoutInSeconds, "SetValueProjection");
		if (!Lock.Success())
		{
			LockerName = Lock.GetLockerName();
			FailReason = EFailReason::VoxelDataLocked;
			return;
		}

		FVoxelDataUtilities::LastOctreeAccelerator OctreeAccelerator(Data);
		for (auto& Hit : Work.HitVoxels)
		{
			if (LIKELY(Data.IsInWorld(Hit.Position)))
			{
				float OldValue = OctreeAccelerator.GetValue(Hit.Position, 0).ToFloat();
				float NewValue;
				if (World->RenderType == EVoxelRenderType::MarchingCubes)
				{
					float CurrentStrength = Strength * (StrengthCurve ? StrengthCurve->FloatCurve.Eval(Hit.Distance / Radius) : 1.f);
					NewValue = FMath::Clamp<float>(OldValue + CurrentStrength, -1, 1);
				}
				else
				{
					NewValue = Work.bAdd ? -1 : 1;
				}
				OctreeAccelerator.SetValue(Hit.Position, NewValue);

				FModifiedVoxelValue Voxel;
				Voxel.Position = Hit.Position;
				Voxel.OldValue = OldValue;
				Voxel.NewValue = NewValue;
				ModifiedVoxels.Add(Voxel);
			}
		}
	}

	World->GetLODManager().UpdateBounds(Work.Bounds, true);

	Branches = EBlueprintSuccess::Success;
}

///////////////////////////////////////////////////////////////////////////////

inline void SetMaterialProjectionHelper(
	FVoxelProjectionEditWork& Work,
	float Radius,
	float LockTimeoutInSeconds,
	FVoxelPaintMaterial PaintMaterial,
	UCurveFloat* StrengthCurve,
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels, 
	EBlueprintSuccess& Branches,
	EFailReason& FailReason,
	FString& LockerName)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProjectionTools_SetMaterialProjection);
	
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;

	if (Work.HitVoxels.Num() == 0) { return; }

	AVoxelWorld* World = Work.World.Get();
	if (!World) { return; }

	FVoxelData& Data = World->GetData();
	{
		FVoxelReadWriteScopeTryLock Lock(Data, Work.Bounds, LockTimeoutInSeconds, "SetMaterialProjection");
		if (!Lock.Success())
		{
			LockerName = Lock.GetLockerName();
			FailReason = EFailReason::VoxelDataLocked;
			return;
		}

		FVoxelDataUtilities::LastOctreeAccelerator OctreeAccelerator(Data);
		for (auto& Hit : Work.HitVoxels)
		{
			if (LIKELY(Data.IsInWorld(Hit.Position)))
			{
				FModifiedVoxelMaterial Voxel;
				Voxel.Position = Hit.Position;

				FVoxelMaterial Material = OctreeAccelerator.GetMaterial(Hit.Position, 0);

				Voxel.OldMaterial = Material;
				PaintMaterial.ApplyToMaterial(Material, StrengthCurve ? StrengthCurve->FloatCurve.Eval(Hit.Distance / Radius) : 1);
				Voxel.NewMaterial = Material;

				OctreeAccelerator.SetMaterial(Hit.Position, Material);

				ModifiedVoxels.Add(Voxel);
			}
		}
	}

	World->GetLODManager().UpdateBounds(Work.Bounds, true);

	Branches = EBlueprintSuccess::Success;
}

///////////////////////////////////////////////////////////////////////////////

void GetVoxelsProjectionHelper(
	FVoxelProjectionEditWork& Work,
	TArray<FGetVoxelProjectionVoxel>& OutVoxels,
	EBlueprintSuccess& Branches)
{
	Branches = EBlueprintSuccess::Failed;

	if (Work.HitVoxels.Num() == 0) { return; }

	AVoxelWorld* World = Work.World.Get();
	if (!World) { return; }

	FVoxelData& Data = World->GetData();
	{
		FVoxelReadScopeLock Lock(Data, Work.Bounds, "GetVoxelsProjection");

		FVoxelDataUtilities::LastOctreeAccelerator OctreeAccelerator(Data);
		for (auto& Hit : Work.HitVoxels)
		{
			if (LIKELY(Data.IsInWorld(Hit.Position)))
			{
				FGetVoxelProjectionVoxel Voxel;
				Voxel.Position = Hit.Position;
				FVoxelValue Value;
				OctreeAccelerator.GetValueAndMaterial(Hit.Position, Value, Voxel.Material, 0);
				Voxel.Value = Value.ToFloat();

				OutVoxels.Add(Voxel);
			}
		}
	}

	Branches = EBlueprintSuccess::Success;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelProjectionTools::UVoxelProjectionTools()
{
	if (!TraceDelegate.IsBound())
	{
		TraceDelegate.BindStatic(&TraceDone);
	}
}

void UVoxelProjectionTools::SetValueProjectionAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	EBlueprintSuccess& Branches,
	EFailReason& FailReason,
	FString& LockerName,
	TArray<FModifiedVoxelValue>& ModifiedVoxels, 
	AVoxelWorld* World, 
	FVector Position,
	FVector Normal,
	float Radius, 
	float Strength,
	UCurveFloat* StrengthCurve,
	float ToolHeight, 
	float EditDistance, 
	float StepInVoxel, 
	float LockTimeoutInSeconds, 
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;
	LockerName = "";

	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto* ModifiedVoxelsPtr = &ModifiedVoxels;
	auto* BranchesPtr = &Branches;
	auto* FailReasonPtr = &FailReason;
	auto* LockerNamePtr = &LockerName;

	if (UWorld* ObjectWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = ObjectWorld->GetLatentActionManager();
		if (!LatentActionManager.FindExistingAction<FVoxelProjectionEditLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			auto Work = MakeShared<FVoxelProjectionEditWork>(World, bShowRaycasts, Strength <= 0);
			FindHitVoxelsForRaycasts<true>(*Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);

			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FVoxelProjectionEditLatentAction(Work, LatentInfo,
				[=](FVoxelProjectionEditWork& Work)
				{
					SetValueProjectionHelper(Work, Radius, LockTimeoutInSeconds, Strength, StrengthCurve, *ModifiedVoxelsPtr, *BranchesPtr, *FailReasonPtr, *LockerNamePtr);
				}
			));
		}
		else
		{
			FailReason = EFailReason::LinetracesPending;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelProjectionTools::SetMaterialProjectionAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	EBlueprintSuccess& Branches, 
	EFailReason& FailReason,
	FString& LockerName,
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels, 
	AVoxelWorld* World,
	FVector Position, 
	FVector Normal,
	FVoxelPaintMaterial PaintMaterial,
	UCurveFloat* StrengthCurve,
	float Radius,
	float ToolHeight,
	float EditDistance,
	float StepInVoxel,
	float LockTimeoutInSeconds, 
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;
	LockerName = "";
	
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto* ModifiedVoxelsPtr = &ModifiedVoxels;
	auto* BranchesPtr = &Branches;
	auto* FailReasonPtr = &FailReason;
	auto* LockerNamePtr = &LockerName;

	if (UWorld* ObjectWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = ObjectWorld->GetLatentActionManager();
		if (!LatentActionManager.FindExistingAction<FVoxelProjectionEditLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			auto Work = MakeShared<FVoxelProjectionEditWork>(World, bShowRaycasts, false);
			FindHitVoxelsForRaycasts<true>(*Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);

			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FVoxelProjectionEditLatentAction(Work, LatentInfo,
				[=](FVoxelProjectionEditWork& Work)
				{
					SetMaterialProjectionHelper(Work, Radius, LockTimeoutInSeconds, PaintMaterial, StrengthCurve, *ModifiedVoxelsPtr, *BranchesPtr, *FailReasonPtr, *LockerNamePtr);
				}
			));
		}
		else
		{
			FailReason = EFailReason::LinetracesPending;
		}
	}
}

void UVoxelProjectionTools::GetVoxelsProjectionAsync(
	UObject* WorldContextObject, 
	struct FLatentActionInfo LatentInfo, 
	EBlueprintSuccess& Branches, 
	TArray<FGetVoxelProjectionVoxel>& OutVoxels,
	AVoxelWorld* World,
	FVector Position, 
	FVector Normal, 
	float Radius, 
	float ToolHeight,
	float EditDistance,
	float StepInVoxel,
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;
	
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto* OutVoxelsPtr = &OutVoxels;
	auto* BranchesPtr = &Branches;

	if (UWorld* ObjectWorld = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = ObjectWorld->GetLatentActionManager();
		if (!LatentActionManager.FindExistingAction<FVoxelProjectionEditLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			auto Work = MakeShared<FVoxelProjectionEditWork>(World, bShowRaycasts, false);
			FindHitVoxelsForRaycasts<true>(*Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);

			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FVoxelProjectionEditLatentAction(Work, LatentInfo,
				[=](FVoxelProjectionEditWork& Work)
				{
					GetVoxelsProjectionHelper(Work, *OutVoxelsPtr, *BranchesPtr);
				}
			));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProjectionTools::SetValueProjectionNew(
	EBlueprintSuccess& Branches,
	EFailReason& FailReason,
	FString& LockerName,
	TArray<FModifiedVoxelValue>& ModifiedVoxels,
	AVoxelWorld* World,
	FVector Position,
	FVector Normal,
	float Radius,
	float Strength,
	UCurveFloat* StrengthCurve,
	float ToolHeight,
	float EditDistance,
	float StepInVoxel,
	float LockTimeoutInSeconds,
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;
	LockerName = "";

	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVoxelProjectionEditWork Work(World, bShowRaycasts, Strength <= 0);
	FindHitVoxelsForRaycasts<false>(Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);
	SetValueProjectionHelper(Work, Radius, LockTimeoutInSeconds, Strength, StrengthCurve, ModifiedVoxels, Branches, FailReason, LockerName);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelProjectionTools::SetMaterialProjectionNew(
	EBlueprintSuccess& Branches,
	EFailReason& FailReason,
	FString& LockerName,
	TArray<FModifiedVoxelMaterial>& ModifiedVoxels,
	AVoxelWorld* World,
	FVector Position,
	FVector Normal,
	FVoxelPaintMaterial PaintMaterial,
	UCurveFloat* StrengthCurve,
	float Radius,
	float ToolHeight,
	float EditDistance,
	float StepInVoxel,
	float LockTimeoutInSeconds,
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;
	FailReason = EFailReason::OtherError;
	LockerName = "";

	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVoxelProjectionEditWork Work(World, bShowRaycasts, false);
	FindHitVoxelsForRaycasts<false>(Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);
	SetMaterialProjectionHelper(Work, Radius, LockTimeoutInSeconds, PaintMaterial, StrengthCurve, ModifiedVoxels, Branches, FailReason, LockerName);
}

void UVoxelProjectionTools::GetVoxelsProjection(
	EBlueprintSuccess& Branches,
	TArray<FGetVoxelProjectionVoxel>& OutVoxels,
	AVoxelWorld* World,
	FVector Position, 
	FVector Normal, 
	float Radius,
	float ToolHeight, 
	float EditDistance,
	float StepInVoxel,
	bool bShowRaycasts)
{
	Branches = EBlueprintSuccess::Failed;

	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVoxelProjectionEditWork Work(World, bShowRaycasts, false);
	FindHitVoxelsForRaycasts<false>(Work, Position, Normal, Radius, ToolHeight, EditDistance, StepInVoxel);
	GetVoxelsProjectionHelper(Work, OutVoxels, Branches);
}
