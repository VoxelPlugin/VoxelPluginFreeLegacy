#pragma once

#include "CoreMinimal.h"
#include <list>
#include <forward_list>
#include "VoxelMaterial.h"
#include "VoxelSave.generated.h"


USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelChunkSave
{
	GENERATED_BODY()

public:
	UPROPERTY()
		uint64 Id;

	UPROPERTY(VisibleAnywhere)
		TArray<float> Values;

	UPROPERTY(VisibleAnywhere)
		TArray<FVoxelMaterial> Materials;

	FVoxelChunkSave();

	FVoxelChunkSave(uint64 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FVoxelMaterial, TFixedAllocator<16 * 16 * 16>>& Materials);
};

USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelWorldSave
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
		int Depth;

	UPROPERTY(VisibleAnywhere)
		TArray<FVoxelChunkSave> Chunks;


	FVoxelWorldSave();

	FVoxelWorldSave(int Depth, std::list<TSharedRef<FVoxelChunkSave>> ChunksList);

	std::list<FVoxelChunkSave> GetChunksList();
};
USTRUCT()
struct FVoxelValueDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint64 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		float Value;

	FVoxelValueDiff();

	FVoxelValueDiff(uint64 Id, int Index, float Value);
};

USTRUCT()
struct FVoxelMaterialDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint64 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial Material;

	FVoxelMaterialDiff();

	FVoxelMaterialDiff(uint64 Id, int Index, FVoxelMaterial Material);
};

struct VoxelValueDiffArray
{
	std::forward_list<uint64> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<float> Values;
	int Size = 0;

	void Add(uint64 Id, int Index, float Value);

	void AddPackets(std::forward_list<TArray<FVoxelValueDiff>>& List, const int MaxSize = 2048);
};

struct VoxelMaterialDiffArray
{
	std::forward_list<uint64> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<FVoxelMaterial> Materials;
	int Size = 0;

	void Add(uint64 Id, int Index, FVoxelMaterial Material);

	void AddPackets(std::forward_list<TArray<FVoxelMaterialDiff>>& List, const int MaxSize = 2048);
};