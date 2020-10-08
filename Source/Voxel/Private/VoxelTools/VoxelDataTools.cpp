// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelAssets/VoxelHeightmapAssetSamplerWrapper.h"
#include "VoxelFeedbackContext.h"

#define VOXEL_DATA_TOOL_PREFIX const FVoxelIntBox Bounds(Position);

void UVoxelDataTools::GetValue(float& Value, AVoxelWorld* World, FIntVector Position)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, VOXEL_DATA_TOOL_PREFIX, Value = Data.GetValue(Position, 0).ToFloat());
}

void UVoxelDataTools::GetInterpolatedValue(float& Value, AVoxelWorld* World, FVector Position)
{
	const FVoxelIntBox Bounds = FVoxelIntBox(Position).Extend(2);
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, NO_PREFIX, Value = FVoxelDataUtilities::MakeBilinearInterpolatedData(Data).GetValue(Position, 0));
}

void UVoxelDataTools::SetValue(AVoxelWorld* World, FIntVector Position, float Value)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, VOXEL_DATA_TOOL_PREFIX, Data.SetValue(Position, FVoxelValue(Value)));
}

void UVoxelDataTools::GetMaterial(FVoxelMaterial& Material, AVoxelWorld* World, FIntVector Position)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, VOXEL_DATA_TOOL_PREFIX, Material = Data.GetMaterial(Position, 0));
}

void UVoxelDataTools::SetMaterial(AVoxelWorld* World, FIntVector Position, FVoxelMaterial Material)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, VOXEL_DATA_TOOL_PREFIX, Data.SetMaterial(Position, Material));
}

void UVoxelDataTools::CacheValues(AVoxelWorld* World, FVoxelIntBox Bounds, bool bMultiThreaded)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelValue>(Bounds, bMultiThreaded));
}

void UVoxelDataTools::CacheMaterials(AVoxelWorld* World, FVoxelIntBox Bounds, bool bMultiThreaded)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelMaterial>(Bounds, bMultiThreaded));
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::GetValueAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	float& Value,
	AVoxelWorld* World,
	FIntVector Position,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Value, Read, DoNotUpdateRender, VOXEL_DATA_TOOL_PREFIX, InValue = Data.GetValue(Position, 0).ToFloat());
}

void UVoxelDataTools::SetValueAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntVector Position,
	float Value,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, VOXEL_DATA_TOOL_PREFIX, Data.SetValue(Position, FVoxelValue(Value)));
}

void UVoxelDataTools::GetMaterialAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelMaterial& Material,
	AVoxelWorld* World,
	FIntVector Position,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Material, Read, DoNotUpdateRender, VOXEL_DATA_TOOL_PREFIX, InMaterial = Data.GetMaterial(Position, 0));
}

void UVoxelDataTools::SetMaterialAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntVector Position,
	FVoxelMaterial Material,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, VOXEL_DATA_TOOL_PREFIX, Data.SetMaterial(Position, Material));
}

void UVoxelDataTools::CacheValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelValue>(Bounds, false));
}

void UVoxelDataTools::CacheMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelMaterial>(Bounds, false));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline bool CheckSave(const FVoxelData& Data, const T& Save)
{
	if (Save.GetDepth() == -1)
	{
		FVoxelMessages::Error("LoadFromSave: Invalid save (Depth == -1). You're trying to load a save object that wasn't initialized");
		return false;
	}
	if (Save.GetDepth() > Data.Depth)
	{
		FVoxelMessages::Warning("LoadFromSave: Save depth is bigger than world depth, the save data outside world bounds will be ignored");
	}
	return true;
}

#define CHECK_SAVE() \
if (!CheckSave(World->GetData(), Save)) \
{ \
	return false; \
}

void UVoxelDataTools::GetSave(AVoxelWorld* World, FVoxelUncompressedWorldSave& OutSave)
{
	GetSave(World, OutSave.NewMutable(), OutSave.Objects);
}

void UVoxelDataTools::GetSave(AVoxelWorld* World, FVoxelUncompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetData().GetSave(OutSave, OutObjects);
}

void UVoxelDataTools::GetCompressedSave(AVoxelWorld* World, FVoxelCompressedWorldSave& OutSave)
{
	GetCompressedSave(World, OutSave.NewMutable(), OutSave.Objects);
}

void UVoxelDataTools::GetCompressedSave(AVoxelWorld* World, FVoxelCompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelUncompressedWorldSaveImpl Save;
	World->GetData().GetSave(Save, OutObjects);
	UVoxelSaveUtilities::CompressVoxelSave(Save, OutSave);
}

