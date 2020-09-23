// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_CellularNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()

public:
	v_flt GetCellular_2D(v_flt x, v_flt y, v_flt frequency) const;
	v_flt GetCellular_3D(v_flt x, v_flt y, v_flt z, v_flt frequency) const;
	
	void GetVoronoi_2D(v_flt x, v_flt y, v_flt m_jitter, v_flt& out_x, v_flt& out_y) const;
	void GetVoronoiNeighbors_2D(
		v_flt x, v_flt y, 
		v_flt m_jitter, 
		v_flt& out_x0, v_flt& out_y0,
		v_flt& out_x1, v_flt& out_y1, v_flt& out_distance1, 
		v_flt& out_x2, v_flt& out_y2, v_flt& out_distance2, 
		v_flt& out_x3, v_flt& out_y3, v_flt& out_distance3) const;

	GENERATED_VOXEL_NOISE_FUNCTION_2D(Crater)
	GENERATED_VOXEL_NOISE_FUNCTION_3D(Crater)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(Cellular, Crater)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(Cellular, Crater)
	
	FN_FORCEINLINE v_flt GetGavoronoi_2D(v_flt x, v_flt y, v_flt frequency, v_flt dirX, v_flt dirY, v_flt dirVariation) const
	{
		return SingleGavoronoi_2D(0, x * frequency, y * frequency, dirX, dirY, dirVariation);
	}
	FN_FORCEINLINE v_flt GetGavoronoiFractal_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt dirX, v_flt dirY, v_flt dirVariation) const
	{
		return This().Fractal_2D([&](auto in_offset, auto in_x, auto in_y)
		{
			return SingleGavoronoi_2D(in_offset, in_x, in_y, dirX, dirY, dirVariation);
		}, x, y, frequency, octaves);
	}

	v_flt GetErosion_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt noise_dx, v_flt noise_dy, v_flt& outDx, v_flt& outDy) const;
	
protected:
	template<EVoxelCellularDistanceFunction CellularDistance>
	v_flt SingleCellular_2D(v_flt x, v_flt y) const;
	template<EVoxelCellularDistanceFunction CellularDistance>
	v_flt SingleCellular_3D(v_flt x, v_flt y, v_flt z) const;

	template<EVoxelCellularDistanceFunction CellularDistance>
	v_flt SingleCellular2Edge_2D(v_flt x, v_flt y) const;
	template<EVoxelCellularDistanceFunction CellularDistance>
	v_flt SingleCellular2Edge_3D(v_flt x, v_flt y, v_flt z) const;
	
	template<EVoxelCellularDistanceFunction CellularDistance>
	void SingleVoronoi_2D(v_flt x, v_flt y, v_flt m_jitter, v_flt& out_x, v_flt& out_y) const;
	
	v_flt SingleCrater_2D(uint8 offset, v_flt x, v_flt y) const;
	v_flt SingleCrater_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const;
	
	v_flt SingleGavoronoi_2D(uint8 offset, v_flt x, v_flt y, v_flt dirX, v_flt dirY, v_flt dirVariation) const;
	v_flt SingleGavoronoi_Erosion_2D(uint8 offset, v_flt x, v_flt y, v_flt dirX, v_flt dirY, v_flt& outDx, v_flt& outDy) const;

protected:
	template<EVoxelCellularDistanceFunction CellularDistance>
	static v_flt CellularDistance_2D(v_flt vecX, v_flt vecY);
	template<EVoxelCellularDistanceFunction CellularDistance>
	static v_flt CellularDistance_3D(v_flt vecX, v_flt vecY, v_flt vecZ);

	void AccumulateCrater(v_flt sqDistance, v_flt& va, v_flt& wt) const;
};