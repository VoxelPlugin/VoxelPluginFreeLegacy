// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelWorld.h"
#include "VoxelAsyncWork.h"
#include "VoxelMessages.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataLock.h"

// TODO REMOVE
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"

#include "LatentActions.h"
#include "Engine/LatentActionManager.h"

class AVoxelWorld;
class FVoxelData;
class FPendingLatentAction;
class FVoxelLatentActionAsyncWork;
struct FLatentActionInfo;
enum class EVoxelLockType;

enum class EVoxelUpdateRender
{
	UpdateRender,
	DoNotUpdateRender
};

class VOXEL_API FVoxelLatentActionAsyncWork : public FVoxelAsyncWorkWithWait
{
public:
	explicit FVoxelLatentActionAsyncWork(FName Name);

	//~ Begin IVoxelQueuedWork Interface
	virtual uint32 GetPriority() const override;
	//~ End IVoxelQueuedWork Interface

	//~ Begin FVoxelLatentActionAsyncWork Interface
	// Called on the game thread
	virtual bool IsValid() const = 0;
	//~ End FVoxelLatentActionAsyncWork Interface

protected:
	~FVoxelLatentActionAsyncWork() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};

class VOXEL_API FVoxelLatentActionAsyncWork_WithWorld : public FVoxelLatentActionAsyncWork
{
public:
	const TWeakObjectPtr<AVoxelWorld> World;
	const TVoxelWeakPtr<FVoxelData> Data;
	const TFunction<void(FVoxelData&)> Function;

	FVoxelLatentActionAsyncWork_WithWorld(FName Name, TWeakObjectPtr<AVoxelWorld> World, TFunction<void(FVoxelData&)> Function);

	//~ Begin FVoxelLatentActionAsyncWork Interface
	virtual void DoWork() override;
	virtual bool IsValid() const override;
	//~ End FVoxelLatentActionAsyncWork Interface

protected:
	~FVoxelLatentActionAsyncWork_WithWorld() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};

class VOXEL_API FVoxelLatentActionAsyncWork_WithoutWorld : public FVoxelLatentActionAsyncWork
{
public:
	const TFunction<void()> Function;
	// Called on the game thread
	const TFunction<bool()> IsValidLambda;

	FVoxelLatentActionAsyncWork_WithoutWorld(FName Name, TFunction<void()> Function, TFunction<bool()> IsValidLambda);

	//~ Begin FVoxelLatentActionAsyncWork Interface
	virtual void DoWork() override;
	virtual bool IsValid() const override;
	//~ End FVoxelLatentActionAsyncWork Interface

protected:
	~FVoxelLatentActionAsyncWork_WithoutWorld() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};

template<typename TValue>
class TVoxelLatentActionAsyncWork_WithWorld_WithValue : public FVoxelLatentActionAsyncWork_WithWorld
{
public:
	TValue Value;
	
	TVoxelLatentActionAsyncWork_WithWorld_WithValue(FName Name, TWeakObjectPtr<AVoxelWorld> World, TFunction<void(FVoxelData&, TValue&)> InFunction)
		: FVoxelLatentActionAsyncWork_WithWorld(Name, World, [InFunction, this](FVoxelData& InData) { InFunction(InData, this->Value); })
	{
	}

protected:
	~TVoxelLatentActionAsyncWork_WithWorld_WithValue() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};

template<typename TValue>
class TVoxelLatentActionAsyncWork_WithoutWorld_WithValue : public FVoxelLatentActionAsyncWork_WithoutWorld
{
public:
	TValue Value;
	
	TVoxelLatentActionAsyncWork_WithoutWorld_WithValue(FName Name, TFunction<void(TValue&)> InFunction, TFunction<bool()> IsValidLambda)
		: FVoxelLatentActionAsyncWork_WithoutWorld(Name, [InFunction, this]() { InFunction(this->Value); }, MoveTemp(IsValidLambda))
	{
	}

protected:
	~TVoxelLatentActionAsyncWork_WithoutWorld_WithValue() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};

template<typename TWork>
class TVoxelLatentAction : public FPendingLatentAction
{
public:
	const FName ExecutionFunction;
	const int32 OutputLink;
	const FWeakObjectPtr CallbackTarget;
	
	const TUniquePtr<TWork, TVoxelAsyncWorkDelete<TWork>> Work;
	const TFunction<void(TWork&)> GameThreadCallback;
	const FName Name;