void UVoxelDataTools::GetSaveAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World,
	FVoxelUncompressedWorldSave& OutSave, 
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		World,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		OutSave,
		[](FVoxelData& Data, FVoxelUncompressedWorldSave& Save)
		{
			Data.GetSave(Save.NewMutable(), Save.Objects);
		},
		EVoxelUpdateRender::DoNotUpdateRender,
		{});
}

void UVoxelDataTools::GetCompressedSaveAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World, 
	FVoxelCompressedWorldSave& OutSave, 
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld_WithValue(
		WorldContextObject,
		LatentInfo,
		World,
		FUNCTION_FNAME,
		bHideLatentWarnings,
		OutSave,
		[](FVoxelData& Data, FVoxelCompressedWorldSave& CompressedSave)
		{
			FVoxelUncompressedWorldSaveImpl Save;
			Data.GetSave(Save, CompressedSave.Objects);
			UVoxelSaveUtilities::CompressVoxelSave(Save, CompressedSave.NewMutable());
		},
		EVoxelUpdateRender::DoNotUpdateRender,
		{});
}

bool UVoxelDataTools::LoadFromSave(const AVoxelWorld* World, const FVoxelUncompressedWorldSave& Save)
{
	return LoadFromSave(World, Save.Const(), Save.Objects);
}

bool UVoxelDataTools::LoadFromSave(const AVoxelWorld* World, const FVoxelUncompressedWorldSaveImpl& Save, const TArray<FVoxelObjectArchiveEntry>& Objects)
{
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_SAVE();

	TArray<FVoxelIntBox> BoundsToUpdate;
	auto& Data = World->GetData();
	
	const FVoxelGeneratorInit WorldInit = World->GetGeneratorInit();
	const FVoxelPlaceableItemLoadInfo LoadInfo{ &WorldInit, &Objects };

	const bool bSuccess = Data.LoadFromSave(Save, LoadInfo, &BoundsToUpdate);

	World->GetLODManager().UpdateBounds(BoundsToUpdate);

	return bSuccess;
}

bool UVoxelDataTools::LoadFromCompressedSave(const AVoxelWorld* World, const FVoxelCompressedWorldSave& Save)
{
	return LoadFromCompressedSave(World, Save.Const(), Save.Objects);
}

bool UVoxelDataTools::LoadFromCompressedSave(const AVoxelWorld* World, const FVoxelCompressedWorldSaveImpl& Save, const TArray<FVoxelObjectArchiveEntry>& Objects)
{
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_SAVE();
	
	FVoxelUncompressedWorldSaveImpl UncompressedSave;
	UVoxelSaveUtilities::DecompressVoxelSave(Save, UncompressedSave);

	return LoadFromSave(World, UncompressedSave, Objects);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::RoundVoxelsImpl(FVoxelData& Data, const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	int32 NumDirtyLeaves = 0;
	uint64 NumVoxels = 0;
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Record stats");
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<FVoxelValue>().IsDirty() && Leaf.GetData<FVoxelValue>().HasAllocation())
			{
				NumDirtyLeaves++;
				NumVoxels += VOXELS_PER_DATA_CHUNK;
			}
		});
	}
	
	FVoxelScopedSlowTask SlowTask(NumDirtyLeaves, VOXEL_LOCTEXT("Rounding voxels"));
	FScopeToolsTimeLogger ToolsLogger(__FUNCTION__, NumVoxels);
		
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds.Extend(2));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<FVoxelValue>().IsDirty() && Leaf.GetData<FVoxelValue>().HasAllocation())
		{
			SlowTask.EnterProgressFrame();
			
			const FVoxelIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const FVoxelValue& Value = Leaf.GetData<FVoxelValue>().Get(Index);
						
				if (Value.IsTotallyEmpty() || Value.IsTotallyFull()) return;
				
				const bool bEmpty = Value.IsEmpty();
				for (int32 OtherX = X - 2; OtherX <= X + 2; OtherX++)
				{
					for (int32 OtherY = Y - 2; OtherY <= Y + 2; OtherY++)
					{
						for (int32 OtherZ = Z - 2; OtherZ <= Z + 2; OtherZ++)
						{
							if (OtherX == X && OtherY == Y && OtherZ == Z) continue;
							const auto OtherValue = OctreeAccelerator.GetValue(OtherX, OtherY, OtherZ, 0);
							if (OtherValue.IsEmpty() != bEmpty) return;
						}
					}
				}
				OctreeAccelerator.SetValue(X, Y, Z, bEmpty ? FVoxelValue::Empty() : FVoxelValue::Full());
			});
		}
	});
}

void UVoxelDataTools::RoundVoxels(AVoxelWorld* World, FVoxelIntBox InBounds)
{
	const FVoxelIntBox Bounds = InBounds.Extend(2);
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundVoxelsImpl(Data, InBounds));
}

