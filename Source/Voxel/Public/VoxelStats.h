// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "VoxelDefinitions.h"

DECLARE_STATS_GROUP(TEXT("Voxel"), STATGROUP_Voxel, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Slow"), STATGROUP_VoxelSlow, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Render"), STATGROUP_VoxelRender, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Async"), STATGROUP_VoxelAsync, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Async Verbose"), STATGROUP_VoxelAsyncVerbose, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Counters"), STATGROUP_VoxelCounters, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Memory"), STATGROUP_VoxelMemory, STATCAT_Advanced);

#if STATS
struct FStat_Voxel_Base
{
	static FORCEINLINE EStatDataType::Type GetStatType()
	{
		return EStatDataType::ST_int64;
	}
	static FORCEINLINE bool IsClearEveryFrame()
	{
		return true;
	}
	static FORCEINLINE bool IsCycleStat()
	{
		return true;
	}
	static FORCEINLINE FPlatformMemory::EMemoryCounterRegion GetMemoryRegion()
	{
		return FPlatformMemory::MCR_Invalid;
	}
};
template<typename T>
struct TStat_Voxel_Initializer
{
	TStat_Voxel_Initializer(const FString& Description)
	{
		T::GetDescriptionRef() = Description;
	}
};

#define VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(Suffix) PREPROCESSOR_JOIN(PREPROCESSOR_JOIN(FStat_Voxel_, __LINE__), Suffix)

// We want to be able to use __FUNCTION__ as description, so it's a bit tricky
#define VOXEL_SCOPE_COUNTER_IMPL_IMPL(StatGroup, Description) \
	struct VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(PREPROCESSOR_NOTHING) : FStat_Voxel_Base \
	{ \
		using TGroup = FStatGroup_##StatGroup; \
		\
		static FORCEINLINE FString& GetDescriptionRef() \
		{ \
			static FString StaticDescription; \
			return StaticDescription; \
		} \
		static FORCEINLINE const char* GetStatName() \
		{ \
			return PREPROCESSOR_TO_STRING(VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(_Name)); \
		} \
		static FORCEINLINE const TCHAR* GetDescription() \
		{ \
			return *GetDescriptionRef(); \
		} \
	}; \
	static FThreadSafeStaticStat<VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(PREPROCESSOR_NOTHING)> VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(_Ptr); \
	static TStat_Voxel_Initializer<VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(PREPROCESSOR_NOTHING)> VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(_Initializer){ Description }; \
	FScopeCycleCounter VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(_CycleCount)(VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(_Ptr.GetStatId()));

#else
#define VOXEL_SCOPE_COUNTER_IMPL_IMPL(StatGroup, Description)
#endif

VOXEL_API FString VoxelStats_RemoveLambdaFromFunctionName(const FString& FunctionName);

#define VOXEL_INLINE_COUNTER_IMPL(Macro, Name, ...) ([&]() -> decltype(auto) { Macro(VoxelStats_RemoveLambdaFromFunctionName(__FUNCTION__) + TEXT(".") + Name); return __VA_ARGS__; }())

#define VOXEL_SCOPE_COUNTER_NAME(Description) __FUNCTION__ + FString(TEXT(".")) + Description

#define VOXEL_SCOPE_COUNTER_IMPL(Description) ensureVoxelSlowNoSideEffects(IsInGameThread()); VOXEL_SCOPE_COUNTER_IMPL_IMPL(STATGROUP_Voxel, Description)
#define VOXEL_SCOPE_COUNTER(Description) VOXEL_SCOPE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_NAME(Description))
#define VOXEL_FUNCTION_COUNTER() VOXEL_SCOPE_COUNTER_IMPL(__FUNCTION__)
#define VOXEL_INLINE_COUNTER(Name, ...) VOXEL_INLINE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_IMPL, Name, __VA_ARGS__)

#define VOXEL_RENDER_SCOPE_COUNTER_IMPL(Description) ensureVoxelSlowNoSideEffects(IsInRenderingThread()); VOXEL_SCOPE_COUNTER_IMPL_IMPL(STATGROUP_VoxelRender, Description)
#define VOXEL_RENDER_SCOPE_COUNTER(Description) VOXEL_RENDER_SCOPE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_NAME(Description))
#define VOXEL_RENDER_FUNCTION_COUNTER() VOXEL_RENDER_SCOPE_COUNTER_IMPL(__FUNCTION__)
#define VOXEL_RENDER_INLINE_COUNTER(Name, ...) VOXEL_INLINE_COUNTER_IMPL(VOXEL_RENDER_SCOPE_COUNTER_IMPL, Name, __VA_ARGS__)

#define VOXEL_ASYNC_SCOPE_COUNTER_IMPL(Description) VOXEL_SCOPE_COUNTER_IMPL_IMPL(STATGROUP_VoxelAsync, Description)
#define VOXEL_ASYNC_SCOPE_COUNTER(Description) VOXEL_ASYNC_SCOPE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_NAME(Description))
#define VOXEL_ASYNC_FUNCTION_COUNTER() VOXEL_ASYNC_SCOPE_COUNTER_IMPL(__FUNCTION__)
#define VOXEL_ASYNC_INLINE_COUNTER(Name, ...) VOXEL_INLINE_COUNTER_IMPL(VOXEL_ASYNC_SCOPE_COUNTER_IMPL, Name, __VA_ARGS__)

