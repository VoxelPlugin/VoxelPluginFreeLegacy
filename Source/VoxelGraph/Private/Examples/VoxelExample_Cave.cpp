// Copyright 2020 Phyronnaz

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnull-dereference"
#else
#pragma warning(push)
#pragma warning(disable : 4101 4701)
#endif

#include "VoxelExample_Cave.h"

using Seed = int32;

class FVoxelExample_CaveInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_CaveInstance, UVoxelExample_Cave>
{
public:
	class FLocalComputeStruct_LocalValue
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
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
		
		FLocalComputeStruct_LocalValue(const float& InBottom_Noise_Frequency, const float& InBottom_Noise_Scale, const float& InTop_Noise_Frequency, const float& InTop_Noise_Scale, const float& InBottom_Top_Merge_Smoothness, const float& InGlobal_Height_Offset, const float& InGlobal_Height_Merge_Smoothness, const float& InGlobal_Height_Noise_Frequency, const float& InGlobal_Height_Noise_Scale, const float& InCave_Height, const float& InCave_Walls_Smoothness, const float& InCave_Radius)
			: Bottom_Noise_Frequency(InBottom_Noise_Frequency)
			, Bottom_Noise_Scale(InBottom_Noise_Scale)
			, Top_Noise_Frequency(InTop_Noise_Frequency)
			, Top_Noise_Scale(InTop_Noise_Scale)
			, Bottom_Top_Merge_Smoothness(InBottom_Top_Merge_Smoothness)
			, Global_Height_Offset(InGlobal_Height_Offset)
			, Global_Height_Merge_Smoothness(InGlobal_Height_Merge_Smoothness)
			, Global_Height_Noise_Frequency(InGlobal_Height_Noise_Frequency)
			, Global_Height_Noise_Scale(InGlobal_Height_Noise_Scale)
			, Cave_Height(InCave_Height)
			, Cave_Walls_Smoothness(InCave_Walls_Smoothness)
			, Cave_Radius(InCave_Radius)
		{
		}
		
		void Init(const FVoxelWorldGeneratorInit& InitStruct)
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
					
					// Init of Top Noise Seed
					Seed Variable_29; // Top Noise Seed output 0
					{
						static FName StaticName = "Top Noise Seed";
						Variable_29 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 3022;
					}
					
					// Init of Bottom Noise Seed
					Seed Variable_30; // Bottom Noise Seed output 0
					{
						static FName StaticName = "Bottom Noise Seed";
						Variable_30 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 3024;
					}
					
					// Init of Global Height Seed
					Seed Variable_32; // Global Height Seed output 0
					{
						static FName StaticName = "Global Height Seed";
						Variable_32 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 1447;
					}
					
					
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
				BufferConstant.Variable_31 = Global_Height_Noise_Frequency;
				
				// Bottom Top Merge Smoothness = 25.0
				BufferConstant.Variable_24 = Bottom_Top_Merge_Smoothness;
				
				// Cave Walls Smoothness = 100.0
				BufferConstant.Variable_25 = Cave_Walls_Smoothness;
				
				// Global Height Merge Smoothness = 15.0
				BufferConstant.Variable_26 = Global_Height_Merge_Smoothness;
				
				// Top Noise Frequency = 0.005
				BufferConstant.Variable_27 = Top_Noise_Frequency;
				
				// Bottom Noise Frequency = 0.008
				BufferConstant.Variable_28 = Bottom_Noise_Frequency;
				
				// Cave Radius = 400.0
				BufferConstant.Variable_15 = Cave_Radius;
				
				// Top Noise Scale = 150.0
				BufferConstant.Variable_33 = Top_Noise_Scale;
				
				// Bottom Noise Scale = 150.0
				BufferConstant.Variable_34 = Bottom_Noise_Scale;
				
				// Cave Height = 100.0
				v_flt Variable_36; // Cave Height = 100.0 output 0
				Variable_36 = Cave_Height;
				
