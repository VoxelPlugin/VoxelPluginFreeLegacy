// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Templates/Casts.h"
#include "Launch/Resources/Version.h"

#define VOXEL_ENGINE_VERSION (ENGINE_MAJOR_VERSION * 100 + ENGINE_MINOR_VERSION)

#if VOXEL_ENGINE_VERSION > 424
#define ONLY_UE_24_AND_LOWER(...)
#else
#define ONLY_UE_24_AND_LOWER(...) __VA_ARGS__
#endif

#if VOXEL_ENGINE_VERSION < 425
#define ONLY_UE_25_AND_HIGHER(...)
#else
#define ONLY_UE_25_AND_HIGHER(...) __VA_ARGS__
#endif

#if VOXEL_ENGINE_VERSION > 425
#define ONLY_UE_25_AND_LOWER(X)
#else
#define ONLY_UE_25_AND_LOWER(...) __VA_ARGS__
#endif

#if VOXEL_ENGINE_VERSION >= 425
#define UE_25_SWITCH(Before, AfterOrEqual) AfterOrEqual
#define UE_25_ONLY(...) __VA_ARGS__
#else
#define UE_25_SWITCH(Before, AfterOrEqual) Before
#define UE_25_ONLY(...)
#endif

#if VOXEL_ENGINE_VERSION >= 426
#define UE_26_SWITCH(Before, AfterOrEqual) AfterOrEqual
#define UE_26_ONLY(...) __VA_ARGS__
#else
#define UE_26_SWITCH(Before, AfterOrEqual) Before
#define UE_26_ONLY(...)
#endif

#if VOXEL_ENGINE_VERSION >= 427
#define UE_27_SWITCH(Before, AfterOrEqual) AfterOrEqual
#define UE_27_ONLY(...) __VA_ARGS__
#else
#define UE_27_SWITCH(Before, AfterOrEqual) Before
#define UE_27_ONLY(...)
#endif

#if VOXEL_ENGINE_VERSION >= 500
#define UE_5_SWITCH(Before, AfterOrEqual) AfterOrEqual
#define UE_5_ONLY(...) __VA_ARGS__
#define UE_5_CONVERT(Type, ...) Type(__VA_ARGS__)
#else
#define UE_5_SWITCH(Before, AfterOrEqual) Before
#define UE_5_ONLY(...)
#define UE_5_CONVERT(Type, ...) __VA_ARGS__
#endif

#if VOXEL_ENGINE_VERSION < 425
using FProperty = UProperty;
using FStructProperty = UStructProperty;
using FSoftObjectProperty = USoftObjectProperty;
using FIntProperty = UIntProperty;
using FFloatProperty = UFloatProperty;
using FBoolProperty = UBoolProperty;
using FNameProperty = UNameProperty;
using FObjectProperty = UObjectProperty;
using FStructProperty = UStructProperty;
using FArrayProperty = UArrayProperty;
using FMapProperty = UMapProperty;
using FSetProperty = USetProperty;

template<typename To, typename From>
To* CastField(From* Src)
{
	return Cast<To>(Src);
}
template<typename To, typename From>
To* CastFieldChecked(From* Src)
{
	return CastChecked<To>(Src);
}

#define LAYOUT_FIELD(Type, Name) Type Name
#define DECLARE_TYPE_LAYOUT(...)
#define IMPLEMENT_TYPE_LAYOUT(...)

// For TTupleElement
#include "MeshAttributeArray.h"
#endif

#if VOXEL_ENGINE_VERSION < 500
template<typename T>
using TObjectPtr = T*;
#endif

#ifndef UE_ASSUME
#define UE_ASSUME(X) ASSUME(X)
#endif