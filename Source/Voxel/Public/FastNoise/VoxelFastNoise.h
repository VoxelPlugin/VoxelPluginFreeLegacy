// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise_CubicNoise.h"
#include "FastNoise/VoxelFastNoise_ValueNoise.h"
#include "FastNoise/VoxelFastNoise_WhiteNoise.h"
#include "FastNoise/VoxelFastNoise_PerlinNoise.h"
#include "FastNoise/VoxelFastNoise_SimplexNoise.h"
#include "FastNoise/VoxelFastNoise_CellularNoise.h"
#include "FastNoise/VoxelFastNoise_GradientPerturb.h"

// You will need to include FastNoise/VoxelFastNoise.inl to use fast noise functions
class FVoxelFastNoise : public
	TVoxelFastNoise_CubicNoise<
	TVoxelFastNoise_ValueNoise<
	TVoxelFastNoise_WhiteNoise<
	TVoxelFastNoise_PerlinNoise<
	TVoxelFastNoise_SimplexNoise<
	TVoxelFastNoise_CellularNoise<
	TVoxelFastNoise_GradientPerturb<
	FVoxelFastNoiseBase>>>>>>>
{
public:
	FVoxelFastNoise() = default;

	UE_DEPRECATED(4.24, "Use SetInterpolation instead. If this is a compiled graph, recompile the graph")
	void SetInterp(EVoxelNoiseInterpolation NewInterpolation) { SetInterpolation(NewInterpolation); }
	
	UE_DEPRECATED(4.24, "Use EVoxelNoiseInterpolation::Linear instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseInterpolation Linear = EVoxelNoiseInterpolation::Linear;
	UE_DEPRECATED(4.24, "Use EVoxelNoiseInterpolation::Hermite instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseInterpolation Hermite = EVoxelNoiseInterpolation::Hermite;
	UE_DEPRECATED(4.24, "Use EVoxelNoiseInterpolation::Quintic instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseInterpolation Quintic = EVoxelNoiseInterpolation::Quintic;
	
	UE_DEPRECATED(4.24, "Use EVoxelNoiseFractalType::FBM instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseFractalType FBM = EVoxelNoiseFractalType::FBM;
	UE_DEPRECATED(4.24, "Use EVoxelNoiseFractalType::Billow instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseFractalType Billow = EVoxelNoiseFractalType::Billow;
	UE_DEPRECATED(4.24, "Use EVoxelNoiseFractalType::RigidMulti instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelNoiseFractalType RigidMulti = EVoxelNoiseFractalType::RigidMulti;
	
	UE_DEPRECATED(4.24, "Use EVoxelCellularDistanceFunction::Euclidean instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularDistanceFunction Euclidean = EVoxelCellularDistanceFunction::Euclidean;
	UE_DEPRECATED(4.24, "Use EVoxelCellularDistanceFunction::Manhattan instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularDistanceFunction Manhattan = EVoxelCellularDistanceFunction::Manhattan;
	UE_DEPRECATED(4.24, "Use EVoxelCellularDistanceFunction::Natural instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularDistanceFunction Natural = EVoxelCellularDistanceFunction::Natural;
	
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::CellValue instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType CellValue = EVoxelCellularReturnType::CellValue;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance = EVoxelCellularReturnType::Distance;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance2 instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance2 = EVoxelCellularReturnType::Distance2;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance2Add instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance2Add = EVoxelCellularReturnType::Distance2Add;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance2Sub instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance2Sub = EVoxelCellularReturnType::Distance2Sub;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance2Mul instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance2Mul = EVoxelCellularReturnType::Distance2Mul;
	UE_DEPRECATED(4.24, "Use EVoxelCellularReturnType::Distance2Div instead. If this is a compiled graph, recompile the graph")
	static constexpr EVoxelCellularReturnType Distance2Div = EVoxelCellularReturnType::Distance2Div;
	
	UE_DEPRECATED(4.24, "Use IQNoise_2D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt IQNoiseDeriv_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const
	{
		return IQNoise_2D_Deriv(x, y, frequency, octaves, outDx, outDy);
	}
	UE_DEPRECATED(4.24, "Use IQNoise_3D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt IQNoiseDeriv_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
	{
		return IQNoise_3D_Deriv(x, y, z, frequency, octaves, outDx, outDy, outDz);
	}
	
	UE_DEPRECATED(4.24, "Use GetValue_2D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetValueDeriv_2D(v_flt x, v_flt y, v_flt frequency, v_flt& outDx, v_flt& outDy) const
	{
		return GetValue_2D_Deriv(x, y, frequency, outDx, outDy);
	}
	UE_DEPRECATED(4.24, "Use GetValue_3D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetValueDeriv_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
	{
		return GetValue_3D_Deriv(x, y, z, frequency, outDx, outDy, outDz);
	}
	
	UE_DEPRECATED(4.24, "Use GetValueFractal_2D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetValueFractalDeriv_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const
	{
		return GetValueFractal_2D_Deriv(x, y, frequency, octaves, outDx, outDy);
	}
	UE_DEPRECATED(4.24, "Use GetValueFractal_3D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetValueFractalDeriv_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
	{
		return GetValueFractal_3D_Deriv(x, y, z, frequency, octaves, outDx, outDy, outDz);
	}
	
	UE_DEPRECATED(4.24, "Use GetPerlin_2D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetPerlinDeriv_2D(v_flt x, v_flt y, v_flt frequency, v_flt& outDx, v_flt& outDy) const
	{
		return GetPerlin_2D_Deriv(x, y, frequency, outDx, outDy);
	}
	UE_DEPRECATED(4.24, "Use GetPerlin_3D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetPerlinDeriv_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
	{
		return GetPerlin_3D_Deriv(x, y, z, frequency, outDx, outDy, outDz);
	}
	
	UE_DEPRECATED(4.24, "Use GetPerlinFractal_2D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetPerlinFractalDeriv_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const
	{
		return GetPerlinFractal_2D_Deriv(x, y, frequency, octaves, outDx, outDy);
	}
	UE_DEPRECATED(4.24, "Use GetPerlinFractal_3D_Deriv instead. If this is a compiled graph, recompile the graph")
	v_flt GetPerlinFractalDeriv_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
	{
		return GetPerlinFractal_3D_Deriv(x, y, z, frequency, octaves, outDx, outDy, outDz);
	}
	
	UE_DEPRECATED(4.24, "GetSimplex_4D has been removed")
	v_flt GetSimplex_4D(v_flt x, v_flt y, v_flt z, v_flt w, v_flt frequency) const { return 0; }
};