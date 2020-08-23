// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"

#define GENERATED_TOOL_FUNCTION_IMPL(Type) \
	if (!VoxelWorld) \
	{ \
		FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World is invalid!"), *FString(__FUNCTION__))); \
		return; \
	} \
	if (!VoxelWorld->IsCreated()) \
	{ \
		FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World isn't created!"), *FString(__FUNCTION__))); \
		return; \
	}

#define GENERATED_TOOL_FUNCTION(Type) \
	VOXEL_FUNCTION_COUNTER(); \
	GENERATED_TOOL_FUNCTION_IMPL(Type)

#define GENERATED_TOOL_FUNCTION_ASYNC(Type) GENERATED_TOOL_FUNCTION(Type)

#define GENERATED_TOOL_FUNCTION_CPP(Type) \
	VOXEL_FUNCTION_COUNTER(); \
	GENERATED_TOOL_FUNCTION_IMPL(Type)

#define GENERATED_TOOL_FUNCTION_ASYNC_CPP(Type) GENERATED_TOOL_FUNCTION_CPP(Type)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_TOOL_PREFIX(Type) \
	if (!Bounds.IsValid()) \
	{ \
		FVoxelMessages::Error(FString::Printf(TEXT("%s: Invalid Bounds! %s"), *FString(__FUNCTION__), *Bounds.ToString())); \
		return; \
	}

#define GENERATED_TOOL_SUFFIX(Type) \
	if (bUpdateRender) \
	{ \
		FVoxelToolHelpers::UpdateWorld(VoxelWorld, Bounds); \
	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_TOOL_CALL(Type, ...) \
	GENERATED_TOOL_PREFIX(Type) \
	EditedBounds = Bounds; \
	auto& WorldData = VoxelWorld->GetData(); \
	{ \
		FVoxelWriteScopeLock Lock(WorldData, Bounds, FUNCTION_FNAME); \
		auto Data = TVoxelDataImpl<FModifiedVoxel##Type>(WorldData, bMultiThreaded, bRecordModified##Type##s); \
		__VA_ARGS__; \
		Modified##Type##s = MoveTemp(Data.ModifiedValues); \
	} \
	GENERATED_TOOL_SUFFIX(Type)

#define GENERATED_TOOL_CALL_ASYNC(Type, ...) \
	GENERATED_TOOL_PREFIX(Type) \
	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld_WithValue( \
		WorldContextObject, \
		LatentInfo, \
		VoxelWorld, \
		FUNCTION_FNAME, \
		bHideLatentWarnings, \
		Modified##Type##s, \
		[=](FVoxelData& WorldData, TArray<FModifiedVoxel##Type>& OutModified) \
		{ \
			FVoxelWriteScopeLock Lock(WorldData, Bounds, FUNCTION_FNAME); \
			auto Data = TVoxelDataImpl<FModifiedVoxel##Type>(WorldData, bMultiThreaded, bRecordModified##Type##s); \
			__VA_ARGS__; \
			OutModified = MoveTemp(Data.ModifiedValues); \
		}, \
		bUpdateRender ? EVoxelUpdateRender::UpdateRender : EVoxelUpdateRender::DoNotUpdateRender, \
		Bounds, \
		[=, &EditedBounds]() \
		{ \
			EditedBounds = Bounds; \
		});

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_TOOL_CALL_CPP(Type, ...) \
	GENERATED_TOOL_PREFIX(Type) \
	if (OutEditedBounds) *OutEditedBounds = Bounds; \
	auto& WorldData = VoxelWorld->GetData(); \
	{ \
		FVoxelWriteScopeLock Lock(WorldData, Bounds, FUNCTION_FNAME); \
		auto Data = TVoxelDataImpl<FModifiedVoxel##Type>(WorldData, bMultiThreaded, OutModified##Type##s != nullptr); \
		__VA_ARGS__; \
		if (OutModified##Type##s) \
		{ \
			if (OutModified##Type##s->Num() == 0) \
			{ \
				*OutModified##Type##s = MoveTemp(Data.ModifiedValues); \
			} \
			else \
			{ \
				OutModified##Type##s->Append(MoveTemp(Data.ModifiedValues)); \
			} \
		} \
	} \
	GENERATED_TOOL_SUFFIX(Type)

#define GENERATED_TOOL_CALL_ASYNC_CPP(Type, ...) \
	GENERATED_TOOL_PREFIX(Type) \
	if (OutEditedBounds) *OutEditedBounds = Bounds; \
	const auto GameThreadTasks = VoxelWorld->GetGameThreadTasks(); \
	auto* Work = new FVoxelToolAsyncWork(FUNCTION_FNAME, *VoxelWorld, [=](FVoxelData& WorldData) \
	{ \
		FVoxelWriteScopeLock Lock(WorldData, Bounds, FUNCTION_FNAME); \
		auto Data = TVoxelDataImpl<FModifiedVoxel##Type>(WorldData, bMultiThreaded, bRecordModified##Type##s); \
		__VA_ARGS__; \
		GameThreadTasks->AddTask([=, Modified = MoveTemp(Data.ModifiedValues)]() \
		{ \
			check(IsInGameThread()); \
			/* Validity of Voxel world is guaranteed by it being queued on the world */ \
			GENERATED_TOOL_SUFFIX(Type); \
			Callback.ExecuteIfBound(Modified); \
		}); \
	}); \
	FVoxelToolHelpers::StartAsyncEditTask(VoxelWorld, Work);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Will autodelete
class FVoxelToolAsyncWork : public FVoxelAsyncWork
{
public:
	const TVoxelWeakPtr<FVoxelData> Data;
	const TFunction<void(FVoxelData&)> Function;
	
	explicit FVoxelToolAsyncWork(FName Name, AVoxelWorld& World, TFunction<void(FVoxelData&)>&& Function)
		: FVoxelAsyncWork(Name, 1e9, true)
		, Data(World.GetDataSharedPtr())
		, Function(MoveTemp(Function))
	{
	}

	//~ Begin IVoxelQueuedWork Interface
	virtual uint32 GetPriority() const override
	{
		return 0;
	}
	virtual void DoWork() override
	{
		const auto PinnedData = Data.Pin();
		if (PinnedData.IsValid())
		{
			Function(*PinnedData);
		}
	}
	//~ End IVoxelQueuedWork Interface
};