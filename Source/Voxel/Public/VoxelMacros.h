// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefinitions.h"

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
#define INTELLISENSE_PARSER 1
#else
#define INTELLISENSE_PARSER 0
#endif

#if INTELLISENSE_PARSER
#define CORE_API
#define ENGINE_API
#undef VOXEL_DEBUG
#define VOXEL_DEBUG 1
// This is defined in the generated.h. It lets you use GetOuterASomeOuter. Resharper/intellisense are confused when it's used, so define it for them
#define INTELLISENSE_DECLARE_WITHIN(Name) DECLARE_WITHIN(Name)
#error "Compiler defined as parser"
#else
#define INTELLISENSE_DECLARE_WITHIN(Name)
#endif

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
#define ensureVoxelSlow(x) (!!(x))
#define ensureVoxelSlowNoSideEffects(x)
#define ensureMsgfVoxelSlowNoSideEffects(...)
#endif

#if DO_THREADSAFE_CHECKS
#define ensureThreadSafe(...) ensure(__VA_ARGS__)
#else
#define ensureThreadSafe(...)
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_LOCTEXT(Text) INVTEXT(Text)

// Inline static helper to avoid rehashing FNames
#ifndef STATIC_FNAME
#define STATIC_FNAME(Name) ([]() -> const FName& { static const FName StaticName = Name; return StaticName; }())
#endif

// Static string helper
#ifndef STATIC_FSTRING
#define STATIC_FSTRING(String) ([]() -> const FString& { static const FString StaticString = String; return StaticString; }())
#endif

#ifndef OBJECT_LINE_ID
#define OBJECT_LINE_ID() ((uint64)this + __LINE__)
#endif

#ifndef GET_MEMBER_NAME_STATIC
#define GET_MEMBER_NAME_STATIC(ClassName, MemberName) STATIC_FNAME(GET_MEMBER_NAME_STRING_CHECKED(ClassName, MemberName))
#endif

#ifndef GET_OWN_MEMBER_NAME
#define GET_OWN_MEMBER_NAME(MemberName) GET_MEMBER_NAME_CHECKED(TDecay<decltype(*this)>::Type, MemberName)
#endif

#ifndef FUNCTION_FNAME
#define FUNCTION_FNAME FName(__FUNCTION__)
#endif

#ifndef FUNCTION_ERROR_IMPL
#define FUNCTION_ERROR_IMPL(FunctionName, Error) (FString(FunctionName) + TEXT(": ") + Error)
#endif

#ifndef FUNCTION_ERROR
#define FUNCTION_ERROR(Error) FUNCTION_ERROR_IMPL(__FUNCTION__, Error)
#endif

#define VOXEL_DEPRECATED(Version, Message) UE_DEPRECATED(0, Message " If this is a C++ voxel graph, you should compile it to C++ again.")

#define VOXEL_THIS_TYPE TDecay<decltype(*this)>::Type
// This is needed in classes, where just doing class Name would fwd declare it in the class scope
#define VOXEL_FWD_DECLARE_CLASS(Name) void PREPROCESSOR_JOIN(__VoxelDeclareDummy_, __LINE__)(class Name);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define VOXEL_UNIQUE_ID() []() { ensureVoxelSlowNoSideEffects(IsInGameThread()); static uint64 __UniqueId = 0; return ++__UniqueId; }()

// UniqueClass: to forbid copying ids from different classes
template<typename UniqueClass>
class TVoxelUniqueId
{
public:
	TVoxelUniqueId() = default;

	bool IsValid() const { return Id != 0; }

	bool operator==(const TVoxelUniqueId& Other) const { return Id == Other.Id; }
	bool operator!=(const TVoxelUniqueId& Other) const { return Id != Other.Id; }

	bool operator<(const TVoxelUniqueId& Other) const { return Id < Other.Id; }
	bool operator>(const TVoxelUniqueId& Other) const { return Id > Other.Id; }

	bool operator<=(const TVoxelUniqueId& Other) const { return Id <= Other.Id; }
	bool operator>=(const TVoxelUniqueId& Other) const { return Id >= Other.Id; }

	friend uint32 GetTypeHash(TVoxelUniqueId InId)
	{
	    return uint32(InId.Id);
	}

	static TVoxelUniqueId New();

private:
	TVoxelUniqueId(uint64 Id)
		: Id(Id)
	{
		ensureVoxelSlow(IsValid());
	}
	
	uint64 Id = 0;
};

#define DECLARE_UNIQUE_VOXEL_ID(Name) \
	using Name = TVoxelUniqueId<class __ ## Name ##_Unique>; \
	template<> \
	Name TVoxelUniqueId<class __ ## Name ##_Unique>::New();

#define DEFINE_UNIQUE_VOXEL_ID(Name) \
	template<> \
	Name TVoxelUniqueId<class __ ## Name ##_Unique>::New() \
	{ \
		static FThreadSafeCounter64 Counter; \
		return Counter.Increment(); \
	}