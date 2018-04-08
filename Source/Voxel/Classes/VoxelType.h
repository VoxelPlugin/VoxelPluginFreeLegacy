#pragma once

#include "CoreMinimal.h"
#include "VoxelType.generated.h"

UENUM()
enum class EVoxelValueType : uint8
{
	// Value shouldn't be used
	IgnoreValue = 0,

	// Use if HaveSameSign
	UseValueIfSameSign = 1,

	// Value should be used
	UseValue = 2
};

UENUM()
enum class EVoxelMaterialType : uint8
{
	// Material shouldn't be used
	IgnoreMaterial = 0,

	// Material should be used
	UseMaterial = 1
};

/**
 * A VoxelType is a value type with a material type
 */
struct VOXEL_API FVoxelType
{
	uint8 Value;

	FORCEINLINE static const FVoxelType UseAll() { return FVoxelType(EVoxelValueType::UseValue, EVoxelMaterialType::UseMaterial); }
	FORCEINLINE static const FVoxelType IgnoreAll() { return FVoxelType(EVoxelValueType::IgnoreValue, EVoxelMaterialType::IgnoreMaterial); }

	FORCEINLINE static const FVoxelType Max(const FVoxelType& A, const FVoxelType& B)
	{
		return FVoxelType(FMath::Max(A.GetValueType(), B.GetValueType()), FMath::Max(A.GetMaterialType(), B.GetMaterialType()));
	}

	FVoxelType()
		: Value(0)
	{
	}

	FVoxelType(uint8 Value)
		: Value(Value)
	{
	}

	FVoxelType(EVoxelValueType ValueType, EVoxelMaterialType MaterialType)
		: Value((uint8)ValueType | ((uint8)MaterialType << 4))
	{
	}

	FORCEINLINE EVoxelValueType GetValueType() const
	{
		return (EVoxelValueType)(0x0F & Value);
	}

	FORCEINLINE EVoxelMaterialType GetMaterialType() const
	{
		return (EVoxelMaterialType)(Value >> 4);
	}

	FORCEINLINE bool operator==(const FVoxelType& V) const
	{
		return Value == V.Value;
	}

	FORCEINLINE bool operator!=(const FVoxelType& V) const
	{
		return Value != V.Value;
	}
};