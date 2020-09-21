// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Math/TransformCalculus2D.h"
#include "Math/TransformCalculus3D.h"
#include "FastNoise/VoxelFastNoiseLUT.h"
#include "FastNoise/VoxelFastNoiseMath.h"
#include "VoxelFastNoiseBase.generated.h"

UENUM(BlueprintType)
enum class EVoxelNoiseInterpolation : uint8
{
	Linear,
	Hermite,
	Quintic
};

UENUM(BlueprintType)
enum class EVoxelNoiseFractalType : uint8
{
	FBM,
	Billow,
	RigidMulti
};

UENUM(BlueprintType)
enum class EVoxelCellularDistanceFunction : uint8
{
	Euclidean,
	Manhattan,
	Natural
};

UENUM(BlueprintType)
enum class EVoxelCellularReturnType : uint8
{
	CellValue,
	Distance,
	Distance2,
	Distance2Add,
	Distance2Sub,
	Distance2Mul,
	Distance2Div
};

#if INTELLISENSE_PARSER
#define FOREACH_ENUM_EVOXELCELLULARDISTANCEFUNCTION(op) \
	op(EVoxelCellularDistanceFunction::Euclidean) \
	op(EVoxelCellularDistanceFunction::Manhattan) \
	op(EVoxelCellularDistanceFunction::Natural) 
#endif

#define DEFINE_VOXEL_NOISE_CLASS() \
	FORCEINLINE const FVoxelFastNoiseBase& This() const \
	{ \
		return static_cast<const FVoxelFastNoiseBase&>(*this); \
	} \
	using FNoiseMath = FVoxelFastNoiseMath;


class FVoxelFastNoiseBase : public FVoxelFastNoiseLUT
{
public:
	DEFINE_VOXEL_NOISE_CLASS();
	
	// Changes the interpolation method used to smooth between noise values
	// Possible interpolation methods (lowest to highest quality) :
	// - Linear
	// - Hermite
	// - Quintic
	// Used in Value, Perlin Noise and Position Warping
	// Default: Quintic
	void SetInterpolation(EVoxelNoiseInterpolation NewInterpolation) { Interpolation = NewInterpolation; }
	EVoxelNoiseInterpolation GetInterpolation() const { return Interpolation; }
	
	// Sets octave lacunarity for all fractal noise types
	// Default: 2.0
	void SetFractalLacunarity(v_flt NewLacunarity) { Lacunarity = NewLacunarity; }
	v_flt GetFractalLacunarity() const { return Lacunarity; }

	// Sets octave gain for all fractal noise types
	// Default: 0.5
	void SetFractalOctavesAndGain(int32 Octaves, v_flt NewGain) { Gain = NewGain; CalculateFractalBounding(Octaves); }
	v_flt GetFractalGain() const { return Gain; }

	// Sets method for combining octaves in all fractal noise types
	// Default: FBM
	void SetFractalType(EVoxelNoiseFractalType NewFractalType) { FractalType = NewFractalType; }
	EVoxelNoiseFractalType GetFractalType() const { return FractalType; }

	// Used by IQ noise
	void SetMatrix(const FMatrix2x2& NewMatrix) { Matrix2 = NewMatrix; }
	void SetMatrix(const FMatrix& NewMatrix) { Matrix3 = NewMatrix; }

	void SetMatrixFromRotation_2D(float RotationInDegrees) { Matrix2 = FMatrix2x2(FQuat2D(FMath::DegreesToRadians(RotationInDegrees))); }
	void SetMatrixFromRotation_3D(const FRotator& Rotation) { Matrix3 = ToMatrix(Rotation); }
	
	// Sets distance function used in cellular noise calculations
	// Default: Euclidean
	void SetCellularDistanceFunction(EVoxelCellularDistanceFunction NewCellularDistanceFunction) { CellularDistanceFunction = NewCellularDistanceFunction; }
	EVoxelCellularDistanceFunction GetCellularDistanceFunction() const { return CellularDistanceFunction; }

	// Sets return type from cellular noise calculations
	// Note: NoiseLookup requires another FVoxelFastNoise object be set with SetCellularNoiseLookup() to function
	// Default: CellValue
	void SetCellularReturnType(EVoxelCellularReturnType NewCellularReturnType) { CellularReturnType = NewCellularReturnType; }
	EVoxelCellularReturnType GetCellularReturnType() const { return CellularReturnType; }
	
	// Sets the maximum distance a cellular point can move from its grid position
	// Setting this high will make artifacts more common
	// Default: 0.45
	void SetCellularJitter(v_flt NewCellularJitter) { CellularJitter = NewCellularJitter; }
	v_flt GetCellularJitter() const { return CellularJitter; }

	// Higher value = flatter crater borders
	// 0 to disable (much faster disabled)
	void SetCraterFalloffExponent(v_flt NewCraterFalloffExponent) { CraterFalloffExponent = NewCraterFalloffExponent; }
	v_flt GetCraterFalloffExponent() const { return CraterFalloffExponent; }

protected:
	EVoxelNoiseInterpolation Interpolation = EVoxelNoiseInterpolation::Quintic;
	
	v_flt Lacunarity = v_flt(2);
	v_flt Gain = v_flt(0.5);
	EVoxelNoiseFractalType FractalType = EVoxelNoiseFractalType::FBM;
	v_flt FractalBounding = 1;
	
	FMatrix2x2 Matrix2;
	FMatrix Matrix3;
	
