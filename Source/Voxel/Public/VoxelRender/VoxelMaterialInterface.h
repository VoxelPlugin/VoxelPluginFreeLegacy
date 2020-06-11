// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Containers/Queue.h"
#include "UObject/GCObject.h"
#include "UObject/WeakObjectPtr.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class FVoxelMaterialInterface;

class VOXEL_API FVoxelMaterialInterfaceManager : public FGCObject
{
public:
	static FVoxelMaterialInterfaceManager& Get()
	{
		if (!Singleton)
		{
			Singleton = new FVoxelMaterialInterfaceManager();
		}
		return *Singleton;
	}
	
private:
	static FVoxelMaterialInterfaceManager* Singleton;

public:
	FVoxelMaterialInterfaceManager();
	
	TVoxelSharedRef<FVoxelMaterialInterface> DefaultMaterial() const;
	TVoxelSharedRef<FVoxelMaterialInterface> CreateMaterial(UMaterialInterface* MaterialInterface);
	TVoxelSharedRef<FVoxelMaterialInterface> CreateMaterialInstance(UMaterialInterface* Parent);

protected:
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface
	
private:
	TVoxelSharedPtr<FVoxelMaterialInterface> DefaultMaterialPtr;

private:
	// Need to preallocate chunks for thread safety without locks
	static constexpr int32 ChunkSize = 1024;
	static constexpr int32 MaxNumChunks = 1024;
	
	struct FMaterialReference
	{
		int32 Index = -1;
		int32 ChunkIndex = -1;
	};
	struct FMaterialInfo
	{
		UMaterialInterface* Material = nullptr;
		bool bIsInstance = false;
		int32 ReferenceCount = 0;
	};
	struct FChunk
	{
		TArray<FMaterialInfo, TFixedAllocator<ChunkSize>> MaterialsInfos_AnyThread;
		TArray<int32> FreeIndices_GameThread;
	};
	TArray<TVoxelSharedPtr<FChunk>, TFixedAllocator<MaxNumChunks>> Chunks;

	TQueue<FMaterialReference, EQueueMode::Mpsc> ReferencesToDecrement;
	
	TMap<UMaterialInterface*, FMaterialReference> MaterialsMap;
	
	FMaterialInfo* GetMaterialInfo_AnyThread(FMaterialReference Reference);
	FMaterialReference AllocateMaterialInfo_GameThread();
	void DestroyMaterialInfo_GameThread(FMaterialInfo& MaterialInfo);

	void ProcessReferencesToDecrement_GameThread();
	
	UMaterialInterface* GetMaterial_AnyThread(FMaterialReference Reference);
	void RemoveReference_AnyThread(FMaterialReference Reference);

	TVoxelSharedRef<FVoxelMaterialInterface> CreateMaterialImpl(UMaterialInterface* MaterialInterface, bool bIsInstance);

private:
	TArray<UMaterialInstanceDynamic*> InstancePool;
	
	UMaterialInstanceDynamic* GetInstanceFromPool();
	void ReturnInstanceToPool(UMaterialInstanceDynamic* Instance);

public:
	void ClearInstancePool();
	
	friend class FVoxelMaterialInterface;
};

class VOXEL_API FVoxelMaterialInterface
{
public:
	~FVoxelMaterialInterface();

	// Will be null if the asset is force deleted
	UMaterialInterface* GetMaterial() const;

private:
	const FVoxelMaterialInterfaceManager::FMaterialReference Reference;

	explicit FVoxelMaterialInterface(FVoxelMaterialInterfaceManager::FMaterialReference Reference);

	friend class FVoxelMaterialInterfaceManager;
};