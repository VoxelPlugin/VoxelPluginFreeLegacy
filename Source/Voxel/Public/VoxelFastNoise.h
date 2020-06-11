// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Math/TransformCalculus2D.h"
#include "Math/TransformCalculus3D.h"
#include "VoxelMinimal.h"

//
// MIT License
//
// Copyright(c) 2017 Jordan Peck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The developer's email is jorzixdan.me2@gzixmail.com (for great email, take
// off every 'zix'.)
//

// VERSION: 0.4.1

// Uncomment the line below to use doubles throughout FVoxelFastNoise instead of floats
//#define FN_USE_DOUBLES

#define FN_CELLULAR_INDEX_MAX 3

typedef v_flt FN_DECIMAL;

class VOXEL_API FVoxelFastNoise
{
public:
	FVoxelFastNoise() = default;
	
	//explicit FVoxelFastNoise(int seed = 1337) { SetSeed(seed); CalculateFractalBounding(); }

	enum NoiseType { Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, Cellular, WhiteNoise, Cubic, CubicFractal };
	enum Interp { Linear, Hermite, Quintic };
	enum FractalType { FBM, Billow, RigidMulti };
	enum CellularDistanceFunction { Euclidean, Manhattan, Natural };
	enum CellularReturnType { CellValue, /*NoiseLookup,*/ Distance, Distance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div };

	// Sets seed used for all noise types
	// Default: 1337
	void SetSeed(int seed);

	// Returns seed used for all noise types
	int GetSeed() const { return m_seed; }

	// Sets frequency for all noise types
	// Default: 0.01
	//void SetFrequency(FN_DECIMAL frequency) { frequency = frequency; }

	// Returns frequency used for all noise types
	//FN_DECIMAL GetFrequency() const { return frequency; }

	// Changes the interpolation method used to smooth between noise values
	// Possible interpolation methods (lowest to highest quality) :
	// - Linear
	// - Hermite
	// - Quintic
	// Used in Value, Perlin Noise and Position Warping
	// Default: Quintic
	void SetInterp(Interp interp) { m_interp = interp; }

	// Returns interpolation method used for supported noise types
	Interp GetInterp() const { return m_interp; }

	// Sets noise return type of GetNoise(...)
	// Default: Simplex
	//void SetNoiseType(NoiseType noiseType) { m_noiseType = noiseType; }

	// Returns the noise type used by GetNoise
	//NoiseType GetNoiseType() const { return m_noiseType; }

	// Sets octave count for all fractal noise types
	// Default: 3
	// This is just cot compute the bounding, functions get the octaves to use in parameter
	// to allow having noise LODs with a single fast noise instance
	//void SetFractalOctaves(int octaves) { CalculateFractalBounding(octaves); }

	// Returns octave count for all fractal noise types
	//int GetFractalOctaves() const { return m_octaves; }
	
	// Sets octave lacunarity for all fractal noise types
	// Default: 2.0
	void SetFractalLacunarity(FN_DECIMAL lacunarity) { m_lacunarity = lacunarity; }

	// Returns octave lacunarity for all fractal noise types
	FN_DECIMAL GetFractalLacunarity() const { return m_lacunarity; }

	// Sets octave gain for all fractal noise types
	// Default: 0.5
	//void SetFractalGain(FN_DECIMAL gain) { m_gain = gain; CalculateFractalBounding(); }
	void SetFractalOctavesAndGain(int octaves, FN_DECIMAL gain) { m_gain = gain; CalculateFractalBounding(octaves); }

	// Returns octave gain for all fractal noise types
	FN_DECIMAL GetFractalGain() const { return m_gain; }

	// Sets method for combining octaves in all fractal noise types
	// Default: FBM
	void SetFractalType(FractalType fractalType) { m_fractalType = fractalType; }

	// Returns method for combining octaves in all fractal noise types
	FractalType GetFractalType() const { return m_fractalType; }


	// Sets distance function used in cellular noise calculations
	// Default: Euclidean
	void SetCellularDistanceFunction(CellularDistanceFunction cellularDistanceFunction) { m_cellularDistanceFunction = cellularDistanceFunction; }

