// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "IntBox.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelProcGen/VoxelProcGenManager.h"
#include "IVoxelPool.h"
#include "VoxelDefaultPool.h"
#include "VoxelMessages.h"

#include "Async/Async.h"
#include "Engine/StaticMesh.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelBlueprintLibrary::IsVoxelPluginPro()
{
	return false;
}

void UVoxelBlueprintLibrary::RaiseInfo(FString Message, UObject* Object)
{
	FVoxelMessages::Info(Message, Object);
}

void UVoxelBlueprintLibrary::RaiseWarning(FString Message, UObject* Object)
{
	FVoxelMessages::Warning(Message, Object);
}

void UVoxelBlueprintLibrary::RaiseError(FString Message, UObject* Object)
{
	FVoxelMessages::Error(Message, Object);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::SpawnVoxelSpawnerActorsInArea(
	TArray<AVoxelSpawnerActor*>& OutActors, 
	AVoxelWorld* World,
	FIntBox Bounds, 
	EVoxelSpawnerActorSpawnType SpawnType)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelBlueprintLibrary::AddInstances(
	AVoxelWorld* const World, 
	UStaticMesh* const Mesh,
	const TArray<FTransform>& Transforms,
	const TArray<FLinearColor>& Colors,
	FVoxelInstancedMeshSettings InstanceSettings,
	const float StartCullDistance, 
	const float EndCullDistance, 
	const TSubclassOf<AVoxelSpawnerActor> ActorTemplate,
	const FVector FloatingDetectionOffset)
{
	VOXEL_PRO_ONLY_VOID();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::Undo(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		FVoxelMessages::Error("Undo: Undo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits");
		return;
	}

	TArray<FIntBox> BoundsToUpdate;
	Data.Undo(BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
}

void UVoxelBlueprintLibrary::Redo(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		FVoxelMessages::Error("Redo: Redo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits");
		return;
	}
	
	TArray<FIntBox> BoundsToUpdate;
	Data.Redo(BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
}

void UVoxelBlueprintLibrary::SaveFrame(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return;
	}
	Data.SaveFrame(FIntBox::Infinite);
}

void UVoxelBlueprintLibrary::ClearFrames(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return;
	}
	Data.ClearFrames();
}

int32 UVoxelBlueprintLibrary::GetHistoryPosition(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return 0;
	}
	return Data.GetHistoryPosition();
}

///////////////////////////////////////////////////////////////////////////////

FVector UVoxelBlueprintLibrary::GetNormal(AVoxelWorld* World, FIntVector Position)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	const auto& Data = World->GetData();
	FVoxelReadScopeLock Lock(Data, FIntBox(Position - FIntVector(1), Position + FIntVector(2)), "GetNormal");
	return FVoxelDataUtilities::GetGradientFromGetValue<int32>(FVoxelDataUtilities::MakeFloatData(Data), Position.X, Position.Y, Position.Z, 0);
}

float UVoxelBlueprintLibrary::GetFloatOutput(AVoxelWorld* World, FName Name, float X, float Y, float Z, float DefaultValue)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	if (!Data.WorldGenerator->FloatOutputsPtr.Contains(Name))
	{
		FString Names;
		for (auto& It : Data.WorldGenerator->FloatOutputsPtr)
		{
			if (!Names.IsEmpty())
			{
				Names += ", ";
			}
			Names += It.Key.ToString();
		}
		FVoxelMessages::Error(FString::Printf(TEXT("GetFloatOutput: No output named %s and with type float found! Valid names: %s"), *Name.ToString(), *Names));
		return 0;
	}

	return Data.GetCustomOutput<v_flt>(DefaultValue, Name, X, Y, Z, 0);
}

int32 UVoxelBlueprintLibrary::GetIntOutput(AVoxelWorld* World, FName Name, float X, float Y, float Z, int32 DefaultValue)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	if (!Data.WorldGenerator->Int32OutputsPtr.Contains(Name))
	{
		FString Names;
		for (auto& It : Data.WorldGenerator->Int32OutputsPtr)
		{
			if (!Names.IsEmpty())
			{
				Names += ", ";
			}
			Names += It.Key.ToString();
		}
		FVoxelMessages::Error(FString::Printf(TEXT("GetIntOutput: No output named %s and with type int found! Valid names: %s"), *Name.ToString(), *Names));
		return 0;
	}

	return Data.GetCustomOutput<int32>(DefaultValue, Name, X, Y, Z, 0);
}

FIntBox UVoxelBlueprintLibrary::GetBounds(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	return World->GetData().WorldBounds;
}

