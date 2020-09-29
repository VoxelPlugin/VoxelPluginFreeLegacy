// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInit.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorPicker.h"
#include "VoxelWorldGeneratorCache.generated.h"

class UVoxelWorldGenerator;
class FVoxelWorldGeneratorInstance;

USTRUCT(BlueprintType)
struct FVoxelWorldGeneratorCacheKey
{
	GENERATED_BODY()

	// Object used to avoid collision between identical generator classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	UObject* Object = nullptr;

	// Name used to avoid collision between identical generator classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	FName Name;

	// If class: will instantiate the class
	// If object: TODO
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key")
	FVoxelWorldGeneratorPicker Picker;
};

inline uint32 GetTypeHash(const FVoxelWorldGeneratorCacheKey& Key)
{
	return HashCombine(HashCombine(GetTypeHash(Key.Object), GetTypeHash(Key.Name)), GetTypeHash(Key.Picker));
}
inline bool operator==(const FVoxelWorldGeneratorCacheKey& A, const FVoxelWorldGeneratorCacheKey& B)
{
	return
		A.Object == B.Object &&
		A.Name == B.Name &&
		A.Picker == B.Picker;
}

UCLASS(BlueprintType)
class VOXEL_API UVoxelWorldGeneratorCache : public UObject
{
public:
	GENERATED_BODY()

public:
	// This is required for DataItemActors to reuse world generators,
	// which allows for smaller update when moving them
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	UVoxelWorldGenerator* FindOrCreateWorldGenerator(FVoxelWorldGeneratorCacheKey Key) const;
	
	// Will create & init a new instance if needed, or reuse a cached one
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> CreateWorldGeneratorInstance(UVoxelWorldGenerator& Generator) const;

public:
	void SetWorldGeneratorInit(const FVoxelWorldGeneratorInit& NewInit)
	{
		WorldGeneratorInit = NewInit;
	}
	void ClearCache()
	{
		ObjectCache.Reset();
		InstanceCache.Reset();
	}

private:
	UPROPERTY()
	FVoxelWorldGeneratorInit WorldGeneratorInit;
	
	UPROPERTY()
	mutable TMap<FVoxelWorldGeneratorCacheKey, UVoxelWorldGenerator*> ObjectCache;
	
	mutable TMap<TWeakObjectPtr<UVoxelWorldGenerator>, TVoxelSharedPtr<FVoxelWorldGeneratorInstance>> InstanceCache;
};