	// Returns the distance function used in cellular noise calculations
	CellularDistanceFunction GetCellularDistanceFunction() const { return m_cellularDistanceFunction; }

	// Sets return type from cellular noise calculations
	// Note: NoiseLookup requires another FVoxelFastNoise object be set with SetCellularNoiseLookup() to function
	// Default: CellValue
	void SetCellularReturnType(CellularReturnType cellularReturnType) { m_cellularReturnType = cellularReturnType; }

	// Returns the return type from cellular noise calculations
	CellularReturnType GetCellularReturnType() const { return m_cellularReturnType; }

	// Noise used to calculate a cell value if cellular return type is NoiseLookup
	// The lookup value is acquired through GetNoise() so ensure you SetNoiseType() on the noise lookup, value, Perlin or simplex is recommended
	void SetCellularNoiseLookup(FVoxelFastNoise* noise) { m_cellularNoiseLookup = noise; }

	// Returns the noise used to calculate a cell value if the cellular return type is NoiseLookup
	FVoxelFastNoise* GetCellularNoiseLookup() const { return m_cellularNoiseLookup; }

	// Sets the 2 distance indices used for distance2 return types
	// Default: 0, 1
	// Note: index0 should be lower than index1
	// Both indices must be >= 0, index1 must be < 4
	void SetCellularDistance2Indices(int cellularDistanceIndex0, int cellularDistanceIndex1);

	// Returns the 2 distance indices used for distance2 return types
	void GetCellularDistance2Indices(int& cellularDistanceIndex0, int& cellularDistanceIndex1) const;

	// Sets the maximum distance a cellular point can move from its grid position
	// Setting this high will make artifacts more common
	// Default: 0.45
	void SetCellularJitter(FN_DECIMAL cellularJitter) { m_cellularJitter = cellularJitter; }

	// Returns the maximum distance a cellular point can move from its grid position
	FN_DECIMAL GetCellularJitter() const { return m_cellularJitter; }

	// Sets the maximum warp distance from original location when using GradientPerturb{Fractal}(...)
	// Default: 1.0
	//void SetGradientPerturbAmp(FN_DECIMAL gradientPerturbAmp) { m_gradientPerturbAmp = gradientPerturbAmp; }

	// Returns the maximum warp distance from original location when using GradientPerturb{Fractal}(...)
	//FN_DECIMAL GetGradientPerturbAmp() const { return m_gradientPerturbAmp; }

	void SetMatrix(FMatrix2x2 matrix) { m_matrix2 = matrix; }
	void SetMatrix(FMatrix matrix) { m_matrix3 = matrix; }

