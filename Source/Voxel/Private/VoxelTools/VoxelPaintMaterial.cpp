// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelUtilities/VoxelMathUtilities.h"

void FVoxelPaintMaterial::ApplyToMaterial(FVoxelMaterial& Material, float Strength) const
{
	// Else goes way beyond target
	Strength = FMath::Clamp(Strength, -1.f, 1.f);
	
	switch (Type)
	{
	case EVoxelPaintMaterialType::Color:
	{
		// Note: the voxel colors are in linear space, even if they are stored in FColors
		FColor ColorToPaint = Color.bUseLinearColor ? Color.LinearColor.ToFColor(false) : Color.Color;
		if (Strength < 0)
		{
			Strength = -Strength;
			ColorToPaint = FColor(0, 0, 0, 0);
		}
		if (Color.bPaintR)
		{
			Material.SetR(FVoxelUtilities::LerpUINT8(Material.GetR(), ColorToPaint.R, Strength));
		}
		if (Color.bPaintG)
		{
			Material.SetG(FVoxelUtilities::LerpUINT8(Material.GetG(), ColorToPaint.G, Strength));
		}
		if (Color.bPaintB)
		{
			Material.SetB(FVoxelUtilities::LerpUINT8(Material.GetB(), ColorToPaint.B, Strength));
		}
		if (Color.bPaintA)
		{
			Material.SetA(FVoxelUtilities::LerpUINT8(Material.GetA(), ColorToPaint.A, Strength));
		}
		break;
	}
	case EVoxelPaintMaterialType::FiveWayBlend:
	{
		float TargetValue = Strength > 0 ? FiveWayBlend.TargetValue : 1.f - FiveWayBlend.TargetValue;
		Strength = FMath::Abs(Strength);

		if (FiveWayBlend.bFourWayBlend)
		{
			const int32 Channel = FMath::Clamp(FiveWayBlend.Channel, 0, 3);
			
			const float R = Material.GetR_AsFloat();
			const float G = Material.GetG_AsFloat();
			const float B = Material.GetB_AsFloat();

			TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>({ R, G, B });

			// Add locked channels to ChannelsToKeepIntact,
			// and make TargetValue relative to the strength that's left to edit
			uint32 ChannelsToKeepIntact = 1u << Channel;
			{
				float LockedChannelsStrength = 0.f;
				for (uint8 LockedChannel : FiveWayBlend.LockedChannels)
				{
					LockedChannel = FMath::Clamp<uint8>(LockedChannel, 0, 3);
					ChannelsToKeepIntact |= 1u << LockedChannel;
					LockedChannelsStrength += Strengths[LockedChannel];
				}
				TargetValue *= 1.f - LockedChannelsStrength;
			}

			Strengths[Channel] = FMath::Clamp(FMath::Lerp(Strengths[Channel], TargetValue, Strength), 0.f, 1.f);

			const TVoxelStaticArray<float, 3> Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<4>(Strengths, ChannelsToKeepIntact);

			Material.SetR(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[0], R));
			Material.SetG(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[1], G));
			Material.SetB(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[2], B));
		}
		else
		{
			const int32 Channel = FMath::Clamp(FiveWayBlend.Channel, 0, 4);

			const float R = Material.GetR_AsFloat();
			const float G = Material.GetG_AsFloat();
			const float B = Material.GetB_AsFloat();
			const float A = Material.GetA_AsFloat();

			TVoxelStaticArray<float, 5> Strengths = FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<5>({ R, G, B, A });

			// Add locked channels to ChannelsToKeepIntact,
			// and make TargetValue relative to the strength that's left to edit
			uint32 ChannelsToKeepIntact = 1u << Channel;
			{
				float LockedChannelsStrength = 0.f;
				for (uint8 LockedChannel : FiveWayBlend.LockedChannels)
				{
					LockedChannel = FMath::Clamp<uint8>(LockedChannel, 0, 4);
					ChannelsToKeepIntact |= 1u << LockedChannel;
					LockedChannelsStrength += Strengths[LockedChannel];
				}
				TargetValue *= 1.f - LockedChannelsStrength;
			}

			Strengths[Channel] = FMath::Clamp(FMath::Lerp(Strengths[Channel], TargetValue, Strength), 0.f, 1.f);

			const TVoxelStaticArray<float, 4> Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<5>(Strengths, ChannelsToKeepIntact);

			Material.SetR(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[0], R));
			Material.SetG(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[1], G));
			Material.SetB(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[2], B));
			Material.SetA(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[3], A));
		}
		
		break;
	}
	case EVoxelPaintMaterialType::SingleIndex:
	{
		Material.SetSingleIndex(SingleIndex.Channel.Channel);
		break;
	}
	case EVoxelPaintMaterialType::MultiIndex:
	{
		float TargetValue = Strength > 0 ? MultiIndex.TargetValue : 1.f - MultiIndex.TargetValue;
		Strength = FMath::Abs(Strength);

		const float Blend0 = Material.GetMultiIndex_Blend0_AsFloat();
		const float Blend1 = Material.GetMultiIndex_Blend1_AsFloat();
		const float Blend2 = Material.GetMultiIndex_Blend2_AsFloat();

		TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>({ Blend0, Blend1, Blend2 });

		uint32 ChannelsToKeepIntact = 0;
		
		// Propagate strengths backward if the indices are equal, and mark them as fixed:
		// we want them to always be 0, and the "main" one have all the strength instead
		// This is to handle cases where indices are eg 1 0 0 0
		{
			if (Material.GetMultiIndex_Index3() == Material.GetMultiIndex_Index2())
			{
				Strengths[2] += Strengths[3];
				Strengths[3] = 0;
				ChannelsToKeepIntact |= 1u << 3;
			}
			else if (Material.GetMultiIndex_Index3() == Material.GetMultiIndex_Index1())
			{
				Strengths[1] += Strengths[3];
				Strengths[3] = 0;
				ChannelsToKeepIntact |= 1u << 3;
			}
			else if (Material.GetMultiIndex_Index3() == Material.GetMultiIndex_Index0())
			{
				Strengths[0] += Strengths[3];
				Strengths[3] = 0;
				ChannelsToKeepIntact |= 1u << 3;
			}
			
			if (Material.GetMultiIndex_Index2() == Material.GetMultiIndex_Index1())
			{
				Strengths[1] += Strengths[2];
				Strengths[2] = 0;
				ChannelsToKeepIntact |= 1u << 2;
			}
			else if (Material.GetMultiIndex_Index2() == Material.GetMultiIndex_Index0())
			{
				Strengths[0] += Strengths[2];
				Strengths[2] = 0;
				ChannelsToKeepIntact |= 1u << 2;
			}
			
			if (Material.GetMultiIndex_Index1() == Material.GetMultiIndex_Index0())
			{
				Strengths[0] += Strengths[1];
				Strengths[1] = 0;
				ChannelsToKeepIntact |= 1u << 1;
			}
		}

		// Add locked channels to ChannelsToKeepIntact,
		// and make TargetValue relative to the strength that's left to edit
		{
			float LockedChannelsStrength = 0.f;
			for (uint8 LockedChannel : MultiIndex.LockedChannels)
			{
				if (Material.GetMultiIndex_Index0() == LockedChannel)
				{
					ChannelsToKeepIntact |= 1u << 0;
					LockedChannelsStrength += Strengths[0];
				}
				else if (Material.GetMultiIndex_Index1() == LockedChannel)
				{
					ChannelsToKeepIntact |= 1u << 1;
					LockedChannelsStrength += Strengths[1];
				}
				else if (Material.GetMultiIndex_Index2() == LockedChannel)
				{
					ChannelsToKeepIntact |= 1u << 2;
					LockedChannelsStrength += Strengths[2];
				}
				else if (Material.GetMultiIndex_Index3() == LockedChannel)
				{
					ChannelsToKeepIntact |= 1u << 3;
					LockedChannelsStrength += Strengths[3];
				}
			}
			TargetValue *= 1.f - LockedChannelsStrength;
		}

		int32 ChannelIndex;
		if (MultiIndex.Channel == Material.GetMultiIndex_Index0())
		{
			ChannelIndex = 0;
		}
		else if (MultiIndex.Channel == Material.GetMultiIndex_Index1())
		{
			ChannelIndex = 1;
		}
		else if (MultiIndex.Channel == Material.GetMultiIndex_Index2())
		{
			ChannelIndex = 2;
		}
		else if (MultiIndex.Channel == Material.GetMultiIndex_Index3())
		{
			ChannelIndex = 3;
		}
		else
		{
			ChannelIndex = 0;
			{
				float MinStrength = Strengths[0];
				if (Strengths[1] < MinStrength) { MinStrength = Strengths[1]; ChannelIndex = 1; }
				if (Strengths[2] < MinStrength) { MinStrength = Strengths[2]; ChannelIndex = 2; }
				if (Strengths[3] < MinStrength) { ChannelIndex = 3; }
			}
			
			Strengths[ChannelIndex] = 0.f;

			switch (ChannelIndex)
			{
			case 0: Material.SetMultiIndex_Index0(MultiIndex.Channel.Channel); break;
			case 1: Material.SetMultiIndex_Index1(MultiIndex.Channel.Channel); break;
			case 2: Material.SetMultiIndex_Index2(MultiIndex.Channel.Channel); break;
			case 3: Material.SetMultiIndex_Index3(MultiIndex.Channel.Channel); break;
			default: ensureVoxelSlow(false);
			}
		}

		// Do not modify the channel we are setting
		ChannelsToKeepIntact |= 1u << ChannelIndex;

		Strengths[ChannelIndex] = FMath::Clamp(FMath::Lerp(Strengths[ChannelIndex], TargetValue, Strength), 0.f, 1.f);

		const auto Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<4>(Strengths, ChannelsToKeepIntact);

		Material.SetMultiIndex_Blend0(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[0], Blend0));
		Material.SetMultiIndex_Blend1(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[1], Blend1));
		Material.SetMultiIndex_Blend2(FVoxelUtilities::FloatToUINT8_ForLerp(Alphas[2], Blend2));
		
		break;
	}
	case EVoxelPaintMaterialType::MultiIndexWetness:
	{
		float TargetValue = MultiIndexWetness.TargetValue;
		if (Strength < 0)
		{
			Strength = -Strength;
			TargetValue = 1.f - TargetValue;
		}
		Material.SetMultiIndex_Wetness(FVoxelUtilities::LerpUINT8(Material.GetMultiIndex_Wetness(), FVoxelUtilities::FloatToUINT8(TargetValue), Strength));
		break;
	}
	case EVoxelPaintMaterialType::MultiIndexRaw:
	{
		TVoxelStaticArray<float, 4> Strengths;
		Strengths[0] = MultiIndexRaw.Strength0;
		Strengths[1] = MultiIndexRaw.Strength1;
		Strengths[2] = MultiIndexRaw.Strength2;
		Strengths[3] = MultiIndexRaw.Strength3;
		const auto Alphas = FVoxelUtilities::XWayBlend_StrengthsToAlphas_Static<4>(Strengths);

		Material.SetMultiIndex_Blend0_AsFloat(Alphas[0]);
		Material.SetMultiIndex_Blend1_AsFloat(Alphas[1]);
		Material.SetMultiIndex_Blend2_AsFloat(Alphas[2]);
		
		Material.SetMultiIndex_Index0(MultiIndexRaw.Channel0.Channel);
		Material.SetMultiIndex_Index1(MultiIndexRaw.Channel1.Channel);
		Material.SetMultiIndex_Index2(MultiIndexRaw.Channel2.Channel);
		Material.SetMultiIndex_Index3(MultiIndexRaw.Channel3.Channel);

		break;
	}
	case EVoxelPaintMaterialType::UV:
	{
		FVector2D TargetUV = UV.UV;
		if (Strength < 0)
		{
			Strength = -Strength;
			TargetUV = FVector2D(1, 1) - TargetUV;
		}
		if (UV.bPaintU)
		{
			Material.SetU(UV.Channel, FVoxelUtilities::LerpUINT8(Material.GetU(UV.Channel), FVoxelUtilities::FloatToUINT8(TargetUV.X), Strength));
		}
		if (UV.bPaintV)
		{
			Material.SetV(UV.Channel, FVoxelUtilities::LerpUINT8(Material.GetV(UV.Channel), FVoxelUtilities::FloatToUINT8(TargetUV.Y), Strength));
		}
		break;
	}
	default: ensure(false);
	}
}
