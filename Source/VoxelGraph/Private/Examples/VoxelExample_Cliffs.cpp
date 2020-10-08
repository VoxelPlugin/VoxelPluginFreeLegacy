// Copyright 2020 Phyronnaz

#include "VoxelExample_Cliffs.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_CliffsInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_CliffsInstance, UVoxelExample_Cliffs>
{
public:
	struct FParams
	{
		const float Cliffs_Slope;
		const float Height;
		const float Overhangs;
		const float Base_Shape_Frequency;
		const float Base_Shape_Offset;
		const int32 Base_Shape_Seed;
		const int32 Sides_Noise_Seed;
		const int32 Top_Noise_Seed;
		const float Sides_Noise_Amplitude;
		const float Sides_Noise_Frequency;
		const float Top_Noise_Frequency;
		const float Top_Noise_Scale;
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
			
			v_flt Variable_21; // Cliffs Slope = 10.0 output 0
			v_flt Variable_27; // Sides Noise Amplitude = 0.2 output 0
			v_flt Variable_28; // Height = 50.0 output 0
			v_flt Variable_29; // Overhangs = 0.2 output 0
			v_flt Variable_30; // Base Shape Frequency = 0.005 output 0
			v_flt Variable_33; // Sides Noise Frequency = 0.1 output 0
			v_flt Variable_35; // Top Noise Frequency = 0.01 output 0
			v_flt Variable_37; // Top Noise Scale = 25.0 output 0
			v_flt Variable_32; // / output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_0; // X output 0
			v_flt Variable_14; // X output 0
			v_flt Variable_24; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_19; // + output 0
			v_flt Variable_17; // * output 0
			v_flt Variable_26; // * output 0
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
					
					// Init of Base Shape Seed = 3323
					FVoxelGraphSeed Variable_31; // Base Shape Seed = 3323 output 0
					Variable_31 = Params.Base_Shape_Seed;
					
					// Init of Sides Noise Seed = 2647
					FVoxelGraphSeed Variable_34; // Sides Noise Seed = 2647 output 0
					Variable_34 = Params.Sides_Noise_Seed;
					
					// Init of Top Noise Seed = 12932
					FVoxelGraphSeed Variable_36; // Top Noise Seed = 12932 output 0
					Variable_36 = Params.Top_Noise_Seed;
					
					
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
				// Cliffs Slope = 10.0
				BufferConstant.Variable_21 = Params.Cliffs_Slope;
				
				// Sides Noise Amplitude = 0.2
				BufferConstant.Variable_27 = Params.Sides_Noise_Amplitude;
				
				// Height = 50.0
				BufferConstant.Variable_28 = Params.Height;
				
				// Overhangs = 0.2
				BufferConstant.Variable_29 = Params.Overhangs;
				
				// Base Shape Frequency = 0.005
				BufferConstant.Variable_30 = Params.Base_Shape_Frequency;
				
				// Base Shape Offset = 0.0
				v_flt Variable_20; // Base Shape Offset = 0.0 output 0
				Variable_20 = Params.Base_Shape_Offset;
				
				// Sides Noise Frequency = 0.1
				BufferConstant.Variable_33 = Params.Sides_Noise_Frequency;
				
				// Top Noise Frequency = 0.01
				BufferConstant.Variable_35 = Params.Top_Noise_Frequency;
				
				// Top Noise Scale = 25.0
				BufferConstant.Variable_37 = Params.Top_Noise_Scale;
				
				// /
				BufferConstant.Variable_32 = Variable_20 / v_flt(10.0f);
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_0_LODToOctaves;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_1_LODToOctaves;
		FVoxelFastNoise _2D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Top Noise Seed = 12932
			FVoxelGraphSeed Variable_36; // Top Noise Seed = 12932 output 0
			Variable_36 = Params.Top_Noise_Seed;
			
			// Init of Base Shape Seed = 3323
			FVoxelGraphSeed Variable_31; // Base Shape Seed = 3323 output 0
			Variable_31 = Params.Base_Shape_Seed;
			
			// Init of Sides Noise Seed = 2647
			FVoxelGraphSeed Variable_34; // Sides Noise Seed = 2647 output 0
			Variable_34 = Params.Sides_Noise_Seed;
			
			// Init of 2D Perlin Noise Fractal
			_2D_Perlin_Noise_Fractal_0_Noise.SetSeed(Variable_31);
			_2D_Perlin_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[0] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[1] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[2] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[3] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[4] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[5] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[6] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[7] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[8] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[9] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[10] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[11] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[12] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[13] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[14] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[15] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[16] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[17] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[18] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[19] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[20] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[21] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[22] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[23] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[24] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[25] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[26] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[27] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[28] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[29] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[30] = 3;
			_2D_Perlin_Noise_Fractal_0_LODToOctaves[31] = 3;
			
			// Init of 2D Perlin Noise Fractal
			_2D_Perlin_Noise_Fractal_1_Noise.SetSeed(Variable_34);
			_2D_Perlin_Noise_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[0] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[1] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[2] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[3] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[4] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[5] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[6] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[7] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[8] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[9] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[10] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[11] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[12] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[13] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[14] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[15] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[16] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[17] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[18] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[19] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[20] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[21] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[22] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[23] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[24] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[25] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[26] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[27] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[28] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[29] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[30] = 3;
			_2D_Perlin_Noise_Fractal_1_LODToOctaves[31] = 3;
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_36);
			_2D_IQ_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_0_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_IQ_Noise_0_Noise.SetMatrixFromRotation_2D(40.0);
			_2D_IQ_Noise_0_LODToOctaves[0] = 15;
			_2D_IQ_Noise_0_LODToOctaves[1] = 15;
			_2D_IQ_Noise_0_LODToOctaves[2] = 15;
			_2D_IQ_Noise_0_LODToOctaves[3] = 15;
			_2D_IQ_Noise_0_LODToOctaves[4] = 15;
			_2D_IQ_Noise_0_LODToOctaves[5] = 15;
			_2D_IQ_Noise_0_LODToOctaves[6] = 15;
			_2D_IQ_Noise_0_LODToOctaves[7] = 15;
			_2D_IQ_Noise_0_LODToOctaves[8] = 15;
			_2D_IQ_Noise_0_LODToOctaves[9] = 15;
			_2D_IQ_Noise_0_LODToOctaves[10] = 15;
			_2D_IQ_Noise_0_LODToOctaves[11] = 15;
			_2D_IQ_Noise_0_LODToOctaves[12] = 15;
			_2D_IQ_Noise_0_LODToOctaves[13] = 15;
			_2D_IQ_Noise_0_LODToOctaves[14] = 15;
			_2D_IQ_Noise_0_LODToOctaves[15] = 15;
			_2D_IQ_Noise_0_LODToOctaves[16] = 15;
			_2D_IQ_Noise_0_LODToOctaves[17] = 15;
			_2D_IQ_Noise_0_LODToOctaves[18] = 15;
			_2D_IQ_Noise_0_LODToOctaves[19] = 15;
			_2D_IQ_Noise_0_LODToOctaves[20] = 15;
			_2D_IQ_Noise_0_LODToOctaves[21] = 15;
			_2D_IQ_Noise_0_LODToOctaves[22] = 15;
			_2D_IQ_Noise_0_LODToOctaves[23] = 15;
			_2D_IQ_Noise_0_LODToOctaves[24] = 15;
			_2D_IQ_Noise_0_LODToOctaves[25] = 15;
			_2D_IQ_Noise_0_LODToOctaves[26] = 15;
			_2D_IQ_Noise_0_LODToOctaves[27] = 15;
			_2D_IQ_Noise_0_LODToOctaves[28] = 15;
			_2D_IQ_Noise_0_LODToOctaves[29] = 15;
			_2D_IQ_Noise_0_LODToOctaves[30] = 15;
			_2D_IQ_Noise_0_LODToOctaves[31] = 15;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
			// X
			BufferX.Variable_24 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// Y
			v_flt Variable_25; // Y output 0
			Variable_25 = Context.GetLocalY();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_30, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.576700, 0.658489);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_23; // 2D Perlin Noise Fractal output 0
			Variable_23 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_24, Variable_25, BufferConstant.Variable_33, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.594076, 0.593483);
			
			// 2D IQ Noise
			v_flt Variable_16; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_16 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_14, Variable_15, BufferConstant.Variable_35, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -0.648057, 0.611352);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.300998, 1.397865);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.723871, 1.259353);
			
			// +
			BufferXY.Variable_19 = Variable_2 + BufferConstant.Variable_32;
			
			// *
			BufferXY.Variable_17 = Variable_16 * BufferConstant.Variable_37;
			
			// *
			BufferXY.Variable_26 = Variable_23 * BufferConstant.Variable_27;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
			// X
			BufferX.Variable_24 = Context.GetLocalX();
			
			// Y
			v_flt Variable_25; // Y output 0
			Variable_25 = Context.GetLocalY();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_30, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.576700, 0.658489);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_23; // 2D Perlin Noise Fractal output 0
			Variable_23 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_24, Variable_25, BufferConstant.Variable_33, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.594076, 0.593483);
			
			// 2D IQ Noise
			v_flt Variable_16; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_16 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_14, Variable_15, BufferConstant.Variable_35, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -0.648057, 0.611352);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.300998, 1.397865);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.723871, 1.259353);
			
			// +
			BufferXY.Variable_19 = Variable_2 + BufferConstant.Variable_32;
			
			// *
			BufferXY.Variable_17 = Variable_16 * BufferConstant.Variable_37;
			
			// *
			BufferXY.Variable_26 = Variable_23 * BufferConstant.Variable_27;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_7; // Z output 0
			Variable_7 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_3; // Z output 0
			Variable_3 = Context.GetLocalZ();
			
			// /
			v_flt Variable_4; // / output 0
			Variable_4 = Variable_3 / BufferConstant.Variable_28;
			
			// +
			v_flt Variable_22; // + output 0
			Variable_22 = Variable_4 + BufferXY.Variable_26;
			
			// Clamp
			v_flt Variable_5; // Clamp output 0
			Variable_5 = FVoxelNodeFunctions::Clamp(Variable_22, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_12; // * output 0
			Variable_12 = Variable_5 * BufferConstant.Variable_29;
			
			// +
			v_flt Variable_6; // + output 0
			Variable_6 = BufferXY.Variable_19 + Variable_12;
			
			// *
			v_flt Variable_10; // * output 0
			Variable_10 = Variable_6 * BufferConstant.Variable_21;
			
			// Clamp
			v_flt Variable_11; // Clamp output 0
			Variable_11 = FVoxelNodeFunctions::Clamp(Variable_10, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_9; // * output 0
			Variable_9 = Variable_11 * BufferConstant.Variable_28;
			
			// Lerp
			v_flt Variable_18; // Lerp output 0
			Variable_18 = FVoxelNodeFunctions::Lerp(v_flt(0.0f), BufferXY.Variable_17, Variable_11);
			
			// +
			v_flt Variable_13; // + output 0
			Variable_13 = Variable_9 + Variable_18;
			
			// -
			v_flt Variable_8; // - output 0
			Variable_8 = Variable_7 - Variable_13;
			
			Outputs.Value = Variable_8;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Z
			v_flt Variable_7; // Z output 0
			Variable_7 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// X
			v_flt Variable_14; // X output 0
			Variable_14 = Context.GetLocalX();
			
			// X
			v_flt Variable_24; // X output 0
			Variable_24 = Context.GetLocalX();
			
			// Y
			v_flt Variable_25; // Y output 0
			Variable_25 = Context.GetLocalY();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			v_flt Variable_3; // Z output 0
			Variable_3 = Context.GetLocalZ();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(Variable_0, Variable_1, BufferConstant.Variable_30, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.576700, 0.658489);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_23; // 2D Perlin Noise Fractal output 0
			Variable_23 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(Variable_24, Variable_25, BufferConstant.Variable_33, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.594076, 0.593483);
			
			// 2D IQ Noise
			v_flt Variable_16; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_16 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(Variable_14, Variable_15, BufferConstant.Variable_35, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -0.648057, 0.611352);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.300998, 1.397865);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.723871, 1.259353);
			
			// /
			v_flt Variable_4; // / output 0
			Variable_4 = Variable_3 / BufferConstant.Variable_28;
			
			// +
			v_flt Variable_19; // + output 0
			Variable_19 = Variable_2 + BufferConstant.Variable_32;
			
			// *
			v_flt Variable_17; // * output 0
			Variable_17 = Variable_16 * BufferConstant.Variable_37;
			
			// *
			v_flt Variable_26; // * output 0
			Variable_26 = Variable_23 * BufferConstant.Variable_27;
			
			// +
			v_flt Variable_22; // + output 0
			Variable_22 = Variable_4 + Variable_26;
			
			// Clamp
			v_flt Variable_5; // Clamp output 0
			Variable_5 = FVoxelNodeFunctions::Clamp(Variable_22, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_12; // * output 0
			Variable_12 = Variable_5 * BufferConstant.Variable_29;
			
			// +
			v_flt Variable_6; // + output 0
			Variable_6 = Variable_19 + Variable_12;
			
			// *
			v_flt Variable_10; // * output 0
			Variable_10 = Variable_6 * BufferConstant.Variable_21;
			
			// Clamp
			v_flt Variable_11; // Clamp output 0
			Variable_11 = FVoxelNodeFunctions::Clamp(Variable_10, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_9; // * output 0
			Variable_9 = Variable_11 * BufferConstant.Variable_28;
			
			// Lerp
			v_flt Variable_18; // Lerp output 0
			Variable_18 = FVoxelNodeFunctions::Lerp(v_flt(0.0f), Variable_17, Variable_11);
			
			// +
			v_flt Variable_13; // + output 0
			Variable_13 = Variable_9 + Variable_18;
			
			// -
			v_flt Variable_8; // - output 0
			Variable_8 = Variable_7 - Variable_13;
			
			Outputs.Value = Variable_8;
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
			
			TVoxelRange<v_flt> Variable_17; // Cliffs Slope = 10.0 output 0
			TVoxelRange<v_flt> Variable_22; // Height = 50.0 output 0
			TVoxelRange<v_flt> Variable_23; // Overhangs = 0.2 output 0
			TVoxelRange<v_flt> Variable_13; // * output 0
			TVoxelRange<v_flt> Variable_20; // * output 0
			TVoxelRange<v_flt> Variable_15; // + output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
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
					
					// Init of 2D Perlin Noise Fractal
					_2D_Perlin_Noise_Fractal_2_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Perlin_Noise_Fractal_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalOctavesAndGain(3, 0.5);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalLacunarity(2.0);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[0] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[1] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[2] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[3] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[4] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[5] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[6] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[7] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[8] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[9] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[10] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[11] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[12] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[13] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[14] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[15] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[16] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[17] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[18] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[19] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[20] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[21] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[22] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[23] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[24] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[25] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[26] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[27] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[28] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[29] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[30] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[31] = 3;
					
					// Init of 2D IQ Noise
					_2D_IQ_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_IQ_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.5);
					_2D_IQ_Noise_1_Noise.SetFractalLacunarity(2.0);
					_2D_IQ_Noise_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_IQ_Noise_1_Noise.SetMatrixFromRotation_2D(40.0);
					_2D_IQ_Noise_1_LODToOctaves[0] = 15;
					_2D_IQ_Noise_1_LODToOctaves[1] = 15;
					_2D_IQ_Noise_1_LODToOctaves[2] = 15;
					_2D_IQ_Noise_1_LODToOctaves[3] = 15;
					_2D_IQ_Noise_1_LODToOctaves[4] = 15;
					_2D_IQ_Noise_1_LODToOctaves[5] = 15;
					_2D_IQ_Noise_1_LODToOctaves[6] = 15;
					_2D_IQ_Noise_1_LODToOctaves[7] = 15;
					_2D_IQ_Noise_1_LODToOctaves[8] = 15;
					_2D_IQ_Noise_1_LODToOctaves[9] = 15;
					_2D_IQ_Noise_1_LODToOctaves[10] = 15;
					_2D_IQ_Noise_1_LODToOctaves[11] = 15;
					_2D_IQ_Noise_1_LODToOctaves[12] = 15;
					_2D_IQ_Noise_1_LODToOctaves[13] = 15;
					_2D_IQ_Noise_1_LODToOctaves[14] = 15;
					_2D_IQ_Noise_1_LODToOctaves[15] = 15;
					_2D_IQ_Noise_1_LODToOctaves[16] = 15;
					_2D_IQ_Noise_1_LODToOctaves[17] = 15;
					_2D_IQ_Noise_1_LODToOctaves[18] = 15;
					_2D_IQ_Noise_1_LODToOctaves[19] = 15;
					_2D_IQ_Noise_1_LODToOctaves[20] = 15;
					_2D_IQ_Noise_1_LODToOctaves[21] = 15;
					_2D_IQ_Noise_1_LODToOctaves[22] = 15;
					_2D_IQ_Noise_1_LODToOctaves[23] = 15;
					_2D_IQ_Noise_1_LODToOctaves[24] = 15;
					_2D_IQ_Noise_1_LODToOctaves[25] = 15;
					_2D_IQ_Noise_1_LODToOctaves[26] = 15;
					_2D_IQ_Noise_1_LODToOctaves[27] = 15;
					_2D_IQ_Noise_1_LODToOctaves[28] = 15;
					_2D_IQ_Noise_1_LODToOctaves[29] = 15;
					_2D_IQ_Noise_1_LODToOctaves[30] = 15;
					_2D_IQ_Noise_1_LODToOctaves[31] = 15;
					
					// Init of 2D Perlin Noise Fractal
					_2D_Perlin_Noise_Fractal_3_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Perlin_Noise_Fractal_3_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Perlin_Noise_Fractal_3_Noise.SetFractalOctavesAndGain(3, 0.5);
					_2D_Perlin_Noise_Fractal_3_Noise.SetFractalLacunarity(2.0);
					_2D_Perlin_Noise_Fractal_3_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[0] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[1] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[2] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[3] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[4] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[5] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[6] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[7] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[8] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[9] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[10] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[11] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[12] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[13] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[14] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[15] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[16] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[17] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[18] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[19] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[20] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[21] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[22] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[23] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[24] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[25] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[26] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[27] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[28] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[29] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[30] = 3;
					_2D_Perlin_Noise_Fractal_3_LODToOctaves[31] = 3;
					
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
				// 2D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_19; // 2D Perlin Noise Fractal output 0
				Variable_19 = { -0.594076f, 0.593483f };
				
				// 2D IQ Noise
				TVoxelRange<v_flt> Variable_12; // 2D IQ Noise output 0
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
				Variable_12 = { -0.648057f, 0.611352f };
				_2D_IQ_Noise_1_Temp_1 = { -1.300998f, 1.397865f };
				_2D_IQ_Noise_1_Temp_2 = { -1.723871f, 1.259353f };
				
				// Top Noise Scale = 25.0
				TVoxelRange<v_flt> Variable_25; // Top Noise Scale = 25.0 output 0
				Variable_25 = Params.Top_Noise_Scale;
				
				// Base Shape Offset = 0.0
				TVoxelRange<v_flt> Variable_16; // Base Shape Offset = 0.0 output 0
				Variable_16 = Params.Base_Shape_Offset;
				
				// Cliffs Slope = 10.0
				BufferConstant.Variable_17 = Params.Cliffs_Slope;
				
				// 2D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_0; // 2D Perlin Noise Fractal output 0
				Variable_0 = { -0.576700f, 0.658489f };
				
				// Height = 50.0
				BufferConstant.Variable_22 = Params.Height;
				
				// Sides Noise Amplitude = 0.2
				TVoxelRange<v_flt> Variable_21; // Sides Noise Amplitude = 0.2 output 0
				Variable_21 = Params.Sides_Noise_Amplitude;
				
				// Overhangs = 0.2
				BufferConstant.Variable_23 = Params.Overhangs;
				
				// *
				BufferConstant.Variable_13 = Variable_12 * Variable_25;
				
				// *
				BufferConstant.Variable_20 = Variable_19 * Variable_21;
				
				// /
				TVoxelRange<v_flt> Variable_24; // / output 0
				Variable_24 = Variable_16 / TVoxelRange<v_flt>(10.0f);
				
				// +
				BufferConstant.Variable_15 = Variable_0 + Variable_24;
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_2_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_2_LODToOctaves;
		FVoxelFastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_3_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_3_LODToOctaves;
		
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
			// Z
			TVoxelRange<v_flt> Variable_5; // Z output 0
			Variable_5 = Context.GetLocalZ();
			
			// Z
			TVoxelRange<v_flt> Variable_1; // Z output 0
			Variable_1 = Context.GetLocalZ();
			
			// /
			TVoxelRange<v_flt> Variable_2; // / output 0
			Variable_2 = Variable_1 / BufferConstant.Variable_22;
			
			// +
			TVoxelRange<v_flt> Variable_18; // + output 0
			Variable_18 = Variable_2 + BufferConstant.Variable_20;
			
			// Clamp
			TVoxelRange<v_flt> Variable_3; // Clamp output 0
			Variable_3 = FVoxelNodeFunctions::Clamp(Variable_18, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// *
			TVoxelRange<v_flt> Variable_10; // * output 0
			Variable_10 = Variable_3 * BufferConstant.Variable_23;
			
			// +
			TVoxelRange<v_flt> Variable_4; // + output 0
			Variable_4 = BufferConstant.Variable_15 + Variable_10;
			
			// *
			TVoxelRange<v_flt> Variable_8; // * output 0
			Variable_8 = Variable_4 * BufferConstant.Variable_17;
			
			// Clamp
			TVoxelRange<v_flt> Variable_9; // Clamp output 0
			Variable_9 = FVoxelNodeFunctions::Clamp(Variable_8, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// *
			TVoxelRange<v_flt> Variable_7; // * output 0
			Variable_7 = Variable_9 * BufferConstant.Variable_22;
			
			// Lerp
			TVoxelRange<v_flt> Variable_14; // Lerp output 0
			Variable_14 = FVoxelNodeFunctions::Lerp(TVoxelRange<v_flt>(0.0f), BufferConstant.Variable_13, Variable_9);
			
			// +
			TVoxelRange<v_flt> Variable_11; // + output 0
			Variable_11 = Variable_7 + Variable_14;
			
			// -
			TVoxelRange<v_flt> Variable_6; // - output 0
			Variable_6 = Variable_5 - Variable_11;
			
			Outputs.Value = Variable_6;
		}
		
	};
	
	FVoxelExample_CliffsInstance(UVoxelExample_Cliffs& Object)
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
			Object.Cliffs_Slope,
			Object.Height,
			Object.Overhangs,
			Object.Base_Shape_Frequency,
			Object.Base_Shape_Offset,
			Object.Base_Shape_Seed,
			Object.Sides_Noise_Seed,
			Object.Top_Noise_Seed,
			Object.Sides_Noise_Amplitude,
			Object.Sides_Noise_Frequency,
			Object.Top_Noise_Frequency,
			Object.Top_Noise_Scale
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
inline v_flt FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_CliffsInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_CliffsInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_CliffsInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_CliffsInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_CliffsInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Cliffs::UVoxelExample_Cliffs()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_Cliffs::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_CliffsInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_Cliffs. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_Cliffs. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Cliffs. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Cliffs. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
