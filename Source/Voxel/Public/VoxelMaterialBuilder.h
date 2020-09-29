// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelMaterial.h"
#include "VoxelContainers/VoxelStaticArray.h"

class VOXEL_API FVoxelMaterialBuilder
{
public:
	FVoxelMaterialBuilder() = default;

public:
	void SetMaterialConfig(EVoxelMaterialConfig InMaterialConfig)
	{
		MaterialConfig = InMaterialConfig;
	}

	void Clear();
	FVoxelMaterial Build() const;

public:
	void AddMultiIndex(uint8 Index, float Strength, bool bLockStrength = false)
	{
		if (MaterialConfig != EVoxelMaterialConfig::MultiIndex || Strength <= 0) 
		{
			return;
		}
		
		for (auto& It : IndicesStrengths)
		{
			if (It.Index == Index)
			{
				It.bLocked |= bLockStrength;
				It.Strength += Strength;
				return;
			}
		}
		
		auto& NewIndex = IndicesStrengths.Emplace_GetRef();
		NewIndex.Index = Index;
		NewIndex.bLocked = bLockStrength;
		NewIndex.Strength = Strength;
	}
	void AddMultiIndex(int32 Index, float Strength, bool bLockStrength = false)
	{
		AddMultiIndex(FVoxelUtilities::ClampToUINT8(Index), Strength, bLockStrength);
	}

	void SetColor(FColor InColor)
	{
		Color = InColor;
	}
	void SetColor(FLinearColor InColor)
	{
		SetColor(FVoxelUtilities::FloatToUINT8(InColor));
	}

	void SetWetness(uint8 InWetness)
	{
		Wetness = InWetness;
	}
	void SetWetness(float InWetness)
	{
		SetWetness(FVoxelUtilities::FloatToUINT8(InWetness));
	}
	void SetWetness(double InWetness)
	{
		SetWetness(float(InWetness));
	}

	void SetSingleIndex(uint8 Index)
	{
		SingleIndex = Index;
	}
	void SetSingleIndex(int32 Index)
	{
		SetSingleIndex(FVoxelUtilities::ClampToUINT8(Index));
	}

	void SetU(int32 Channel, uint8 Value)
	{
		if (0 <= Channel && Channel < 4)
		{
			Us[Channel] = Value;
		}
	}
	void SetU(int32 Channel, float Value)
	{
		SetU(Channel, FVoxelUtilities::FloatToUINT8(Value));
	}
	void SetU(int32 Channel, double Value)
	{
		SetU(Channel, float(Value));
	}

	void SetV(int32 Channel, uint8 Value)
	{
		if (0 <= Channel && Channel < 4)
		{
			Vs[Channel] = Value;
		}
	}
	void SetV(int32 Channel, float Value)
	{
		SetV(Channel, FVoxelUtilities::FloatToUINT8(Value));
	}
	void SetV(int32 Channel, double Value)
	{
		SetV(Channel, float(Value));
	}

	FVoxelMaterialBuilder& operator=(const FVoxelMaterial& Other)
	{
		// Override this material by another
		// Mainly here to support SetMaterial in graphs
		MaterialOverride = Other;
		return *this;
	}

private:
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig(-1);

	TOptional<FVoxelMaterial> MaterialOverride;
	
	FColor Color{ ForceInit };
	uint8 SingleIndex = 0;
	uint8 Wetness = 0;

	TVoxelStaticArray<uint8, 4> Us{ ForceInit };
	TVoxelStaticArray<uint8, 4> Vs{ ForceInit };

	struct FIndexStrength
	{
		uint8 Index = 0;
		bool bLocked = false;
		float Strength = 0;
	};
	// Since we usually have very few of them, it's faster to have a TArray than a TMap
	// or a static array of size 256, which is expensive to initialize
	TArray<FIndexStrength, TFixedAllocator<256>> IndicesStrengths;
};