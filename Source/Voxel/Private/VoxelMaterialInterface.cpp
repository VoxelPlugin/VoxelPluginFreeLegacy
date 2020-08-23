// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelMaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

FAutoConsoleCommandWithWorldAndArgs ClearInstancePoolCmd(
	TEXT("voxel.renderer.ClearMaterialInstancePool"),
	TEXT("Clear material instance pool"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const auto&, auto*) { FVoxelMaterialInterfaceManager::Get().ClearInstancePool(); }));

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Material Instances Pool"), STAT_VoxelMaterialInstancesPool, STATGROUP_VoxelCounters);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Material Instances Used"), STAT_VoxelMaterialInstancesUsed, STATGROUP_VoxelCounters);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialInterfaceManager* FVoxelMaterialInterfaceManager::Singleton = nullptr;

FVoxelMaterialInterfaceManager::FVoxelMaterialInterfaceManager()
{
	UMaterial* Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	check(Material);
	DefaultMaterialPtr = CreateMaterial(Material);
}

TVoxelSharedRef<FVoxelMaterialInterface> FVoxelMaterialInterfaceManager::DefaultMaterial() const
{
	check(DefaultMaterialPtr.IsValid());
	return DefaultMaterialPtr.ToSharedRef();
}

TVoxelSharedRef<FVoxelMaterialInterface> FVoxelMaterialInterfaceManager::CreateMaterial(UMaterialInterface* MaterialInterface)
{
	return CreateMaterialImpl(MaterialInterface, false);
}

// To access SetParentInternal
class FMaterialInstanceResource
{
public:
	static void SetParent(UMaterialInstanceDynamic& Instance, UMaterialInterface* Parent)
	{
		VOXEL_FUNCTION_COUNTER();
		Instance.SetParentInternal(Parent, false);
	}
};