	//2D
	FN_DECIMAL IQNoise_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves) const;
	FN_DECIMAL IQNoiseDeriv_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;

	FN_DECIMAL GetValue_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency) const;
	FN_DECIMAL GetValueFractal_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves) const;
	FN_DECIMAL GetValueDeriv_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL GetValueFractalDeriv_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;

	FN_DECIMAL GetPerlin_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency) const;
	FN_DECIMAL GetPerlinFractal_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves) const;
	FN_DECIMAL GetPerlinDeriv_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL GetPerlinFractalDeriv_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;

	FN_DECIMAL GetSimplex_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency) const;
	FN_DECIMAL GetSimplexFractal_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves) const;

	FN_DECIMAL GetCellular_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency) const;
	
	void GetVoronoi_2D(FN_DECIMAL x, FN_DECIMAL y, float m_jitter, FN_DECIMAL& out_x, FN_DECIMAL& out_y) const;
	void GetVoronoiNeighbors_2D(
		FN_DECIMAL x, FN_DECIMAL y, 
		float m_jitter, 
		FN_DECIMAL& out_x0, FN_DECIMAL& out_y0,
		FN_DECIMAL& out_x1, FN_DECIMAL& out_y1, FN_DECIMAL& out_distance1, 
		FN_DECIMAL& out_x2, FN_DECIMAL& out_y2, FN_DECIMAL& out_distance2, 
		FN_DECIMAL& out_x3, FN_DECIMAL& out_y3, FN_DECIMAL& out_distance3) const;

	FN_DECIMAL GetWhiteNoise_2D(FN_DECIMAL x, FN_DECIMAL y) const;
	FN_DECIMAL GetWhiteNoiseInt_2D(int x, int y) const;

	FN_DECIMAL GetCubic_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency) const;
	FN_DECIMAL GetCubicFractal_2D(FN_DECIMAL x, FN_DECIMAL y, float frequency, int octaves) const;

	//FN_DECIMAL GetNoise(FN_DECIMAL x, FN_DECIMAL y) const;

	void GradientPerturb_2D(FN_DECIMAL& x, FN_DECIMAL& y, float frequency, float m_gradientPerturbAmp) const;
	void GradientPerturbFractal_2D(FN_DECIMAL& x, FN_DECIMAL& y, float frequency, int octaves, float m_gradientPerturbAmp) const;

	//3D
	FN_DECIMAL IQNoise_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves) const;
	FN_DECIMAL IQNoiseDeriv_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;

	FN_DECIMAL GetValue_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency) const;
	FN_DECIMAL GetValueFractal_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves) const;
	FN_DECIMAL GetValueDeriv_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL GetValueFractalDeriv_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;

	FN_DECIMAL GetPerlin_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency) const;
	FN_DECIMAL GetPerlinFractal_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves) const;
	FN_DECIMAL GetPerlinDeriv_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL GetPerlinFractalDeriv_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;

	FN_DECIMAL GetSimplex_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency) const;
	FN_DECIMAL GetSimplexFractal_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves) const;

	FN_DECIMAL GetCellular_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency) const;

	FN_DECIMAL GetWhiteNoise_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;
	FN_DECIMAL GetWhiteNoiseInt_3D(int x, int y, int z) const;

	FN_DECIMAL GetCubic_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency) const;
	FN_DECIMAL GetCubicFractal_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, float frequency, int octaves) const;

	//FN_DECIMAL GetNoise(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;

	void GradientPerturb_3D(FN_DECIMAL& x, FN_DECIMAL& y, FN_DECIMAL& z, float frequency, float m_gradientPerturbAmp) const;
	void GradientPerturbFractal_3D(FN_DECIMAL& x, FN_DECIMAL& y, FN_DECIMAL& z, float frequency, int octaves, float m_gradientPerturbAmp) const;

	//4D
	FN_DECIMAL GetSimplex_4D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w, float frequency) const;

	FN_DECIMAL GetWhiteNoise_4D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w) const;
	FN_DECIMAL GetWhiteNoiseInt_4D(int x, int y, int z, int w) const;