				// Global Height Noise Scale = 200.0
				BufferConstant.Variable_38 = Global_Height_Noise_Scale;
				
				// Global Height Offset = 150.0
				BufferConstant.Variable_39 = Global_Height_Offset;
				
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
		FastNoise _2D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_0_LODToOctaves;
		FastNoise _2D_Perlin_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_0_LODToOctaves;
		FastNoise _2D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_1_LODToOctaves;
		
		const float& Bottom_Noise_Frequency;
		const float& Bottom_Noise_Scale;
		const float& Top_Noise_Frequency;
		const float& Top_Noise_Scale;
		const float& Bottom_Top_Merge_Smoothness;
		const float& Global_Height_Offset;
		const float& Global_Height_Merge_Smoothness;
		const float& Global_Height_Noise_Frequency;
		const float& Global_Height_Noise_Scale;
		const float& Cave_Height;
		const float& Cave_Walls_Smoothness;
		const float& Cave_Radius;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of Bottom Noise Seed
			Seed Variable_30; // Bottom Noise Seed output 0
			{
				static FName StaticName = "Bottom Noise Seed";
				Variable_30 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 3024;
			}
			
			// Init of Global Height Seed
			Seed Variable_32; // Global Height Seed output 0
			{
				static FName StaticName = "Global Height Seed";
				Variable_32 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 1447;
			}
			
