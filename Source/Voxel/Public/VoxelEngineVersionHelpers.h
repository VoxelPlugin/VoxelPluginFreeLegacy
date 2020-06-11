// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Launch/Resources/Version.h"

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