// Copyright 2018 Phyronnaz

#pragma once

#include <shared_mutex>

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelDiff.h"
#include "VoxelSave.h"
#include "VoxelDirection.h"

class FValueOctree;
class FVoxelWorldGeneratorInstance;

/**
 * Class that handle voxel data
 */
class VOXEL_API FVoxelData
{
public:
	/**
	 * Constructor
	 * @param	LOD				LOD of this world; Size = DATA_CHUNK_SIZE * 2^LOD
	 * @param	WorldGenerator	Generator for this world
	 * @param	bMultiplayer	Is this for a multiplayer world
	 */
	FVoxelData(int LOD, TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator, bool bMultiplayer);
	~FVoxelData();

	// LOD of the octree
	const int LOD;
	const bool bMultiplayer;

	TSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator;

	// Size = DATA_CHUNK_SIZE * 2^LOD
	FORCEINLINE int32 Size() const;
	FORCEINLINE FIntBox GetBounds() const;

	
	/**
	 * Lock Box in read/write
	 * @param	Box		Box to lock
	 * @return	Locked octrees
	 */
	TArray<uint64> BeginSet(const FIntBox& Box);
	/**
	 * End the lock on LockedOctrees
	 * @param	LockedOctrees		Returned by BeginSet
	 */
	 void EndSet(TArray<uint64>& LockedOctrees);
	 
	/**
	 * Lock Box in read only
	 * @param	Box		Box to lock
	 * @return	Locked octrees
	 */
	 TArray<uint64> BeginGet(const FIntBox& Box);
	/**
	 * End the lock on LockedOctrees
	 * @param	LockedOctrees		Returned by BeginGet
	 */
	void EndGet(TArray<uint64>& LockedOctrees);
	
	/**
	 * Heuristic
	 */
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const;

	/**
	* Get values and materials
	*/
	void GetValuesAndMaterials(float Values[], FVoxelMaterial Materials[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const;

	/**
	 * Get the value at position. Requires BeginGet
	 */
	float GetValue(int X, int Y, int Z) const;
	float GetValue(const FIntVector& P) const;
	/**
	 * Get the material at position. Requires BeginGet
	 */
	FVoxelMaterial GetMaterial(int X, int Y, int Z) const;
	FVoxelMaterial GetMaterial(const FIntVector& P) const;
	/**
	 * Get the value and material at position. Requires BeginGet
	 */
	void GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const;
	void GetValueAndMaterial(const FIntVector& P, float& OutValue, FVoxelMaterial& OutMaterial) const;

	/**
	 * Requires BeginGet with an offset of 1 (final box has size 3)
	 */
	FVector GetGradient(int X, int Y, int Z);
	FVector GetGradient(const FIntVector& P);

	/**
	 * Set value at position. Requires BeginSet
	 * @param	X,Y,Z	Position in voxel space
	 * @param	Value 		Value to set
	 */
	void SetValue(int X, int Y, int Z, float Value);
	void SetValue(const FIntVector& P, float Value);
	/**
	 * Set value at position. Requires BeginSet
	 * @param	X,Y,Z		Position in voxel space
	 * @param	Value 		Value to set
	 * @param	LastOctree	The last octree used. Can improve access time. Can be nullptr
	 */
	void SetValue(int X, int Y, int Z, float Value, FValueOctree*& LastOctree);
	void SetValue(const FIntVector& P, float Value, FValueOctree*& LastOctree);

	/**
	 * Set material at position. Requires BeginSet
	 * @param	X,Y,Z		Position in voxel space
	 * @param	Material	Material to set
	 */
	void SetMaterial(int X, int Y, int Z, FVoxelMaterial Material);
	void SetMaterial(const FIntVector& P, FVoxelMaterial Material);
	/**
	 * Set material at position. Requires BeginSet
	 * @param	X,Y,Z		Position in voxel space
	 * @param	Material	Material to set
	 * @param	LastOctree	The last octree used. Can improve access time. Can be nullptr
	 */
	void SetMaterial(int X, int Y, int Z, FVoxelMaterial Material, FValueOctree*& LastOctree);
	void SetMaterial(const FIntVector& P, FVoxelMaterial Material, FValueOctree*& LastOctree);

	/**
	 * Set value and material at position. Requires BeginSet
	 * @param	X,Y,Z		Position in voxel space
	 * @param	Value		Value to set
	 * @param	Material	Material to set
	 * @param	LastOctree	The last octree used. Can improve access time. Can be nullptr
	 */
	void SetValueAndMaterial(int X, int Y, int Z, float Value, FVoxelMaterial Material, FValueOctree*& LastOctree);
	void SetValueAndMaterial(const FIntVector& P, float Value, FVoxelMaterial Material, FValueOctree*& LastOctree);
	

	/**
	 * Is Position in this world?
	 * @param	X,Y,Z	Position in voxel space
	 */
	FORCEINLINE bool IsInWorld(int X, int Y, int Z) const;
	FORCEINLINE bool IsInWorld(const FIntVector& P) const;

	/**
	 * Clamp (X, Y, Z) in the world
	 */
	FORCEINLINE void ClampToWorld(int& X, int& Y, int& Z) const;

	/**
	 * Get a save of this world
	 */
	void GetSave(FVoxelWorldSave& OutSave);

	/**
	 * Load this world from save
	 * @param	Save						Save to load from
	 * @param	bReset						Reset all chunks? Set to false if the world is unmodified
	 * @return	OutModifiedPositions		The modified positions
	 */
	void LoadFromSaveAndGetModifiedPositions(const FVoxelWorldSave& Save, TArray<FIntVector>& OutModifiedPositions, bool bReset);
	

	/**
	 * Set a new world generator
	 */
	void SetWorldGenerator(TSharedRef<FVoxelWorldGeneratorInstance> NewGenerator);

	/**
	 * P(bounds) = -1: discard
	 * P(bounds) = 0 : call with smaller bounds
	 * P(bounds) = 1 : keep
	 */	
	// TODO: ModifiedPositions
	template<typename Predicate>
	void DiscardValuesByPredicate(Predicate P)
	{
		DiscardValuesByPredicateF(std::function<int(const FIntBox&)>(P));
	}
	void DiscardValuesByPredicateF(const std::function<int(const FIntBox&)>& P);

private:
	FValueOctree* const MainOctree;
};