void UVoxelBlueprintLibrary::ClearAllData(AVoxelWorld* World, bool bUpdateRender)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	World->GetData().ClearData();

	if(bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(FIntBox::Infinite);
	}
}

void UVoxelBlueprintLibrary::ClearDirtyData(AVoxelWorld* World, bool bUpdateRender)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	auto& Data = World->GetData();

	TArray<FIntBox> OutBoundsToUpdate;

	{
		FVoxelWriteScopeLock Lock(Data, FIntBox::Infinite, FUNCTION_FNAME);
		Data.ClearOctreeData(OutBoundsToUpdate);
	}

	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(OutBoundsToUpdate);
	}
}

void UVoxelBlueprintLibrary::SetBoxAsDirty(AVoxelWorld* World, const FIntBox& Bounds, bool bSetValuesAsDirty, bool bSetMaterialsAsDirty)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();

	FVoxelData& Data = World->GetData();
	{
		FVoxelWriteScopeLock Lock(Data, Bounds, FUNCTION_FNAME);

		if (bSetValuesAsDirty)
		{
			Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
			{

			});
		}
		if (bSetMaterialsAsDirty)
		{
			Data.Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
			{

			});
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::UpdatePosition(AVoxelWorld* World, FIntVector Position)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetLODManager().UpdateBounds(FIntBox(Position));
}

void UVoxelBlueprintLibrary::UpdateBounds(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_BOUNDS_ARE_VALID_VOID();
	World->GetLODManager().UpdateBounds(Bounds);
}

void UVoxelBlueprintLibrary::UpdateAll(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetLODManager().UpdateBounds(FIntBox::Infinite);
}

void UVoxelBlueprintLibrary::ApplyLODSettings(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->UpdateDynamicLODSettings();
	World->GetLODManager().ForceLODsUpdate();
}

bool UVoxelBlueprintLibrary::AreCollisionsEnabled(AVoxelWorld* World, FVector Position, int32& LOD, bool bConvertToVoxelSpace /*= true*/)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	LOD = 0;
	Position = FVoxelToolHelpers::GetRealPosition(World, Position, bConvertToVoxelSpace);

	auto& LODManager = World->GetLODManager();
	if (!LODManager.Settings.WorldBounds.ContainsFloat(Position))
	{
		return false;
	}

	uint8 OutLOD;
	const bool bResult = LODManager.AreCollisionsEnabled(FVoxelUtilities::RoundToInt(Position), OutLOD);
	LOD = OutLOD;

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelBlueprintLibrary::GetTaskCount(AVoxelWorld* World)
{
	return World && World->IsCreated() ? FMath::Max(World->GetRenderer().GetTaskCount(), 0) : 0;
}

void UVoxelBlueprintLibrary::ApplyNewMaterials(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	World->GetRenderer().ApplyNewMaterials();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::BindVoxelChunkEvents(
	AVoxelWorld* World,
	FChunkDynamicDelegate OnActivate,
	FChunkDynamicDelegate OnDeactivate,
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 ActivationDistanceInChunks)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto OnActivateLambda = [OnActivate](const FIntBox& Bounds)
	{
		OnActivate.ExecuteIfBound(Bounds);
	};
	auto OnDeactivateLambda = [OnDeactivate](const FIntBox& Bounds)
	{
		OnDeactivate.ExecuteIfBound(Bounds);
	};

	World->GetProcGenManager().BindEvent(
		bFireExistingOnes,
		FMath::Max(1, ChunkSize),
		FMath::Max(0, ActivationDistanceInChunks),
		FChunkDelegate::CreateLambda(OnActivateLambda),
		FChunkDelegate::CreateLambda(OnDeactivateLambda));
}

void UVoxelBlueprintLibrary::BindVoxelGenerationEvent(
	AVoxelWorld* World,
	FChunkDynamicDelegate OnGenerate,
	bool bFireExistingOnes,
	int32 ChunkSize,
	int32 GenerationDistanceInChunks)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto OnGenerateLambda = [OnGenerate](const FIntBox& Bounds)
	{
		OnGenerate.ExecuteIfBound(Bounds);
	};

	World->GetProcGenManager().BindGenerationEvent(
		bFireExistingOnes,
		FMath::Max(1, ChunkSize),
		FMath::Max(0, GenerationDistanceInChunks),
		FChunkDelegate::CreateLambda(OnGenerateLambda));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelBlueprintLibrary::IsValidRef(AVoxelWorld* World, FVoxelToolRenderingRef Ref)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	return Ref.Id.IsValid() && World->GetToolRenderingManager().IsValidTool(Ref.Id);
}

FVoxelToolRenderingRef UVoxelBlueprintLibrary::CreateToolRendering(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	return { World->GetToolRenderingManager().CreateTool() };
}

