// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelDiff.h"
#include "VoxelSave.h"
#include "VoxelValue.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelMath.h"

class FVoxelWorldGeneratorInstance;
class FVoxelPlaceableItem;

/**
 * Class that handle voxel data
 */
class VOXEL_API FVoxelData
{
public:
	class FVoxelScopeSetLock
	{
	public:
		FVoxelScopeSetLock(FVoxelData* Data, const FIntBox& Bounds, const FString& Name)
			: Data(Data)
		{
			Data->BeginSet(Bounds, LockedOctrees, Name);
		}
		template<ESPMode Mode>
		FVoxelScopeSetLock(const TSharedPtr<FVoxelData, Mode>& Data, const FIntBox& Bounds, const FString& Name)
			: FVoxelScopeSetLock(Data.Get(), Bounds, Name)
		{
		}

		~FVoxelScopeSetLock()
		{
			Data->EndSet(LockedOctrees);
		}

	private:
		TArray<FVoxelId> LockedOctrees;
		FVoxelData* const Data;
	};

	class FVoxelScopeGetLock
	{
	public:
		FVoxelScopeGetLock(FVoxelData* Data, const FIntBox& Bounds, const FString& Name)
			: Data(Data)
		{
			Data->BeginGet(Bounds, LockedOctrees, Name);
		}
		template<ESPMode Mode>
		FVoxelScopeGetLock(const TSharedPtr<FVoxelData, Mode>& Data, const FIntBox& Bounds, const FString& Name)
			: FVoxelScopeGetLock(Data.Get(), Bounds, Name)
		{
		}

		~FVoxelScopeGetLock()
		{
			Data->EndGet(LockedOctrees);
		}

	private:
		TArray<FVoxelId> LockedOctrees;
		FVoxelData* const Data;
	};

public:
	/**
	 * Won't work if a setter is called in the map bounds after the map creation
	 */
	struct MapAccelerator
	{
	private:
		struct FLastMapQuery
		{
			FIntVector Position;
			const FVoxelDataCell** Chunk = nullptr;
			bool bValid = false;
		};

		FVoxelMap Map;
		FLastMapQuery LastQuery;
		const FVoxelData& Data;

	public:
		MapAccelerator(const FIntBox& Bounds, const FVoxelData* InData)
			: Data(*InData)
		{
			Data.GetOctree()->GetMap(Bounds, Map);
			Map.Compact();
		}

		inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int QueryLOD)
		{
			FIntVector Position = FVoxelIntVector::DivideFloor(FIntVector(X, Y, Z), VOXEL_CELL_SIZE);
			if (!LastQuery.bValid || LastQuery.Position != Position)
			{
				LastQuery.Position = Position;
				LastQuery.Chunk = Map.Find(Position);
				LastQuery.bValid = true;
			}
			if (LastQuery.Chunk)
			{
				int Index = FVoxelDataCellUtilities::IndexFromCoordinates(X - Position.X * VOXEL_CELL_SIZE, Y - Position.Y * VOXEL_CELL_SIZE, Z - Position.Z * VOXEL_CELL_SIZE);
				auto* Chunk = *LastQuery.Chunk;
				if (OutValue)
				{
					*OutValue = Chunk->GetBuffer<FVoxelValue>()[Index];
				}
				if (OutMaterial)
				{
					*OutMaterial = Chunk->GetBuffer<FVoxelMaterial>()[Index];
				}
			}
			else
			{
				Data.GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
			}
		}
		inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int QueryLOD) { GetValueAndMaterial(X, Y, Z, &OutValue, &OutMaterial, QueryLOD); }

		inline FVoxelValue GetValue(int X, int Y, int Z, int QueryLOD)
		{
			FVoxelValue Value;
			GetValueAndMaterial(X, Y, Z, &Value, nullptr, QueryLOD);
			return Value;
		}
		inline FVoxelValue GetValue(const FIntVector& P, int QueryLOD) { return GetValue(P.X, P.Y, P.Z, QueryLOD); }

		inline FVoxelMaterial GetMaterial(int X, int Y, int Z, int QueryLOD)
		{
			FVoxelMaterial Material;
			GetValueAndMaterial(X, Y, Z, nullptr, &Material, QueryLOD);
			return Material;
		}
		inline FVoxelMaterial GetMaterial(const FIntVector& P, int QueryLOD) { return GetMaterial(P.X, P.Y, P.Z, QueryLOD); }
	};