private:
	unsigned char m_perm[512];
	unsigned char m_perm12[512];

	int m_seed = 1337;
	//FN_DECIMAL frequency = FN_DECIMAL(0.01);
	Interp m_interp = Quintic;
	//NoiseType m_noiseType = Simplex;

	//int m_octaves = 3;
	FN_DECIMAL m_lacunarity = FN_DECIMAL(2);
	FN_DECIMAL m_gain = FN_DECIMAL(0.5);
	FractalType m_fractalType = FBM;
	FN_DECIMAL m_fractalBounding = 1;

	CellularDistanceFunction m_cellularDistanceFunction = Euclidean;
	CellularReturnType m_cellularReturnType = CellValue;
	FVoxelFastNoise* m_cellularNoiseLookup = nullptr;
	int m_cellularDistanceIndex0 = 0;
	int m_cellularDistanceIndex1 = 1;
	FN_DECIMAL m_cellularJitter = FN_DECIMAL(0.45);

	//FN_DECIMAL m_gradientPerturbAmp = FN_DECIMAL(1);
	
	FMatrix2x2 m_matrix2;
	FMatrix m_matrix3;

	void CalculateFractalBounding(int octaves);

	//2D
	FN_DECIMAL SingleValueFractalFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleValueFractalBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleValueFractalRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleValue_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y) const;
	FN_DECIMAL SingleValueFractalDerivFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SingleValueFractalDerivBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SingleValueFractalDerivRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SingleValueDeriv_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;

	FN_DECIMAL SinglePerlinFractalFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SinglePerlinFractalBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SinglePerlinFractalRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SinglePerlin_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y) const;
	FN_DECIMAL SinglePerlinFractalDerivFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SinglePerlinFractalDerivBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SinglePerlinFractalDerivRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;
	FN_DECIMAL SinglePerlinDeriv_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL& outDx, FN_DECIMAL& outDy) const;

	FN_DECIMAL SingleSimplexFractalFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleSimplexFractalBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleSimplexFractalRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleSimplexFractalBlend_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleSimplex_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y) const;

	FN_DECIMAL SingleCubicFractalFBM_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleCubicFractalBillow_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleCubicFractalRigidMulti_2D(FN_DECIMAL x, FN_DECIMAL y, int octaves) const;
	FN_DECIMAL SingleCubic_2D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y) const;

	FN_DECIMAL SingleCellular_2D(FN_DECIMAL x, FN_DECIMAL y) const;
	FN_DECIMAL SingleCellular2Edge_2D(FN_DECIMAL x, FN_DECIMAL y) const;

	void SingleGradientPerturb_2D(unsigned char offset, FN_DECIMAL warpAmp, FN_DECIMAL frequency, FN_DECIMAL& x, FN_DECIMAL& y) const;

	//3D
	FN_DECIMAL SingleValueFractalFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleValueFractalBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleValueFractalRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleValue_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;
	FN_DECIMAL SingleValueFractalDerivFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SingleValueFractalDerivBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SingleValueFractalDerivRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SingleValueDeriv_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;

	FN_DECIMAL SinglePerlinFractalFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SinglePerlinFractalBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SinglePerlinFractalRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SinglePerlin_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;
	FN_DECIMAL SinglePerlinFractalDerivFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SinglePerlinFractalDerivBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SinglePerlinFractalDerivRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;
	FN_DECIMAL SinglePerlinDeriv_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL& outDx, FN_DECIMAL& outDy, FN_DECIMAL& outDz) const;

	FN_DECIMAL SingleSimplexFractalFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleSimplexFractalBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleSimplexFractalRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleSimplex_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;

	FN_DECIMAL SingleCubicFractalFBM_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleCubicFractalBillow_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleCubicFractalRigidMulti_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, int octaves) const;
	FN_DECIMAL SingleCubic_3D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;

	FN_DECIMAL SingleCellular_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;
	FN_DECIMAL SingleCellular2Edge_3D(FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) const;

	void SingleGradientPerturb_3D(unsigned char offset, FN_DECIMAL warpAmp, FN_DECIMAL frequency, FN_DECIMAL& x, FN_DECIMAL& y, FN_DECIMAL& z) const;

	//4D
	FN_DECIMAL SingleSimplex_4D(unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w) const;

	inline unsigned char Index2D_12(unsigned char offset, int x, int y) const;
	inline unsigned char Index3D_12(unsigned char offset, int x, int y, int z) const;
	inline unsigned char Index4D_32(unsigned char offset, int x, int y, int z, int w) const;
	inline unsigned char Index2D_256(unsigned char offset, int x, int y) const;
	inline unsigned char Index3D_256(unsigned char offset, int x, int y, int z) const;
	inline unsigned char Index4D_256(unsigned char offset, int x, int y, int z, int w) const;

	inline FN_DECIMAL ValCoord2DFast(unsigned char offset, int x, int y) const;
	inline FN_DECIMAL ValCoord3DFast(unsigned char offset, int x, int y, int z) const;
	inline FN_DECIMAL GradCoord2D(unsigned char offset, int x, int y, FN_DECIMAL xd, FN_DECIMAL yd) const;
	inline FN_DECIMAL GradCoord2D(unsigned char offset, int x, int y, FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL& outGradX, FN_DECIMAL& outGradY) const;
	inline FN_DECIMAL GradCoord3D(unsigned char offset, int x, int y, int z, FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL zd) const;
	inline FN_DECIMAL GradCoord3D(unsigned char offset, int x, int y, int z, FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL zd, FN_DECIMAL& outGradX, FN_DECIMAL& outGradY, FN_DECIMAL& outGradZ) const;
	inline FN_DECIMAL GradCoord4D(unsigned char offset, int x, int y, int z, int w, FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL zd, FN_DECIMAL wd) const;
};