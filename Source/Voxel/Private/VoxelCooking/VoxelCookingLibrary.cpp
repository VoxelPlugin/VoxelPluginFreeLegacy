// Copyright 2020 Phyronnaz

#include "VoxelCooking/VoxelCookingLibrary.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelDefaultPool.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelWorldRootComponent.h"

#include "HAL/Event.h"

#include "IPhysXCooking.h"
#include "IPhysXCookingModule.h"

#include "PhysXIncludes.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Interface_CollisionDataProviderCore.h"
#include "Engine/Private/PhysicsEngine/PhysXSupport.h" // For FPhysXInputStream

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
struct FVoxelCookingTaskData
{
	IVoxelRenderer& Renderer;
	FVoxelCookedDataImpl& CookedData;
	IPhysXCooking& PhysXCooking;
	
	const int32 NumChunksToBuild;
	const FVoxelCookingSettings CookingSettings;
	
	FEvent* const DoneEvent;
	FThreadSafeCounter NumChunksBuilt;

	FThreadSafeCounter64 MeshingTime;
	FThreadSafeCounter64 CollisionTime;

	FVoxelCookingTaskData(IVoxelRenderer& Renderer, FVoxelCookedDataImpl& CookedData, int32 NumChunksToBuild, const FVoxelCookingSettings& CookingSettings)
		: Renderer(Renderer)
		, CookedData(CookedData)
		, PhysXCooking(*GetPhysXCookingModule()->GetPhysXCooking())
		, NumChunksToBuild(NumChunksToBuild)
		, CookingSettings(CookingSettings)
		, DoneEvent(FPlatformProcess::GetSynchEventFromPool())
	{
		CookedData.SetNumChunks(NumChunksToBuild);
	}
	~FVoxelCookingTaskData()
	{
		check(NumChunksBuilt.GetValue() == NumChunksToBuild);
		FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
	}

	void ChunkDone(TArray<uint8>&& Data)
	{
		const int32 NumBuilt = NumChunksBuilt.Increment();
		if (CookingSettings.bLogProgress)
		{
			LOG_VOXEL(Log, TEXT("VOXEL COOKING: %d/%d"), NumBuilt, NumChunksToBuild);
		}

		CookedData.GetChunk(NumBuilt - 1).Data = MoveTemp(Data);

		if (NumBuilt == NumChunksToBuild)
		{
			DoneEvent->Trigger();
		}
	}
};

class FVoxelCookingTask : public IVoxelQueuedWork
{
public:
	const FIntVector ChunkPosition;
	FVoxelCookingTaskData& TaskData;

	FVoxelCookingTask(const FIntVector& ChunkPosition, FVoxelCookingTaskData& TaskData)
		: IVoxelQueuedWork(STATIC_FNAME("Cooking Task"), 0)
		, ChunkPosition(ChunkPosition)
		, TaskData(TaskData)
	{
	}

	virtual void DoThreadedWork() override
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		TArray<uint32> Indices;
		TArray<FVector> Vertices;
		{
			VOXEL_ASYNC_SCOPE_COUNTER("Creating geometry");
			
			const uint64 StartTime = FPlatformTime::Cycles64();
			TaskData.Renderer.CreateGeometry_AnyThread(0, ChunkPosition, Indices, Vertices);
			const uint64 EndTime = FPlatformTime::Cycles64();
			TaskData.MeshingTime.Add(EndTime - StartTime);
		}
		
		TArray<uint8> Buffer;
		if (Indices.Num() > 0)
		{
			static const FName PhysXFormat = FPlatformProperties::GetPhysicsFormat();
			
			EPhysXMeshCookFlags CookFlags = EPhysXMeshCookFlags::Default;
			if (!TaskData.CookingSettings.bCleanCollisionMesh) 
			{
				CookFlags |= EPhysXMeshCookFlags::DeformableMesh;
			}
			if (TaskData.CookingSettings.bFastCollisionCook)
			{
				CookFlags |= EPhysXMeshCookFlags::FastCook;
			}

			check(Indices.Num() % 3 == 0);
			
			TArray<FTriIndices> TriIndices;
			TriIndices.SetNumUninitialized(Indices.Num() / 3);
			FMemory::Memcpy(TriIndices.GetData(), Indices.GetData(), Indices.Num() * sizeof(int32));

			// Put the chunk in global space, as tri meshes don't support individual transforms
			for (auto& Vertex : Vertices)
			{
				Vertex = (Vertex + FVector(ChunkPosition)) * TaskData.CookingSettings.VoxelSize;
			}
			
			constexpr bool bFlipNormals = true; // Always true due to the order of the vertices (clock wise vs not)

			bool bResult;
			{
				VOXEL_ASYNC_SCOPE_COUNTER("Cooking collision");
			
				const uint64 StartTime = FPlatformTime::Cycles64();
				bResult = TaskData.PhysXCooking.CookTriMesh(PhysXFormat, CookFlags, Vertices, TriIndices, {}, bFlipNormals, Buffer);
				const uint64 EndTime = FPlatformTime::Cycles64();
				TaskData.CollisionTime.Add(EndTime - StartTime);
			}

			if (!bResult)
			{
				Buffer.Reset();
				LOG_VOXEL(Warning, TEXT("VOXEL COOKING: Failed to cook chunk at %s with %d indices"), *ChunkPosition.ToString(), Indices.Num());
			}
		}

		TaskData.ChunkDone(MoveTemp(Buffer));
		delete this;
	}
	virtual void Abandon() override
	{
		check(false);
	}
	virtual uint32 GetPriority() const override
	{
		return 0;
	}
};
#endif