#define CHECK_TOOL_RENDERING_REF() \
	if (!Ref.Id.IsValid()) { FVoxelMessages::Error(FUNCTION_ERROR("Unitilialized tool rendering reference")); return; } \
	if (!World->GetToolRenderingManager().IsValidTool(Ref.Id))  { FVoxelMessages::Error(FUNCTION_ERROR("Outdated tool rendering reference")); return; }

void UVoxelBlueprintLibrary::DestroyToolRendering(AVoxelWorld* World, FVoxelToolRenderingRef Ref)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_TOOL_RENDERING_REF();

	World->GetToolRenderingManager().RemoveTool(Ref.Id);
}

void UVoxelBlueprintLibrary::SetToolRenderingMaterial(AVoxelWorld* World, FVoxelToolRenderingRef Ref, UMaterialInterface* Material)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_TOOL_RENDERING_REF();

	World->GetToolRenderingManager().EditTool(Ref.Id, [&](auto& Tool) { Tool.Material = FVoxelMaterialInterface::Create(Material); });
}

void UVoxelBlueprintLibrary::SetToolRenderingBounds(AVoxelWorld* World, FVoxelToolRenderingRef Ref, FBox Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_TOOL_RENDERING_REF();

	World->GetToolRenderingManager().EditTool(Ref.Id, [&](auto& Tool) { Tool.WorldBounds = Bounds; });
}

void UVoxelBlueprintLibrary::SetToolRenderingEnabled(AVoxelWorld* World, FVoxelToolRenderingRef Ref, bool bEnabled)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_TOOL_RENDERING_REF();

	World->GetToolRenderingManager().EditTool(Ref.Id, [&](auto& Tool) { Tool.bEnabled = bEnabled; });
}

#undef CHECK_TOOL_RENDERING_REF

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::CreateGlobalVoxelThreadPool(
	UObject* WorldContextObject,
	const TMap<EVoxelTaskType, int32>& PriorityCategoriesOverrides,
	const TMap<EVoxelTaskType, int32>& PriorityOffsetsOverrides,
	int32 NumberOfThreads,
	bool bConstantPriorities)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (IsGlobalVoxelPoolCreated(WorldContextObject->GetWorld()))
	{
		FVoxelMessages::Error("CreateGlobalVoxelThreadPool called but global pool already created! Creator: " +
			IVoxelPool::GetGlobalPoolCreator(WorldContextObject->GetWorld()));
		return;
	}
	const auto Pool = FVoxelDefaultPool::Create(
		FMath::Max(1, NumberOfThreads),
		bConstantPriorities,
		PriorityCategoriesOverrides,
		PriorityOffsetsOverrides);
	IVoxelPool::SetGlobalVoxelPool(WorldContextObject->GetWorld(), Pool, "CreateGlobalVoxelThreadPool");
}

void UVoxelBlueprintLibrary::DestroyGlobalVoxelThreadPool(UObject* WorldContextObject)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!IsGlobalVoxelPoolCreated(WorldContextObject->GetWorld()))
	{
		FVoxelMessages::Error("DestroyGlobalVoxelThreadPool called but global pool isn't created!");
		return;
	}
	IVoxelPool::DestroyGlobalVoxelPool(WorldContextObject->GetWorld());
}

bool UVoxelBlueprintLibrary::IsGlobalVoxelPoolCreated(UObject* WorldContextObject)
{
	VOXEL_FUNCTION_COUNTER();
	
	return IVoxelPool::IsGlobalVoxelPoolCreated(WorldContextObject->GetWorld());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FIntBox UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(AVoxelWorld* World, FVector GlobalPosition, float Radius)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FIntBox::SafeConstruct(
		World->GlobalToLocal(GlobalPosition - Radius, EVoxelWorldCoordinatesRounding::RoundDown),
		World->GlobalToLocal(GlobalPosition + Radius, EVoxelWorldCoordinatesRounding::RoundUp)
	);
}

FIntBox UVoxelBlueprintLibrary::GetRenderBoundsOverlappingDataBounds(AVoxelWorld* World, FIntBox Bounds, int32 LOD)
{
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_BOUNDS_ARE_VALID();

	LOD = FVoxelUtilities::ClampChunkDepth(LOD);

	Bounds = Bounds.MakeMultipleOfBigger(RENDER_CHUNK_SIZE << LOD);

	return Bounds.Extend(2 << LOD); // Account for the normals reads
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet)
{
	VOXEL_FUNCTION_COUNTER();
	
	OutSet.Reset();
	for (auto& P : InSet)
	{
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z - 1));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 0));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 1));
	}
}

#undef LOCTEXT_NAMESPACE