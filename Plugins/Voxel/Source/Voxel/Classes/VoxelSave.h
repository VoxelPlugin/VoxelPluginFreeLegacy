#pragma once

#include "CoreMinimal.h"
#include <list>
#include <forward_list>
#include "VoxelSave.generated.h"


USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelChunkSave
{
	GENERATED_BODY()

public:
	UPROPERTY()
		uint32 Id;

	UPROPERTY(VisibleAnywhere)
		TArray<float> Values;

	UPROPERTY(VisibleAnywhere)
		TArray<FColor> Colors;

	FVoxelChunkSave();

	FVoxelChunkSave(uint32 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FColor, TFixedAllocator<16 * 16 * 16>>& Colors);
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

	FVoxelWorldSave(int Depth, std::list<FVoxelChunkSave> ChunksList);

	std::list<FVoxelChunkSave> GetChunksList();
};
USTRUCT()
struct FVoxelValueDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		float Value;

	FVoxelValueDiff();

	FVoxelValueDiff(uint32 Id, int Index, float Value);
};

USTRUCT()
struct FVoxelColorDiff
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint32 Id;

	UPROPERTY(EditAnywhere)
		int Index;

	UPROPERTY(EditAnywhere)
		FColor Color;

	FVoxelColorDiff();

	FVoxelColorDiff(uint32 Id, int Index, FColor Color);
};

struct VoxelValueDiffArray
{
	std::forward_list<uint32> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<float> Values;
	int Size = 0;

	void Add(uint32 Id, int Index, float Value);

	void AddPackets(std::forward_list<TArray<FVoxelValueDiff>>& List, const int MaxSize = 2048);
};

struct VoxelColorDiffArray
{
	std::forward_list<uint32> Ids;
	std::forward_list<int> Indexes;
	std::forward_list<FColor> Colors;
	int Size = 0;

	void Add(uint32 Id, int Index, FColor Color);

	void AddPackets(std::forward_list<TArray<FVoxelColorDiff>>& List, const int MaxSize = 2048);
};