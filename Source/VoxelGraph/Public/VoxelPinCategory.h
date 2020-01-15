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
	Wildcard  UMETA(Hidden)
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

namespace FVoxelPinCategory
{
	VOXELGRAPH_API EVoxelPinCategory DataPinToPin(EVoxelDataPinCategory Category);
	VOXELGRAPH_API EVoxelPinCategory FromString(const FName& String);
	VOXELGRAPH_API FName GetName(EVoxelPinCategory Category);

	VOXELGRAPH_API FString GetDefaultValue(EVoxelPinCategory Category);
	VOXELGRAPH_API FString GetDefaultValue(EVoxelDataPinCategory Category);

	VOXELGRAPH_API FString GetTypeString(EVoxelPinCategory Category);
			inline FString GetTypeString(EVoxelDataPinCategory Category) { return GetTypeString(DataPinToPin(Category)); }
	VOXELGRAPH_API FString GetRangeTypeString(EVoxelPinCategory Category);

	VOXELGRAPH_API FVoxelNodeType ConvertDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);
	VOXELGRAPH_API FVoxelNodeRangeType ConvertRangeDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);
	VOXELGRAPH_API FString ConvertStringDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue);

	VOXELGRAPH_API FString ToString(EVoxelPinCategory Category, FVoxelNodeType Value);
}