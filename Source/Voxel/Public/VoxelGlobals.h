// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"
#include "Stats/Stats.h"
#include "Launch/Resources/Version.h"
#include "Misc/ScopedSlowTask.h"
#include "VoxelSharedPtr.h" // As it's used pretty much everywhere, include it here
#include "VoxelUserGlobals.h"

/**
 * To change a definition, add it to VoxelUserGlobals.h
 */

// Enable this to enable double precision in world generators and voxel graphs
#ifndef VOXEL_DOUBLE_PRECISION
#define VOXEL_DOUBLE_PRECISION 0
#endif

// Enables slow voxel checks
#ifndef VOXEL_DEBUG
#define VOXEL_DEBUG 0
#endif

// Disable if the stats file is too big
// Expensive
#ifndef VOXEL_SLOW_STATS
#define VOXEL_SLOW_STATS 0
#endif

// Will check that the data octree is locked for read/write
// Expensive
#ifndef DO_THREADSAFE_CHECKS
#define DO_THREADSAFE_CHECKS VOXEL_DEBUG
#endif

// Size of a render chunk
// Bigger = less draw calls
// Smaller = faster edits
// Must be a power of 2
#ifndef RENDER_CHUNK_SIZE
#define RENDER_CHUNK_SIZE 32
#endif

// Size of a data chunk
// Should leave it to default
#ifndef DATA_CHUNK_SIZE
#define DATA_CHUNK_SIZE 16
#endif

// Symbols not exported before 4.23
// See https://github.com/EpicGames/UnrealEngine/pull/5590
#ifndef ENABLE_VOXEL_DISTANCE_FIELDS
#define ENABLE_VOXEL_DISTANCE_FIELDS (ENGINE_MINOR_VERSION >= 23)
#endif

// No tessellation support on some platforms
#ifndef ENABLE_TESSELLATION
#define ENABLE_TESSELLATION (!PLATFORM_ANDROID && !PLATFORM_SWITCH)
#endif


// Enables recording detailed mesher stats (eg profiles every GetValue call)
// In my tests, adds a cost < 5% of the total generation time with a flat world generator,
// which is the worst cast for this as no time is spent generating values with it.
// Should be cheap enough to leave on
// NOTE: stats are recorded all the time and can only ever be cleared by the user! They are relatively small so should have no impact on memory,
// but might be an issue at some point!
#ifndef ENABLE_MESHER_STATS
#define ENABLE_MESHER_STATS (!UE_BUILD_SHIPPING)
#endif

// Records memory stats about voxels in addition to UE's stat system
// Unlike UE's stat system, it can be used in shipping builds
// Use UVoxelBlueprintLibrary::GetMemoryUsageInMB to get the info
#ifndef ENABLE_VOXEL_MEMORY_STATS
#define ENABLE_VOXEL_MEMORY_STATS 1
#endif

// Record stats about voxel data accelerators
// Minimal impact on performance
#ifndef VOXEL_DATA_ACCELERATOR_STATS
#define VOXEL_DATA_ACCELERATOR_STATS VOXEL_DEBUG
#endif

// No support for indices optimizations on some platforms
// Note: I have yet to find any performance improvements due to this
#ifndef ENABLE_OPTIMIZE_INDICES
#define ENABLE_OPTIMIZE_INDICES PLATFORM_WINDOWS
#endif

#ifndef EIGHT_BITS_VOXEL_VALUE
#define EIGHT_BITS_VOXEL_VALUE 0
#endif

// If true, Voxel Materials will default to R = G = B = A = 255
// else to R = G = B = A = 0
#ifndef VOXEL_MATERIAL_DEFAULT_IS_WHITE
#define VOXEL_MATERIAL_DEFAULT_IS_WHITE 0
#endif

/**
 * Voxel material config: use those to reduce the size of a FVoxelMaterial
 */

#ifndef VOXEL_MATERIAL_ENABLE_R
#define VOXEL_MATERIAL_ENABLE_R 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_G
#define VOXEL_MATERIAL_ENABLE_G 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_B
#define VOXEL_MATERIAL_ENABLE_B 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_A
#define VOXEL_MATERIAL_ENABLE_A 1
#endif

// Each additional UV channel uses 2 bytes
#ifndef VOXEL_MATERIAL_ENABLE_UV0
#define VOXEL_MATERIAL_ENABLE_UV0 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV1
#define VOXEL_MATERIAL_ENABLE_UV1 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV2
#define VOXEL_MATERIAL_ENABLE_UV2 0
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV3
#define VOXEL_MATERIAL_ENABLE_UV3 0
#endif

