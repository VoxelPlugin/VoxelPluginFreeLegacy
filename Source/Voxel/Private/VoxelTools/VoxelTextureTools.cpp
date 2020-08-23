// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelTextureTools.h"
#include "VoxelTools/VoxelToolHelpers.h"

enum class EMinMax : uint8
{
	Min,
	Max
};

template<EMinMax MinMax>
FVoxelFloatTexture MinMaxImpl(const FVoxelFloatTexture& Texture, const float Radius)
{
	VOXEL_TOOL_FUNCTION_COUNTER(Texture.Texture.GetSizeX() * Texture.Texture.GetSizeY());
	
	auto& Data = Texture.Texture;
	const int32 SizeX = Data.GetSizeX();
	const int32 SizeY = Data.GetSizeY();
	
	const auto NewTextureDataPtr = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	auto& NewTextureData = *NewTextureDataPtr;
	NewTextureData.SetSize(SizeX, SizeY);
	
	const int32 CeilRadius = FMath::CeilToInt(Radius);
	const int32 RadiusSquared = FMath::CeilToInt(FMath::Square(Radius));
	for (int32 X = 0; X < SizeX; X++)
	{
		for (int32 Y = 0; Y < SizeY; Y++)
		{
			float MinMaxValue = MinMax == EMinMax::Min ? MAX_flt : -MAX_flt;
			for (int32 U = -CeilRadius; U <= CeilRadius; U++)
			{
				for (int32 V = -CeilRadius; V <= CeilRadius; V++)
				{
					if (U * U + V * V <= RadiusSquared)
					{
						const float OtherValue = Data.SampleRaw(X + U, Y + V, EVoxelSamplerMode::Clamp);
						MinMaxValue = MinMax == EMinMax::Min ? FMath::Min(MinMaxValue, OtherValue) : FMath::Max(MinMaxValue, OtherValue);
					}
				}
			}
			NewTextureData.SetValue(X, Y, MinMaxValue);
		}
	}

	return FVoxelFloatTexture{ TVoxelTexture<float>{NewTextureDataPtr} };
}

FVoxelFloatTexture UVoxelTextureTools::Minimum(FVoxelFloatTexture Texture, float Radius)
{
	VOXEL_FUNCTION_COUNTER();
	return MinMaxImpl<EMinMax::Min>(Texture, Radius);
}

FVoxelFloatTexture UVoxelTextureTools::Maximum(FVoxelFloatTexture Texture, float Radius)
{
	VOXEL_FUNCTION_COUNTER();
	return MinMaxImpl<EMinMax::Max>(Texture, Radius);
}