public:
	/**
	 * Works with setters
	 */
	struct LastOctreeAccelerator
	{
	private:
		FVoxelDataOctree* LastOctree = nullptr;
		const FVoxelData& Data;

	public:
		LastOctreeAccelerator(const FVoxelData* Data)
			: Data(*Data)
		{
		}

		inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int QueryLOD)
		{
			if (UNLIKELY(!Data.IsInWorld(X, Y, Z)))
			{
				Data.WorldGenerator->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD, FVoxelPlaceableItemHolder());
			}
			else
			{
				if (UNLIKELY(!LastOctree || !LastOctree->IsInOctree(X, Y, Z) || !LastOctree->IsLeaf()))
				{
					LastOctree = Data.GetOctree()->GetLeaf(X, Y, Z);
				}
				LastOctree->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
			}
		}
		template<typename T>
		void SetValueOrMaterial(int X, int Y, int Z, const T& Value)
		{
			if (UNLIKELY(!Data.IsInWorld(X, Y, Z)))
			{
				return;
			}
			if (UNLIKELY(!LastOctree || !LastOctree->IsInOctree(X, Y, Z) || !LastOctree->IsLeaf()))
			{
				LastOctree = Data.GetOctree()->GetLeaf(X, Y, Z);
			}
			LastOctree->SetValueOrMaterial<T>(X, Y, Z, Value);
		}

		inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int QueryLOD) { GetValueAndMaterial(X, Y, Z, &OutValue, &OutMaterial, QueryLOD); }
		inline void GetValueAndMaterial(const FIntVector& P, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int QueryLOD) { GetValueAndMaterial(P.X, P.Y, P.Z, &OutValue, &OutMaterial, QueryLOD); }

		inline FVoxelValue GetValue(int X, int Y, int Z, int QueryLOD) { FVoxelValue Value; GetValueAndMaterial(X, Y, Z, &Value, nullptr, QueryLOD); return Value; }
		inline FVoxelValue GetValue(const FIntVector& P, int QueryLOD) { return GetValue(P.X, P.Y, P.Z, QueryLOD); }

		inline FVoxelMaterial GetMaterial(int X, int Y, int Z, int QueryLOD) { FVoxelMaterial Material; GetValueAndMaterial(X, Y, Z, nullptr, &Material, QueryLOD); return Material; }
		inline FVoxelMaterial GetMaterial(const FIntVector& P, int QueryLOD) { return GetMaterial(P.X, P.Y, P.Z, QueryLOD); }

		inline void SetValue(int X, int Y, int Z, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(X, Y, Z, Value); }
		inline void SetValue(const FIntVector& P, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(P.X, P.Y, P.Z, Value); }

		inline void SetMaterial(int X, int Y, int Z, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(X, Y, Z, Material); }
		inline void SetMaterial(const FIntVector& P, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(P.X, P.Y, P.Z, Material); }
	};