	TVoxelLatentAction(
		const FLatentActionInfo& LatentInfo,
		TWork* Work,
		FName Name,
		TFunction<void(TWork&)> GameThreadCallback)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)

		, Work(Work)
		, GameThreadCallback(MoveTemp(GameThreadCallback))
		, Name(Name)
	{
		check(Work);
	}
	virtual ~TVoxelLatentAction() override
	{
		if (!Work->IsDone())
		{
			const double StartTime = FPlatformTime::Seconds();
			Work->WaitForCompletion();
			const double Elapsed = FPlatformTime::Seconds() - StartTime;
			if (Elapsed > 0.001)
			{
				LOG_VOXEL(
					Warning, 
					TEXT("Voxel Latent Action: waited %fs for %s on game thread. This is likely because the object that triggered the latent call was destroyed."),
					Elapsed,
					*Name.ToString());
			}
		}
		if (!bCallbackCalled && Work->IsValid() && !Work->WasAbandoned())
		{
			// Always call callback
			GameThreadCallback(*Work);
		}
	}

	//~ Begin FPendingLatentAction Interface
	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		const bool bFinished = Work->IsDone();
		if (bFinished && ensure(!bCallbackCalled) && Work->IsValid() && !Work->WasAbandoned())
		{
			bCallbackCalled = true;
			GameThreadCallback(*Work);
		}
		Response.FinishAndTriggerIf(bFinished, ExecutionFunction, OutputLink, CallbackTarget);
	}
#if WITH_EDITOR
	virtual FString GetDescription() const override
	{
		return FString::Printf(TEXT("%s: Waiting for completion"), *Name.ToString());
	}
#endif
	//~ End FPendingLatentAction Interface

private:
	bool bCallbackCalled = false;
};

struct VOXEL_API FVoxelToolHelpers
{
	// Avoids having to include the LOD Manager header in every tool file
	static void UpdateWorld(AVoxelWorld* World, const FVoxelIntBox& Bounds);
	// If World is null, will start an async on AnyThread. Else will use the voxel world thread pool.
	static void StartAsyncEditTask(AVoxelWorld* World, IVoxelQueuedWork* Work);

	static float GetRealDistance(AVoxelWorld* World, float Distance, bool bConvertToVoxelSpace);
	static FVoxelVector GetRealPosition(AVoxelWorld* World, const FVector& Position, bool bConvertToVoxelSpace);
	static FTransform GetRealTransform(AVoxelWorld* World, FTransform Transform, bool bConvertToVoxelSpace);

	template<typename T>
	static auto GetRealTemplate(AVoxelWorld* World, T Value, bool bConvertToVoxelSpace);

	static bool StartLatentAction(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FName Name,
		bool bHideLatentWarnings,
		TFunction<FPendingLatentAction*()> CreateLatentAction);
	
	template<typename TWork, typename TCreateWork>
	static bool StartAsyncLatentActionImpl(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FName Name,
		bool bHideLatentWarnings,
		TCreateWork CreateWork,
		TFunction<void(TWork&)> GameThreadCallback)
	{
		return StartLatentAction(WorldContextObject, LatentInfo, Name, bHideLatentWarnings, [&]()
		{
			TWork* Work = CreateWork();
			StartAsyncEditTask(World, Work);
			return new TVoxelLatentAction<TWork>(LatentInfo, Work, Name, MoveTemp(GameThreadCallback));
		});
	}

	static bool StartAsyncLatentAction_WithWorld(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FName Name,
		bool bHideLatentWarnings,
		TFunction<void(FVoxelData&)> DoWork,
		EVoxelUpdateRender UpdateRender,
		const FVoxelIntBox& BoundsToUpdate);
	static bool StartAsyncLatentAction_WithoutWorld(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FName Name,
		bool bHideLatentWarnings,
		TFunction<void()> DoWork,
		TFunction<bool()> IsValid = []() { return true; });
	
