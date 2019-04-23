// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelToolsHelpers.h"
#include "VoxelTools/VoxelLatentActionHelpers.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelData/VoxelSaveUtilities.h"

#define LOCTEXT_NAMESPACE "Voxel"

bool UVoxelDataTools::GetValue(float& Value, AVoxelWorld* World, const FIntVector& Position, bool bAllowFailure)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::Read>(__FUNCTION__, World, FIntBox(Position), false, bAllowFailure, [&](FVoxelData& Data)
	{
		Value = Data.GetValue(Position, 0).ToFloat();
	});
}

bool UVoxelDataTools::SetValue(AVoxelWorld* World, const FIntVector& Position, float Value, bool bUpdateRender, bool bAllowFailure)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, FIntBox(Position), bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		Data.SetValue(Position, Value);
	});
}

bool UVoxelDataTools::GetMaterial(FVoxelMaterial& Material, AVoxelWorld* World, const FIntVector& Position, bool bAllowFailure)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::Read>(__FUNCTION__, World, FIntBox(Position), false, bAllowFailure, [&](FVoxelData& Data)
	{
		Material = Data.GetMaterial(Position, 0);
	});
}

bool UVoxelDataTools::SetMaterial(AVoxelWorld* World, const FIntVector& Position, FVoxelMaterial Material, bool bUpdateRender, bool bAllowFailure)
{
	CHECK_VOXELWORLD_IS_CREATED();
	return FVoxelToolsHelpers::EditToolsHelper<EVoxelLockType::ReadWrite>(__FUNCTION__, World, FIntBox(Position), bUpdateRender, bAllowFailure, [&](FVoxelData& Data)
	{
		Data.SetMaterial(Position, Material);
	});
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::GetValueAsync(UObject* WorldContextObject, FLatentActionInfo LatentInfo, float& Value, AVoxelWorld* World, const FIntVector& Position)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelperWithValue(WorldContextObject, LatentInfo, __FUNCTION__, World, Value, [Position](FVoxelData& Data, float& Value)
	{
		FVoxelReadScopeLock Lock(Data, FIntBox(Position), __FUNCTION__);
		Value = Data.GetValue(Position, 0).ToFloat();
	});
}

void UVoxelDataTools::SetValueAsync(UObject* WorldContextObject, FLatentActionInfo LatentInfo, AVoxelWorld* World, const FIntVector& Position, float Value, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, FIntBox(Position), bUpdateRender, [Position, Value](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, FIntBox(Position), __FUNCTION__);
		Data.SetValue(Position, Value);
	});
}

void UVoxelDataTools::GetMaterialAsync(UObject* WorldContextObject, FLatentActionInfo LatentInfo, FVoxelMaterial& Material, AVoxelWorld* World, const FIntVector& Position)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelperWithValue(WorldContextObject, LatentInfo, __FUNCTION__, World, Material, [Position](FVoxelData& Data, FVoxelMaterial& Material)
	{
		FVoxelReadScopeLock Lock(Data, FIntBox(Position), __FUNCTION__);
		Material = Data.GetMaterial(Position, 0);
	});
}

void UVoxelDataTools::SetMaterialAsync(UObject* WorldContextObject, FLatentActionInfo LatentInfo, AVoxelWorld* World, const FIntVector& Position, FVoxelMaterial Material, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, FIntBox(Position), bUpdateRender, [Position, Material](FVoxelData& Data)
	{
		FVoxelReadWriteScopeLock Lock(Data, FIntBox(Position), __FUNCTION__);
		Data.SetMaterial(Position, Material);
	});
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
inline bool CheckSave(const FVoxelData& Data, const T& Save)
{
	if (Save.GetDepth() == -1)
	{
		FVoxelBPErrors::Error(LOCTEXT("LoadFromSaveInvalidDepth", "LoadFromSave: Invalid save (Depth == -1). You're trying to load a save object that wasn't initialized"));
		return false;
	}
	if (Save.GetDepth() > Data.Depth)
	{
		FVoxelBPErrors::Warning(LOCTEXT("LoadFromSaveDepthBigger", "LoadFromSave: Save depth is bigger than world depth, the save data outside world bounds will be ignored"));
	}
	return true;
}

