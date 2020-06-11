// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelConfigEnums.h"
#include "VoxelStaticArray.h"

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
		if (MaterialConfig != EVoxelMaterialConfig::MultiIndex) 
		{
			return;
		}
		
		if (Strength > 0)
		{
			if (Strengths[Index] == 0)
			{
				ensureVoxelSlowNoSideEffects(!Indices.Contains(Index));
				Indices.Add(Index);
			}
			Strengths[Index] += Strength;
		}

		if (bLockStrength)
		{
			LockedStrengths.Set(Index);
		}
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
	
	TVoxelStaticArray<float, 256> Strengths{ ForceInit };
	TVoxelStaticBitArray<256> LockedStrengths{ ForceInit };
	TArray<uint8, TFixedAllocator<256>> Indices;
};