FVoxelCookedData UVoxelCookingLibrary::CookVoxelDataImpl(const FVoxelCookingSettings& Settings, const FVoxelUncompressedWorldSaveImpl* Save)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	if (!Settings.Generator.IsValid())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid generator"));
		return {};
	}

	AVoxelWorld* VoxelWorld = NewObject<AVoxelWorld>();
	VoxelWorld->RenderOctreeDepth = Settings.RenderOctreeDepth;
	VoxelWorld->VoxelSize = Settings.VoxelSize;
	VoxelWorld->RenderType = Settings.RenderType;
	VoxelWorld->Generator = Settings.Generator;

	const auto Pool = FVoxelDefaultPool::Create(Settings.ThreadCount, true, {}, {});
	const auto Data = FVoxelData::Create(FVoxelDataSettings(VoxelWorld, EVoxelPlayType::Game));
	const auto DebugManager = FVoxelDebugManager::Create(FVoxelDebugManagerSettings(VoxelWorld, EVoxelPlayType::Game, Pool, Data));

	if (Save)
	{
		Data->LoadFromSave(*Save, {});
	}

	const auto Renderer = FVoxelDefaultRenderer::Create(FVoxelRendererSettings(
		VoxelWorld,
		EVoxelPlayType::Game,
		nullptr,
		Data,
		Pool,
		nullptr,
		DebugManager,
		false));

	const FIntVector Min = Data->WorldBounds.Min;
	const FIntVector Max = Data->WorldBounds.Max;
	
	const FIntVector NumChunksPerAxis = (Max - Min) / RENDER_CHUNK_SIZE;
	const int64 TotalNumChunks = int64(NumChunksPerAxis.X) * int64(NumChunksPerAxis.Y) * int64(NumChunksPerAxis.Z);

	if (TotalNumChunks > MAX_int32)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Depth too high"));
		return {};
	}

	const double StartTime = FPlatformTime::Seconds();
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Starting cooking with %lld tasks"), TotalNumChunks);

	FVoxelCookedData CookedData;
	
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	FVoxelCookingTaskData TaskData(*Renderer, CookedData.Mutable(), TotalNumChunks, Settings);

	for (int32 X = Min.X; X < Max.X; X += RENDER_CHUNK_SIZE)
	{
		for (int32 Y = Min.Y; Y < Max.Y; Y += RENDER_CHUNK_SIZE)
		{
			for (int32 Z = Min.Z; Z < Max.Z; Z += RENDER_CHUNK_SIZE)
			{
				const FIntVector ChunkPosition = FIntVector(X, Y, Z);
				auto* Task = new FVoxelCookingTask(ChunkPosition, TaskData);
				Pool->QueueTask({}, Task);
			}
		}
	}

	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Waiting for tasks"));
	TaskData.DoneEvent->Wait();
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Done"));

	const double EndTime = FPlatformTime::Seconds();

	TaskData.CookedData.RemoveEmptyChunks();
	TaskData.CookedData.UpdateAllocatedSize();

	const double GameThreadTime = EndTime - StartTime;
	const double MeshingTime = TaskData.MeshingTime.GetValue() * FPlatformTime::GetSecondsPerCycle64();
	const double CollisionTime = TaskData.CollisionTime.GetValue() * FPlatformTime::GetSecondsPerCycle64();
	const double OverheadTime = GameThreadTime - (MeshingTime + CollisionTime) / Settings.ThreadCount;
	
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Game Thread time: %fs"), GameThreadTime);
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Async Thread meshing time: %fs"), MeshingTime);
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Async Thread collision time: %fs"), CollisionTime);
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Overhead time: %fs (%f%%)"), OverheadTime, 100. * OverheadTime / GameThreadTime);
#else
	ensure(false);
#endif
	
	Renderer->Destroy();
	DebugManager->Destroy();
	
	return CookedData;
}

FVoxelCookingSettings UVoxelCookingLibrary::MakeVoxelCookingSettingsFromVoxelWorld(AVoxelWorld* World, int32 ThreadCount)
{
	if (!World)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid Voxel World"));
		return {};
	}

	FVoxelCookingSettings  Settings;
	Settings.ThreadCount = ThreadCount;
	Settings.RenderOctreeDepth = World->RenderOctreeDepth;
	Settings.VoxelSize = World->VoxelSize;
	Settings.RenderType = World->RenderType;
	Settings.Generator = World->Generator;

	return Settings;
}

void UVoxelCookingLibrary::LoadCookedVoxelData(FVoxelCookedData CookedData, AVoxelWorld* World)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!World)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid voxel world!"));
		return;
	}
	if (World->IsCreated())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Voxel world is already created!"));
		return;
	}

	UVoxelWorldRootComponent& WorldRoot = World->GetWorldRoot();

	const auto& Chunks = CookedData.Const().GetChunks();

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	TArray<physx::PxTriangleMesh*> TriMeshes;
	TriMeshes.Reserve(Chunks.Num());
	
	for (auto& Chunk : Chunks)
	{
		if (ensure(Chunk.Data.Num() > 0))
		{
			FPhysXInputStream Buffer(Chunk.Data.GetData(), Chunk.Data.Num());
			physx::PxTriangleMesh* CookedMesh = GPhysXSDK->createTriangleMesh(Buffer);
			TriMeshes.Add(CookedMesh);
		}
	}

	WorldRoot.SetCookedTriMeshes(TriMeshes);
#else
	ensure(false);
#endif

	World->ApplyCollisionSettingsToRoot();
	
	LOG_VOXEL(Log, TEXT("VOXEL COOKING: Loaded cooked data"));
}