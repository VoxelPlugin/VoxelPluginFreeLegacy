// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelPinCategory.generated.h"

struct FVoxelNodeType;
struct FVoxelNodeRangeType;

UENUM()
enum class EVoxelPinCategory : uint8
{
	Exec,
	Boolean,
	Int,
	Float,
	Material,
	Color,
	Seed,
	Wildcard  UMETA(Hidden),
	Vector  UMETA(Hidden)
};

UENUM()
enum class EVoxelDataPinCategory : uint8
{
	Boolean,
	Int,
	Float,
	Material,
	Color
};

struct VOXELGRAPH_API FVoxelPinCategory
{
	static EVoxelPinCategory DataPinToPin(EVoxelDataPinCategory Category);
	static EVoxelPinCategory FromString(const FName& String);
	static FName GetName(EVoxelPinCategory Category);

	static FString GetDefaultValue(EVoxelPinCategory Category);
	static FString GetDefaultValue(EVoxelDataPinCategory Category);

	static FString GetTypeString(EVoxelPinCategory Category);
	static FString GetTypeString(EVoxelDataPinCategory Category) { return GetTypeString(DataPinToPin(Category)); }
	static FString GetRangeTypeString(EVoxelPinCategory Category);

	static FVoxelNodeType ConvertDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);
	static FVoxelNodeRangeType ConvertRangeDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);
	static FString ConvertStringDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);

	static FString ToString(EVoxelPinCategory Category, FVoxelNodeType Value);
	static FString ToString(EVoxelPinCategory Category, FVoxelNodeRangeType Value);

	static bool IsInRange(EVoxelPinCategory Category, FVoxelNodeType Value, FVoxelNodeRangeType Range);
};