#pragma once
#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

class VoxelData;
class UVoxelWorldGenerator;

class ValueOctree
{
public:
	ValueOctree(FIntVector Position, int Depth, UVoxelWorldGenerator* WorldGenerator);

	// Center of the octree
	const FIntVector Position;
	const int Depth;



	int Width();

	bool IsLeaf();

	bool IsDirty();

	signed char GetValue(FIntVector GlobalPosition);
	FColor GetColor(FIntVector GlobalPosition);

	void SetValue(FIntVector GlobalPosition, signed char Value);
	void SetColor(FIntVector GlobalPosition, FColor Color);

	bool IsInChunk(FIntVector GlobalPosition);

	FIntVector LocalToGlobal(FIntVector LocalPosition);
	FIntVector GlobalToLocal(FIntVector GlobalPosition);

	void AddChunksToArray(TArray<FVoxelChunkSaveStruct> SaveArray);
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

private:
	/*
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x

	*/
	TArray<ValueOctree*, TFixedAllocator<8>> Childs;

	UVoxelWorldGenerator* WorldGenerator;

	TArray<signed char> Values;
	TArray<FColor> Colors;

	bool bIsDirty;
	bool bIsLeaf;

	void CreateChilds();

	ValueOctree* GetChild(FIntVector GlobalPosition);
};