void UVoxelDataTools::RoundVoxelsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox InBounds,
	bool bHideLatentWarnings)
{
	const FVoxelIntBox Bounds = InBounds.Extend(2);
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundVoxelsImpl(Data, InBounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::ClearUnusedMaterialsImpl(FVoxelData& Data, const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	int32 NumDirtyLeaves = 0;
	uint64 NumVoxels = 0;
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Record stats");
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<FVoxelMaterial>().IsDirty() && Leaf.GetData<FVoxelMaterial>().HasAllocation())
			{
				NumDirtyLeaves++;
				NumVoxels += VOXELS_PER_DATA_CHUNK;
			}
		});
	}
	
	FVoxelScopedSlowTask SlowTask(NumDirtyLeaves, VOXEL_LOCTEXT("Clearing unused materials"));
	FScopeToolsTimeLogger ToolsLogger(__FUNCTION__, NumVoxels);
		
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds.Extend(2));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<FVoxelMaterial>().IsDirty() && Leaf.GetData<FVoxelMaterial>().HasAllocation())
		{
			SlowTask.EnterProgressFrame();

			bool bEdited = false;
			
			const FVoxelIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const FVoxelMaterial Material = Leaf.GetData<FVoxelMaterial>().Get(Index);

				if (Material == FVoxelMaterial::Default()) return;
				
				const FVoxelValue Value = OctreeAccelerator.GetValue(X, Y, Z, 0);
				if (!Value.IsEmpty()) // Only not empty voxels materials can affect the surface
				{
					for (int32 OtherX = X - 1; OtherX <= X + 1; OtherX++)
					{
						for (int32 OtherY = Y - 1; OtherY <= Y + 1; OtherY++)
						{
							for (int32 OtherZ = Z - 1; OtherZ <= Z + 1; OtherZ++)
							{
								if (OtherX == X && OtherY == Y && OtherZ == Z) continue;
								const auto OtherValue = OctreeAccelerator.GetValue(OtherX, OtherY, OtherZ, 0);
								if (OtherValue.IsEmpty()) return;
							}
						}
					}
				}
				OctreeAccelerator.SetMaterial(X, Y, Z, FVoxelMaterial::Default());
				bEdited = true;
			});

			if (bEdited)
			{
				Leaf.GetData<FVoxelMaterial>().Compress(Data);
			}
		}
	});
}

void UVoxelDataTools::ClearUnusedMaterials(AVoxelWorld* World, FVoxelIntBox InBounds)
{
	const FVoxelIntBox Bounds = InBounds.Extend(1);
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, ClearUnusedMaterialsImpl(Data, InBounds));
}

void UVoxelDataTools::ClearUnusedMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox InBounds,
	bool bHideLatentWarnings)
{
	const FVoxelIntBox Bounds = InBounds.Extend(1);
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, ClearUnusedMaterialsImpl(Data, InBounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::GetVoxelsValueAndMaterialImpl(
	FVoxelData& Data,
	TArray<FVoxelValueMaterial>& Voxels,
	const FVoxelIntBox& Bounds,
	const TArray<FIntVector>& Positions)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Positions.Num());
	const FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds);
	for (auto& Position : Positions)
	{
		if (Data.IsInWorld(Position))
		{
			FVoxelValueMaterial Voxel;
			Voxel.Position = Position;
			Voxel.Value = OctreeAccelerator.GetValue(Position, 0).ToFloat();
			Voxel.Material = OctreeAccelerator.GetMaterial(Position, 0);
			Voxels.Add(Voxel);
		}
	}
}

void UVoxelDataTools::GetVoxelsValueAndMaterial(
	TArray<FVoxelValueMaterial>& Voxels,
	AVoxelWorld* World,
	const TArray<FIntVector>& Positions)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Positions.Num() == 0)
	{
		Voxels.Reset();
		return;
	}
	
	const FVoxelIntBox Bounds(Positions);
	CHECK_BOUNDS_ARE_VALID_VOID();
	VOXEL_TOOL_HELPER_BODY(Read, DoNotUpdateRender, GetVoxelsValueAndMaterialImpl(Data, Voxels, Bounds, Positions));
}

