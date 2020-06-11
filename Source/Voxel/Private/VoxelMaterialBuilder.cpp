// Copyright 2020 Phyronnaz

#include "VoxelMaterialBuilder.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

void FVoxelMaterialBuilder::Clear()
{
	MaterialConfig = EVoxelMaterialConfig(-1);

	MaterialOverride.Reset();

	Color = FColor{ ForceInit };
	SingleIndex = 0;
	Wetness = 0;

	Us.Memzero();
	Vs.Memzero();

	// Avoid expensive memzero
	for (uint8 Index : Indices)
	{
		Strengths[Index] = 0;
		LockedStrengths.Clear(Index);
	}

	Indices.Reset();
}

FVoxelMaterial FVoxelMaterialBuilder::Build() const
{
	if (MaterialOverride.IsSet())
	{
		return MaterialOverride.GetValue();
	}
	
	FVoxelMaterial Material{ ForceInit };

	if (MaterialConfig == EVoxelMaterialConfig::RGB)
	{
		Material.SetColor(Color);

		Material.SetU0(Us[0]);
		Material.SetU1(Us[1]);
		Material.SetU2(Us[2]);
		Material.SetU3(Us[3]);

		Material.SetV0(Vs[0]);
		Material.SetV1(Vs[1]);
		Material.SetV2(Vs[2]);
		Material.SetV3(Vs[3]);
	}
	else if (MaterialConfig == EVoxelMaterialConfig::SingleIndex)
	{
		Material.SetColor(Color);
		Material.SetSingleIndex(SingleIndex);

		Material.SetU0(Us[0]);
		Material.SetU1(Us[1]);
		Material.SetU2(Us[2]);
		Material.SetU3(Us[3]);

		Material.SetV0(Vs[0]);
		Material.SetV1(Vs[1]);
		Material.SetV2(Vs[2]);
		Material.SetV3(Vs[3]);
	}
	else
	{
		check(MaterialConfig == EVoxelMaterialConfig::MultiIndex);

		int32 NumIndices;
		TVoxelStaticArray<uint8, 4> MaterialIndices{ ForceInit };

		if (Indices.Num() > 4)
		{
			const TVoxelStaticArray<TTuple<int32, uint8>, 4> Stack = FVoxelUtilities::FindTopXElements<4>(Indices, [&](uint8 A, uint8 B) { return Strengths[A] < Strengths[B]; });

			NumIndices = 4;
			MaterialIndices[0] = Stack[0].Get<1>();
			MaterialIndices[1] = Stack[1].Get<1>();
			MaterialIndices[2] = Stack[2].Get<1>();
			MaterialIndices[3] = Stack[3].Get<1>();
		}
		else
		{
			NumIndices = Indices.Num();
			if (NumIndices > 0) MaterialIndices[0] = Indices[0];
			if (NumIndices > 1) MaterialIndices[1] = Indices[1];
			if (NumIndices > 2) MaterialIndices[2] = Indices[2];
			if (NumIndices > 3) MaterialIndices[3] = Indices[3];
		}

		TVoxelStaticArray<float, 3> Alphas;
		
		if (NumIndices > 0)
		{
			TVoxelStaticArray<float, 4> MaterialStrengths{ ForceInit };
			uint32 LockedChannels = 0;

			for (int32 Index = 0; Index < NumIndices; Index++)
			{
				const uint8 MaterialIndex = MaterialIndices[Index];

				MaterialStrengths[Index] = Strengths[MaterialIndex];
				if (LockedStrengths.Test(MaterialIndex))
				{
					LockedChannels |= 1 << Index;
				}
			}
			
			Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<4>(MaterialStrengths, LockedChannels);
		}
		else
		{
			Alphas.Memzero();
		}

		Material.SetMultiIndex_Wetness(Wetness);

		Material.SetMultiIndex_Blend0_AsFloat(Alphas[0]);
		Material.SetMultiIndex_Blend1_AsFloat(Alphas[1]);
		Material.SetMultiIndex_Blend2_AsFloat(Alphas[2]);

		Material.SetMultiIndex_Index0(MaterialIndices[0]);
		Material.SetMultiIndex_Index1(MaterialIndices[1]);
		Material.SetMultiIndex_Index2(MaterialIndices[2]);
		Material.SetMultiIndex_Index3(MaterialIndices[3]);

		Material.SetU2(Us[2]);
		Material.SetU3(Us[3]);
		Material.SetV2(Vs[2]);
		Material.SetV3(Vs[3]);
	}

	return Material;
}