			// Init of Top Noise Seed
			Seed Variable_29; // Top Noise Seed output 0
			{
				static FName StaticName = "Top Noise Seed";
				Variable_29 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 3022;
			}
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_32);
			_2D_IQ_Noise_0_Noise.SetInterp(FastNoise::Quintic);
			_2D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_0_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_0_Noise.SetFractalType(FastNoise::FBM);
			_2D_IQ_Noise_0_Noise.SetMatrix(FMatrix2x2(FQuat2D(FMath::DegreesToRadians(40.000000))));
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
			_2D_Perlin_Noise_Fractal_0_Noise.SetInterp(FastNoise::Quintic);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_0_Noise.SetFractalType(FastNoise::FBM);
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
			_2D_Perlin_Noise_Fractal_1_Noise.SetInterp(FastNoise::Quintic);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_1_Noise.SetFractalType(FastNoise::FBM);
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
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(BufferX.Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
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
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(BufferX.Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(BufferX.Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
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
			Variable_23 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(Variable_19, Variable_20, BufferConstant.Variable_31, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(Variable_3, Variable_4, BufferConstant.Variable_28, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
			// 2D Perlin Noise Fractal
			v_flt Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D(Variable_0, Variable_1, BufferConstant.Variable_27, _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
			FVoxelMaterial Material;
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
		
		FLocalComputeStruct_LocalMaterial(const float& InBottom_Noise_Frequency, const float& InBottom_Noise_Scale, const float& InTop_Noise_Frequency, const float& InTop_Noise_Scale, const float& InBottom_Top_Merge_Smoothness, const float& InGlobal_Height_Offset, const float& InGlobal_Height_Merge_Smoothness, const float& InGlobal_Height_Noise_Frequency, const float& InGlobal_Height_Noise_Scale, const float& InCave_Height, const float& InCave_Walls_Smoothness, const float& InCave_Radius)
			: Bottom_Noise_Frequency(InBottom_Noise_Frequency)
			, Bottom_Noise_Scale(InBottom_Noise_Scale)
			, Top_Noise_Frequency(InTop_Noise_Frequency)
			, Top_Noise_Scale(InTop_Noise_Scale)
			, Bottom_Top_Merge_Smoothness(InBottom_Top_Merge_Smoothness)
			, Global_Height_Offset(InGlobal_Height_Offset)
			, Global_Height_Merge_Smoothness(InGlobal_Height_Merge_Smoothness)
			, Global_Height_Noise_Frequency(InGlobal_Height_Noise_Frequency)
			, Global_Height_Noise_Scale(InGlobal_Height_Noise_Scale)
			, Cave_Height(InCave_Height)
			, Cave_Walls_Smoothness(InCave_Walls_Smoothness)
			, Cave_Radius(InCave_Radius)
		{
		}
		
		void Init(const FVoxelWorldGeneratorInit& InitStruct)
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
		
		const float& Bottom_Noise_Frequency;
		const float& Bottom_Noise_Scale;
		const float& Top_Noise_Frequency;
		const float& Top_Noise_Scale;
		const float& Bottom_Top_Merge_Smoothness;
		const float& Global_Height_Offset;
		const float& Global_Height_Merge_Smoothness;
		const float& Global_Height_Noise_Frequency;
		const float& Global_Height_Noise_Scale;
		const float& Cave_Height;
		const float& Cave_Walls_Smoothness;
		const float& Cave_Radius;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
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
		
		FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ(const float& InBottom_Noise_Frequency, const float& InBottom_Noise_Scale, const float& InTop_Noise_Frequency, const float& InTop_Noise_Scale, const float& InBottom_Top_Merge_Smoothness, const float& InGlobal_Height_Offset, const float& InGlobal_Height_Merge_Smoothness, const float& InGlobal_Height_Noise_Frequency, const float& InGlobal_Height_Noise_Scale, const float& InCave_Height, const float& InCave_Walls_Smoothness, const float& InCave_Radius)
			: Bottom_Noise_Frequency(InBottom_Noise_Frequency)
			, Bottom_Noise_Scale(InBottom_Noise_Scale)
			, Top_Noise_Frequency(InTop_Noise_Frequency)
			, Top_Noise_Scale(InTop_Noise_Scale)
			, Bottom_Top_Merge_Smoothness(InBottom_Top_Merge_Smoothness)
			, Global_Height_Offset(InGlobal_Height_Offset)
			, Global_Height_Merge_Smoothness(InGlobal_Height_Merge_Smoothness)
			, Global_Height_Noise_Frequency(InGlobal_Height_Noise_Frequency)
			, Global_Height_Noise_Scale(InGlobal_Height_Noise_Scale)
			, Cave_Height(InCave_Height)
			, Cave_Walls_Smoothness(InCave_Walls_Smoothness)
			, Cave_Radius(InCave_Radius)
		{
		}
		
		void Init(const FVoxelWorldGeneratorInit& InitStruct)
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
		
		const float& Bottom_Noise_Frequency;
		const float& Bottom_Noise_Scale;
		const float& Top_Noise_Frequency;
		const float& Top_Noise_Scale;
		const float& Bottom_Top_Merge_Smoothness;
		const float& Global_Height_Offset;
		const float& Global_Height_Merge_Smoothness;
		const float& Global_Height_Noise_Frequency;
		const float& Global_Height_Noise_Scale;
		const float& Cave_Height;
		const float& Cave_Walls_Smoothness;
		const float& Cave_Radius;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(TVoxelRange<T>*)nullptr;
			}
			
			TVoxelRange<v_flt> Value;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
			TVoxelRange<v_flt> Variable_29; // Global Height Noise Frequency = 0.005 output 0
			TVoxelRange<v_flt> Variable_24; // Bottom Top Merge Smoothness = 25.0 output 0
			TVoxelRange<v_flt> Variable_25; // Cave Walls Smoothness = 100.0 output 0
			TVoxelRange<v_flt> Variable_26; // Global Height Merge Smoothness = 15.0 output 0
			TVoxelRange<v_flt> Variable_27; // Top Noise Frequency = 0.005 output 0
			TVoxelRange<v_flt> Variable_28; // Bottom Noise Frequency = 0.008 output 0
			TVoxelRange<v_flt> Variable_15; // Cave Radius = 400.0 output 0
			TVoxelRange<v_flt> Variable_30; // Top Noise Scale = 150.0 output 0
			TVoxelRange<v_flt> Variable_31; // Bottom Noise Scale = 150.0 output 0
			TVoxelRange<v_flt> Variable_35; // Global Height Noise Scale = 200.0 output 0
			TVoxelRange<v_flt> Variable_36; // Global Height Offset = 150.0 output 0
			TVoxelRange<v_flt> Variable_34; // / output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_19; // X output 0
			TVoxelRange<v_flt> Variable_3; // X output 0
			TVoxelRange<v_flt> Variable_0; // X output 0
			TVoxelRange<v_flt> Variable_12; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_14; // - output 0
			TVoxelRange<v_flt> Variable_7; // * output 0
			TVoxelRange<v_flt> Variable_6; // * output 0
			TVoxelRange<v_flt> Variable_22; // + output 0
		};
		
		FLocalComputeStruct_LocalValueRangeAnalysis(const float& InBottom_Noise_Frequency, const float& InBottom_Noise_Scale, const float& InTop_Noise_Frequency, const float& InTop_Noise_Scale, const float& InBottom_Top_Merge_Smoothness, const float& InGlobal_Height_Offset, const float& InGlobal_Height_Merge_Smoothness, const float& InGlobal_Height_Noise_Frequency, const float& InGlobal_Height_Noise_Scale, const float& InCave_Height, const float& InCave_Walls_Smoothness, const float& InCave_Radius)
			: Bottom_Noise_Frequency(InBottom_Noise_Frequency)
			, Bottom_Noise_Scale(InBottom_Noise_Scale)
			, Top_Noise_Frequency(InTop_Noise_Frequency)
			, Top_Noise_Scale(InTop_Noise_Scale)
			, Bottom_Top_Merge_Smoothness(InBottom_Top_Merge_Smoothness)
			, Global_Height_Offset(InGlobal_Height_Offset)
			, Global_Height_Merge_Smoothness(InGlobal_Height_Merge_Smoothness)
			, Global_Height_Noise_Frequency(InGlobal_Height_Noise_Frequency)
			, Global_Height_Noise_Scale(InGlobal_Height_Noise_Scale)
			, Cave_Height(InCave_Height)
			, Cave_Walls_Smoothness(InCave_Walls_Smoothness)
			, Cave_Radius(InCave_Radius)
		{
		}
		
		void Init(const FVoxelWorldGeneratorInit& InitStruct)
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
				// Global Height Noise Frequency = 0.005
				BufferConstant.Variable_29 = Global_Height_Noise_Frequency;
				
				// Bottom Top Merge Smoothness = 25.0
				BufferConstant.Variable_24 = Bottom_Top_Merge_Smoothness;
				
				// Cave Walls Smoothness = 100.0
				BufferConstant.Variable_25 = Cave_Walls_Smoothness;
				
				// Global Height Merge Smoothness = 15.0
				BufferConstant.Variable_26 = Global_Height_Merge_Smoothness;
				
				// Top Noise Frequency = 0.005
				BufferConstant.Variable_27 = Top_Noise_Frequency;
				
				// Bottom Noise Frequency = 0.008
				BufferConstant.Variable_28 = Bottom_Noise_Frequency;
				
				// Cave Radius = 400.0
				BufferConstant.Variable_15 = Cave_Radius;
				
				// Top Noise Scale = 150.0
				BufferConstant.Variable_30 = Top_Noise_Scale;
				
				// Bottom Noise Scale = 150.0
				BufferConstant.Variable_31 = Bottom_Noise_Scale;
				
				// Cave Height = 100.0
				TVoxelRange<v_flt> Variable_33; // Cave Height = 100.0 output 0
				Variable_33 = Cave_Height;
				
				// Global Height Noise Scale = 200.0
				BufferConstant.Variable_35 = Global_Height_Noise_Scale;
				
				// Global Height Offset = 150.0
				BufferConstant.Variable_36 = Global_Height_Offset;
				
				// /
				BufferConstant.Variable_34 = Variable_33 / TVoxelRange<v_flt>(2.0f);
				
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
		FastNoise _2D_Perlin_Noise_Fractal_2_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_2_LODToOctaves;
		FastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		FastNoise _2D_Perlin_Noise_Fractal_3_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_3_LODToOctaves;
		
		const float& Bottom_Noise_Frequency;
		const float& Bottom_Noise_Scale;
		const float& Top_Noise_Frequency;
		const float& Top_Noise_Scale;
		const float& Bottom_Top_Merge_Smoothness;
		const float& Global_Height_Offset;
		const float& Global_Height_Merge_Smoothness;
		const float& Global_Height_Noise_Frequency;
		const float& Global_Height_Noise_Scale;
		const float& Cave_Height;
		const float& Cave_Walls_Smoothness;
		const float& Cave_Radius;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of 2D Perlin Noise Fractal
			_2D_Perlin_Noise_Fractal_2_Noise.SetSeed(Seed(1337));
			_2D_Perlin_Noise_Fractal_2_Noise.SetInterp(FastNoise::Quintic);
			_2D_Perlin_Noise_Fractal_2_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_2_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_2_Noise.SetFractalType(FastNoise::FBM);
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
			_2D_IQ_Noise_1_Noise.SetSeed(Seed(1339));
			_2D_IQ_Noise_1_Noise.SetInterp(FastNoise::Quintic);
			_2D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_1_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_1_Noise.SetFractalType(FastNoise::FBM);
			_2D_IQ_Noise_1_Noise.SetMatrix(FMatrix2x2(FQuat2D(FMath::DegreesToRadians(40.000000))));
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
			_2D_Perlin_Noise_Fractal_3_Noise.SetSeed(Seed(1338));
			_2D_Perlin_Noise_Fractal_3_Noise.SetInterp(FastNoise::Quintic);
			_2D_Perlin_Noise_Fractal_3_Noise.SetFractalOctavesAndGain(3, 0.5);
			_2D_Perlin_Noise_Fractal_3_Noise.SetFractalLacunarity(2.0);
			_2D_Perlin_Noise_Fractal_3_Noise.SetFractalType(FastNoise::FBM);
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
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Z
			TVoxelRange<v_flt> Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_19; // X output 0
			Variable_19 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_4; // Y output 0
			Variable_4 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_17; // Z output 0
			Variable_17 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_3; // X output 0
			Variable_3 = Context.GetLocalX();
			
			// X
			TVoxelRange<v_flt> Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_13; // Y output 0
			Variable_13 = Context.GetLocalY();
			
			// Y
			TVoxelRange<v_flt> Variable_20; // Y output 0
			Variable_20 = Context.GetLocalY();
			
			// Y
			TVoxelRange<v_flt> Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_12; // X output 0
			Variable_12 = Context.GetLocalX();
			
			// Vector Length.Vector Length
			TVoxelRange<v_flt> Variable_37; // Vector Length.Vector Length output 0
			Variable_37 = FVoxelNodeFunctions::VectorLength(Variable_12, Variable_13, TVoxelRange<v_flt>(0.0f));
			
			// 2D Perlin Noise Fractal
			TVoxelRange<v_flt> Variable_2; // 2D Perlin Noise Fractal output 0
			Variable_2 = { -0.643471f, 0.527891f };
			
			// 2D IQ Noise
			TVoxelRange<v_flt> Variable_23; // 2D IQ Noise output 0
			TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
			TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
			Variable_23 = { -0.779186f, 0.705623f };
			_2D_IQ_Noise_1_Temp_1 = { -1.482251f, 1.789484f };
			_2D_IQ_Noise_1_Temp_2 = { -1.568460f, 1.481016f };
			
			// 2D Perlin Noise Fractal
			TVoxelRange<v_flt> Variable_5; // 2D Perlin Noise Fractal output 0
			Variable_5 = { -0.643471f, 0.527891f };
			
			// *
			TVoxelRange<v_flt> Variable_21; // * output 0
			Variable_21 = Variable_23 * BufferConstant.Variable_35;
			
			// -
			TVoxelRange<v_flt> Variable_14; // - output 0
			Variable_14 = BufferConstant.Variable_15 - Variable_37;
			
			// *
			TVoxelRange<v_flt> Variable_7; // * output 0
			Variable_7 = Variable_5 * BufferConstant.Variable_31;
			
			// *
			TVoxelRange<v_flt> Variable_6; // * output 0
			Variable_6 = Variable_2 * BufferConstant.Variable_30;
			
			// -
			TVoxelRange<v_flt> Variable_10; // - output 0
			Variable_10 = Variable_6 - Variable_11;
			
			// +
			TVoxelRange<v_flt> Variable_22; // + output 0
			Variable_22 = Variable_21 + BufferConstant.Variable_36;
			
			// -
			TVoxelRange<v_flt> Variable_9; // - output 0
			Variable_9 = Variable_8 - Variable_7;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_55; // Smooth Union.- output 0
			Variable_55 = Variable_10 - Variable_9;
			
			// -
			TVoxelRange<v_flt> Variable_18; // - output 0
			Variable_18 = Variable_17 - Variable_22;
			
			// Smooth Union./
			TVoxelRange<v_flt> Variable_56; // Smooth Union./ output 0
			Variable_56 = Variable_55 / BufferConstant.Variable_24;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_57; // Smooth Union.* output 0
			Variable_57 = Variable_56 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.+
			TVoxelRange<v_flt> Variable_58; // Smooth Union.+ output 0
			Variable_58 = Variable_57 + TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.Clamp
			TVoxelRange<v_flt> Variable_59; // Smooth Union.Clamp output 0
			Variable_59 = FVoxelNodeFunctions::Clamp(Variable_58, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Union.1 - X
			TVoxelRange<v_flt> Variable_63; // Smooth Union.1 - X output 0
			Variable_63 = 1 - Variable_59;
			
			// Smooth Union.Lerp
			TVoxelRange<v_flt> Variable_60; // Smooth Union.Lerp output 0
			Variable_60 = FVoxelNodeFunctions::Lerp(Variable_10, Variable_9, Variable_59);
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_62; // Smooth Union.* output 0
			Variable_62 = BufferConstant.Variable_24 * Variable_59 * Variable_63;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_61; // Smooth Union.- output 0
			Variable_61 = Variable_60 - Variable_62;
			
			// +
			TVoxelRange<v_flt> Variable_32; // + output 0
			Variable_32 = Variable_61 + BufferConstant.Variable_34;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_38; // Smooth Union.- output 0
			Variable_38 = Variable_14 - Variable_32;
			
			// Smooth Union./
			TVoxelRange<v_flt> Variable_39; // Smooth Union./ output 0
			Variable_39 = Variable_38 / BufferConstant.Variable_25;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_40; // Smooth Union.* output 0
			Variable_40 = Variable_39 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.+
			TVoxelRange<v_flt> Variable_41; // Smooth Union.+ output 0
			Variable_41 = Variable_40 + TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.Clamp
			TVoxelRange<v_flt> Variable_42; // Smooth Union.Clamp output 0
			Variable_42 = FVoxelNodeFunctions::Clamp(Variable_41, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Union.1 - X
			TVoxelRange<v_flt> Variable_46; // Smooth Union.1 - X output 0
			Variable_46 = 1 - Variable_42;
			
			// Smooth Union.Lerp
			TVoxelRange<v_flt> Variable_43; // Smooth Union.Lerp output 0
			Variable_43 = FVoxelNodeFunctions::Lerp(Variable_14, Variable_32, Variable_42);
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_45; // Smooth Union.* output 0
			Variable_45 = BufferConstant.Variable_25 * Variable_42 * Variable_46;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_44; // Smooth Union.- output 0
			Variable_44 = Variable_43 - Variable_45;
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_54; // Smooth Intersection.- output 0
			Variable_54 = Variable_18 - Variable_44;
			
			// Smooth Intersection./
			TVoxelRange<v_flt> Variable_47; // Smooth Intersection./ output 0
			Variable_47 = Variable_54 / BufferConstant.Variable_26;
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_48; // Smooth Intersection.* output 0
			Variable_48 = Variable_47 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_16; // Smooth Intersection.- output 0
			Variable_16 = TVoxelRange<v_flt>(0.5f) - Variable_48;
			
			// Smooth Intersection.Clamp
			TVoxelRange<v_flt> Variable_49; // Smooth Intersection.Clamp output 0
			Variable_49 = FVoxelNodeFunctions::Clamp(Variable_16, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Intersection.Lerp
			TVoxelRange<v_flt> Variable_50; // Smooth Intersection.Lerp output 0
			Variable_50 = FVoxelNodeFunctions::Lerp(Variable_18, Variable_44, Variable_49);
			
			// Smooth Intersection.1 - X
			TVoxelRange<v_flt> Variable_52; // Smooth Intersection.1 - X output 0
			Variable_52 = 1 - Variable_49;
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_51; // Smooth Intersection.* output 0
			Variable_51 = BufferConstant.Variable_26 * Variable_49 * Variable_52;
			
			// Smooth Intersection.+
			TVoxelRange<v_flt> Variable_53; // Smooth Intersection.+ output 0
			Variable_53 = Variable_50 + Variable_51;
			
			Outputs.Value = Variable_53;
		}
		
	};
	
	FVoxelExample_CaveInstance(const float& InBottom_Noise_Frequency, const float& InBottom_Noise_Scale, const float& InTop_Noise_Frequency, const float& InTop_Noise_Scale, const float& InBottom_Top_Merge_Smoothness, const float& InGlobal_Height_Offset, const float& InGlobal_Height_Merge_Smoothness, const float& InGlobal_Height_Noise_Frequency, const float& InGlobal_Height_Noise_Scale, const float& InCave_Height, const float& InCave_Walls_Smoothness, const float& InCave_Radius, bool bEnableRangeAnalysis)
		: TVoxelGraphGeneratorInstanceHelper(
		{
			{"Value", 1}
		},
		{
		},
		{
			{"Value", NoTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr<v_flt>>()}
		},
		{
		},
		{
			{"Value", NoTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr<v_flt>>()}
		},
		{
			{"Value", WithTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr_Transform<v_flt>>()}
		},
		{
		},
		{
			{"Value", WithTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr_Transform<v_flt>>()}
		},
		bEnableRangeAnalysis)
		, Bottom_Noise_Frequency(InBottom_Noise_Frequency)
		, Bottom_Noise_Scale(InBottom_Noise_Scale)
		, Top_Noise_Frequency(InTop_Noise_Frequency)
		, Top_Noise_Scale(InTop_Noise_Scale)
		, Bottom_Top_Merge_Smoothness(InBottom_Top_Merge_Smoothness)
		, Global_Height_Offset(InGlobal_Height_Offset)
		, Global_Height_Merge_Smoothness(InGlobal_Height_Merge_Smoothness)
		, Global_Height_Noise_Frequency(InGlobal_Height_Noise_Frequency)
		, Global_Height_Noise_Scale(InGlobal_Height_Noise_Scale)
		, Cave_Height(InCave_Height)
		, Cave_Walls_Smoothness(InCave_Walls_Smoothness)
		, Cave_Radius(InCave_Radius)
		, LocalValue(Bottom_Noise_Frequency, Bottom_Noise_Scale, Top_Noise_Frequency, Top_Noise_Scale, Bottom_Top_Merge_Smoothness, Global_Height_Offset, Global_Height_Merge_Smoothness, Global_Height_Noise_Frequency, Global_Height_Noise_Scale, Cave_Height, Cave_Walls_Smoothness, Cave_Radius)
		, LocalMaterial(Bottom_Noise_Frequency, Bottom_Noise_Scale, Top_Noise_Frequency, Top_Noise_Scale, Bottom_Top_Merge_Smoothness, Global_Height_Offset, Global_Height_Merge_Smoothness, Global_Height_Noise_Frequency, Global_Height_Noise_Scale, Cave_Height, Cave_Walls_Smoothness, Cave_Radius)
		, LocalUpVectorXUpVectorYUpVectorZ(Bottom_Noise_Frequency, Bottom_Noise_Scale, Top_Noise_Frequency, Top_Noise_Scale, Bottom_Top_Merge_Smoothness, Global_Height_Offset, Global_Height_Merge_Smoothness, Global_Height_Noise_Frequency, Global_Height_Noise_Scale, Cave_Height, Cave_Walls_Smoothness, Cave_Radius)
		, LocalValueRangeAnalysis(Bottom_Noise_Frequency, Bottom_Noise_Scale, Top_Noise_Frequency, Top_Noise_Scale, Bottom_Top_Merge_Smoothness, Global_Height_Offset, Global_Height_Merge_Smoothness, Global_Height_Noise_Frequency, Global_Height_Noise_Scale, Cave_Height, Cave_Walls_Smoothness, Cave_Radius)
	{
	}
	
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) override final
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
	
	inline void ReportRangeAnalysisFailure() const {}
	
private:
	const float Bottom_Noise_Frequency;
	const float Bottom_Noise_Scale;
	const float Top_Noise_Frequency;
	const float Top_Noise_Scale;
	const float Bottom_Top_Merge_Smoothness;
	const float Global_Height_Offset;
	const float Global_Height_Merge_Smoothness;
	const float Global_Height_Noise_Frequency;
	const float Global_Height_Noise_Scale;
	const float Cave_Height;
	const float Cave_Walls_Smoothness;
	const float Cave_Radius;
	FLocalComputeStruct_LocalValue LocalValue;
	FLocalComputeStruct_LocalMaterial LocalMaterial;
	FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ LocalUpVectorXUpVectorYUpVectorZ;
	FLocalComputeStruct_LocalValueRangeAnalysis LocalValueRangeAnalysis;
	
};

template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValue::FOutputs::GetRef<v_flt, 1>()
{
	return Value;
}
template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalMaterial::FOutputs::GetRef<FVoxelMaterial, 2>()
{
	return Material;
}
template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 3>()
{
	return UpVectorX;
}
template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 4>()
{
	return UpVectorY;
}
template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 5>()
{
	return UpVectorZ;
}
template<>
inline auto& FVoxelExample_CaveInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::GetRef<v_flt, 1>()
{
	return Value;
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

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Cave::UVoxelExample_Cave()
{
	bEnableRangeAnalysis = true;
}

TMap<FName, int32> UVoxelExample_Cave::GetDefaultSeeds() const
{
	return {
		{ "Top Noise Seed", 3022 },
		{ "Bottom Noise Seed", 3024 },
		{ "Global Height Seed", 1447 },
		};
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelExample_Cave::GetTransformableInstance()
{
	return MakeVoxelShared<FVoxelExample_CaveInstance>(
		Bottom_Noise_Frequency,
		Bottom_Noise_Scale,
		Top_Noise_Frequency,
		Top_Noise_Scale,
		Bottom_Top_Merge_Smoothness,
		Global_Height_Offset,
		Global_Height_Merge_Smoothness,
		Global_Height_Noise_Frequency,
		Global_Height_Noise_Scale,
		Cave_Height,
		Cave_Walls_Smoothness,
		Cave_Radius,
		bEnableRangeAnalysis);
}

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma warning(pop)
#endif