void UVoxelDataTools::GetVoxelsValueAndMaterialAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	TArray<FVoxelValueMaterial>& Voxels, 
	AVoxelWorld* World, 
	const TArray<FIntVector>& Positions, 
	bool bHideLatentWarnings)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (Positions.Num() == 0)
	{
		Voxels.Reset();
		return;
	}
	
	const FVoxelIntBox Bounds(Positions);
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, GetVoxelsValueAndMaterialImpl(Data, InVoxels, Bounds, Positions));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDataMemoryUsageInMB UVoxelDataTools::GetDataMemoryUsageInMB(AVoxelWorld* World)
{
	CHECK_VOXELWORLD_IS_CREATED();
	VOXEL_FUNCTION_COUNTER();

	constexpr double OneMB = double(1 << 20);

	auto& Data = World->GetData();

	FVoxelDataMemoryUsageInMB MemoryUsage;

	MemoryUsage.DirtyValues = Data.GetDirtyMemory().Values.GetValue() / OneMB;
	MemoryUsage.DirtyMaterials = Data.GetDirtyMemory().Materials.GetValue() / OneMB;

	MemoryUsage.CachedValues = Data.GetCachedMemory().Values.GetValue() / OneMB;
	MemoryUsage.CachedMaterials = Data.GetCachedMemory().Materials.GetValue() / OneMB;

	return MemoryUsage;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::ClearCachedValues(
	AVoxelWorld* World,
	FVoxelIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.ClearCacheInBounds<FVoxelValue>(Bounds));
}

void UVoxelDataTools::ClearCachedValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.ClearCacheInBounds<FVoxelValue>(Bounds));
}

void UVoxelDataTools::ClearCachedMaterials(
	AVoxelWorld* World,
	FVoxelIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.ClearCacheInBounds<FVoxelMaterial>(Bounds));
}

void UVoxelDataTools::ClearCachedMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.ClearCacheInBounds<FVoxelMaterial>(Bounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::CheckForSingleValues(
	AVoxelWorld* World,
	FVoxelIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CheckIsSingle<FVoxelValue>(Bounds));
}

void UVoxelDataTools::CheckForSingleValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CheckIsSingle<FVoxelValue>(Bounds));
}

void UVoxelDataTools::CheckForSingleMaterials(
	AVoxelWorld* World,
	FVoxelIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CheckIsSingle<FVoxelMaterial>(Bounds));
}