// 1+ : one to store "classic" UV: global or per voxel
// Others to store the voxel material UVs
#ifndef NUM_VOXEL_TEXTURE_COORDINATES
#define NUM_VOXEL_TEXTURE_COORDINATES (1 + VOXEL_MATERIAL_ENABLE_UV0 + VOXEL_MATERIAL_ENABLE_UV1 + VOXEL_MATERIAL_ENABLE_UV2 + VOXEL_MATERIAL_ENABLE_UV3)
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace VoxelGlobalsUtils
{
	template<typename T>
	inline constexpr bool IsPowerOfTwo(T Value)
	{
		return ((Value & (Value - 1)) == T(0));
	}
}

static_assert(VoxelGlobalsUtils::IsPowerOfTwo(RENDER_CHUNK_SIZE), "RENDER_CHUNK_SIZE must be a power of 2");
static_assert(VoxelGlobalsUtils::IsPowerOfTwo(DATA_CHUNK_SIZE), "DATA_CHUNK_SIZE must be a power of 2");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_DOUBLE_PRECISION
using v_flt = double;

#define MIN_vflt MIN_dbl
#define MAX_vflt MAX_dbl
#else
using v_flt = float;

#define MIN_vflt MIN_flt
#define MAX_vflt MAX_flt
#endif

#define VOXELS_PER_DATA_CHUNK (DATA_CHUNK_SIZE * DATA_CHUNK_SIZE * DATA_CHUNK_SIZE)

using FVoxelCellIndex = uint16;
static_assert(VOXELS_PER_DATA_CHUNK < TNumericLimits<FVoxelCellIndex>::Max(), "CellIndex type is too small");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if defined(__INTELLISENSE__) || defined(__RSCPP_VERSION)
#define INTELLISENSE_PARSER
#endif

#ifdef INTELLISENSE_PARSER
#define CORE_API
#define ENGINE_API
#undef VOXEL_DEBUG
#define VOXEL_DEBUG 1
#error "Compiler defined as parser"
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_PRO_ONLY_IMPL(ReturnValue) \
	FVoxelMessages::ShowVoxelPluginProError(FUNCTION_ERROR("This function requires Voxel Plugin Pro!")); \
	return ReturnValue;

#define VOXEL_PRO_ONLY() VOXEL_PRO_ONLY_IMPL({});
#define VOXEL_PRO_ONLY_VOID() VOXEL_PRO_ONLY_IMPL(PREPROCESSOR_NOTHING);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_DEBUG
#define checkVoxelSlow(x) check(x)
#define checkfVoxelSlow(x, ...) checkf(x, ##__VA_ARGS__)
#define ensureVoxelSlow(x) ensure(x)
#define ensureVoxelSlowNoSideEffects(x) ensure(x)
#define ensureMsgfVoxelSlowNoSideEffects(x, ...) ensureMsgf(x, ##__VA_ARGS__)
#undef FORCEINLINE
#define FORCEINLINE FORCEINLINE_DEBUGGABLE_ACTUAL
#else
#define checkVoxelSlow(x)
#define checkfVoxelSlow(x, ...)
#define ensureVoxelSlow(x) x
#define ensureVoxelSlowNoSideEffects(x)
#define ensureMsgfVoxelSlowNoSideEffects(...)
#endif

#if DO_THREADSAFE_CHECKS
#define ensureThreadSafe(...) ensure(__VA_ARGS__)
#else
#define ensureThreadSafe(...)
#endif

DECLARE_STATS_GROUP(TEXT("Voxel"), STATGROUP_Voxel, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Counters"), STATGROUP_VoxelCounters, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Memory"), STATGROUP_VoxelMemory, STATCAT_Advanced);

VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Log, All);

#define LOG_VOXEL(Verbosity, Format, ...) UE_LOG(LogVoxel, Verbosity, Format, ##__VA_ARGS__)

// Inline static helper to avoid rehashing FNames
#define STATIC_FNAME(Name) []() -> const FName& { static const FName StaticName = Name; return StaticName; }()

// Static string helper
#define STATIC_FSTRING(String) []() -> const FString& { static const FString StaticString = String; return StaticString; }()

#define UNIQUE_ID() []() { ensureVoxelSlowNoSideEffects(IsInGameThread()); static uint64 Id = 0; return ++Id; }()
#define OBJECT_LINE_ID() ((uint64)this + __LINE__)
#define GET_MEMBER_NAME_STATIC(ClassName, MemberName) STATIC_FNAME(GET_MEMBER_NAME_STRING_CHECKED(ClassName, MemberName))