#define CHECK_SAVE() \
if (!CheckSave(World->GetData(), Save)) \
{ \
	return; \
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

void UVoxelDataTools::LoadFromSave(AVoxelWorld* World, const FVoxelUncompressedWorldSave& Save, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_SAVE();

	TArray<FIntBox> BoundsToUpdate;
	auto& Data = World->GetData();
	Data.LoadFromSave(Save, BoundsToUpdate);
	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(BoundsToUpdate);
	}
}

void UVoxelDataTools::LoadFromCompressedSave(AVoxelWorld* World, const FVoxelCompressedWorldSave& Save, bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_SAVE();
	
	FVoxelUncompressedWorldSave UncompressedSave;
	UVoxelSaveUtilities::DecompressVoxelSave(Save, UncompressedSave);

	TArray<FIntBox> BoundsToUpdate;
	auto& Data = World->GetData();
	Data.LoadFromSave(UncompressedSave, BoundsToUpdate);
	if (bUpdateRender)
	{
		World->GetLODManager().UpdateBounds(BoundsToUpdate);
	}
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelDataTools::GetSaveAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelUncompressedWorldSave& OutSave)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelperWithValue(WorldContextObject, LatentInfo, __FUNCTION__, World, OutSave, [](FVoxelData& Data, FVoxelUncompressedWorldSave& Save)
	{
		Data.GetSave(Save);
	});
}

void UVoxelDataTools::GetCompressedSaveAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVoxelCompressedWorldSave& OutSave)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	FVoxelLatentActionHelpers::AsyncHelperWithValue(WorldContextObject, LatentInfo, __FUNCTION__, World, OutSave, [](FVoxelData& Data, FVoxelCompressedWorldSave& Save)
	{
		FVoxelUncompressedWorldSave UncompressedSave;
		Data.GetSave(UncompressedSave);
		UVoxelSaveUtilities::CompressVoxelSave(UncompressedSave, Save);
	});
}

template<typename TLambda>
bool LoadFromSaveAsyncHelper(
		UObject* WorldContextObject, 
		FLatentActionInfo LatentInfo,
		const FString& Name, 
		AVoxelWorld* World, 
		bool bUpdateRender, 
		TLambda Lambda)
{
	return FVoxelLatentActionHelpers::StartAction<FVoxelLatentActionAsyncWorkLambdaWithValue<TArray<FIntBox>>>(
			WorldContextObject, 
			LatentInfo, 
			World, 
			Name + ": Waiting for completion", 
			[&]()
		{
			return MakeShared<FVoxelLatentActionAsyncWorkLambdaWithValue<TArray<FIntBox>>>(World, Lambda);
		},
			[bUpdateRender](FVoxelLatentActionAsyncWorkLambdaWithValue<TArray<FIntBox>>& Work)
		{
			if (bUpdateRender)
			{
				Work.World->GetLODManager().UpdateBounds(Work.Value);
			}
		});
}

void UVoxelDataTools::LoadFromSaveAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FVoxelUncompressedWorldSave& Save,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_SAVE();
	LoadFromSaveAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, bUpdateRender, [SaveCopy = Save](FVoxelData& Data, TArray<FIntBox>& BoundsToUpdate)
	{
		Data.LoadFromSave(SaveCopy, BoundsToUpdate);
	});
}

void UVoxelDataTools::LoadFromCompressedSaveAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	const FVoxelCompressedWorldSave& Save,
	bool bUpdateRender)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	CHECK_SAVE();
	LoadFromSaveAsyncHelper(WorldContextObject, LatentInfo, __FUNCTION__, World, bUpdateRender, [SaveCopy = Save](FVoxelData& Data, TArray<FIntBox>& BoundsToUpdate)
	{
		FVoxelUncompressedWorldSave UncompressedSave;
		UVoxelSaveUtilities::DecompressVoxelSave(SaveCopy, UncompressedSave);
		Data.LoadFromSave(UncompressedSave, BoundsToUpdate);
	});
}

#undef LOCTEXT_NAMESPACE