void UVoxelDataTools::CheckForSingleMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CheckIsSingle<FVoxelMaterial>(Bounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void UVoxelDataTools::CompressIntoHeightmapImpl(FVoxelData& Data, TVoxelHeightmapAssetSamplerWrapper<T>& Wrapper, const bool bCheckAllLeaves)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	ensure(Wrapper.Scale == 1.f);
	
	FVoxelScopedSlowTask SlowTask(2, VOXEL_LOCTEXT("Compressing into heightmap"));
	SlowTask.EnterProgressFrame();

	TMap<FIntPoint, TMap<int32, FVoxelDataOctreeLeaf*>> LeavesColumns;
	{
		if (bCheckAllLeaves)
		{
			// Need to subdivide
			FVoxelOctreeUtilities::IterateEntireTree(Data.GetOctree(), [&](FVoxelDataOctreeBase& Tree)
			{
				if (!Tree.IsLeaf())
				{
					auto& Parent = Tree.AsParent();
					if (!Parent.HasChildren())
					{
						ensureThreadSafe(Parent.IsLockedForWrite());
						Parent.CreateChildren();
					}
				}
			});
		}
		
		int32 NumLeaves = 0;
		FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<FVoxelValue>().IsDirty() || bCheckAllLeaves)
			{
				NumLeaves++;
			}
		});

		FVoxelScopedSlowTask LocalSlowTask(NumLeaves, VOXEL_LOCTEXT("Caching data"));

		FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			ensureThreadSafe(Leaf.IsLockedForWrite());

			auto& DataHolder = Leaf.GetData<FVoxelValue>();
			if (bCheckAllLeaves)
			{
				if (!DataHolder.HasData())
				{
					DataHolder.CreateData(Data, [&](FVoxelValue* RESTRICT DataPtr)
					{
						TVoxelQueryZone<FVoxelValue> QueryZone(Leaf.GetBounds(), DataPtr);
						Leaf.GetFromGeneratorAndAssets(*Data.Generator, QueryZone, 0);
					});
					// Reduce memory usage
					DataHolder.Compress(Data);
				}
			}
			else
			{
				if (!DataHolder.IsDirty())
				{
					// Flush cache
					DataHolder.ClearData(Data);
					return;
				}
			}

			LocalSlowTask.EnterProgressFrame();

			const FIntVector Min = Leaf.GetMin();
			
			auto& Column = LeavesColumns.FindOrAdd(FIntPoint(Min.X, Min.Y));
			check(!Column.Contains(Min.Z));
			Column.Add(Min.Z, &Leaf);
		});
	}
	
	SlowTask.EnterProgressFrame();
	FVoxelScopedSlowTask LocalSlowTask(LeavesColumns.Num(), VOXEL_LOCTEXT("Finding heights"));

	FVoxelMutableDataAccelerator Accelerator(Data, FVoxelIntBox::Infinite);
	for (auto& ColumnsIt : LeavesColumns)
	{
		LocalSlowTask.EnterProgressFrame();
		
		const FIntPoint LeafMinXY = ColumnsIt.Key;
		auto& Leaves = ColumnsIt.Value;

		int32 MinLeafMinZ = MAX_int32;
		int32 MaxLeafMinZ = MIN_int32;
		for (auto& LeavesIt : Leaves)
		{
			MinLeafMinZ = FMath::Min(LeavesIt.Key, MinLeafMinZ);
			MaxLeafMinZ = FMath::Max(LeavesIt.Key, MaxLeafMinZ);
		}
		check(MinLeafMinZ != MAX_int32);
		check(MaxLeafMinZ != MIN_int32);

		const auto GetHeightmapPosition = [&](int32 X, int32 Y)
		{
			// Note: HeightmapAssets are offset by (-Wrapper.GetWidth() / 2, -Wrapper.GetHeight() / 2)
			return FIntPoint(LeafMinXY.X + X + Wrapper.GetWidth() / 2, LeafMinXY.Y + Y + Wrapper.GetHeight() / 2);
		};
		const auto IsInBounds = [&](const FIntPoint& HeightmapPosition)
		{
			return
				HeightmapPosition.X >= 0 &&
				HeightmapPosition.Y >= 0 &&
				HeightmapPosition.X < Wrapper.GetWidth() &&
				HeightmapPosition.Y < Wrapper.GetHeight();
		};

		TStaticArray<float, DATA_CHUNK_SIZE * DATA_CHUNK_SIZE> NewHeights;
		for (int32 X = 0; X < DATA_CHUNK_SIZE; X++)
		{
			for (int32 Y = 0; Y < DATA_CHUNK_SIZE; Y++)
			{
				const FIntPoint HeightmapPosition = GetHeightmapPosition(X, Y);
				if (!IsInBounds(HeightmapPosition)) continue;

				const auto GetNewHeight = [&]()
				{
					const float HeightmapHeight = Wrapper.GetHeight(HeightmapPosition.X, HeightmapPosition.Y, EVoxelSamplerMode::Clamp);
					if (HeightmapHeight >= MaxLeafMinZ + DATA_CHUNK_SIZE)
					{
						// Heightmap is above all leaves, can't do anything
						return HeightmapHeight;
					}
					
					// Go down chunk by chunk until we find the height
					for (int32 LeafMinZ = MaxLeafMinZ; LeafMinZ >= MinLeafMinZ; LeafMinZ -= DATA_CHUNK_SIZE)
					{
						auto* Leaf = Leaves.FindRef(LeafMinZ);
						if (!Leaf)
						{
							checkVoxelSlow(HeightmapHeight <= LeafMinZ + DATA_CHUNK_SIZE);
							if (LeafMinZ < HeightmapHeight)
							{
								// HeightmapHeight above all remaining leaves, can't do anything
								return HeightmapHeight;
							}
						}
						else
						{
							// Go down until we find the height
							auto& DataHolder = Leaf->GetData<FVoxelValue>();
							if (DataHolder.IsSingleValue() && !DataHolder.GetSingleValue().IsEmpty())
							{
								// Fast path
								continue;
							}

							for (int32 Z = DATA_CHUNK_SIZE - 1; Z >= 0; Z--)
							{
								const FVoxelValue Value = DataHolder.Get(FVoxelDataOctreeUtilities::IndexFromCoordinates(X, Y, Z));
								if (!Value.IsEmpty())
								{
									FVoxelValue ValueAbove;
									if (Z + 1 < DATA_CHUNK_SIZE)
									{
										ValueAbove = DataHolder.Get(FVoxelDataOctreeUtilities::IndexFromCoordinates(X, Y, Z + 1));
									}
									else
									{
										ValueAbove = Accelerator.GetValue(Leaf->GetMin() + FIntVector(X, Y, Z + 1), 0);
									}
									// Note: not true on world upper bound
									ensure(ValueAbove.IsEmpty());

									const float NewHeight = LeafMinZ + Z + FVoxelUtilities::GetAbsDistanceFromDensities(Value.ToFloat(), ValueAbove.ToFloat());
									const float OldHeight = HeightmapHeight;

									// Mark leaves that will have their value changed by the new height as dirty so that they don't change
									const int32 StartLeafMinZ = FMath::FloorToInt(FMath::Min(NewHeight, OldHeight) / DATA_CHUNK_SIZE) * DATA_CHUNK_SIZE;
									const int32 EndLeafMinZ = FMath::CeilToInt(FMath::Max(NewHeight, OldHeight) / DATA_CHUNK_SIZE) * DATA_CHUNK_SIZE;
									for (int32 ItLeafMinZ = StartLeafMinZ; ItLeafMinZ <= EndLeafMinZ; ItLeafMinZ += DATA_CHUNK_SIZE)
									{
										if (Leaves.Contains(ItLeafMinZ)) continue; // Values already correctly stored

										// Leaf is defaulting to generator value, but this value is going to change
										// Mark the leaf as dirty

										const FIntVector LeafPosition = FIntVector(LeafMinXY.X, LeafMinXY.Y, ItLeafMinZ) + DATA_CHUNK_SIZE / 2;
										if (!Data.IsInWorld(LeafPosition)) continue;
										
										auto* ItLeaf = FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(Data.GetOctree(), LeafPosition);
										check(ItLeaf);
										check(!ItLeaf->GetData<FVoxelValue>().HasData());

										ItLeaf->InitForEdit<FVoxelValue>(Data);
										ItLeaf->GetData<FVoxelValue>().SetIsDirty(true, Data);

										// & Add it to the map
										Leaves.Add(ItLeafMinZ, ItLeaf);

										// Update min/max as well
										MinLeafMinZ = FMath::Min(ItLeafMinZ, MinLeafMinZ);
										MaxLeafMinZ = FMath::Max(ItLeafMinZ, MaxLeafMinZ);
									}
									return NewHeight;
								}
							}
						}
					}
					// Not found anything, default to old height
					return HeightmapHeight;
				};
				NewHeights[X + DATA_CHUNK_SIZE * Y] = GetNewHeight();
			}
		}

		// Write the heights
		for (int32 X = 0; X < DATA_CHUNK_SIZE; X++)
		{
			for (int32 Y = 0; Y < DATA_CHUNK_SIZE; Y++)
			{
				const FIntPoint HeightmapPosition = GetHeightmapPosition(X, Y);
				if (!IsInBounds(HeightmapPosition)) continue;

				Wrapper.SetHeight(HeightmapPosition.X, HeightmapPosition.Y, NewHeights[X + DATA_CHUNK_SIZE * Y]);
			}
		}
	}
}

