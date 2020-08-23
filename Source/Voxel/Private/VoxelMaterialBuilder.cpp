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

	IndicesStrengths.Reset();
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
		
		TVoxelStaticArray<uint8, 4> MaterialIndices{ ForceInit };
		TVoxelStaticArray<float, 3> Alphas{ ForceInit };
		
		if (IndicesStrengths.Num() > 0)
		{
			TVoxelStaticArray<float, 4> MaterialStrengths{ ForceInit };
			uint32 LockedChannels = 0;

			const auto AddIndexStrength = [&](int32 Index, const FIndexStrength& IndexStrength)
			{
				MaterialIndices[Index] = IndexStrength.Index;
				MaterialStrengths[Index] = IndexStrength.Strength;
				if (IndexStrength.bLocked)
				{
					LockedChannels |= 1 << IndexStrength.Index;
				}
			};

			if (IndicesStrengths.Num() > 4)
			{
				const TVoxelStaticArray<TTuple<int32, FIndexStrength>, 4> Stack = FVoxelUtilities::FindTopXElements<4>(IndicesStrengths, [&](auto& A, auto& B) { return A.Strength < B.Strength; });

				AddIndexStrength(0, Stack[0].Get<1>());
				AddIndexStrength(1, Stack[1].Get<1>());
				AddIndexStrength(2, Stack[2].Get<1>());
				AddIndexStrength(3, Stack[3].Get<1>());
			}
			else
			{
				for (int32 Index = 0; Index < IndicesStrengths.Num(); Index++)
				{
					AddIndexStrength(Index, IndicesStrengths[Index]);
				}
			}

			Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<4>(MaterialStrengths, LockedChannels);
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