#define VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER_IMPL(Description) VOXEL_SCOPE_COUNTER_IMPL_IMPL(STATGROUP_VoxelAsyncVerbose, Description)
#define VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER(Description) VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_NAME(Description))
#define VOXEL_ASYNC_VERBOSE_FUNCTION_COUNTER() VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER_IMPL(__FUNCTION__)
#define VOXEL_ASYNC_VERBOSE_INLINE_COUNTER(Name, ...) VOXEL_INLINE_COUNTER_IMPL(VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER_IMPL, Name, __VA_ARGS__)

#if VOXEL_SLOW_STATS
#define VOXEL_SLOW_SCOPE_COUNTER_IMPL(Description) VOXEL_SCOPE_COUNTER_IMPL_IMPL(STATGROUP_VoxelSlow, Description)
#else
#define VOXEL_SLOW_SCOPE_COUNTER_IMPL(Description)
#endif
#define VOXEL_SLOW_SCOPE_COUNTER(Description) VOXEL_SLOW_SCOPE_COUNTER_IMPL(VOXEL_SCOPE_COUNTER_NAME(Description))
#define VOXEL_SLOW_FUNCTION_COUNTER() VOXEL_SLOW_SCOPE_COUNTER_IMPL(__FUNCTION__)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if ENABLE_VOXEL_MEMORY_STATS
struct FVoxelMemoryCounterRef
{
	FThreadSafeCounter64* UsageCounterPtr = nullptr;
	FThreadSafeCounter64* PeakCounterPtr = nullptr;
};
VOXEL_API TMap<const TCHAR*, FVoxelMemoryCounterRef>& GetVoxelMemoryCounters();

struct FVoxelMemoryCounterStaticRef
{
	VOXEL_API FVoxelMemoryCounterStaticRef(const TCHAR* Name, const FVoxelMemoryCounterRef& Ref);
};

#define VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName) PREPROCESSOR_JOIN(StatName, _MemoryUsage)
#define VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName) PREPROCESSOR_JOIN(StatName, _MemoryPeak)

#define DECLARE_VOXEL_MEMORY_STAT(Name, StatName, Group, API) \
	extern API FThreadSafeCounter64 VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName); \
	extern API FThreadSafeCounter64 VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName); \
	inline const TCHAR* Get ## StatName ## StaticName() { return Name; } \
	DECLARE_MEMORY_STAT_EXTERN(Name, StatName ## _Stat, Group, API)

#define DEFINE_VOXEL_MEMORY_STAT(StatName) \
	FThreadSafeCounter64 VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName); \
	FThreadSafeCounter64 VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName); \
	static FVoxelMemoryCounterStaticRef StaticRef ## StatName(Get ## StatName ## StaticName(), FVoxelMemoryCounterRef{ &VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName), &VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName) }); \
	DEFINE_STAT(StatName ## _Stat)

#define INC_VOXEL_MEMORY_STAT_BY_IMPL(StatName, Amount) \
	INC_MEMORY_STAT_BY(StatName ## _Stat, Amount) \
	VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName).Set(FMath::Max<uint64>( \
		VOXEL_MEMORY_PEAK_COUNTER_NAME(StatName).GetValue(), \
		Amount + VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName).Add(Amount))); // Max is not atomic, but w/e should be fine anyways

#define DEC_VOXEL_MEMORY_STAT_BY_IMPL(StatName, Amount) \
	DEC_MEMORY_STAT_BY(StatName ## _Stat, Amount); \
	VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName).Subtract(Amount); \
	ensureVoxelSlowNoSideEffects(VOXEL_MEMORY_USAGE_COUNTER_NAME(StatName).GetValue() >= 0);

///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_MEMORY_STAT(TEXT("Total Voxel Memory"), STAT_TotalVoxelMemory, STATGROUP_VoxelMemory, VOXEL_API);

#define INC_VOXEL_MEMORY_STAT_BY(StatName, Amount) \
	INC_VOXEL_MEMORY_STAT_BY_IMPL(StatName, Amount) \
	INC_VOXEL_MEMORY_STAT_BY_IMPL(STAT_TotalVoxelMemory, Amount)

#define DEC_VOXEL_MEMORY_STAT_BY(StatName, Amount) \
	DEC_VOXEL_MEMORY_STAT_BY_IMPL(StatName, Amount) \
	DEC_VOXEL_MEMORY_STAT_BY_IMPL(STAT_TotalVoxelMemory, Amount)

#else
#define DECLARE_VOXEL_MEMORY_STAT(Name, StatName, Group, API) DECLARE_MEMORY_STAT_EXTERN(Name, StatName ## _Stat, Group, API)
#define DEFINE_VOXEL_MEMORY_STAT(StatName) DEFINE_STAT(StatName ## _Stat)

#define INC_VOXEL_MEMORY_STAT_BY(StatName, Amount) INC_MEMORY_STAT_BY(StatName ## _Stat, Amount);
#define DEC_VOXEL_MEMORY_STAT_BY(StatName, Amount) DEC_MEMORY_STAT_BY(StatName ## _Stat, Amount);
#endif