template VOXEL_API void UVoxelDataTools::CompressIntoHeightmapImpl<uint16>(FVoxelData& Data, TVoxelHeightmapAssetSamplerWrapper<uint16>& Wrapper, bool bCheckAllLeaves);
template VOXEL_API void UVoxelDataTools::CompressIntoHeightmapImpl<float>(FVoxelData& Data, TVoxelHeightmapAssetSamplerWrapper<float>& Wrapper, bool bCheckAllLeaves);

void UVoxelDataTools::CompressIntoHeightmap(
	AVoxelWorld* World, 
	UVoxelHeightmapAsset* HeightmapAsset,
	bool bHeightmapAssetMatchesWorld)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	auto& Data = World->GetData();
	FVoxelWriteScopeLock Lock(Data, FVoxelIntBox::Infinite, "");

	bool bCheckAllLeaves = false;
	if (!HeightmapAsset)
	{
		if (World->Generator.IsObject()) // We don't want to edit the default object otherwise
		{
			HeightmapAsset = Cast<UVoxelHeightmapAsset>(World->Generator.Object);
		}
	}
	else 
	{
		if (!bHeightmapAssetMatchesWorld)
		{
			bCheckAllLeaves = true;

			const int32 Size = FVoxelUtilities::GetSizeFromDepth<DATA_CHUNK_SIZE>(Data.Depth);
			if (Size > 20000)
			{
				FVoxelMessages::Error(FUNCTION_ERROR("Heightmap size would be too large!"));
				return;
			}
			if (auto* Heightmap = Cast<UVoxelHeightmapAssetUINT16>(HeightmapAsset))
			{
				HeightmapAsset->HeightOffset = -Size / 2; // Height = 0 should be bottom of the world since all heights are positive
				HeightmapAsset->HeightScale = float(Size) / MAX_uint16; // Distribute the heights
				HeightmapAsset->AdditionalThickness = Size; // Just to be safe

				// Init the heightmap data
				Heightmap->GetData().SetSize(Size, Size, false, {});
				Heightmap->GetData().SetAllHeightsTo(0);
			}
			if (auto* Heightmap = Cast<UVoxelHeightmapAssetFloat>(HeightmapAsset))
			{
				HeightmapAsset->HeightOffset = 0; // Heights can be negative too here
				HeightmapAsset->HeightScale = 1.f; // No need to distribute
				HeightmapAsset->AdditionalThickness = Size; // Fill below

				// Init the heightmap data
				Heightmap->GetData().SetSize(Size, Size, false, {});
				Heightmap->GetData().SetAllHeightsTo(0);
			}
		}
	}
	
	if (auto* UINT16Heightmap = Cast<UVoxelHeightmapAssetUINT16>(HeightmapAsset))
	{
		TVoxelHeightmapAssetSamplerWrapper<uint16> Wrapper(UINT16Heightmap);
		CompressIntoHeightmapImpl(Data, Wrapper, bCheckAllLeaves);
		UINT16Heightmap->Save();
	}
	else if (auto* FloatHeightmap = Cast<UVoxelHeightmapAssetFloat>(HeightmapAsset))
	{
		TVoxelHeightmapAssetSamplerWrapper<float> Wrapper(FloatHeightmap);
		CompressIntoHeightmapImpl(Data, Wrapper, bCheckAllLeaves);
		FloatHeightmap->Save();
	}
	else
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Generator is not an heightmap!"));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::RoundToGeneratorImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, bool bPreserveNormals)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	int32 NumDirtyLeaves = 0;
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<FVoxelValue>().IsDirty())
		{
			NumDirtyLeaves++;
		}
	});
	
	FVoxelScopedSlowTask SlowTask(NumDirtyLeaves, VOXEL_LOCTEXT("Round To Generator"));
		
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds.Extend(2));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (!Leaf.GetData<FVoxelValue>().IsDirty())
		{
			return;
		}

		SlowTask.EnterProgressFrame();

		// Do not try to round if single value
		if (!Leaf.GetData<FVoxelValue>().IsSingleValue())
		{
			const FVoxelIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const FVoxelValue Value = Leaf.GetData<FVoxelValue>().Get(Index);
				const FVoxelValue GeneratorValue = Data.Generator->Get<FVoxelValue>(X, Y, Z, 0, FVoxelItemStack::Empty);

				if (Value == GeneratorValue) return;
				if (Value.IsEmpty() != GeneratorValue.IsEmpty()) return;

				const auto CheckNeighbor = [&](int32 DX, int32 DY, int32 DZ)
				{
					const FVoxelValue OtherValue = OctreeAccelerator.GetValue(X + DX, Y + DY, Z + DZ, 0);
					const FVoxelValue OtherGeneratorValue = Data.Generator->Get<FVoxelValue>(X + DX, Y + DY, Z + DZ, 0, FVoxelItemStack::Empty);
					return OtherValue.IsEmpty() == OtherGeneratorValue.IsEmpty();
				};

				if (bPreserveNormals)
				{
					for (int32 DX = -1; DX <= 1; DX++)
					{
						for (int32 DY = -1; DY <= 1; DY++)
						{
							for (int32 DZ = -1; DZ <= 1; DZ++)
							{
								if (DX == 0 && DY == 0 && DZ == 0) continue;
								if (!CheckNeighbor(DX, DY, DZ)) return;
							}
						}
					}
				}
				else
				{
					if (!CheckNeighbor(-1, 0, 0)) return;
					if (!CheckNeighbor(+1, 0, 0)) return;
					if (!CheckNeighbor(0, -1, 0)) return;
					if (!CheckNeighbor(0, +1, 0)) return;
					if (!CheckNeighbor(0, 0, -1)) return;
					if (!CheckNeighbor(0, 0, +1)) return;
				}

				OctreeAccelerator.SetValue(X, Y, Z, GeneratorValue);
			});
		}

		// But always check this, as else we get a lot of space used by single values!
		FVoxelDataUtilities::CheckIfSameAsGenerator<FVoxelValue>(Data, Leaf);
	});
}

