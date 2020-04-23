// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/VoxelDataTools.h"
#include "IntBox.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelSpawners/VoxelHierarchicalInstancedStaticMeshComponent.h"
#include "VoxelEvents/VoxelEventManager.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "IVoxelPool.h"
#include "VoxelDefaultPool.h"
#include "VoxelMessages.h"
#include "VoxelWorldGeneratorUtilities.h"

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

int32 UVoxelBlueprintLibrary::NumberOfCores()
{
	return FPlatformMisc::NumberOfCores();
}

float UVoxelBlueprintLibrary::GetMemoryUsageInMB(EVoxelMemoryUsageType Type)
{
#if ENABLE_VOXEL_MEMORY_STATS
#define CASE(X) return X.GetValue() / double(1 << 20);
	switch (Type)
	{
	case EVoxelMemoryUsageType::Total:
		CASE(STAT_TotalVoxelMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsDirtyValuesData:
		CASE(STAT_VoxelDataOctreeDirtyValuesMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsDirtyMaterialsData:
		CASE(STAT_VoxelDataOctreeDirtyMaterialsMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsDirtyFoliageData:
		CASE(STAT_VoxelDataOctreeDirtyFoliageMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsCachedValuesData:
		CASE(STAT_VoxelDataOctreeCachedValuesMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsCachedMaterialsData:
		CASE(STAT_VoxelDataOctreeCachedMaterialsMemory_MemoryUsage);
	case EVoxelMemoryUsageType::VoxelsCachedFoliageData:
		CASE(STAT_VoxelDataOctreeCachedFoliageMemory_MemoryUsage);
	case EVoxelMemoryUsageType::UndoRedo:
		CASE(STAT_VoxelUndoRedoMemory_MemoryUsage);
	case EVoxelMemoryUsageType::Multiplayer:
		CASE(STAT_VoxelMultiplayerMemory_MemoryUsage);
	case EVoxelMemoryUsageType::IntermediateBuffers:
		CASE(STAT_VoxelChunkMeshMemory_MemoryUsage);
	case EVoxelMemoryUsageType::MeshesIndices:
		CASE(STAT_VoxelProcMeshMemory_Indices_MemoryUsage);
	case EVoxelMemoryUsageType::MeshesTessellationIndices:
		CASE(STAT_VoxelProcMeshMemory_Adjacency_MemoryUsage);
	case EVoxelMemoryUsageType::MeshesVertices:
		CASE(STAT_VoxelProcMeshMemory_Positions_MemoryUsage);
	case EVoxelMemoryUsageType::MeshesColors:
		CASE(STAT_VoxelProcMeshMemory_Colors_MemoryUsage);
	case EVoxelMemoryUsageType::MeshesUVsAndTangents:
		CASE(STAT_VoxelProcMeshMemory_UVs_Tangents_MemoryUsage);
	case EVoxelMemoryUsageType::DataAssets:
		CASE(STAT_VoxelDataAssetMemory_MemoryUsage);
	case EVoxelMemoryUsageType::HeightmapAssets:
		CASE(STAT_VoxelHeightmapAssetMemory_MemoryUsage);
	case EVoxelMemoryUsageType::UncompressedSaves:
		CASE(STAT_VoxelUncompressedSavesMemory_MemoryUsage);
	case EVoxelMemoryUsageType::CompressedSaves:
		CASE(STAT_VoxelCompressedSavesMemory_MemoryUsage);
	default:
		ensure(false);
		return 0.f;
	}
#undef CASE
#else
	FVoxelMessages::Error(FUNCTION_ERROR("Requires ENABLE_VOXEL_MEMORY_STATS=1"));
	return 0.f;
#endif
}

float UVoxelBlueprintLibrary::GetPeakMemoryUsageInMB(EVoxelMemoryUsageType Type)
{
#if ENABLE_VOXEL_MEMORY_STATS
#define CASE(X) return X.GetValue() / double(1 << 20);
	switch (Type)
	{
	case EVoxelMemoryUsageType::Total:
		CASE(STAT_TotalVoxelMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsDirtyValuesData:
		CASE(STAT_VoxelDataOctreeDirtyValuesMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsDirtyMaterialsData:
		CASE(STAT_VoxelDataOctreeDirtyMaterialsMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsDirtyFoliageData:
		CASE(STAT_VoxelDataOctreeDirtyFoliageMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsCachedValuesData:
		CASE(STAT_VoxelDataOctreeCachedValuesMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsCachedMaterialsData:
		CASE(STAT_VoxelDataOctreeCachedMaterialsMemory_MemoryPeak);
	case EVoxelMemoryUsageType::VoxelsCachedFoliageData:
		CASE(STAT_VoxelDataOctreeCachedFoliageMemory_MemoryPeak);
	case EVoxelMemoryUsageType::UndoRedo:
		CASE(STAT_VoxelUndoRedoMemory_MemoryPeak);
	case EVoxelMemoryUsageType::Multiplayer:
		CASE(STAT_VoxelMultiplayerMemory_MemoryPeak);
	case EVoxelMemoryUsageType::IntermediateBuffers:
		CASE(STAT_VoxelChunkMeshMemory_MemoryPeak);
	case EVoxelMemoryUsageType::MeshesIndices:
		CASE(STAT_VoxelProcMeshMemory_Indices_MemoryPeak);
	case EVoxelMemoryUsageType::MeshesTessellationIndices:
		CASE(STAT_VoxelProcMeshMemory_Adjacency_MemoryPeak);
	case EVoxelMemoryUsageType::MeshesVertices:
		CASE(STAT_VoxelProcMeshMemory_Positions_MemoryPeak);
	case EVoxelMemoryUsageType::MeshesColors:
		CASE(STAT_VoxelProcMeshMemory_Colors_MemoryPeak);
	case EVoxelMemoryUsageType::MeshesUVsAndTangents:
		CASE(STAT_VoxelProcMeshMemory_UVs_Tangents_MemoryPeak);
	case EVoxelMemoryUsageType::DataAssets:
		CASE(STAT_VoxelDataAssetMemory_MemoryPeak);
	case EVoxelMemoryUsageType::HeightmapAssets:
		CASE(STAT_VoxelHeightmapAssetMemory_MemoryPeak);
	case EVoxelMemoryUsageType::UncompressedSaves:
		CASE(STAT_VoxelUncompressedSavesMemory_MemoryPeak);
	case EVoxelMemoryUsageType::CompressedSaves:
		CASE(STAT_VoxelCompressedSavesMemory_MemoryPeak);
	default:
		ensure(false);
		return 0.f;
	}
#undef CASE
#else
	FVoxelMessages::Error(FUNCTION_ERROR("Requires ENABLE_VOXEL_MEMORY_STATS=1"));
	return 0.f;
#endif
}

void UVoxelBlueprintLibrary::LogMemoryStats()
{
#if ENABLE_VOXEL_MEMORY_STATS
	struct FNameAndUsage
	{
		const TCHAR* Name = nullptr;
		int64 Usage = 0;
	};
	TArray<FNameAndUsage> Usages;
	TArray<FNameAndUsage> Peaks;
	for (auto& It : GetVoxelMemoryCounters())
	{
		Usages.Add(FNameAndUsage{ It.Key, It.Value.UsageCounterPtr->GetValue() });
		Peaks.Add(FNameAndUsage{ It.Key, It.Value.PeakCounterPtr->GetValue() });
	}

	Usages.Sort([](FNameAndUsage A, FNameAndUsage B) { return A.Usage > B.Usage; });
	Peaks.Sort([](FNameAndUsage A, FNameAndUsage B) { return A.Usage > B.Usage; });
	
	LOG_VOXEL(Log, TEXT("--------------------------------------"));
	LOG_VOXEL(Log, TEXT("Voxel Memory Usage:"));
	for (auto& Usage : Usages)
	{
		LOG_VOXEL(Log, TEXT("%50s: %6fMB"), Usage.Name, Usage.Usage / double(1 << 20));
	}
	LOG_VOXEL(Log, TEXT("--------------------------------------"));
	
	LOG_VOXEL(Log, TEXT("--------------------------------------"));
	LOG_VOXEL(Log, TEXT("Voxel Memory Peaks:"));
	for (auto& Usage : Peaks)
	{
		LOG_VOXEL(Log, TEXT("%50s: %6fMB"), Usage.Name, Usage.Usage / double(1 << 20));
	}
	LOG_VOXEL(Log, TEXT("--------------------------------------"));
#else
	FVoxelMessages::Error(FUNCTION_ERROR("Requires ENABLE_VOXEL_MEMORY_STATS=1"));
	return 0.f;
#endif
}

float UVoxelBlueprintLibrary::GetEstimatedCollisionsMemoryUsageInMB(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	uint64 MemoryUsage = 0;
	for (auto* Component : World->GetComponents())
	{
		auto* ProcMeshComponent = Cast<UVoxelProceduralMeshComponent>(Component);
		if (!ProcMeshComponent) continue;
		ProcMeshComponent->IterateSections([&](auto& Settings, const FVoxelProcMeshBuffers& Buffers)
		{
			MemoryUsage += Buffers.IndexBuffer.GetAllocatedSize();
			MemoryUsage += Buffers.VertexBuffers.PositionVertexBuffer.GetNumVertices() * Buffers.VertexBuffers.PositionVertexBuffer.GetStride();
		});
	}
	return MemoryUsage / double(1 << 20);
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

AVoxelSpawnerActor* UVoxelBlueprintLibrary::SpawnVoxelSpawnerActorByInstanceIndex(
	AVoxelWorld* World, 
	UVoxelHierarchicalInstancedStaticMeshComponent* Component, 
	int32 InstanceIndex)
{
	VOXEL_PRO_ONLY();
}

void UVoxelBlueprintLibrary::AddInstances(
	AVoxelWorld* const World, 
	UStaticMesh* const Mesh,
	const TArray<FTransform>& Transforms,
	const TArray<FLinearColor>& Colors,
	FVoxelInstancedMeshSettings InstanceSettings,
	FVoxelSpawnerActorSettings ActorSettings,
	const FVector FloatingDetectionOffset)
{
	VOXEL_PRO_ONLY_VOID();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelBlueprintLibrary::RegenerateSpawners(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelBlueprintLibrary::MarkSpawnersDirty(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_PRO_ONLY_VOID();
}

FVoxelSpawnersSave UVoxelBlueprintLibrary::GetSpawnersSave(AVoxelWorld* World)
{
	VOXEL_PRO_ONLY();
}

void UVoxelBlueprintLibrary::LoadFromSpawnersSave(AVoxelWorld* World, const FVoxelSpawnersSave& Save)
{
	VOXEL_PRO_ONLY_VOID();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelBlueprintLibrary::Undo(AVoxelWorld* World, TArray<FIntBox>& OutUpdatedBounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return false;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		FVoxelMessages::Error("Undo: Undo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits");
		return false;
	}

	OutUpdatedBounds.Reset();
	if (!Data.Undo(OutUpdatedBounds))
	{
		return false;
	}
	
	World->GetLODManager().UpdateBounds(OutUpdatedBounds);
	return true;
}

bool UVoxelBlueprintLibrary::Undo(AVoxelWorld* World)
{
	TArray<FIntBox> Dummy;
	return Undo(World, Dummy);
}

bool UVoxelBlueprintLibrary::Redo(AVoxelWorld* World, TArray<FIntBox>& OutUpdatedBounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	if (!Data.bEnableUndoRedo)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("bEnableUndoRedo is false!"));
		return false;
	}
	if (!Data.IsCurrentFrameEmpty())
	{
		FVoxelMessages::Error("Redo: Redo called but edits have been made since last SaveFrame. Please call SaveFrame after every edits");
		return false;
	}

	OutUpdatedBounds.Reset();
	if (!Data.Redo(OutUpdatedBounds))
	{
		return false;
	}
	
	World->GetLODManager().UpdateBounds(OutUpdatedBounds);
	return true;
}

bool UVoxelBlueprintLibrary::Redo(AVoxelWorld* World)
{
	TArray<FIntBox> Dummy;
	return Redo(World, Dummy);
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

	if (!Data.WorldGenerator->GetOutputsPtrMap<v_flt>().Contains(Name))
	{
		FVoxelMessages::Error(FUNCTION_ERROR(FVoxelUtilities::GetMissingWorldGeneratorOutputErrorString<v_flt>(Name, *Data.WorldGenerator)));
		return 0;
	}

	return Data.GetCustomOutput<v_flt>(DefaultValue, Name, X, Y, Z, 0);
}

int32 UVoxelBlueprintLibrary::GetIntOutput(AVoxelWorld* World, FName Name, float X, float Y, float Z, int32 DefaultValue)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	if (!Data.WorldGenerator->GetOutputsPtrMap<int32>().Contains(Name))
	{
		FVoxelMessages::Error(FUNCTION_ERROR(FVoxelUtilities::GetMissingWorldGeneratorOutputErrorString<int32>(Name, *Data.WorldGenerator)));
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

	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(FIntBox::Infinite);
	}
}

void UVoxelBlueprintLibrary::ClearValueData(AVoxelWorld* World, bool bUpdateRender)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	{
		FVoxelWriteScopeLock Lock(Data, FIntBox::Infinite, FUNCTION_FNAME);
		FVoxelDataUtilities::ClearData<FVoxelValue>(Data);
	}
	
	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(FIntBox::Infinite);
	}
}

void UVoxelBlueprintLibrary::ClearMaterialData(AVoxelWorld* World, bool bUpdateRender)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetData();

	{
		FVoxelWriteScopeLock Lock(Data, FIntBox::Infinite, FUNCTION_FNAME);
		FVoxelDataUtilities::ClearData<FVoxelMaterial>(Data);
	}
		
	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(FIntBox::Infinite);
	}
}

bool UVoxelBlueprintLibrary::HasValueData(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();

	FVoxelReadScopeLock Lock(Data, FIntBox::Infinite, FUNCTION_FNAME);
	return FVoxelDataUtilities::HasData<FVoxelValue>(World->GetData());
}

bool UVoxelBlueprintLibrary::HasMaterialData(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();
	
	auto& Data = World->GetData();
	
	FVoxelReadScopeLock Lock(Data, FIntBox::Infinite, FUNCTION_FNAME);
	return FVoxelDataUtilities::HasData<FVoxelMaterial>(Data);
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

bool UVoxelBlueprintLibrary::AreCollisionsEnabled(AVoxelWorld* World, FVector InPosition, int32& LOD, bool bConvertToVoxelSpace /*= true*/)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	LOD = 0;
	const FVoxelVector Position = FVoxelToolHelpers::GetRealPosition(World, InPosition, bConvertToVoxelSpace);

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

	World->UpdateDynamicRendererSettings();
	World->GetRenderer().ApplyNewMaterials();
}

void UVoxelBlueprintLibrary::RecreateRender(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	World->RecreateRender();
}

void UVoxelBlueprintLibrary::RecreateSpawners(AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	World->RecreateSpawners();
}

void UVoxelBlueprintLibrary::Recreate(AVoxelWorld* World, bool bSaveData)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVoxelUncompressedWorldSave Save;
	if (bSaveData)
	{
		UVoxelDataTools::GetSave(World, Save);
	}

	World->RecreateAll();

	if (bSaveData)
	{
		ensure(UVoxelDataTools::LoadFromSave(World, Save));
	}
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

	World->GetEventManager().BindEvent(
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

	World->GetEventManager().BindGenerationEvent(
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

	World->GetToolRenderingManager().EditTool(Ref.Id, [&](auto& Tool) { Tool.Material = FVoxelMaterialInterfaceManager::Get().CreateMaterial(Material); });
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

	LOD = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(LOD);

	Bounds = Bounds.MakeMultipleOfBigger(RENDER_CHUNK_SIZE << LOD);

	return Bounds.Extend(2 << LOD); // Account for the normals reads
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


FVoxelPaintMaterial UVoxelBlueprintLibrary::CreateFiveWayBlendPaintMaterial(
	int32 Channel,
	float TargetValue,
	bool bPaintR,
	bool bPaintG,
	bool bPaintB,
	bool bPaintA)
{
	if (!(0 <= Channel && Channel < 5))
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Channel needs to be between 0 and 4"));
		Channel = 0;
	}
	return FVoxelPaintMaterial::CreateFiveWayBlend(
		Channel,
		TargetValue,
		bPaintR,
		bPaintG,
		bPaintB,
		bPaintA);
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