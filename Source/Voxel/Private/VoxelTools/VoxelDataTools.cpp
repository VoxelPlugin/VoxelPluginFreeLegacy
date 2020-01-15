// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelData/VoxelDataAccelerator.h"

#include "Misc/ScopedSlowTask.h"

#define VOXEL_DATA_TOOL_PREFIX const FIntBox Bounds(Position);

void UVoxelDataTools::GetValue(float& Value, AVoxelWorld* World, FIntVector Position)
{
	VOXEL_TOOL_HELPER(Read, DoNotUpdateRender, VOXEL_DATA_TOOL_PREFIX, Value = Data.GetValue(Position, 0).ToFloat());
}

void UVoxelDataTools::GetInterpolatedValue(float& Value, AVoxelWorld* World, FVector Position)
{
	const FIntBox Bounds = FIntBox(Position).Extend(2);
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

void UVoxelDataTools::CacheValues(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelValue>(Bounds));
}

void UVoxelDataTools::CacheMaterials(AVoxelWorld* World, FIntBox Bounds)
{
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelMaterial>(Bounds));
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
	FIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelValue>(Bounds));
}

void UVoxelDataTools::CacheMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox Bounds,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, Data.CacheBounds<FVoxelMaterial>(Bounds));
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
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	World->GetData().GetSave(OutSave);
}

void UVoxelDataTools::GetCompressedSave(AVoxelWorld* World, FVoxelCompressedWorldSave& OutSave)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelUncompressedWorldSave Save;
	World->GetData().GetSave(Save);
	UVoxelSaveUtilities::CompressVoxelSave(Save, OutSave);
}

bool UVoxelDataTools::LoadFromSave(AVoxelWorld* World, const FVoxelUncompressedWorldSave& Save)
{
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_SAVE();

	TArray<FIntBox> BoundsToUpdate;
	auto& Data = World->GetData();
	const bool bSuccess = Data.LoadFromSave(World, Save, BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
	return bSuccess;
}

bool UVoxelDataTools::LoadFromCompressedSave(AVoxelWorld* World, const FVoxelCompressedWorldSave& Save)
{
	CHECK_VOXELWORLD_IS_CREATED();
	CHECK_SAVE();
	
	FVoxelUncompressedWorldSave UncompressedSave;
	UVoxelSaveUtilities::DecompressVoxelSave(Save, UncompressedSave);

	TArray<FIntBox> BoundsToUpdate;
	auto& Data = World->GetData();
	const bool bSuccess = Data.LoadFromSave(World, UncompressedSave, BoundsToUpdate);
	World->GetLODManager().UpdateBounds(BoundsToUpdate);
	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::RoundVoxelsImpl(FVoxelData& Data, const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	
	int32 NumDirtyLeaves = 0;
	uint64 NumVoxels = 0;
	{
		VOXEL_SCOPE_COUNTER("Record stats");
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<FVoxelValue>().IsDirty() && Leaf.GetData<FVoxelValue>().GetDataPtr())
			{
				NumDirtyLeaves++;
				NumVoxels += VOXELS_PER_DATA_CHUNK;
			}
		});
	}
	
	FScopedSlowTask SlowTask(NumDirtyLeaves, NSLOCTEXT("Voxel", "Rounding", "Rounding voxels"));
	FScopeToolsTimeLogger ToolsLogger(__FUNCTION__, NumVoxels);
		
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds.Extend(2));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<FVoxelValue>().IsDirty() && Leaf.GetData<FVoxelValue>().GetDataPtr())
		{
			SlowTask.EnterProgressFrame();
			
			const FIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const FVoxelValue& Value = Leaf.GetData<FVoxelValue>().GetDataPtr()[Index];
						
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
				// Not recorded in undo history, but not needed as RoundVoxels is lossless
				OctreeAccelerator.SetValue(X, Y, Z, bEmpty ? FVoxelValue::Empty() : FVoxelValue::Full());
			});
		}
	});
}

void UVoxelDataTools::RoundVoxels(AVoxelWorld* World, FIntBox InBounds)
{
	const FIntBox Bounds = InBounds.Extend(2);
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundVoxelsImpl(Data, InBounds));
}

void UVoxelDataTools::RoundVoxelsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox InBounds,
	bool bHideLatentWarnings)
{
	const FIntBox Bounds = InBounds.Extend(2);
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, RoundVoxelsImpl(Data, InBounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::ClearUnusedMaterialsImpl(FVoxelData& Data, const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	
	int32 NumDirtyLeaves = 0;
	uint64 NumVoxels = 0;
	{
		VOXEL_SCOPE_COUNTER("Record stats");
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<FVoxelMaterial>().IsDirty() && Leaf.GetData<FVoxelMaterial>().GetDataPtr())
			{
				NumDirtyLeaves++;
				NumVoxels += VOXELS_PER_DATA_CHUNK;
			}
		});
	}
	
	FScopedSlowTask SlowTask(NumDirtyLeaves, NSLOCTEXT("Voxel", "Clearing", "Clearing unused materials"));
	FScopeToolsTimeLogger ToolsLogger(__FUNCTION__, NumVoxels);
		
	FVoxelMutableDataAccelerator OctreeAccelerator(Data, Bounds.Extend(2));
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<FVoxelMaterial>().IsDirty() && Leaf.GetData<FVoxelMaterial>().GetDataPtr())
		{
			SlowTask.EnterProgressFrame();
			
			const FIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const FVoxelMaterial& Material = Leaf.GetData<FVoxelMaterial>().GetDataPtr()[Index];

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
				// Note: not recorded in undo history
				OctreeAccelerator.SetMaterial(X, Y, Z, FVoxelMaterial::Default());
			});
		}
	});
}

void UVoxelDataTools::ClearUnusedMaterials(AVoxelWorld* World, FIntBox InBounds)
{
	const FIntBox Bounds = InBounds.Extend(1);
	VOXEL_TOOL_HELPER(Write, DoNotUpdateRender, NO_PREFIX, ClearUnusedMaterialsImpl(Data, InBounds));
}

void UVoxelDataTools::ClearUnusedMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FIntBox InBounds,
	bool bHideLatentWarnings)
{
	const FIntBox Bounds = InBounds.Extend(1);
	VOXEL_TOOL_LATENT_HELPER(Write, DoNotUpdateRender, NO_PREFIX, ClearUnusedMaterialsImpl(Data, InBounds));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::GetVoxelsValueAndMaterialImpl(
	FVoxelData& Data,
	TArray<FVoxelValueMaterial>& Voxels,
	const FIntBox& Bounds,
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
	
	const FIntBox Bounds(Positions);
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
	
	const FIntBox Bounds(Positions);
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(Voxels, Read, DoNotUpdateRender, NO_PREFIX, GetVoxelsValueAndMaterialImpl(Data, InVoxels, Bounds, Positions));
}