void UVoxelDataTools::RoundToGenerator(AVoxelWorld* World, FVoxelIntBox Bounds, bool bPreserveNormals)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundToGeneratorImpl(Data, Bounds, bPreserveNormals));
}

void UVoxelDataTools::RoundToGeneratorAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVoxelIntBox Bounds,
	bool bPreserveNormals,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundToGeneratorImpl(Data, Bounds, bPreserveNormals));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::CheckIfSameAsGeneratorImpl(FVoxelData& Data, const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	int32 NumLeaves = 0;
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		NumLeaves++;
	});

	FVoxelScopedSlowTask SlowTask(NumLeaves, VOXEL_LOCTEXT("Check If Same As Generator"));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		SlowTask.EnterProgressFrame();
		ensureThreadSafe(Leaf.IsLockedForWrite());
		if (Leaf.GetData<FVoxelValue>().IsDirty())
		{
			FVoxelDataUtilities::CheckIfSameAsGenerator<FVoxelValue>(Data, Leaf);
		}
		if (Leaf.GetData<FVoxelMaterial>().IsDirty())
		{
			FVoxelDataUtilities::CheckIfSameAsGenerator<FVoxelMaterial>(Data, Leaf);
		}
	});
}

void UVoxelDataTools::CheckIfSameAsGenerator(AVoxelWorld* World, FVoxelIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, CheckIfSameAsGeneratorImpl(Data, Bounds));
}

