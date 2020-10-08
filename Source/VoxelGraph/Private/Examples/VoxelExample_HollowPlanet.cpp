// Copyright 2020 Phyronnaz

#include "VoxelExample_HollowPlanet.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_HollowPlanetInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_HollowPlanetInstance, UVoxelExample_HollowPlanet>
{
public:
	struct FParams
	{
		const float Intersection_Smoothness;
		const float Noise_Bias;
		const float Noise_Frequency;
		const float Noise_Scale;
		const float Radius;
		const int32 Seed;
		const bool Use_IQ_Noise;
	};
	
	class FLocalComputeStruct_LocalValue
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			void Init(const FVoxelGraphOutputsInit& Init)
			{
			}
			
			template<typename T, uint32 Index>
			T Get() const;
			template<typename T, uint32 Index>
			void Set(T Value);
			
			v_flt Value;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
			v_flt Variable_11; // Radius = 250.0 output 0
			bool Variable_22; // Use IQ Noise = True output 0
			v_flt Variable_16; // Noise Bias = 0.2 output 0
			v_flt Variable_17; // Intersection Smoothness = 10.0 output 0
			v_flt Variable_19; // Noise Frequency = 4.0 output 0
			v_flt Variable_10; // Noise Scale = 20.0 output 0
			v_flt Variable_12; // * -1 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_6; // X output 0
			v_flt Variable_0; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_7; // Y output 0
			v_flt Variable_1; // Y output 0
		};
		
		FLocalComputeStruct_LocalValue(const FParams& InParams)
			: Params(InParams)
		{
		}
		
		void Init(const FVoxelGeneratorInit& InitStruct)
		{
			////////////////////////////////////////////////////
			//////////////////// Init nodes ////////////////////
			////////////////////////////////////////////////////
			{
				////////////////////////////////////////////////////
				/////////////// Constant nodes init ////////////////
				////////////////////////////////////////////////////
				{
					/////////////////////////////////////////////////////////////////////////////////
					//////// First compute all seeds in case they are used by constant nodes ////////
					/////////////////////////////////////////////////////////////////////////////////
					
					// Init of Seed = 1443
					FVoxelGraphSeed Variable_20; // Seed = 1443 output 0
					Variable_20 = Params.Seed;
					
					
					////////////////////////////////////////////////////
					///////////// Then init constant nodes /////////////
					////////////////////////////////////////////////////
					
				}
				
				////////////////////////////////////////////////////
				//////////////////// Other inits ///////////////////
				////////////////////////////////////////////////////
				Function0_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Radius = 250.0
				BufferConstant.Variable_11 = Params.Radius;
				
				// Use IQ Noise = True
				BufferConstant.Variable_22 = Params.Use_IQ_Noise;
				
				// Noise Bias = 0.2
				BufferConstant.Variable_16 = Params.Noise_Bias;
				
				// Intersection Smoothness = 10.0
				BufferConstant.Variable_17 = Params.Intersection_Smoothness;
				
				// Noise Frequency = 4.0
				BufferConstant.Variable_19 = Params.Noise_Frequency;
				
				// Noise Scale = 20.0
				BufferConstant.Variable_10 = Params.Noise_Scale;
				
				// * -1
				BufferConstant.Variable_12 = BufferConstant.Variable_10 * -1;
				
			}
		}
		void ComputeX(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			Function0_X_Compute(Context, BufferX);
		}
		void ComputeXYWithCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYWithoutCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithoutCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYZWithCache(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			Function0_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs);
		}
		void ComputeXYZWithoutCache(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			Function0_XYZWithoutCache_Compute(Context, Outputs);
		}
		
		inline FBufferX GetBufferX() const { return {}; }
		inline FBufferXY GetBufferXY() const { return {}; }
		inline FOutputs GetOutputs() const { return {}; }
		
	private:
		FBufferConstant BufferConstant;
		
		const FParams& Params;
		
		FVoxelFastNoise _3D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _3D_IQ_Noise_0_LODToOctaves;
		FVoxelFastNoise _3D_Perlin_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _3D_Perlin_Noise_Fractal_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 1443
			FVoxelGraphSeed Variable_20; // Seed = 1443 output 0
			Variable_20 = Params.Seed;
			
			// Init of 3D IQ Noise
			_3D_IQ_Noise_0_Noise.SetSeed(Variable_20);
			_3D_IQ_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.5);
			_3D_IQ_Noise_0_Noise.SetFractalLacunarity(2.0);
			_3D_IQ_Noise_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_IQ_Noise_0_Noise.SetMatrixFromRotation_3D(FRotator(40.000000, 45.000000, 50.000000));
			_3D_IQ_Noise_0_LODToOctaves[0] = 15;
			_3D_IQ_Noise_0_LODToOctaves[1] = 15;
			_3D_IQ_Noise_0_LODToOctaves[2] = 15;
			_3D_IQ_Noise_0_LODToOctaves[3] = 15;
			_3D_IQ_Noise_0_LODToOctaves[4] = 15;
			_3D_IQ_Noise_0_LODToOctaves[5] = 15;
			_3D_IQ_Noise_0_LODToOctaves[6] = 15;
			_3D_IQ_Noise_0_LODToOctaves[7] = 15;
			_3D_IQ_Noise_0_LODToOctaves[8] = 15;
			_3D_IQ_Noise_0_LODToOctaves[9] = 15;
			_3D_IQ_Noise_0_LODToOctaves[10] = 15;
			_3D_IQ_Noise_0_LODToOctaves[11] = 15;
			_3D_IQ_Noise_0_LODToOctaves[12] = 15;
			_3D_IQ_Noise_0_LODToOctaves[13] = 15;
			_3D_IQ_Noise_0_LODToOctaves[14] = 15;
			_3D_IQ_Noise_0_LODToOctaves[15] = 15;
			_3D_IQ_Noise_0_LODToOctaves[16] = 15;
			_3D_IQ_Noise_0_LODToOctaves[17] = 15;
			_3D_IQ_Noise_0_LODToOctaves[18] = 15;
			_3D_IQ_Noise_0_LODToOctaves[19] = 15;
			_3D_IQ_Noise_0_LODToOctaves[20] = 15;
			_3D_IQ_Noise_0_LODToOctaves[21] = 15;
			_3D_IQ_Noise_0_LODToOctaves[22] = 15;
			_3D_IQ_Noise_0_LODToOctaves[23] = 15;
			_3D_IQ_Noise_0_LODToOctaves[24] = 15;
			_3D_IQ_Noise_0_LODToOctaves[25] = 15;
			_3D_IQ_Noise_0_LODToOctaves[26] = 15;
			_3D_IQ_Noise_0_LODToOctaves[27] = 15;
			_3D_IQ_Noise_0_LODToOctaves[28] = 15;
			_3D_IQ_Noise_0_LODToOctaves[29] = 15;
			_3D_IQ_Noise_0_LODToOctaves[30] = 15;
			_3D_IQ_Noise_0_LODToOctaves[31] = 15;
			
			// Init of 3D Perlin Noise Fractal
			_3D_Perlin_Noise_Fractal_0_Noise.SetSeed(Variable_20);
			_3D_Perlin_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(3, 0.5);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[0] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[1] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[2] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[3] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[4] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[5] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[6] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[7] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[8] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[9] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[10] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[11] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[12] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[13] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[14] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[15] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[16] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[17] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[18] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[19] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[20] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[21] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[22] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[23] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[24] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[25] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[26] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[27] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[28] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[29] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[30] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[31] = 3;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_6 = Context.GetLocalX();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_7 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_6 = Context.GetLocalX();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_7 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Normalize.Vector Length
			v_flt Variable_24; // Normalize.Vector Length output 0
			Variable_24 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferXY.Variable_1, Variable_2);
			
			// Vector Length
			v_flt Variable_5; // Vector Length output 0
			Variable_5 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_6, BufferXY.Variable_7, Variable_8);
			
			// Normalize./
			v_flt Variable_25; // Normalize./ output 0
			Variable_25 = BufferX.Variable_0 / Variable_24;
			
			// Normalize./
			v_flt Variable_26; // Normalize./ output 0
			Variable_26 = BufferXY.Variable_1 / Variable_24;
			
			// Normalize./
			v_flt Variable_27; // Normalize./ output 0
			Variable_27 = Variable_2 / Variable_24;
			
			// 3D IQ Noise
			v_flt Variable_18; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_0_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_0_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_0_Temp_3; // 3D IQ Noise output 3
			Variable_18 = _3D_IQ_Noise_0_Noise.IQNoise_3D_Deriv(Variable_25, Variable_26, Variable_27, BufferConstant.Variable_19, _3D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_0_Temp_1,_3D_IQ_Noise_0_Temp_2,_3D_IQ_Noise_0_Temp_3);
			Variable_18 = FMath::Clamp<v_flt>(Variable_18, -0.732619, 0.767129);
			_3D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_1, -1.577728, 1.771872);
			_3D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_2, -1.779903, 1.388687);
			_3D_IQ_Noise_0_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_3, -1.667376, 1.695596);
			
			// 3D Perlin Noise Fractal
			v_flt Variable_3; // 3D Perlin Noise Fractal output 0
			Variable_3 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(Variable_25, Variable_26, Variable_27, BufferConstant.Variable_19, _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_3 = FMath::Clamp<v_flt>(Variable_3, -0.419158, 0.458317);
			
			// Switch (float)
			v_flt Variable_21; // Switch (float) output 0
			Variable_21 = FVoxelNodeFunctions::Switch(Variable_18, Variable_3, BufferConstant.Variable_22);
			
			// +
			v_flt Variable_14; // + output 0
			Variable_14 = Variable_21 + BufferConstant.Variable_16;
			
			// 2D Noise SDF.*
			v_flt Variable_29; // 2D Noise SDF.* output 0
			Variable_29 = Variable_14 * BufferConstant.Variable_10;
			
			// 2D Noise SDF.*
			v_flt Variable_31; // 2D Noise SDF.* output 0
			Variable_31 = Variable_14 * BufferConstant.Variable_12;
			
			// 2D Noise SDF.+
			v_flt Variable_9; // 2D Noise SDF.+ output 0
			Variable_9 = Variable_31 + BufferConstant.Variable_11;
			
			// 2D Noise SDF.+
			v_flt Variable_4; // 2D Noise SDF.+ output 0
			Variable_4 = Variable_29 + BufferConstant.Variable_11;
			
			// 2D Noise SDF.-
			v_flt Variable_28; // 2D Noise SDF.- output 0
			Variable_28 = Variable_5 - Variable_4;
			
			// 2D Noise SDF.-
			v_flt Variable_30; // 2D Noise SDF.- output 0
			Variable_30 = Variable_5 - Variable_9;
			
			// * -1
			v_flt Variable_13; // * -1 output 0
			Variable_13 = Variable_30 * -1;
			
			// Smooth Intersection
			v_flt Variable_15; // Smooth Intersection output 0
			Variable_15 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_28, Variable_13, BufferConstant.Variable_17);
			
			// Set High Quality Value.*
			v_flt Variable_23; // Set High Quality Value.* output 0
			Variable_23 = Variable_15 * v_flt(0.2f);
			
			Outputs.Value = Variable_23;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_6; // X output 0
			Variable_6 = Context.GetLocalX();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Y
			v_flt Variable_7; // Y output 0
			Variable_7 = Context.GetLocalY();
			
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Normalize.Vector Length
			v_flt Variable_24; // Normalize.Vector Length output 0
			Variable_24 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// Vector Length
			v_flt Variable_5; // Vector Length output 0
			Variable_5 = FVoxelNodeFunctions::VectorLength(Variable_6, Variable_7, Variable_8);
			
			// Normalize./
			v_flt Variable_25; // Normalize./ output 0
			Variable_25 = Variable_0 / Variable_24;
			
			// Normalize./
			v_flt Variable_26; // Normalize./ output 0
			Variable_26 = Variable_1 / Variable_24;
			
			// Normalize./
			v_flt Variable_27; // Normalize./ output 0
			Variable_27 = Variable_2 / Variable_24;
			
			// 3D IQ Noise
			v_flt Variable_18; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_0_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_0_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_0_Temp_3; // 3D IQ Noise output 3
			Variable_18 = _3D_IQ_Noise_0_Noise.IQNoise_3D_Deriv(Variable_25, Variable_26, Variable_27, BufferConstant.Variable_19, _3D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_0_Temp_1,_3D_IQ_Noise_0_Temp_2,_3D_IQ_Noise_0_Temp_3);
			Variable_18 = FMath::Clamp<v_flt>(Variable_18, -0.732619, 0.767129);
			_3D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_1, -1.577728, 1.771872);
			_3D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_2, -1.779903, 1.388687);
			_3D_IQ_Noise_0_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_3, -1.667376, 1.695596);
			
			// 3D Perlin Noise Fractal
			v_flt Variable_3; // 3D Perlin Noise Fractal output 0
			Variable_3 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(Variable_25, Variable_26, Variable_27, BufferConstant.Variable_19, _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_3 = FMath::Clamp<v_flt>(Variable_3, -0.419158, 0.458317);
			
			// Switch (float)
			v_flt Variable_21; // Switch (float) output 0
			Variable_21 = FVoxelNodeFunctions::Switch(Variable_18, Variable_3, BufferConstant.Variable_22);
			
			// +
			v_flt Variable_14; // + output 0
			Variable_14 = Variable_21 + BufferConstant.Variable_16;
			
			// 2D Noise SDF.*
			v_flt Variable_29; // 2D Noise SDF.* output 0
			Variable_29 = Variable_14 * BufferConstant.Variable_10;
			
			// 2D Noise SDF.*
			v_flt Variable_31; // 2D Noise SDF.* output 0
			Variable_31 = Variable_14 * BufferConstant.Variable_12;
			
			// 2D Noise SDF.+
			v_flt Variable_9; // 2D Noise SDF.+ output 0
			Variable_9 = Variable_31 + BufferConstant.Variable_11;
			
			// 2D Noise SDF.+
			v_flt Variable_4; // 2D Noise SDF.+ output 0
			Variable_4 = Variable_29 + BufferConstant.Variable_11;
			
			// 2D Noise SDF.-
			v_flt Variable_28; // 2D Noise SDF.- output 0
			Variable_28 = Variable_5 - Variable_4;
			
			// 2D Noise SDF.-
			v_flt Variable_30; // 2D Noise SDF.- output 0
			Variable_30 = Variable_5 - Variable_9;
			
			// * -1
			v_flt Variable_13; // * -1 output 0
			Variable_13 = Variable_30 * -1;
			
			// Smooth Intersection
			v_flt Variable_15; // Smooth Intersection output 0
			Variable_15 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_28, Variable_13, BufferConstant.Variable_17);
			
			// Set High Quality Value.*
			v_flt Variable_23; // Set High Quality Value.* output 0
			Variable_23 = Variable_15 * v_flt(0.2f);
			
			Outputs.Value = Variable_23;
		}
		
	};
	class FLocalComputeStruct_LocalMaterial
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			void Init(const FVoxelGraphOutputsInit& Init)
			{
				MaterialBuilder.SetMaterialConfig(Init.MaterialConfig);
			}
			
			template<typename T, uint32 Index>
			T Get() const;
			template<typename T, uint32 Index>
			void Set(T Value);
			
			FVoxelMaterialBuilder MaterialBuilder;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
		};
		
		FLocalComputeStruct_LocalMaterial(const FParams& InParams)
			: Params(InParams)
		{
		}
		
		void Init(const FVoxelGeneratorInit& InitStruct)
		{
			////////////////////////////////////////////////////
			//////////////////// Init nodes ////////////////////
			////////////////////////////////////////////////////
			{
				////////////////////////////////////////////////////
				/////////////// Constant nodes init ////////////////
				////////////////////////////////////////////////////
				{
					/////////////////////////////////////////////////////////////////////////////////
					//////// First compute all seeds in case they are used by constant nodes ////////
					/////////////////////////////////////////////////////////////////////////////////
					
					
					////////////////////////////////////////////////////
					///////////// Then init constant nodes /////////////
					////////////////////////////////////////////////////
					
				}
				
				////////////////////////////////////////////////////
				//////////////////// Other inits ///////////////////
				////////////////////////////////////////////////////
				Function0_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
			}
		}
		void ComputeX(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			Function0_X_Compute(Context, BufferX);
		}
		void ComputeXYWithCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYWithoutCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithoutCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYZWithCache(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			Function0_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs);
		}
		void ComputeXYZWithoutCache(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			Function0_XYZWithoutCache_Compute(Context, Outputs);
		}
		
		inline FBufferX GetBufferX() const { return {}; }
		inline FBufferXY GetBufferXY() const { return {}; }
		inline FOutputs GetOutputs() const { return {}; }
		
	private:
		FBufferConstant BufferConstant;
		
		const FParams& Params;
		
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
		}
		
	};
	class FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			void Init(const FVoxelGraphOutputsInit& Init)
			{
			}
			
			template<typename T, uint32 Index>
			T Get() const;
			template<typename T, uint32 Index>
			void Set(T Value);
			
			v_flt UpVectorX;
			v_flt UpVectorY;
			v_flt UpVectorZ;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
		};
		
		FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ(const FParams& InParams)
			: Params(InParams)
		{
		}
		
		void Init(const FVoxelGeneratorInit& InitStruct)
		{
			////////////////////////////////////////////////////
			//////////////////// Init nodes ////////////////////
			////////////////////////////////////////////////////
			{
				////////////////////////////////////////////////////
				/////////////// Constant nodes init ////////////////
				////////////////////////////////////////////////////
				{
					/////////////////////////////////////////////////////////////////////////////////
					//////// First compute all seeds in case they are used by constant nodes ////////
					/////////////////////////////////////////////////////////////////////////////////
					
					
					////////////////////////////////////////////////////
					///////////// Then init constant nodes /////////////
					////////////////////////////////////////////////////
					
				}
				
				////////////////////////////////////////////////////
				//////////////////// Other inits ///////////////////
				////////////////////////////////////////////////////
				Function0_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
			}
		}
		void ComputeX(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			Function0_X_Compute(Context, BufferX);
		}
		void ComputeXYWithCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYWithoutCache(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			Function0_XYWithoutCache_Compute(Context, BufferX, BufferXY);
		}
		void ComputeXYZWithCache(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			Function0_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs);
		}
		void ComputeXYZWithoutCache(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			Function0_XYZWithoutCache_Compute(Context, Outputs);
		}
		
		inline FBufferX GetBufferX() const { return {}; }
		inline FBufferXY GetBufferXY() const { return {}; }
		inline FOutputs GetOutputs() const { return {}; }
		
	private:
		FBufferConstant BufferConstant;
		
		const FParams& Params;
		
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
		}
		
	};
	class FLocalComputeStruct_LocalValueRangeAnalysis
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			void Init(const FVoxelGraphOutputsInit& Init)
			{
			}
			
			template<typename T, uint32 Index>
			TVoxelRange<T> Get() const;
			template<typename T, uint32 Index>
			void Set(TVoxelRange<T> Value);
			
			TVoxelRange<v_flt> Value;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
			TVoxelRange<v_flt> Variable_14; // Intersection Smoothness = 10.0 output 0
			TVoxelRange<v_flt> Variable_6; // 2D Noise SDF.+ output 0
			TVoxelRange<v_flt> Variable_1; // 2D Noise SDF.+ output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_3; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_4; // Y output 0
		};
		
		FLocalComputeStruct_LocalValueRangeAnalysis(const FParams& InParams)
			: Params(InParams)
		{
		}
		
		void Init(const FVoxelGeneratorInit& InitStruct)
		{
			////////////////////////////////////////////////////
			//////////////////// Init nodes ////////////////////
			////////////////////////////////////////////////////
			{
				////////////////////////////////////////////////////
				/////////////// Constant nodes init ////////////////
				////////////////////////////////////////////////////
				{
					/////////////////////////////////////////////////////////////////////////////////
					//////// First compute all seeds in case they are used by constant nodes ////////
					/////////////////////////////////////////////////////////////////////////////////
					
					
					////////////////////////////////////////////////////
					///////////// Then init constant nodes /////////////
					////////////////////////////////////////////////////
					
					// Init of 3D Perlin Noise Fractal
					_3D_Perlin_Noise_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_Perlin_Noise_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(3, 0.5);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[0] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[1] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[2] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[3] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[4] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[5] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[6] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[7] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[8] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[9] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[10] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[11] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[12] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[13] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[14] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[15] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[16] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[17] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[18] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[19] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[20] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[21] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[22] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[23] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[24] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[25] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[26] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[27] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[28] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[29] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[30] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[31] = 3;
					
					// Init of 3D IQ Noise
					_3D_IQ_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_IQ_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_3D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.5);
					_3D_IQ_Noise_1_Noise.SetFractalLacunarity(2.0);
					_3D_IQ_Noise_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_IQ_Noise_1_Noise.SetMatrixFromRotation_3D(FRotator(40.000000, 45.000000, 50.000000));
					_3D_IQ_Noise_1_LODToOctaves[0] = 15;
					_3D_IQ_Noise_1_LODToOctaves[1] = 15;
					_3D_IQ_Noise_1_LODToOctaves[2] = 15;
					_3D_IQ_Noise_1_LODToOctaves[3] = 15;
					_3D_IQ_Noise_1_LODToOctaves[4] = 15;
					_3D_IQ_Noise_1_LODToOctaves[5] = 15;
					_3D_IQ_Noise_1_LODToOctaves[6] = 15;
					_3D_IQ_Noise_1_LODToOctaves[7] = 15;
					_3D_IQ_Noise_1_LODToOctaves[8] = 15;
					_3D_IQ_Noise_1_LODToOctaves[9] = 15;
					_3D_IQ_Noise_1_LODToOctaves[10] = 15;
					_3D_IQ_Noise_1_LODToOctaves[11] = 15;
					_3D_IQ_Noise_1_LODToOctaves[12] = 15;
					_3D_IQ_Noise_1_LODToOctaves[13] = 15;
					_3D_IQ_Noise_1_LODToOctaves[14] = 15;
					_3D_IQ_Noise_1_LODToOctaves[15] = 15;
					_3D_IQ_Noise_1_LODToOctaves[16] = 15;
					_3D_IQ_Noise_1_LODToOctaves[17] = 15;
					_3D_IQ_Noise_1_LODToOctaves[18] = 15;
					_3D_IQ_Noise_1_LODToOctaves[19] = 15;
					_3D_IQ_Noise_1_LODToOctaves[20] = 15;
					_3D_IQ_Noise_1_LODToOctaves[21] = 15;
					_3D_IQ_Noise_1_LODToOctaves[22] = 15;
					_3D_IQ_Noise_1_LODToOctaves[23] = 15;
					_3D_IQ_Noise_1_LODToOctaves[24] = 15;
					_3D_IQ_Noise_1_LODToOctaves[25] = 15;
					_3D_IQ_Noise_1_LODToOctaves[26] = 15;
					_3D_IQ_Noise_1_LODToOctaves[27] = 15;
					_3D_IQ_Noise_1_LODToOctaves[28] = 15;
					_3D_IQ_Noise_1_LODToOctaves[29] = 15;
					_3D_IQ_Noise_1_LODToOctaves[30] = 15;
					_3D_IQ_Noise_1_LODToOctaves[31] = 15;
					
				}
				
				////////////////////////////////////////////////////
				//////////////////// Other inits ///////////////////
				////////////////////////////////////////////////////
				Function0_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Noise Bias = 0.2
				TVoxelRange<v_flt> Variable_13; // Noise Bias = 0.2 output 0
				Variable_13 = Params.Noise_Bias;
				
				// 3D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_0; // 3D Perlin Noise Fractal output 0
				Variable_0 = { -0.419158f, 0.458317f };
				
				// Intersection Smoothness = 10.0
				BufferConstant.Variable_14 = Params.Intersection_Smoothness;
				
				// Noise Scale = 20.0
				TVoxelRange<v_flt> Variable_7; // Noise Scale = 20.0 output 0
				Variable_7 = Params.Noise_Scale;
				
				// Radius = 250.0
				TVoxelRange<v_flt> Variable_8; // Radius = 250.0 output 0
				Variable_8 = Params.Radius;
				
				// 3D IQ Noise
				TVoxelRange<v_flt> Variable_15; // 3D IQ Noise output 0
				TVoxelRange<v_flt> _3D_IQ_Noise_1_Temp_1; // 3D IQ Noise output 1
				TVoxelRange<v_flt> _3D_IQ_Noise_1_Temp_2; // 3D IQ Noise output 2
				TVoxelRange<v_flt> _3D_IQ_Noise_1_Temp_3; // 3D IQ Noise output 3
				Variable_15 = { -0.732619f, 0.767129f };
				_3D_IQ_Noise_1_Temp_1 = { -1.577728f, 1.771872f };
				_3D_IQ_Noise_1_Temp_2 = { -1.779903f, 1.388687f };
				_3D_IQ_Noise_1_Temp_3 = { -1.667376f, 1.695596f };
				
				// Use IQ Noise = True
				FVoxelBoolRange Variable_17; // Use IQ Noise = True output 0
				Variable_17 = Params.Use_IQ_Noise;
				
				// Switch (float)
				TVoxelRange<v_flt> Variable_16; // Switch (float) output 0
				Variable_16 = FVoxelNodeFunctions::Switch(Variable_15, Variable_0, Variable_17);
				
				// * -1
				TVoxelRange<v_flt> Variable_9; // * -1 output 0
				Variable_9 = Variable_7 * -1;
				
				// +
				TVoxelRange<v_flt> Variable_11; // + output 0
				Variable_11 = Variable_16 + Variable_13;
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_20; // 2D Noise SDF.* output 0
				Variable_20 = Variable_11 * Variable_7;
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_22; // 2D Noise SDF.* output 0
				Variable_22 = Variable_11 * Variable_9;
				
				// 2D Noise SDF.+
				BufferConstant.Variable_6 = Variable_22 + Variable_8;
				
				// 2D Noise SDF.+
				BufferConstant.Variable_1 = Variable_20 + Variable_8;
				
			}
		}
		void ComputeXYZWithoutCache(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			Function0_XYZWithoutCache_Compute(Context, Outputs);
		}
		
		inline FBufferX GetBufferX() const { return {}; }
		inline FBufferXY GetBufferXY() const { return {}; }
		inline FOutputs GetOutputs() const { return {}; }
		
	private:
		FBufferConstant BufferConstant;
		
		const FParams& Params;
		
		FVoxelFastNoise _3D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _3D_Perlin_Noise_Fractal_1_LODToOctaves;
		FVoxelFastNoise _3D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _3D_IQ_Noise_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// X
			TVoxelRange<v_flt> Variable_3; // X output 0
			Variable_3 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_4; // Y output 0
			Variable_4 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_5; // Z output 0
			Variable_5 = Context.GetLocalZ();
			
			// Vector Length
			TVoxelRange<v_flt> Variable_2; // Vector Length output 0
			Variable_2 = FVoxelNodeFunctions::VectorLength(Variable_3, Variable_4, Variable_5);
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_21; // 2D Noise SDF.- output 0
			Variable_21 = Variable_2 - BufferConstant.Variable_6;
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_19; // 2D Noise SDF.- output 0
			Variable_19 = Variable_2 - BufferConstant.Variable_1;
			
			// * -1
			TVoxelRange<v_flt> Variable_10; // * -1 output 0
			Variable_10 = Variable_21 * -1;
			
			// Smooth Intersection
			TVoxelRange<v_flt> Variable_12; // Smooth Intersection output 0
			Variable_12 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_19, Variable_10, BufferConstant.Variable_14);
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_18; // Set High Quality Value.* output 0
			Variable_18 = Variable_12 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_18;
		}
		
	};
	
	FVoxelExample_HollowPlanetInstance(UVoxelExample_HollowPlanet& Object)
			: TVoxelGraphGeneratorInstanceHelper(
			{
				{ "Value", 1 },
			},
			{
			},
			{
			},
			{
				{
					{ "Value", NoTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr<v_flt>>() },
				},
				{
				},
				{
				},
				{
					{ "Value", NoTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr<v_flt>>() },
				}
			},
			{
				{
					{ "Value", WithTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr_Transform<v_flt>>() },
				},
				{
				},
				{
				},
				{
					{ "Value", WithTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr_Transform<v_flt>>() },
				}
			},
			Object)
		, Params(FParams
		{
			Object.Intersection_Smoothness,
			Object.Noise_Bias,
			Object.Noise_Frequency,
			Object.Noise_Scale,
			Object.Radius,
			Object.Seed,
			Object.Use_IQ_Noise
		})
		, LocalValue(Params)
		, LocalMaterial(Params)
		, LocalUpVectorXUpVectorYUpVectorZ(Params)
		, LocalValueRangeAnalysis(Params)
	{
	}
	
	virtual void InitGraph(const FVoxelGeneratorInit& InitStruct) override final
	{
		LocalValue.Init(InitStruct);
		LocalMaterial.Init(InitStruct);
		LocalUpVectorXUpVectorYUpVectorZ.Init(InitStruct);
		LocalValueRangeAnalysis.Init(InitStruct);
	}
	
	template<uint32... Permutation>
	auto& GetTarget() const;
	
	template<uint32... Permutation>
	auto& GetRangeTarget() const;
	
private:
	FParams Params;
	FLocalComputeStruct_LocalValue LocalValue;
	FLocalComputeStruct_LocalMaterial LocalMaterial;
	FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ LocalUpVectorXUpVectorYUpVectorZ;
	FLocalComputeStruct_LocalValueRangeAnalysis LocalValueRangeAnalysis;
	
};

template<>
inline v_flt FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_HollowPlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_HollowPlanetInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_HollowPlanetInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_HollowPlanetInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_HollowPlanetInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_HollowPlanet::UVoxelExample_HollowPlanet()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_HollowPlanet::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_HollowPlanetInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_HollowPlanet. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_HollowPlanet. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_HollowPlanet. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_HollowPlanet. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