public:
	/**
	 * Constructor
	 * @param	Depth			Depth of this world; Size = VOXEL_CELL_SIZE * 2^Depth
	 * @param	WorldGenerator	Generator for this world
	 * @param	bMultiplayer	Is this for a multiplayer world
	 * @param	bEnableUndoRedo	Enable Undo/Redo?
	 */
	FVoxelData(int Depth, const FIntBox& WorldBounds, TSharedRef<const FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> WorldGenerator, bool bEnableMultiplayer, bool bEnableUndoRedo)
		: Depth(Depth)
		, WorldBounds(WorldBounds)
		, bEnableMultiplayer(bEnableMultiplayer)
		, bEnableUndoRedo(bEnableUndoRedo)
		, WorldGenerator(WorldGenerator)
		, Octree(MakeUnique<FVoxelDataOctree>(&WorldGenerator.Get(), Depth, bEnableMultiplayer, bEnableUndoRedo))
		, HistoryPosition(0)
		, MaxHistoryPosition(0)
	{
		check(Depth > 0);
		check(Octree->GetBounds().Contains(WorldBounds));
	}
	FVoxelData(int Depth, TSharedRef<const FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> WorldGenerator, bool bEnableMultiplayer, bool bEnableUndoRedo)
		: FVoxelData(Depth, FVoxelUtilities::GetBoundsFromDepth<VOXEL_CELL_SIZE>(Depth), WorldGenerator, bEnableMultiplayer, bEnableUndoRedo)
	{

	}

	const int Depth;
	const FIntBox WorldBounds;
	const bool bEnableMultiplayer;
	const bool bEnableUndoRedo;

	TSharedRef<const FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> const WorldGenerator;

	// Size = VOXEL_CELL_SIZE * 2^Depth
	inline int32 Size() const { return VOXEL_CELL_SIZE << Depth; }
	inline FIntBox GetBounds() const
	{
		return WorldBounds;
	}
	inline FVoxelDataOctree* GetOctree() const { return Octree.Get(); }

public:
	/**
	 * Lock Box in read/write
	 * @param	Box			Box to lock
	 * @param	OutOctrees	Locked octrees
	 */
	void BeginSet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees, FString Name);
	bool TryBeginSet(const FIntBox& Box, int TimeoutMicroSeconds, TArray<FVoxelId>& OutOctrees, FString& InOutName);
	/**
	 * End the lock on LockedOctrees
	 * @param	LockedOctrees		Returned by BeginSet
	 */
	 void EndSet(TArray<FVoxelId>& LockedOctrees);
	 
	/**
	 * Lock Box in read only
	 * @param	Box		Box to lock
	 * @param	OutOctrees	Locked octrees
	 */
	 void BeginGet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees, FString Name);
	 bool TryBeginGet(const FIntBox& Box, int TimeoutMicroSeconds, TArray<FVoxelId>& OutOctrees, FString& InOutName);
	/**
	 * End the lock on LockedOctrees
	 * @param	LockedOctrees		Returned by BeginGet
	 */
	void EndGet(TArray<FVoxelId>& LockedOctrees);
	
public:	
	void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD) const;

	inline bool IsEmpty(const FIntBox& Bounds, int InLOD) const { return Octree->IsEmpty(Bounds, InLOD) != EVoxelEmptyState::Unknown; }
	
	template<typename T, typename F>
	void SetValueOrMaterialLambda(const FIntBox& Bounds, F Lambda) { Octree->SetValueOrMaterialLambda<T>(Bounds.Overlap(WorldBounds), Lambda); }

	template<bool bOnlyIfDirty, typename F>
	void CallLambdaOnValuesInBounds(const FIntBox& Bounds, F Lambda) { Octree->CallLambdaOnValuesInBounds<bOnlyIfDirty>(Bounds, Lambda); }

	void CacheMostUsedChunks(
		uint32 Threshold,
		uint32 MaxCacheSize, 
		uint32& NumChunksSubdivided, 
		uint32& NumChunksCached, 
		uint32& NumRemovedFromCache,
		uint32& TotalNumCachedChunks);

	void Cache(const FIntBox& Bounds, bool bIsManualCache) { Octree->Cache(Bounds, bIsManualCache); }
	void Compact(uint32& NumDeleted);

