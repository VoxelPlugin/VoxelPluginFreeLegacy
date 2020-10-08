// Copyright 2020 Phyronnaz

#include "VoxelExample_Cave.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_CaveInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_CaveInstance, UVoxelExample_Cave>
{
public:
	struct FParams
	{
		const float Bottom_Noise_Frequency;
		const float Bottom_Noise_Scale;
		const int32 Bottom_Noise_Seed;
		const int32 Global_Height_Seed;
		const int32 Top_Noise_Seed;
		const float Top_Noise_Frequency;
		const float Top_Noise_Scale;
		const float Bottom_Top_Merge_Smoothness;
		const float Global_Height_Merge_Smoothness;
		const float Global_Height_Noise_Frequency;
		const float Global_Height_Noise_Scale;
		const float Global_Height_Offset;
		const float Cave_Height;
		const float Cave_Radius;
		const float Cave_Walls_Smoothness;
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
			
			v_flt Variable_31; // Global Height Noise Frequency = 0.005 output 0
			v_flt Variable_24; // Bottom Top Merge Smoothness = 25.0 output 0
			v_flt Variable_25; // Cave Walls Smoothness = 100.0 output 0
			v_flt Variable_26; // Global Height Merge Smoothness = 15.0 output 0
			v_flt Variable_27; // Top Noise Frequency = 0.005 output 0
			v_flt Variable_28; // Bottom Noise Frequency = 0.008 output 0
			v_flt Variable_15; // Cave Radius = 400.0 output 0
			v_flt Variable_33; // Top Noise Scale = 150.0 output 0
			v_flt Variable_34; // Bottom Noise Scale = 150.0 output 0
			v_flt Variable_38; // Global Height Noise Scale = 200.0 output 0
			v_flt Variable_39; // Global Height Offset = 150.0 output 0
			v_flt Variable_37; // / output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_3; // X output 0
			v_flt Variable_19; // X output 0
			v_flt Variable_0; // X output 0
			v_flt Variable_12; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_6; // * output 0
			v_flt Variable_7; // * output 0
			v_flt Variable_14; // - output 0
			v_flt Variable_22; // + output 0
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
					
					// Init of Top Noise Seed = 3022
					FVoxelGraphSeed Variable_29; // Top Noise Seed = 3022 output 0
					Variable_29 = Params.Top_Noise_Seed;
					
					// Init of Bottom Noise Seed = 3024
					FVoxelGraphSeed Variable_30; // Bottom Noise Seed = 3024 output 0
					Variable_30 = Params.Bottom_Noise_Seed;
					
					// Init of Global Height Seed = 1447
					FVoxelGraphSeed Variable_32; // Global Height Seed = 1447 output 0
					Variable_32 = Params.Global_Height_Seed;
					
					
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
				// Global Height Noise Frequency = 0.005
				BufferConstant.Variable_31 = Params.Global_Height_Noise_Frequency;
				
				// Bottom Top Merge Smoothness = 25.0
				BufferConstant.Variable_24 = Params.Bottom_Top_Merge_Smoothness;
				
				// Cave Walls Smoothness = 100.0
				BufferConstant.Variable_25 = Params.Cave_Walls_Smoothness;
				
				// Global Height Merge Smoothness = 15.0
				BufferConstant.Variable_26 = Params.Global_Height_Merge_Smoothness;
				
				// Top Noise Frequency = 0.005
				BufferConstant.Variable_27 = Params.Top_Noise_Frequency;
				
				// Bottom Noise Frequency = 0.008
				BufferConstant.Variable_28 = Params.Bottom_Noise_Frequency;
				
				// Cave Radius = 400.0
				BufferConstant.Variable_15 = Params.Cave_Radius;
				
				// Top Noise Scale = 150.0
				BufferConstant.Variable_33 = Params.Top_Noise_Scale;
				
				// Bottom Noise Scale = 150.0
				BufferConstant.Variable_34 = Params.Bottom_Noise_Scale;
				
				// Cave Height = 100.0
				v_flt Variable_36; // Cave Height = 100.0 output 0
				Variable_36 = Params.Cave_Height;
				
				// Global Height Noise Scale = 200.0
				BufferConstant.Variable_38 = Params.Global_Height_Noise_Scale;
				
				// Global Height Offset = 150.0
				BufferConstant.Variable_39 = Params.Global_Height_Offset;
				
				// /
				BufferConstant.Variable_37 = Variable_36 / v_flt(2.0f);
				
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
		
		FVoxelFastNoise _2D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_0_LODToOctaves;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_0_LODToOctaves;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Bottom Noise Seed = 3024
			FVoxelGraphSeed Variable_30; // Bottom Noise Seed = 3024 output 0
			Variable_30 = Params.Bottom_Noise_Seed;
			
			// Init of Global Height Seed = 1447
			FVoxelGraphSeed Variable_32; // Global Height Seed = 1447 output 0
			Variable_32 = Params.Global_Height_Seed;
			
			// Init of Top Noise Seed = 3022
			FVoxelGraphSeed Variable_29; // Top Noise Seed = 3022 output 0
			Variable_29 = Params.Top_Noise_Seed;
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_32);
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
			
			// Init of 2D Perlin Noise Fractal
			_2D_Perlin_Noise_Fractal_0_Noise.SetSeed(Variable_30);
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
			_2D_Perlin_Noise_Fractal_1_Noise.SetSeed(Variable_29);
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
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_3 = Context.GetLocalX();
			
			// X
			BufferX.Variable_19 = Context.GetLocalX();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_12 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Y
			v_flt Variable_13; // Y output 0
			Variable_13 = Context.GetLocalY();
			
			// Y
			v_flt Variable_4; // Y output 0
			Variable_4 = Context.GetLocalY();
			
			// Y
			v_flt Variable_20; // Y output 0
			Variable_20 = Context.GetLocalY();
			
			// 2D IQ Noise
			v_flt Variable_23; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.779186, 0.705623);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.482251, 1.789484);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.568460, 1.481016);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_5 = FMath::Clamp<v_flt>(Variable_5, -0.643471, 0.527891);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.643471, 0.527891);
			
			// Vector Length.Vector Length
			v_flt Variable_40; // Vector Length.Vector Length output 0
			Variable_40 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_12, Variable_13, v_flt(0.0f));
			
			// *
			v_flt Variable_21; // * output 0
			Variable_21 = Variable_23 * BufferConstant.Variable_38;
			
			// *
			BufferXY.Variable_6 = Variable_2 * BufferConstant.Variable_33;
			
			// *
			BufferXY.Variable_7 = Variable_5 * BufferConstant.Variable_34;
			
			// -
			BufferXY.Variable_14 = BufferConstant.Variable_15 - Variable_40;
			
			// +
			BufferXY.Variable_22 = Variable_21 + BufferConstant.Variable_39;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_3 = Context.GetLocalX();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Y
			v_flt Variable_13; // Y output 0
			Variable_13 = Context.GetLocalY();
			
			// Y
			v_flt Variable_4; // Y output 0
			Variable_4 = Context.GetLocalY();
			
			// Y
			v_flt Variable_20; // Y output 0
			Variable_20 = Context.GetLocalY();
			
			// X
			BufferX.Variable_19 = Context.GetLocalX();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_12 = Context.GetLocalX();
			
			// 2D IQ Noise
			v_flt Variable_23; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.779186, 0.705623);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.482251, 1.789484);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.568460, 1.481016);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_5 = FMath::Clamp<v_flt>(Variable_5, -0.643471, 0.527891);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.643471, 0.527891);
			
			// Vector Length.Vector Length
			v_flt Variable_40; // Vector Length.Vector Length output 0
			Variable_40 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_12, Variable_13, v_flt(0.0f));
			
			// *
			v_flt Variable_21; // * output 0
			Variable_21 = Variable_23 * BufferConstant.Variable_38;
			
			// *
			BufferXY.Variable_6 = Variable_2 * BufferConstant.Variable_33;
			
			// *
			BufferXY.Variable_7 = Variable_5 * BufferConstant.Variable_34;
			
			// -
			BufferXY.Variable_14 = BufferConstant.Variable_15 - Variable_40;
			
			// +
			BufferXY.Variable_22 = Variable_21 + BufferConstant.Variable_39;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_17; // Z output 0
			Variable_17 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// -
			v_flt Variable_10; // - output 0
			Variable_10 = BufferXY.Variable_6 - Variable_11;
			
			// -
			v_flt Variable_9; // - output 0
			Variable_9 = Variable_8 - BufferXY.Variable_7;
			
			// -
			v_flt Variable_18; // - output 0
			Variable_18 = Variable_17 - BufferXY.Variable_22;
			
			// Smooth Union.-
			v_flt Variable_58; // Smooth Union.- output 0
			Variable_58 = Variable_10 - Variable_9;
			
			// Smooth Union./
			v_flt Variable_59; // Smooth Union./ output 0
			Variable_59 = Variable_58 / BufferConstant.Variable_24;
			
			// Smooth Union.*
			v_flt Variable_60; // Smooth Union.* output 0
			Variable_60 = Variable_59 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_61; // Smooth Union.+ output 0
			Variable_61 = Variable_60 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_62; // Smooth Union.Clamp output 0
			Variable_62 = FVoxelNodeFunctions::Clamp(Variable_61, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.Lerp
			v_flt Variable_63; // Smooth Union.Lerp output 0
			Variable_63 = FVoxelNodeFunctions::Lerp(Variable_10, Variable_9, Variable_62);
			
			// Smooth Union.1 - X
			v_flt Variable_66; // Smooth Union.1 - X output 0
			Variable_66 = 1 - Variable_62;
			
			// Smooth Union.*
			v_flt Variable_65; // Smooth Union.* output 0
			Variable_65 = BufferConstant.Variable_24 * Variable_62 * Variable_66;
			
			// Smooth Union.-
			v_flt Variable_64; // Smooth Union.- output 0
			Variable_64 = Variable_63 - Variable_65;
			
			// +
			v_flt Variable_35; // + output 0
			Variable_35 = Variable_64 + BufferConstant.Variable_37;
			
			// Smooth Union.-
			v_flt Variable_41; // Smooth Union.- output 0
			Variable_41 = BufferXY.Variable_14 - Variable_35;
			
			// Smooth Union./
			v_flt Variable_42; // Smooth Union./ output 0
			Variable_42 = Variable_41 / BufferConstant.Variable_25;
			
			// Smooth Union.*
			v_flt Variable_43; // Smooth Union.* output 0
			Variable_43 = Variable_42 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_44; // Smooth Union.+ output 0
			Variable_44 = Variable_43 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_45; // Smooth Union.Clamp output 0
			Variable_45 = FVoxelNodeFunctions::Clamp(Variable_44, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.1 - X
			v_flt Variable_49; // Smooth Union.1 - X output 0
			Variable_49 = 1 - Variable_45;
			
			// Smooth Union.Lerp
			v_flt Variable_46; // Smooth Union.Lerp output 0
			Variable_46 = FVoxelNodeFunctions::Lerp(BufferXY.Variable_14, Variable_35, Variable_45);
			
			// Smooth Union.*
			v_flt Variable_48; // Smooth Union.* output 0
			Variable_48 = BufferConstant.Variable_25 * Variable_45 * Variable_49;
			
			// Smooth Union.-
			v_flt Variable_47; // Smooth Union.- output 0
			Variable_47 = Variable_46 - Variable_48;
			
			// Smooth Intersection.-
			v_flt Variable_57; // Smooth Intersection.- output 0
			Variable_57 = Variable_18 - Variable_47;
			
			// Smooth Intersection./
			v_flt Variable_50; // Smooth Intersection./ output 0
			Variable_50 = Variable_57 / BufferConstant.Variable_26;
			
			// Smooth Intersection.*
			v_flt Variable_51; // Smooth Intersection.* output 0
			Variable_51 = Variable_50 * v_flt(0.5f);
			
			// Smooth Intersection.-
			v_flt Variable_16; // Smooth Intersection.- output 0
			Variable_16 = v_flt(0.5f) - Variable_51;
			
			// Smooth Intersection.Clamp
			v_flt Variable_52; // Smooth Intersection.Clamp output 0
			Variable_52 = FVoxelNodeFunctions::Clamp(Variable_16, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Intersection.1 - X
			v_flt Variable_55; // Smooth Intersection.1 - X output 0
			Variable_55 = 1 - Variable_52;
			
			// Smooth Intersection.Lerp
			v_flt Variable_53; // Smooth Intersection.Lerp output 0
			Variable_53 = FVoxelNodeFunctions::Lerp(Variable_18, Variable_47, Variable_52);
			
			// Smooth Intersection.*
			v_flt Variable_54; // Smooth Intersection.* output 0
			Variable_54 = BufferConstant.Variable_26 * Variable_52 * Variable_55;
			
			// Smooth Intersection.+
			v_flt Variable_56; // Smooth Intersection.+ output 0
			Variable_56 = Variable_53 + Variable_54;
			
			Outputs.Value = Variable_56;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// X
			v_flt Variable_3; // X output 0
			Variable_3 = Context.GetLocalX();
			
			// Z
			v_flt Variable_17; // Z output 0
			Variable_17 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Y
			v_flt Variable_13; // Y output 0
			Variable_13 = Context.GetLocalY();
			
			// Y
			v_flt Variable_4; // Y output 0
			Variable_4 = Context.GetLocalY();
			
			// Y
			v_flt Variable_20; // Y output 0
			Variable_20 = Context.GetLocalY();
			
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// X
			v_flt Variable_19; // X output 0
			Variable_19 = Context.GetLocalX();
			
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// X
			v_flt Variable_12; // X output 0
			Variable_12 = Context.GetLocalX();
			
			// 2D IQ Noise
			v_flt Variable_23; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_23 = FMath::Clamp<v_flt>(Variable_23, -0.779186, 0.705623);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.482251, 1.789484);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.568460, 1.481016);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_5 = FMath::Clamp<v_flt>(Variable_5, -0.643471, 0.527891);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, -0.643471, 0.527891);
			
			// Vector Length.Vector Length
			v_flt Variable_40; // Vector Length.Vector Length output 0
			Variable_40 = FVoxelNodeFunctions::VectorLength(Variable_12, Variable_13, v_flt(0.0f));
			
			// *
			v_flt Variable_21; // * output 0
			Variable_21 = Variable_23 * BufferConstant.Variable_38;
			
			// *
			v_flt Variable_6; // * output 0
			Variable_6 = Variable_2 * BufferConstant.Variable_33;
			
			// *
			v_flt Variable_7; // * output 0
			Variable_7 = Variable_5 * BufferConstant.Variable_34;
			
			// -
			v_flt Variable_14; // - output 0
			Variable_14 = BufferConstant.Variable_15 - Variable_40;
			
			// +
			v_flt Variable_22; // + output 0
			Variable_22 = Variable_21 + BufferConstant.Variable_39;
			
			// -
			v_flt Variable_10; // - output 0
			Variable_10 = Variable_6 - Variable_11;
			
			// -
			v_flt Variable_9; // - output 0
			Variable_9 = Variable_8 - Variable_7;
			
			// -
			v_flt Variable_18; // - output 0
			Variable_18 = Variable_17 - Variable_22;
			
			// Smooth Union.-
			v_flt Variable_58; // Smooth Union.- output 0
			Variable_58 = Variable_10 - Variable_9;
			
			// Smooth Union./
			v_flt Variable_59; // Smooth Union./ output 0
			Variable_59 = Variable_58 / BufferConstant.Variable_24;
			
			// Smooth Union.*
			v_flt Variable_60; // Smooth Union.* output 0
			Variable_60 = Variable_59 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_61; // Smooth Union.+ output 0
			Variable_61 = Variable_60 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_62; // Smooth Union.Clamp output 0
			Variable_62 = FVoxelNodeFunctions::Clamp(Variable_61, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.Lerp
			v_flt Variable_63; // Smooth Union.Lerp output 0
			Variable_63 = FVoxelNodeFunctions::Lerp(Variable_10, Variable_9, Variable_62);
			
			// Smooth Union.1 - X
			v_flt Variable_66; // Smooth Union.1 - X output 0
			Variable_66 = 1 - Variable_62;
			
			// Smooth Union.*
			v_flt Variable_65; // Smooth Union.* output 0
			Variable_65 = BufferConstant.Variable_24 * Variable_62 * Variable_66;
			
			// Smooth Union.-
			v_flt Variable_64; // Smooth Union.- output 0
			Variable_64 = Variable_63 - Variable_65;
			
			// +
			v_flt Variable_35; // + output 0
			Variable_35 = Variable_64 + BufferConstant.Variable_37;
			
			// Smooth Union.-
			v_flt Variable_41; // Smooth Union.- output 0
			Variable_41 = Variable_14 - Variable_35;
			
			// Smooth Union./
			v_flt Variable_42; // Smooth Union./ output 0
			Variable_42 = Variable_41 / BufferConstant.Variable_25;
			
			// Smooth Union.*
			v_flt Variable_43; // Smooth Union.* output 0
			Variable_43 = Variable_42 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_44; // Smooth Union.+ output 0
			Variable_44 = Variable_43 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_45; // Smooth Union.Clamp output 0
			Variable_45 = FVoxelNodeFunctions::Clamp(Variable_44, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.1 - X
			v_flt Variable_49; // Smooth Union.1 - X output 0
			Variable_49 = 1 - Variable_45;
			
			// Smooth Union.Lerp
			v_flt Variable_46; // Smooth Union.Lerp output 0
			Variable_46 = FVoxelNodeFunctions::Lerp(Variable_14, Variable_35, Variable_45);
			
			// Smooth Union.*
			v_flt Variable_48; // Smooth Union.* output 0
			Variable_48 = BufferConstant.Variable_25 * Variable_45 * Variable_49;
			
			// Smooth Union.-
			v_flt Variable_47; // Smooth Union.- output 0
			Variable_47 = Variable_46 - Variable_48;
			
			// Smooth Intersection.-
			v_flt Variable_57; // Smooth Intersection.- output 0
			Variable_57 = Variable_18 - Variable_47;
			
			// Smooth Intersection./
			v_flt Variable_50; // Smooth Intersection./ output 0
			Variable_50 = Variable_57 / BufferConstant.Variable_26;
			
			// Smooth Intersection.*
			v_flt Variable_51; // Smooth Intersection.* output 0
			Variable_51 = Variable_50 * v_flt(0.5f);
			
			// Smooth Intersection.-
			v_flt Variable_16; // Smooth Intersection.- output 0
			Variable_16 = v_flt(0.5f) - Variable_51;
			
			// Smooth Intersection.Clamp
			v_flt Variable_52; // Smooth Intersection.Clamp output 0
			Variable_52 = FVoxelNodeFunctions::Clamp(Variable_16, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Intersection.1 - X
			v_flt Variable_55; // Smooth Intersection.1 - X output 0
			Variable_55 = 1 - Variable_52;
			
			// Smooth Intersection.Lerp
			v_flt Variable_53; // Smooth Intersection.Lerp output 0
			Variable_53 = FVoxelNodeFunctions::Lerp(Variable_18, Variable_47, Variable_52);
			
			// Smooth Intersection.*
			v_flt Variable_54; // Smooth Intersection.* output 0
			Variable_54 = BufferConstant.Variable_26 * Variable_52 * Variable_55;
			
			// Smooth Intersection.+
			v_flt Variable_56; // Smooth Intersection.+ output 0
			Variable_56 = Variable_53 + Variable_54;
			
			Outputs.Value = Variable_56;
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
			
			TVoxelRange<v_flt> Variable_18; // Bottom Top Merge Smoothness = 25.0 output 0
			TVoxelRange<v_flt> Variable_19; // Cave Walls Smoothness = 100.0 output 0
			TVoxelRange<v_flt> Variable_20; // Global Height Merge Smoothness = 15.0 output 0
			TVoxelRange<v_flt> Variable_11; // Cave Radius = 400.0 output 0
			TVoxelRange<v_flt> Variable_25; // / output 0
			TVoxelRange<v_flt> Variable_2; // * output 0
			TVoxelRange<v_flt> Variable_3; // * output 0
			TVoxelRange<v_flt> Variable_16; // + output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_8; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_10; // - output 0
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
					_2D_Perlin_Noise_Fractal_2_Noise.SetSeed(FVoxelGraphSeed(1338));
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
					
					// Init of 2D IQ Noise
					_2D_IQ_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1339));
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
				// Bottom Top Merge Smoothness = 25.0
				BufferConstant.Variable_18 = Params.Bottom_Top_Merge_Smoothness;
				
				// 2D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_1; // 2D Perlin Noise Fractal output 0
				Variable_1 = { -0.643471f, 0.527891f };
				
				// Cave Walls Smoothness = 100.0
				BufferConstant.Variable_19 = Params.Cave_Walls_Smoothness;
				
				// Global Height Merge Smoothness = 15.0
				BufferConstant.Variable_20 = Params.Global_Height_Merge_Smoothness;
				
				// Cave Radius = 400.0
				BufferConstant.Variable_11 = Params.Cave_Radius;
				
				// Top Noise Scale = 150.0
				TVoxelRange<v_flt> Variable_21; // Top Noise Scale = 150.0 output 0
				Variable_21 = Params.Top_Noise_Scale;
				
				// Cave Height = 100.0
				TVoxelRange<v_flt> Variable_24; // Cave Height = 100.0 output 0
				Variable_24 = Params.Cave_Height;
				
				// 2D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_0; // 2D Perlin Noise Fractal output 0
				Variable_0 = { -0.643471f, 0.527891f };
				
				// Global Height Noise Scale = 200.0
				TVoxelRange<v_flt> Variable_26; // Global Height Noise Scale = 200.0 output 0
				Variable_26 = Params.Global_Height_Noise_Scale;
				
				// 2D IQ Noise
				TVoxelRange<v_flt> Variable_17; // 2D IQ Noise output 0
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
				Variable_17 = { -0.779186f, 0.705623f };
				_2D_IQ_Noise_1_Temp_1 = { -1.482251f, 1.789484f };
				_2D_IQ_Noise_1_Temp_2 = { -1.568460f, 1.481016f };
				
				// Global Height Offset = 150.0
				TVoxelRange<v_flt> Variable_27; // Global Height Offset = 150.0 output 0
				Variable_27 = Params.Global_Height_Offset;
				
				// Bottom Noise Scale = 150.0
				TVoxelRange<v_flt> Variable_22; // Bottom Noise Scale = 150.0 output 0
				Variable_22 = Params.Bottom_Noise_Scale;
				
				// /
				BufferConstant.Variable_25 = Variable_24 / TVoxelRange<v_flt>(2.0f);
				
				// *
				BufferConstant.Variable_2 = Variable_0 * Variable_21;
				
				// *
				BufferConstant.Variable_3 = Variable_1 * Variable_22;
				
				// *
				TVoxelRange<v_flt> Variable_15; // * output 0
				Variable_15 = Variable_17 * Variable_26;
				
				// +
				BufferConstant.Variable_16 = Variable_15 + Variable_27;
				
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
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_3_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_3_LODToOctaves;
		FVoxelFastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		
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
			TVoxelRange<v_flt> Variable_4; // Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_13; // Z output 0
			Variable_13 = Context.GetLocalZ();
			
			// Z
			TVoxelRange<v_flt> Variable_7; // Z output 0
			Variable_7 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// -
			TVoxelRange<v_flt> Variable_6; // - output 0
			Variable_6 = BufferConstant.Variable_2 - Variable_7;
			
			// Vector Length.Vector Length
			TVoxelRange<v_flt> Variable_28; // Vector Length.Vector Length output 0
			Variable_28 = FVoxelNodeFunctions::VectorLength(Variable_8, Variable_9, TVoxelRange<v_flt>(0.0f));
			
			// -
			TVoxelRange<v_flt> Variable_14; // - output 0
			Variable_14 = Variable_13 - BufferConstant.Variable_16;
			
			// -
			TVoxelRange<v_flt> Variable_5; // - output 0
			Variable_5 = Variable_4 - BufferConstant.Variable_3;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_46; // Smooth Union.- output 0
			Variable_46 = Variable_6 - Variable_5;
			
			// -
			TVoxelRange<v_flt> Variable_10; // - output 0
			Variable_10 = BufferConstant.Variable_11 - Variable_28;
			
			// Smooth Union./
			TVoxelRange<v_flt> Variable_47; // Smooth Union./ output 0
			Variable_47 = Variable_46 / BufferConstant.Variable_18;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_48; // Smooth Union.* output 0
			Variable_48 = Variable_47 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.+
			TVoxelRange<v_flt> Variable_49; // Smooth Union.+ output 0
			Variable_49 = Variable_48 + TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.Clamp
			TVoxelRange<v_flt> Variable_50; // Smooth Union.Clamp output 0
			Variable_50 = FVoxelNodeFunctions::Clamp(Variable_49, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Union.1 - X
			TVoxelRange<v_flt> Variable_54; // Smooth Union.1 - X output 0
			Variable_54 = 1 - Variable_50;
			
			// Smooth Union.Lerp
			TVoxelRange<v_flt> Variable_51; // Smooth Union.Lerp output 0
			Variable_51 = FVoxelNodeFunctions::Lerp(Variable_6, Variable_5, Variable_50);
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_53; // Smooth Union.* output 0
			Variable_53 = BufferConstant.Variable_18 * Variable_50 * Variable_54;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_52; // Smooth Union.- output 0
			Variable_52 = Variable_51 - Variable_53;
			
			// +
			TVoxelRange<v_flt> Variable_23; // + output 0
			Variable_23 = Variable_52 + BufferConstant.Variable_25;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_29; // Smooth Union.- output 0
			Variable_29 = Variable_10 - Variable_23;
			
			// Smooth Union./
			TVoxelRange<v_flt> Variable_30; // Smooth Union./ output 0
			Variable_30 = Variable_29 / BufferConstant.Variable_19;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_31; // Smooth Union.* output 0
			Variable_31 = Variable_30 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.+
			TVoxelRange<v_flt> Variable_32; // Smooth Union.+ output 0
			Variable_32 = Variable_31 + TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.Clamp
			TVoxelRange<v_flt> Variable_33; // Smooth Union.Clamp output 0
			Variable_33 = FVoxelNodeFunctions::Clamp(Variable_32, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Union.Lerp
			TVoxelRange<v_flt> Variable_34; // Smooth Union.Lerp output 0
			Variable_34 = FVoxelNodeFunctions::Lerp(Variable_10, Variable_23, Variable_33);
			
			// Smooth Union.1 - X
			TVoxelRange<v_flt> Variable_37; // Smooth Union.1 - X output 0
			Variable_37 = 1 - Variable_33;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_36; // Smooth Union.* output 0
			Variable_36 = BufferConstant.Variable_19 * Variable_33 * Variable_37;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_35; // Smooth Union.- output 0
			Variable_35 = Variable_34 - Variable_36;
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_45; // Smooth Intersection.- output 0
			Variable_45 = Variable_14 - Variable_35;
			
			// Smooth Intersection./
			TVoxelRange<v_flt> Variable_38; // Smooth Intersection./ output 0
			Variable_38 = Variable_45 / BufferConstant.Variable_20;
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_39; // Smooth Intersection.* output 0
			Variable_39 = Variable_38 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_12; // Smooth Intersection.- output 0
			Variable_12 = TVoxelRange<v_flt>(0.5f) - Variable_39;
			
			// Smooth Intersection.Clamp
			TVoxelRange<v_flt> Variable_40; // Smooth Intersection.Clamp output 0
			Variable_40 = FVoxelNodeFunctions::Clamp(Variable_12, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Intersection.Lerp
			TVoxelRange<v_flt> Variable_41; // Smooth Intersection.Lerp output 0
			Variable_41 = FVoxelNodeFunctions::Lerp(Variable_14, Variable_35, Variable_40);
			
			// Smooth Intersection.1 - X
			TVoxelRange<v_flt> Variable_43; // Smooth Intersection.1 - X output 0
			Variable_43 = 1 - Variable_40;
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_42; // Smooth Intersection.* output 0
			Variable_42 = BufferConstant.Variable_20 * Variable_40 * Variable_43;
			
			// Smooth Intersection.+
			TVoxelRange<v_flt> Variable_44; // Smooth Intersection.+ output 0
			Variable_44 = Variable_41 + Variable_42;
			
			Outputs.Value = Variable_44;
		}
		
	};
	
	FVoxelExample_CaveInstance(UVoxelExample_Cave& Object)
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
			Object.Bottom_Noise_Frequency,
			Object.Bottom_Noise_Scale,
			Object.Bottom_Noise_Seed,
			Object.Global_Height_Seed,
			Object.Top_Noise_Seed,
			Object.Top_Noise_Frequency,
			Object.Top_Noise_Scale,
			Object.Bottom_Top_Merge_Smoothness,
			Object.Global_Height_Merge_Smoothness,
			Object.Global_Height_Noise_Frequency,
			Object.Global_Height_Noise_Scale,
			Object.Global_Height_Offset,
			Object.Cave_Height,
			Object.Cave_Radius,
			Object.Cave_Walls_Smoothness
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
inline v_flt FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_CaveInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_CaveInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_CaveInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_CaveInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_CaveInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Cave::UVoxelExample_Cave()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_Cave::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_CaveInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_Cave. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_Cave. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Cave. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Cave. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