TVoxelSharedRef<FVoxelMaterialInterface> FVoxelMaterialInterfaceManager::CreateMaterialInstance(UMaterialInterface* Parent)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!IsValid(Parent))
	{
		return DefaultMaterial();
	}

	UMaterialInstanceDynamic* Instance = GetInstanceFromPool();
	check(Instance);
	
	FMaterialInstanceResource::SetParent(*Instance, Parent);

	return CreateMaterialImpl(Instance, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelMaterialInterfaceManager::AddReferencedObjects(FReferenceCollector& Collector)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	Collector.AddReferencedObjects(InstancePool);

	for (auto& Chunk : Chunks)
	{
		check(Chunk.IsValid());
		for (auto& MaterialInfo : Chunk->MaterialsInfos_AnyThread)
		{
			Collector.AddReferencedObject(MaterialInfo.Material);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialInterfaceManager::FMaterialInfo* FVoxelMaterialInterfaceManager::GetMaterialInfo_AnyThread(FMaterialReference Reference)
{
	if (!Chunks.IsValidIndex(Reference.ChunkIndex))
	{
		return nullptr;
	}
	
	auto& Chunk = Chunks[Reference.ChunkIndex];
	if (!Chunk.IsValid())
	{
		return nullptr;
	}

	if (!Chunk->MaterialsInfos_AnyThread.IsValidIndex(Reference.Index))
	{
		return nullptr;
	}

	return &Chunk->MaterialsInfos_AnyThread[Reference.Index];
}

FVoxelMaterialInterfaceManager::FMaterialReference FVoxelMaterialInterfaceManager::AllocateMaterialInfo_GameThread()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	// Free pending references
	ProcessReferencesToDecrement_GameThread();

	for (int32 ChunkIndex = 0; ChunkIndex < Chunks.Num(); ChunkIndex++)
	{
		auto& Chunk = Chunks[ChunkIndex];
		check(Chunk.IsValid());
		if (Chunk->FreeIndices_GameThread.Num() > 0)
		{
			const int32 Index = Chunk->FreeIndices_GameThread.Pop();
			return FMaterialReference{ Index, ChunkIndex };
		}
		if (Chunk->MaterialsInfos_AnyThread.Num() < ChunkSize)
		{
			const int32 Index = Chunk->MaterialsInfos_AnyThread.Emplace();
			return FMaterialReference{ Index, ChunkIndex };
		}
	}

	if (Chunks.Num() == MaxNumChunks)
	{
		LOG_VOXEL(Fatal, TEXT("FVoxelMaterialInterfaceManager: ran out of material info slots. Total slots: %d"), ChunkSize * MaxNumChunks);
	}

	const int32 ChunkIndex = Chunks.Add(MakeVoxelShared<FChunk>());
	const int32 Index = Chunks[ChunkIndex]->MaterialsInfos_AnyThread.Emplace();
	return FMaterialReference{ Index, ChunkIndex };
}

void FVoxelMaterialInterfaceManager::DestroyMaterialInfo_GameThread(FMaterialInfo& MaterialInfo)
{
	if (MaterialInfo.bIsInstance)
	{
		ensure(!MaterialInfo.Material || MaterialInfo.Material->IsA<UMaterialInstanceDynamic>());
		ReturnInstanceToPool(Cast<UMaterialInstanceDynamic>(MaterialInfo.Material));
	}
}

void FVoxelMaterialInterfaceManager::ProcessReferencesToDecrement_GameThread()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	FMaterialReference Reference;
	while (ReferencesToDecrement.Dequeue(Reference))
	{
		FMaterialInfo* MaterialInfo = GetMaterialInfo_AnyThread(Reference);
		if (ensure(MaterialInfo))
		{
			MaterialInfo->ReferenceCount--;
			ensure(MaterialInfo->ReferenceCount >= 0);
			if (MaterialInfo->ReferenceCount == 0)
			{
				Chunks[Reference.ChunkIndex]->FreeIndices_GameThread.Add(Reference.Index);
				DestroyMaterialInfo_GameThread(*MaterialInfo);
				*MaterialInfo = {};
			}
		}
	}
}

UMaterialInterface* FVoxelMaterialInterfaceManager::GetMaterial_AnyThread(FMaterialReference Reference)
{
	auto* MaterialInfo = GetMaterialInfo_AnyThread(Reference);
	return ensure(MaterialInfo) ? MaterialInfo->Material : nullptr;
}

void FVoxelMaterialInterfaceManager::RemoveReference_AnyThread(FMaterialReference Reference)
{
	ReferencesToDecrement.Enqueue(Reference);
}

TVoxelSharedRef<FVoxelMaterialInterface> FVoxelMaterialInterfaceManager::CreateMaterialImpl(UMaterialInterface* MaterialInterface, bool bIsInstance)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	if (!IsValid(MaterialInterface))
	{
		return DefaultMaterial();
	}

	FMaterialReference& Reference = MaterialsMap.FindOrAdd(MaterialInterface);
	
	FMaterialInfo* MaterialInfo = GetMaterialInfo_AnyThread(Reference);
	if (!MaterialInfo || MaterialInfo->Material != MaterialInterface)
	{
		Reference = AllocateMaterialInfo_GameThread();
		MaterialInfo = GetMaterialInfo_AnyThread(Reference);
		check(MaterialInfo && !MaterialInfo->Material && MaterialInfo->ReferenceCount == 0);
		MaterialInfo->Material = MaterialInterface;
		MaterialInfo->bIsInstance = bIsInstance;
	}
	check(MaterialInfo && MaterialInfo->Material == MaterialInterface && MaterialInfo->bIsInstance == bIsInstance);

	MaterialInfo->ReferenceCount++;
	
	return TVoxelSharedRef<FVoxelMaterialInterface>(new FVoxelMaterialInterface(Reference));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInstanceDynamic* FVoxelMaterialInterfaceManager::GetInstanceFromPool()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	INC_DWORD_STAT(STAT_VoxelMaterialInstancesUsed);

	UMaterialInstanceDynamic* Instance = nullptr;

	while (!Instance && InstancePool.Num() > 0)
	{
		Instance = InstancePool.Pop(false);
		DEC_DWORD_STAT(STAT_VoxelMaterialInstancesPool);
	}

	if (!Instance)
	{
		Instance = NewObject<UMaterialInstanceDynamic>();
	}

	check(Instance);
	return Instance;
}

void FVoxelMaterialInterfaceManager::ReturnInstanceToPool(UMaterialInstanceDynamic* Instance)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	DEC_DWORD_STAT(STAT_VoxelMaterialInstancesUsed);

	if (Instance)
	{
		Instance->ClearParameterValues();

		InstancePool.Add(Instance);
		INC_DWORD_STAT(STAT_VoxelMaterialInstancesPool);
	}
}

void FVoxelMaterialInterfaceManager::ClearInstancePool()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	LOG_VOXEL(Log, TEXT("Clear Material Instance Pool: %d instances removed"), InstancePool.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelMaterialInstancesPool, InstancePool.Num());
	InstancePool.Reset();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelMaterialInterface::FVoxelMaterialInterface(FVoxelMaterialInterfaceManager::FMaterialReference Reference)
	: Reference(Reference)
{
}

FVoxelMaterialInterface::~FVoxelMaterialInterface()
{
	FVoxelMaterialInterfaceManager::Get().RemoveReference_AnyThread(Reference);
}

UMaterialInterface* FVoxelMaterialInterface::GetMaterial() const
{
	VOXEL_SLOW_FUNCTION_COUNTER();
	return FVoxelMaterialInterfaceManager::Get().GetMaterial_AnyThread(Reference);
}