public:
	inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int QueryLOD) const
	{
		if (UNLIKELY(!IsInWorld(X, Y, Z)))
		{
			WorldGenerator->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD, FVoxelPlaceableItemHolder());
		}
		else
		{
			Octree->GetLeaf(X, Y, Z)->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
		}
	}
	template<typename T>
	void SetValueOrMaterial(int X, int Y, int Z, const T& Value)
	{
		if (LIKELY(IsInWorld(X, Y, Z)))
		{
			Octree->GetLeaf(X, Y, Z)->SetValueOrMaterial<T>(X, Y, Z, Value);
		}
	}

	/**
	 * Get the value & material at position. Requires BeginGet
	 */
	inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int QueryLOD) const { GetValueAndMaterial(X, Y, Z, &OutValue, &OutMaterial, QueryLOD); }
	inline void GetValueAndMaterial(const FIntVector& P, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int QueryLOD) const { GetValueAndMaterial(P.X, P.Y, P.Z, &OutValue, &OutMaterial, QueryLOD); }
	/**
	 * Get the value at position. Requires BeginGet
	 */
	inline FVoxelValue GetValue(int X, int Y, int Z, int QueryLOD) const { FVoxelValue Value; GetValueAndMaterial(X, Y, Z, &Value, nullptr, QueryLOD); return Value; }
	inline FVoxelValue GetValue(const FIntVector& P, int QueryLOD) const { return GetValue(P.X, P.Y, P.Z, QueryLOD); }
	/**
	 * Get the material at position. Requires BeginGet
	 */
	inline FVoxelMaterial GetMaterial(int X, int Y, int Z, int QueryLOD) const { FVoxelMaterial Material; GetValueAndMaterial(X, Y, Z, nullptr, &Material, QueryLOD); return Material; }
	inline FVoxelMaterial GetMaterial(const FIntVector& P, int QueryLOD) const { return GetMaterial(P.X, P.Y, P.Z, QueryLOD); }

	/**
	 * Requires BeginGet with an offset of 1 (final box has size 3)
	 */
	template<typename T>
	inline static FVector GetGradient(T& Data, int X, int Y, int Z, int QueryLOD)
	{
		FVector Gradient;
		Gradient.X = Data.GetValue(X + 1, Y, Z, QueryLOD).ToFloat() - Data.GetValue(X - 1, Y, Z, QueryLOD).ToFloat();
		Gradient.Y = Data.GetValue(X, Y + 1, Z, QueryLOD).ToFloat() - Data.GetValue(X, Y - 1, Z, QueryLOD).ToFloat();
		Gradient.Z = Data.GetValue(X, Y, Z + 1, QueryLOD).ToFloat() - Data.GetValue(X, Y, Z - 1, QueryLOD).ToFloat();
		return Gradient.GetSafeNormal();
	}
	template<typename T>
	inline static FVector GetGradient(T& Data, const FIntVector& P, int QueryLOD)
	{
		return GetGradient(Data, P.X, P.Y, P.Z, QueryLOD);
	}

	inline FVector GetGradient(int X, int Y, int Z, int QueryLOD) const { return GetGradient(*this, X, Y, Z, QueryLOD); }
	inline FVector GetGradient(const FIntVector& P, int QueryLOD) const { return GetGradient(*this, P, QueryLOD); }

	/**
	 * Set value at position. Requires BeginSet
	 */
	inline void SetValue(int X, int Y, int Z, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(X, Y, Z, Value); }
	inline void SetValue(const FIntVector& P, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(P.X, P.Y, P.Z, Value); }

	/**
	 * Set material at position. Requires BeginSet
	 */
	inline void SetMaterial(int X, int Y, int Z, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(X, Y, Z, Material); }
	inline void SetMaterial(const FIntVector& P, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(P.X, P.Y, P.Z, Material); }

