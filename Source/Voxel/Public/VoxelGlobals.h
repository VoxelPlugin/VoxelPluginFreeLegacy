// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"
#include "Stats/Stats.h"
#include "Launch/Resources/Version.h"
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
#ifndef VOXEL_SLOW_STATS
#define VOXEL_SLOW_STATS 1
#endif

// Will check that the data octree is locked for read/write
#ifndef DO_THREADSAFE_CHECKS
#define DO_THREADSAFE_CHECKS (VOXEL_DEBUG || !UE_BUILD_SHIPPING)
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

// Max depth of a world
// Should leave it to default
#ifndef MAX_WORLD_DEPTH
#define MAX_WORLD_DEPTH 26
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

// No support for indices optimizations on some platforms
// Note: I have yet to find any performance improvements due to this
#ifndef ENABLE_OPTIMIZE_INDICES
#define ENABLE_OPTIMIZE_INDICES PLATFORM_WINDOWS
#endif

#ifndef EIGHT_BITS_VOXEL_VALUE
#define EIGHT_BITS_VOXEL_VALUE 0
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
static_assert(MAX_WORLD_DEPTH % 2 == 0, "MAX_WORLD_DEPTH must be a multiple of 2");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if VOXEL_DOUBLE_PRECISION
using v_flt = double;
#else
using v_flt = float;
#endif

#define MIN_vflt TNumericLimits<v_flt>::Min()
#define MAX_vflt TNumericLimits<v_flt>::Max()

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
#define ensureMsgfVoxelSlow(x, ...) ensureMsgf(x, ##__VA_ARGS__)
#undef FORCEINLINE
#define FORCEINLINE FORCEINLINE_DEBUGGABLE_ACTUAL
#else
#define checkVoxelSlow(x)
#define checkfVoxelSlow(x, ...)
#define ensureVoxelSlow(x)
#define ensureMsgfVoxelSlow(...)
#endif

#if DO_THREADSAFE_CHECKS
#define ensureThreadSafe(...) ensure(__VA_ARGS__)
#else
#define ensureThreadSafe(...)
#endif

DECLARE_STATS_GROUP(TEXT("Voxel"), STATGROUP_Voxel, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Memory"), STATGROUP_VoxelMemory, STATCAT_Advanced);

VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Verbose, All);

// Inline static helper to avoid rehashing FNames
#define STATIC_FNAME(Name) []() -> const FName& { static const FName StaticName = Name; return StaticName; }()

// Static string helper
#define STATIC_FSTRING(String) []() -> const FString& { static const FString StaticString = String; return StaticString; }()

#define UNIQUE_ID() []() { ensureVoxelSlow(IsInGameThread()); static uint64 Id = 0; return ++Id; }()
#define OBJECT_LINE_ID() ((uint64)this + __LINE__)
#define GET_MEMBER_NAME_STATIC(ClassName, MemberName) STATIC_FNAME(GET_MEMBER_NAME_STRING_CHECKED(ClassName, MemberName))

#define FUNCTION_FNAME FName(__FUNCTION__)
#define FUNCTION_ERROR(Error) (FString(__FUNCTION__) + TEXT(": ") + Error)

#define PREPROCESSOR_EXPAND(X) X

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

namespace VoxelPrivateImpl
{
	template<typename TUnique>
	inline const UEnum& GetStaticEnum(const TCHAR* Name, TUnique Unique)
	{
		static const UEnum* Enum = FindObjectChecked<UEnum>((UObject*)ANY_PACKAGE, Name);
		return *Enum;
	}
}

#define GET_STATIC_UENUM(Enum) VoxelPrivateImpl::GetStaticEnum(TEXT(#Enum), []() { sizeof(Enum); })

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
#define ONLY_UE_23_AND_HIGHER(X)
#else
#define ONLY_UE_23_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 23
#define ONLY_UE_23_AND_LOWER(X)
#else
#define ONLY_UE_23_AND_LOWER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION < 24
#define ONLY_UE_24_AND_HIGHER(...)
#else
#define ONLY_UE_24_AND_HIGHER(...) __VA_ARGS__
#endif

#if ENGINE_MINOR_VERSION > 24
#define ONLY_UE_24_AND_LOWER(X)
#else
#define ONLY_UE_24_AND_LOWER(...) __VA_ARGS__
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