#define FUNCTION_FNAME FName(__FUNCTION__)
#define FUNCTION_ERROR_IMPL(FunctionName, Error) (FString(FunctionName) + TEXT(": ") + Error)
#define FUNCTION_ERROR(Error) FUNCTION_ERROR_IMPL(__FUNCTION__, Error)

#define VOXEL_LOCTEXT(Text) NSLOCTEXT("Voxel", Text, Text)

#if STATS
struct FStat_Voxel_Base
{
	typedef FStatGroup_STATGROUP_Voxel TGroup;

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
#define VOXEL_SCOPE_COUNTER(Description) \
	struct VOXEL_SCOPE_COUNTER_STAT_CLASS_NAME(PREPROCESSOR_NOTHING) : FStat_Voxel_Base \
	{ \
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

#define VOXEL_FUNCTION_COUNTER() VOXEL_SCOPE_COUNTER(__FUNCTION__)
#else
#define VOXEL_SCOPE_COUNTER(Name)
#define VOXEL_FUNCTION_COUNTER()
#endif

#if VOXEL_SLOW_STATS
#define VOXEL_SLOW_FUNCTION_COUNTER() VOXEL_FUNCTION_COUNTER()
#define VOXEL_SLOW_SCOPE_COUNTER(Name) VOXEL_SCOPE_COUNTER(Name)
#else
#define VOXEL_SLOW_FUNCTION_COUNTER()
#define VOXEL_SLOW_SCOPE_COUNTER(Name)
#endif

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Use this to set the plugin to use a custom feedback context for progress bars/slow tasks
VOXEL_API void SetVoxelFeedbackContext(FFeedbackContext& FeedbackContext);

class VOXEL_API FVoxelScopedSlowTask : public FScopedSlowTask
{
public:
	explicit FVoxelScopedSlowTask(float InAmountOfWork, const FText& InDefaultMessage = FText(), bool bInEnabled = true);
};

struct FVoxelScopedFeedbackContext
{
	explicit FVoxelScopedFeedbackContext(FFeedbackContext& FeedbackContext)
	{
		SetVoxelFeedbackContext(FeedbackContext);
	}
	~FVoxelScopedFeedbackContext()
	{
		SetVoxelFeedbackContext(*GWarn);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if ENGINE_MINOR_VERSION < 22
#define ONLY_UE_22_AND_HIGHER(X)
#else
#define ONLY_UE_22_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 22
#define ONLY_UE_22_AND_LOWER(X)
#else
#define ONLY_UE_22_AND_LOWER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION < 23
#define ONLY_UE_23_AND_HIGHER(...)
#else
#define ONLY_UE_23_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 23
#define ONLY_UE_23_AND_LOWER(...)
#else
#define ONLY_UE_23_AND_LOWER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION < 24
#define ONLY_UE_24_AND_HIGHER(...)
#else
#define ONLY_UE_24_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 24
#define ONLY_UE_24_AND_LOWER(...)
#else
#define ONLY_UE_24_AND_LOWER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION < 25
#define ONLY_UE_25_AND_HIGHER(...)
#else
#define ONLY_UE_25_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 25
#define ONLY_UE_25_AND_LOWER(X)
#else
#define ONLY_UE_25_AND_LOWER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION >= 25
#define UE_25_SWITCH(Before, AfterOrEqual) AfterOrEqual
#else
#define UE_25_SWITCH(Before, AfterOrEqual) Before
#endif

#if ENGINE_MINOR_VERSION < 23
// Not defined in the engine in 4.22
template <typename T>
struct TIsConst
{
    static constexpr bool Value = false;
};

template <typename T>
struct TIsConst<const T>
{
    static constexpr bool Value = true;
};

// Insight stuff
#define CPUPROFILERTRACE_ENABLED 0
#define TRACE_THREAD_GROUP_SCOPE(...)
#endif

#if ENGINE_MINOR_VERSION >= 25

// Not updating the code to stay back compatible
#undef UProperty
#define UProperty FProperty

#undef UStructProperty
#define UStructProperty FStructProperty

#undef USoftObjectProperty
#define USoftObjectProperty FSoftObjectProperty

#undef UIntProperty
#define UIntProperty FIntProperty

#undef UFloatProperty
#define UFloatProperty FFloatProperty

#undef UBoolProperty
#define UBoolProperty FBoolProperty

#undef UArrayProperty
#define UArrayProperty FArrayProperty
#else
#define LAYOUT_FIELD(Type, Name) Type Name
#define DECLARE_TYPE_LAYOUT(...)
#define IMPLEMENT_TYPE_LAYOUT(...)
#endif