	EVoxelCellularDistanceFunction CellularDistanceFunction = EVoxelCellularDistanceFunction::Euclidean;
	EVoxelCellularReturnType CellularReturnType = EVoxelCellularReturnType::CellValue;
	v_flt CellularJitter = v_flt(0.45);
	v_flt CraterFalloffExponent = 0.f;

protected:
	template<typename T>
	void Interpolate_2D(
		T fx, T fy,
		T& xs, T& ys) const;
	template<typename T>
	void Interpolate_2D_Deriv(
		T fx, T fy,
		T& xs, T& ys,
		T& dx, T& dy) const;
	
	template<typename T>
	void Interpolate_3D(
		T fx, T fy, T fz,
		T& xs, T& ys, T& zs) const;
	template<typename T>
	void Interpolate_3D_Deriv(
		T fx, T fy, T fz,
		T& xs, T& ys, T& zs,
		T& dx, T& dy, T& dz) const;
	
protected:
	template<typename T>
	v_flt Fractal_2D(T GetNoise, v_flt x, v_flt y, v_flt frequency, int32 octaves) const;
	template<typename T>
	v_flt Fractal_2D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const;
	
private:
	template<typename T>
	v_flt FractalFBM_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const;
	template<typename T>
	v_flt FractalFBM_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const;
	
	template<typename T>
	v_flt FractalBillow_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const;
	template<typename T>
	v_flt FractalBillow_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const;
	
	template<typename T>
	v_flt FractalRigidMulti_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const;
	template<typename T>
	v_flt FractalRigidMulti_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const;
	
protected:
	template<typename T>
	v_flt Fractal_3D(T GetNoise, v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves) const;
	template<typename T>
	v_flt Fractal_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;

private:
	template<typename T>
	v_flt FractalFBM_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const;
	template<typename T>
	v_flt FractalFBM_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;
	
	template<typename T>
	v_flt FractalBillow_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const;
	template<typename T>
	v_flt FractalBillow_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;
	
	template<typename T>
	v_flt FractalRigidMulti_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const;
	template<typename T>
	v_flt FractalRigidMulti_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;

private:
	void CalculateFractalBounding(int32 Octaves);

	template<typename T> friend class TVoxelFastNoise_ValueNoise;
	template<typename T> friend class TVoxelFastNoise_CubicNoise;
	template<typename T> friend class TVoxelFastNoise_WhiteNoise;
	template<typename T> friend class TVoxelFastNoise_PerlinNoise;
	template<typename T> friend class TVoxelFastNoise_SimplexNoise;
	template<typename T> friend class TVoxelFastNoise_CellularNoise;
	template<typename T> friend class TVoxelFastNoise_GradientPerturb;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define DEFINE_VOXEL_NOISE_LAMBDA(ClassName, Function) \
	struct FLambda_##Function \
	{ \
		const ClassName& This; \
		\
		template<typename... TArgs> \
		FN_FORCEINLINE_MATH decltype(auto) operator()(TArgs&&... Args) const \
		{ \
			return This.Function(Forward<TArgs>(Args)...); \
		} \
	}; \
	friend FLambda_##Function;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_NOISE_FUNCTION_2D(FunctionName) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## _2D(v_flt x, v_flt y, v_flt frequency) const \
	{ \
		return Single ## FunctionName ## _2D(0, x * frequency, y * frequency); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_2D_DERIV(FunctionName) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## _2D_Deriv(v_flt x, v_flt y, v_flt frequency, v_flt& outDx, v_flt& outDy) const \
	{ \
		return Single ## FunctionName ## _2D_Deriv(0, x * frequency, y * frequency, outDx, outDy); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_3D(FunctionName) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## _3D(v_flt x, v_flt y, v_flt z, v_flt frequency) const \
	{ \
		return Single ## FunctionName ## _3D(0, x * frequency, y * frequency, z * frequency); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_3D_DERIV(FunctionName) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## _3D_Deriv(v_flt x, v_flt y, v_flt z, v_flt frequency, v_flt& outDx, v_flt& outDy, v_flt& outDz) const \
	{ \
		return Single ## FunctionName ## _3D_Deriv(0, x * frequency, y * frequency, z * frequency, outDx, outDy, outDz); \
	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(ClassName, FunctionName) \
	DEFINE_VOXEL_NOISE_LAMBDA(TVoxelFastNoise_ ## ClassName ## Noise<T>, Single ## FunctionName ## _2D) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## Fractal_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves) const \
	{ \
		return This().Fractal_2D(FLambda_ ## Single ## FunctionName ## _2D { *this }, x, y, frequency, octaves); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D_DERIV(ClassName, FunctionName) \
	DEFINE_VOXEL_NOISE_LAMBDA(TVoxelFastNoise_ ## ClassName ## Noise<T>, Single ## FunctionName ## _2D_Deriv) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## Fractal_2D_Deriv(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const \
	{ \
		return This().Fractal_2D_Deriv(FLambda_ ## Single ## FunctionName ## _2D_Deriv { *this }, x, y, frequency, octaves, outDx, outDy); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(ClassName, FunctionName) \
	DEFINE_VOXEL_NOISE_LAMBDA(TVoxelFastNoise_ ## ClassName ## Noise<T>, Single ## FunctionName ## _3D) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## Fractal_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves) const \
	{ \
		return This().Fractal_3D(FLambda_ ## Single ## FunctionName ## _3D { *this }, x, y, z, frequency, octaves); \
	}

#define GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D_DERIV(ClassName, FunctionName) \
	DEFINE_VOXEL_NOISE_LAMBDA(TVoxelFastNoise_ ## ClassName ## Noise<T>, Single ## FunctionName ## _3D_Deriv) \
	FN_FORCEINLINE v_flt Get ## FunctionName ## Fractal_3D_Deriv(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const \
	{ \
		return This().Fractal_3D_Deriv(FLambda_ ## Single ## FunctionName ## _3D_Deriv { *this }, x, y, z, frequency, octaves, outDx, outDy, outDz); \
	}