void UVoxelDataTools::CheckIfSameAsGeneratorAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVoxelIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, CheckIfSameAsGeneratorImpl(Data, Bounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void UVoxelDataTools::SetBoxAsDirtyImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, bool bCompress)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	const int32 Count = Bounds.Overlap(Data.WorldBounds).MakeMultipleOfRoundUp(DATA_CHUNK_SIZE).Count() / int64(VOXELS_PER_DATA_CHUNK);
	FVoxelScopedSlowTask SlowTask(Count, VOXEL_LOCTEXT("Set Box as Dirty"));
	FVoxelOctreeUtilities::IterateTreeInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Tree)
	{
		if (Tree.IsLeaf())
		{
			SlowTask.EnterProgressFrame();
			
			auto& Leaf = Tree.AsLeaf();
			ensureThreadSafe(Leaf.IsLockedForWrite());
			
			Leaf.InitForEdit<T>(Data);
			if (bCompress)
			{
				// Else memory usage explodes
				Leaf.GetData<T>().Compress(Data);
			}
			Leaf.GetData<T>().SetIsDirty(true, Data);
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				Parent.CreateChildren();
			}
		}
	});
}

template VOXEL_API void UVoxelDataTools::SetBoxAsDirtyImpl<FVoxelValue>(FVoxelData&, const FVoxelIntBox&, bool);
template VOXEL_API void UVoxelDataTools::SetBoxAsDirtyImpl<FVoxelMaterial>(FVoxelData&, const FVoxelIntBox&, bool);

void UVoxelDataTools::SetBoxAsDirty(
	AVoxelWorld* World, 
	FVoxelIntBox Bounds, 
	bool bDirtyValues, 
	bool bDirtyMaterials)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, 
		if (bDirtyValues) 
		{
			SetBoxAsDirtyImpl<FVoxelValue>(Data, Bounds, true);
		}
		if (bDirtyMaterials) 
		{
			SetBoxAsDirtyImpl<FVoxelMaterial>(Data, Bounds, true);
		});
}

void UVoxelDataTools::SetBoxAsDirtyAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World,
	FVoxelIntBox Bounds, 
	bool bDirtyValues, 
	bool bDirtyMaterials, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, 
		if (bDirtyValues) 
		{
			SetBoxAsDirtyImpl<FVoxelValue>(Data, Bounds, true);
		}
		if (bDirtyMaterials) 
		{
			SetBoxAsDirtyImpl<FVoxelMaterial>(Data, Bounds, true);
		});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define FINDCLOSESTNONEMPTYVOXEL_PREFIX \
	const FVoxelVector Position = FVoxelToolHelpers::GetRealTemplate(World, InPosition, bConvertToVoxelSpace); \
	const FVoxelIntBox Bounds = FVoxelIntBox(Position, Position + 1);

FVoxelFindClosestNonEmptyVoxelResult UVoxelDataTools::FindClosestNonEmptyVoxelImpl(
	FVoxelData& Data,
	const FVoxelVector& Position, 
	bool bReadMaterial)
{
	FVoxelFindClosestNonEmptyVoxelResult Result;

	const FVoxelConstDataAccelerator Accelerator(Data);
	
	v_flt Distance = MAX_vflt;
	for (auto& Neighbor : FVoxelUtilities::GetNeighbors(Position))
	{
		const FVoxelValue Value = Accelerator.GetValue(Neighbor, 0);
		if (!Value.IsEmpty())
		{
			const v_flt PointDistance = (FVoxelVector(Neighbor) - Position).SizeSquared();
			if (PointDistance < Distance)
			{
				Distance = PointDistance;
				Result.bSuccess = true;
				Result.Position = Neighbor;
				Result.Value = Value.ToFloat();
			}
		}
	}

	if (Result.bSuccess && bReadMaterial)
	{
		Result.Material = Accelerator.GetMaterial(Result.Position, 0);
	}
	
	return Result;
}

void UVoxelDataTools::FindClosestNonEmptyVoxel(
	FVoxelFindClosestNonEmptyVoxelResult& Result,
	AVoxelWorld* World,
	FVector InPosition,
	bool bReadMaterial,
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, FINDCLOSESTNONEMPTYVOXEL_PREFIX, Result = FindClosestNonEmptyVoxelImpl(Data, Position, bReadMaterial));
}

void UVoxelDataTools::FindClosestNonEmptyVoxelAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	FVoxelFindClosestNonEmptyVoxelResult& Result,
	AVoxelWorld* World,
	FVector InPosition,
	bool bReadMaterial,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Result, Read, DoNotUpdateRender, FINDCLOSESTNONEMPTYVOXEL_PREFIX, InResult = FindClosestNonEmptyVoxelImpl(Data, Position, bReadMaterial));
}