	template<typename T, typename TDoWork>
	static bool StartAsyncLatentAction_WithWorld_WithValue(
		UObject* WorldContextObject, 
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FName Name, 
		bool bHideLatentWarnings,
		T& Value,
		TDoWork DoWork,
		EVoxelUpdateRender UpdateRender,
		const FVoxelIntBox& BoundsToUpdate,
		TFunction<void()> GameThreadCallback = nullptr)
	{
		using FWork = TVoxelLatentActionAsyncWork_WithWorld_WithValue<T>;
		return StartAsyncLatentActionImpl<FWork>(
			WorldContextObject,
			LatentInfo,
			World,
			Name,
			bHideLatentWarnings,
			[&]() { return new FWork(Name, World, DoWork); },
			[=, WeakWorldContextObject = MakeWeakObjectPtr(WorldContextObject), &Value](FWork& Work)
			{
				if (WeakWorldContextObject.IsValid())
				{
					Value = MoveTemp(Work.Value);
					if (GameThreadCallback)
					{
						GameThreadCallback();
					}
				}
				if (UpdateRender == EVoxelUpdateRender::UpdateRender && Work.World.IsValid())
				{
					UpdateWorld(Work.World.Get(), BoundsToUpdate);
				}
			});
	}
	template<typename T, typename TDoWork>
	static bool StartAsyncLatentAction_WithoutWorld_WithValue(
		UObject* WorldContextObject, 
		FLatentActionInfo LatentInfo,
		FName Name, 
		bool bHideLatentWarnings,
		T& Value,
		TDoWork DoWork,
		TFunction<bool()> IsValid = []() { return true; })
	{
		using FWork = TVoxelLatentActionAsyncWork_WithoutWorld_WithValue<T>;
		return StartAsyncLatentActionImpl<FWork>(
			WorldContextObject,
			LatentInfo,
			nullptr,
			Name,
			bHideLatentWarnings,
			[&]() { return new FWork(Name, DoWork, MoveTemp(IsValid)); },
			[=, WeakWorldContextObject = MakeWeakObjectPtr(WorldContextObject), &Value](FWork& Work)
			{
				if (WeakWorldContextObject.IsValid())
				{
					Value = MoveTemp(Work.Value);
				}
			});
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
inline auto FVoxelToolHelpers::GetRealTemplate<float>(AVoxelWorld* World, float Value, bool bConvertToVoxelSpace)
{
	return GetRealDistance(World, Value, bConvertToVoxelSpace);
}
template<>
inline auto FVoxelToolHelpers::GetRealTemplate<FVector>(AVoxelWorld* World, FVector Value, bool bConvertToVoxelSpace)
{
	return GetRealPosition(World, Value, bConvertToVoxelSpace);
}
template<>
inline auto FVoxelToolHelpers::GetRealTemplate<FTransform>(AVoxelWorld* World, FTransform Value, bool bConvertToVoxelSpace)
{
	return GetRealTransform(World, Value, bConvertToVoxelSpace);
}

#define GET_VOXEL_TOOL_REAL(Value) FVoxelToolHelpers::GetRealTemplate(World, Value, bConvertToVoxelSpace)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_VOXELWORLD_IS_CREATED_IMPL(World, ReturnValue) \
if (!World) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World is invalid!"), *FString(__FUNCTION__))); \
	return ReturnValue; \
} \
if (!World->IsCreated()) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World isn't created!"), *FString(__FUNCTION__))); \
	return ReturnValue; \
}
#define CHECK_VOXELWORLD_IS_CREATED() CHECK_VOXELWORLD_IS_CREATED_IMPL(World, {});
#define CHECK_VOXELWORLD_IS_CREATED_VOID() CHECK_VOXELWORLD_IS_CREATED_IMPL(World, PREPROCESSOR_NOTHING);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_OBJECT_PARAMETER_IMPL(Object, ReturnValue) \
if (!Object) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: "#Object" is invalid!"), *FString(__FUNCTION__))); \
	return ReturnValue; \
}
#define CHECK_OBJECT_PARAMETER(Object) CHECK_OBJECT_PARAMETER_IMPL(Object, {});
#define CHECK_OBJECT_PARAMETER_VOID(Object) CHECK_OBJECT_PARAMETER_IMPL(Object, PREPROCESSOR_NOTHING);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE_IMPL(ReturnValue) \
if (!World && bConvertToVoxelSpace) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: Voxel World is invalid, but bConvertToVoxelSpace = true!"), *FString(__FUNCTION__))); \
	return ReturnValue; \
}
#define CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE() CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE_IMPL({});
#define CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE_VOID() CHECK_VOXELWORLD_FOR_CONVERT_TO_VOXEL_SPACE_IMPL(PREPROCESSOR_NOTHING);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_BOUNDS_ARE_VALID_IMPL(ReturnValue) \
if (!Bounds.IsValid()) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: Invalid Bounds! %s"), *FString(__FUNCTION__), *Bounds.ToString())); \
	return ReturnValue; \
}
#define CHECK_BOUNDS_ARE_VALID() CHECK_BOUNDS_ARE_VALID_IMPL({});
#define CHECK_BOUNDS_ARE_VALID_VOID() CHECK_BOUNDS_ARE_VALID_IMPL(PREPROCESSOR_NOTHING);