public:
	/**
	 * Is Position in this world?
	 */
	template<typename TNumeric>
	inline bool IsInWorld(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		return WorldBounds.IsInside(X, Y, Z);
	}
	template<typename TVector>
	inline bool IsInWorld(const TVector& P) const { return IsInWorld(P.X, P.Y, P.Z); }

	/**
	 * Clamp (X, Y, Z) in the world
	 */
	inline void ClampToWorld(int& X, int& Y, int& Z) const
	{
		int32 S = Size() / 2;
		X = FMath::Clamp(X, -WorldBounds.Min.X, WorldBounds.Max.X - 1);
		Y = FMath::Clamp(Y, -WorldBounds.Min.Y, WorldBounds.Max.Y - 1);
		Z = FMath::Clamp(Z, -WorldBounds.Min.Z, WorldBounds.Max.Z - 1);
	}

public:
	/**
	 * Get a save of this world
	 */
	void GetSave(FVoxelUncompressedWorldSave& OutSave);

	/**
	 * Load this world from save
	 * @param	Save						Save to load from
	 * @param	OutBoundsToUpdate			The modified bounds
	 */
	void LoadFromSave(const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate);


public:
	/**
	 * Undo one frame and add it to the redo stack. Current frame must be empty
	 */
	void Undo(TArray<FIntBox>& OutBoundsToUpdate)
	{
		FVoxelScopeSetLock Lock(this, FIntBox::Infinite, "Undo");
		if (HistoryPosition > 0)
		{
			HistoryPosition--;
			Octree->Undo(HistoryPosition, OutBoundsToUpdate);
		}
	}
	/**
	 * Redo one frame and add it to the undo stack. Current frame must be empty
	 */
	void Redo(TArray<FIntBox>& OutBoundsToUpdate)
	{
		FVoxelScopeSetLock Lock(this, FIntBox::Infinite, "Redo");
		if (HistoryPosition < MaxHistoryPosition)
		{
			HistoryPosition++;
			Octree->Redo(HistoryPosition, OutBoundsToUpdate);
		}
	}
	/**
	 * Clear all the frames
	 */
	void ClearFrames()
	{
		FVoxelScopeSetLock Lock(this, FIntBox::Infinite, "ClearFrames");
		Octree->ClearFrames();
		HistoryPosition = 0;
		MaxHistoryPosition = 0;
	}
	/**
	 * Add the current frame to the undo stack. Clear the redo stack
	 */
	void SaveFrame()
	{
		FVoxelScopeSetLock Lock(this, FIntBox::Infinite, "SaveFrame");
		Octree->SaveFrame(HistoryPosition);
		HistoryPosition++;
		MaxHistoryPosition = HistoryPosition;
	}
	/**
	 * Check that the current frame is empty (safe to call Undo/Redo)
	 */
	bool CheckIfCurrentFrameIsEmpty()
	{
		FVoxelScopeGetLock Lock(this, FIntBox::Infinite, "CheckIfCurrentFrameIsEmpty");
		return Octree->CheckIfCurrentFrameIsEmpty();
	}
	/**
	 * Get the history position
	 */
	int GetHistoryPosition() const { return HistoryPosition; }
	/**
	 * Get the max history position (including redo frames)
	 */
	inline int GetMaxHistory() const { return MaxHistoryPosition; }

public:
	template<typename T, typename... TArgs>
	void AddItem(TArgs&&... Args)
	{
		TUniquePtr<T> Item = MakeUnique<T>(Forward<TArgs>(Args)...);
		Octree->AddItem<T>(Item.Get());
		Items.Add(MoveTemp(Item));
	}

private:
	TUniquePtr<FVoxelDataOctree> const Octree;
	int HistoryPosition;
	int MaxHistoryPosition;
	TArray<TUniquePtr<FVoxelPlaceableItem>> Items;
	uint32 CacheTime = 0;
	FCriticalSection CacheTimeSection;
	std::shared_timed_mutex MainLock;
};

using FVoxelScopeSetLock = FVoxelData::FVoxelScopeSetLock;
using FVoxelScopeGetLock = FVoxelData::FVoxelScopeGetLock;