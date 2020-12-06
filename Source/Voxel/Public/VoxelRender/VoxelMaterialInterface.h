// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelGCObject.h"
#include "Containers/Queue.h"


class UMaterialInterface;
class UMaterialInstanceDynamic;
class FVoxelMaterialInterface;

// This code is a bit complex to handle material reinstancing when they are recompiled
// Reinstancing reconstructs the object in-place, invaliding any weak pointer to it, but keeping the raw pointer the same
class VOXEL_API FVoxelMaterialInterfaceManager : public FVoxelGCObject
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
	// Will handle Parent being an instance
	TVoxelSharedRef<FVoxelMaterialInterface> CreateMaterialInstance(UMaterialInterface* Parent);

protected:
	//~ Begin FVoxelGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return "FVoxelMaterialInterfaceManager"; }
	//~ End FVoxelGCObject Interface
	
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
	// If true, it's a material instance the plugin created & we can set parameters on it
	bool IsMaterialInstance() const { return bIsInstance; }
	
private:
	const FVoxelMaterialInterfaceManager::FMaterialReference Reference;
	const bool bIsInstance;
	
	FVoxelMaterialInterface(FVoxelMaterialInterfaceManager::FMaterialReference Reference, bool bIsInstance);

	friend class FVoxelMaterialInterfaceManager;
};