#define CHECK_BOUNDS_ARE_32BITS_IMPL(ReturnValue) \
if (!FVoxelUtilities::CountIs32Bits(Bounds.Size())) \
{ \
	FVoxelMessages::Error(FString::Printf(TEXT("%s: Bounds size is too big! %s"), *FString(__FUNCTION__), *Bounds.ToString())); \
	return ReturnValue; \
}
#define CHECK_BOUNDS_ARE_32BITS() CHECK_BOUNDS_ARE_32BITS_IMPL({});
#define CHECK_BOUNDS_ARE_32BITS_VOID() CHECK_BOUNDS_ARE_32BITS_IMPL(PREPROCESSOR_NOTHING);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_TOOL_HELPER_BODY(InLockType, InUpdateRender, ...) \
	auto& Data = World->GetData(); \
	{ \
		TVoxelScopeLock<EVoxelLockType::InLockType> Lock(Data, Bounds, FUNCTION_FNAME); \
		__VA_ARGS__; \
	} \
	if (EVoxelUpdateRender::InUpdateRender == EVoxelUpdateRender::UpdateRender) \
	{ \
		FVoxelToolHelpers::UpdateWorld(World, Bounds); \
	}

#define VOXEL_TOOL_LATENT_HELPER_BODY(InLockType, InUpdateRender, ...) \
	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld( \
		WorldContextObject, \
		LatentInfo, \
		World, \
		FUNCTION_FNAME, \
		bHideLatentWarnings, \
		[=](FVoxelData& Data) \
		{ \
			TVoxelScopeLock<EVoxelLockType::InLockType> Lock(Data, Bounds, FUNCTION_FNAME); \
			__VA_ARGS__; \
		}, \
		EVoxelUpdateRender::InUpdateRender, \
		Bounds);

#define VOXEL_TOOL_LATENT_HELPER_WITH_VALUE_BODY(InValue, InLockType, InUpdateRender, ...) \
	FVoxelToolHelpers::StartAsyncLatentAction_WithWorld_WithValue( \
		WorldContextObject, \
		LatentInfo, \
		World, \
		FUNCTION_FNAME, \
		bHideLatentWarnings, \
		InValue, \
		[=](FVoxelData& Data, decltype(InValue) In ## InValue) \
		{ \
			static_assert(TIsReferenceType<decltype(InValue)>::Value, "Value is not a reference!"); \
			static_assert(!TIsConst<decltype(InValue)>::Value, "Value is const!"); \
			TVoxelScopeLock<EVoxelLockType::InLockType> Lock(Data, Bounds, FUNCTION_FNAME); \
			__VA_ARGS__; \
		}, \
		EVoxelUpdateRender::InUpdateRender, \
		Bounds);

#define VOXEL_TOOL_HELPER(InLockType, InUpdateRender, Prefix, ...) \
	VOXEL_FUNCTION_COUNTER(); \
	CHECK_VOXELWORLD_IS_CREATED_VOID(); \
	Prefix \
	CHECK_BOUNDS_ARE_VALID_VOID(); \
	VOXEL_TOOL_HELPER_BODY(InLockType, InUpdateRender, __VA_ARGS__)

#define VOXEL_TOOL_LATENT_HELPER(InLockType, InUpdateRender, Prefix, ...) \
	VOXEL_FUNCTION_COUNTER(); \
	CHECK_VOXELWORLD_IS_CREATED_VOID(); \
	Prefix \
	CHECK_BOUNDS_ARE_VALID_VOID(); \
	VOXEL_TOOL_LATENT_HELPER_BODY(InLockType, InUpdateRender, __VA_ARGS__)

#define VOXEL_TOOL_LATENT_HELPER_WITH_VALUE(InValue, InLockType, InUpdateRender, Prefix, ...) \
	VOXEL_FUNCTION_COUNTER(); \
	CHECK_VOXELWORLD_IS_CREATED_VOID(); \
	Prefix \
	CHECK_BOUNDS_ARE_VALID_VOID(); \
	VOXEL_TOOL_LATENT_HELPER_WITH_VALUE_BODY(InValue, InLockType, InUpdateRender, __VA_ARGS__)

#define NO_PREFIX