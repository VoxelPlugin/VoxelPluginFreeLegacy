// Copyright 2020 Phyronnaz

#include "VoxelExample_RingWorld.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_RingWorldInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_RingWorldInstance, UVoxelExample_RingWorld>
{
public:
	struct FParams
	{
		const float Radius;
		const float RingEdgesHardness;
		const float Scale;
		const float Thickness;
		const float Width_in_Degrees;
		const float RiverDepth;
		const float RiverWidth;
		const FColor BeachColor;
		const FColor MountainsColorHigh;
		const FColor MountainsColorLowHigh;
		const FColor MountainsColorLowLow;
		const FVoxelRichCurve MoutainsMaskCurve;
		const FColor PlainsColorHigh;
		const FColor PlainsColorLow;
		const float PlainsNoiseFrequency;
		const float PlainsNoiseHeight;
		const FVoxelRichCurve PlainsNoiseStrengthCurve;
		const FVoxelRichCurve RingMainShapeCurve;
		const FColor RingOuterColor;
		const FColor RiverColor;
		const FVoxelRichCurve RiverDepthCurve;
		const float MountainsNoiseFrequency;
		const float MountainsNoiseHeight;
		const float BaseNoiseFrquency;
		const float BaseNoiseHeight;
		const float BaseHeight;
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
			
			v_flt Variable_51; // MountainsNoiseFrequency = 0.2 output 0
			v_flt Variable_22; // RingEdgesHardness = 10.0 output 0
			v_flt Variable_60; // PlainsNoiseFrequency = 0.2 output 0
			v_flt Variable_48; // MountainsNoiseHeight = 500.0 output 0
			v_flt Variable_42; // BaseNoiseHeight = 250.0 output 0
			v_flt Variable_89; // RiverWidth = 1.0 output 0
			v_flt Variable_39; // BaseHeight = 1000.0 output 0
			v_flt Variable_57; // PlainsNoiseHeight = 250.0 output 0
			v_flt Variable_45; // BaseNoiseFrquency = 0.005 output 0
			v_flt Variable_65; // RiverDepth = 100.0 output 0
			v_flt Variable_84; // Scale = 10.0 output 0
			v_flt Variable_85; // * output 0
			v_flt Variable_36; // / output 0
			v_flt Variable_6; // + output 0
			v_flt Variable_86; // / output 0
			v_flt Variable_35; // - output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_0; // X output 0
			v_flt Variable_24; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_1; // Y output 0
			v_flt Variable_44; // * output 0
			v_flt Variable_59; // * output 0
			v_flt Variable_50; // * output 0
			v_flt Variable_74; // * output 0
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
				// MountainsNoiseFrequency = 0.2
				BufferConstant.Variable_51 = Params.MountainsNoiseFrequency;
				
				// Radius = 7000.0
				v_flt Variable_5; // Radius = 7000.0 output 0
				Variable_5 = Params.Radius;
				
				// RingEdgesHardness = 10.0
				BufferConstant.Variable_22 = Params.RingEdgesHardness;
				
				// Width in Degrees = 50.0
				v_flt Variable_30; // Width in Degrees = 50.0 output 0
				Variable_30 = Params.Width_in_Degrees;
				
				// PlainsNoiseFrequency = 0.2
				BufferConstant.Variable_60 = Params.PlainsNoiseFrequency;
				
				// MountainsNoiseHeight = 500.0
				BufferConstant.Variable_48 = Params.MountainsNoiseHeight;
				
				// BaseNoiseHeight = 250.0
				BufferConstant.Variable_42 = Params.BaseNoiseHeight;
				
				// RiverWidth = 1.0
				BufferConstant.Variable_89 = Params.RiverWidth;
				
				// BaseHeight = 1000.0
				BufferConstant.Variable_39 = Params.BaseHeight;
				
				// Thickness = 500.0
				v_flt Variable_90; // Thickness = 500.0 output 0
				Variable_90 = Params.Thickness;
				
				// PlainsNoiseHeight = 250.0
				BufferConstant.Variable_57 = Params.PlainsNoiseHeight;
				
				// BaseNoiseFrquency = 0.005
				BufferConstant.Variable_45 = Params.BaseNoiseFrquency;
				
				// RiverDepth = 100.0
				BufferConstant.Variable_65 = Params.RiverDepth;
				
				// Scale = 10.0
				BufferConstant.Variable_84 = Params.Scale;
				
				// *
				v_flt Variable_91; // * output 0
				Variable_91 = Variable_90 * BufferConstant.Variable_84;
				
				// *
				BufferConstant.Variable_85 = Variable_5 * BufferConstant.Variable_84;
				
				// /
				BufferConstant.Variable_36 = Variable_30 / v_flt(360.0f);
				
				// +
				BufferConstant.Variable_6 = BufferConstant.Variable_85 + Variable_91;
				
				// /
				BufferConstant.Variable_86 = BufferConstant.Variable_85 / BufferConstant.Variable_84;
				
				// -
				BufferConstant.Variable_35 = v_flt(0.5f) - BufferConstant.Variable_36;
				
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
		
		FVoxelFastNoise _2D_Simplex_Noise_0_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _2D_Simplex_Noise_Fractal_0_LODToOctaves;
		FVoxelFastNoise _2D_Simplex_Noise_1_Noise;
		FVoxelFastNoise _2D_Perlin_Noise_0_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Simplex_Noise_Fractal_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 2D Simplex Noise
			_2D_Simplex_Noise_0_Noise.SetSeed(FVoxelGraphSeed(1000));
			_2D_Simplex_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Simplex Noise Fractal
			_2D_Simplex_Noise_Fractal_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Simplex_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Simplex_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(5, 0.5);
			_2D_Simplex_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_2D_Simplex_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[0] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[1] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[2] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[3] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[4] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[5] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[6] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[7] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[8] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[9] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[10] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[11] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[12] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[13] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[14] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[15] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[16] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[17] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[18] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[19] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[20] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[21] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[22] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[23] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[24] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[25] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[26] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[27] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[28] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[29] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[30] = 5;
			_2D_Simplex_Noise_Fractal_0_LODToOctaves[31] = 5;
			
			// Init of 2D Simplex Noise
			_2D_Simplex_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Simplex_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Perlin Noise
			_2D_Perlin_Noise_0_Noise.SetSeed(FVoxelGraphSeed(1338));
			_2D_Perlin_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Simplex Noise Fractal
			_2D_Simplex_Noise_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Simplex_Noise_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Simplex_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(8, 0.5);
			_2D_Simplex_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
			_2D_Simplex_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[0] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[1] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[2] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[3] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[4] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[5] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[6] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[7] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[8] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[9] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[10] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[11] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[12] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[13] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[14] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[15] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[16] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[17] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[18] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[19] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[20] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[21] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[22] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[23] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[24] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[25] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[26] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[27] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[28] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[29] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[30] = 8;
			_2D_Simplex_Noise_Fractal_1_LODToOctaves[31] = 8;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_24 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			v_flt Variable_28; // Y output 0
			Variable_28 = Context.GetLocalY();
			
			// Atan2
			v_flt Variable_23; // Atan2 output 0
			Variable_23 = FVoxelNodeFunctions::Atan2(Variable_28, BufferX.Variable_24);
			
			// /
			v_flt Variable_25; // / output 0
			Variable_25 = Variable_23 / v_flt(3.141593f);
			
			// *
			v_flt Variable_27; // * output 0
			Variable_27 = BufferConstant.Variable_86 * Variable_25;
			
			// *
			BufferXY.Variable_44 = Variable_27 * BufferConstant.Variable_45;
			
			// *
			BufferXY.Variable_59 = Variable_27 * BufferConstant.Variable_60;
			
			// *
			BufferXY.Variable_50 = Variable_27 * BufferConstant.Variable_51;
			
			// 2D Simplex Noise
			v_flt Variable_77; // 2D Simplex Noise output 0
			Variable_77 = _2D_Simplex_Noise_0_Noise.GetSimplex_2D(Variable_27, Variable_27, v_flt(0.001f));
			Variable_77 = FMath::Clamp<v_flt>(Variable_77, -0.997888, 0.997883);
			
			// *
			BufferXY.Variable_74 = Variable_77 * v_flt(0.1f);
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_24 = Context.GetLocalX();
			
			// Y
			v_flt Variable_28; // Y output 0
			Variable_28 = Context.GetLocalY();
			
			// Atan2
			v_flt Variable_23; // Atan2 output 0
			Variable_23 = FVoxelNodeFunctions::Atan2(Variable_28, BufferX.Variable_24);
			
			// /
			v_flt Variable_25; // / output 0
			Variable_25 = Variable_23 / v_flt(3.141593f);
			
			// *
			v_flt Variable_27; // * output 0
			Variable_27 = BufferConstant.Variable_86 * Variable_25;
			
			// *
			BufferXY.Variable_44 = Variable_27 * BufferConstant.Variable_45;
			
			// *
			BufferXY.Variable_59 = Variable_27 * BufferConstant.Variable_60;
			
			// *
			BufferXY.Variable_50 = Variable_27 * BufferConstant.Variable_51;
			
			// 2D Simplex Noise
			v_flt Variable_77; // 2D Simplex Noise output 0
			Variable_77 = _2D_Simplex_Noise_0_Noise.GetSimplex_2D(Variable_27, Variable_27, v_flt(0.001f));
			Variable_77 = FMath::Clamp<v_flt>(Variable_77, -0.997888, 0.997883);
			
			// *
			BufferXY.Variable_74 = Variable_77 * v_flt(0.1f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_70; // Z output 0
			Variable_70 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_12; // Z output 0
			Variable_12 = Context.GetLocalZ();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferXY.Variable_1, Variable_2);
			
			// +
			v_flt Variable_29; // + output 0
			Variable_29 = Variable_3 + v_flt(0.001f);
			
			// /
			v_flt Variable_13; // / output 0
			Variable_13 = Variable_12 / Variable_29;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_29 - BufferConstant.Variable_6;
			
			// /
			v_flt Variable_71; // / output 0
			Variable_71 = Variable_70 / Variable_29;
			
			// ACOS
			v_flt Variable_14; // ACOS output 0
			Variable_14 = FVoxelNodeFunctions::Acos(Variable_13);
			
			// /
			v_flt Variable_10; // / output 0
			Variable_10 = Variable_14 / v_flt(3.141593f);
			
			// 1 - X
			v_flt Variable_11; // 1 - X output 0
			Variable_11 = 1 - Variable_10;
			
			// *
			v_flt Variable_26; // * output 0
			Variable_26 = BufferConstant.Variable_86 * Variable_10;
			
			// +
			v_flt Variable_73; // + output 0
			Variable_73 = Variable_71 + BufferXY.Variable_74;
			
			// Min (float)
			v_flt Variable_9; // Min (float) output 0
			Variable_9 = FVoxelNodeFunctions::Min<v_flt>(Variable_10, Variable_11);
			
			// *
			v_flt Variable_43; // * output 0
			Variable_43 = Variable_26 * BufferConstant.Variable_45;
			
			// *
			v_flt Variable_49; // * output 0
			Variable_49 = Variable_26 * BufferConstant.Variable_51;
			
			// ACOS
			v_flt Variable_72; // ACOS output 0
			Variable_72 = FVoxelNodeFunctions::Acos(Variable_73);
			
			// *
			v_flt Variable_58; // * output 0
			Variable_58 = Variable_26 * BufferConstant.Variable_60;
			
			// -
			v_flt Variable_15; // - output 0
			Variable_15 = Variable_9 - BufferConstant.Variable_35;
			
			// 2D Simplex Noise Fractal
			v_flt Variable_40; // 2D Simplex Noise Fractal output 0
			Variable_40 = _2D_Simplex_Noise_Fractal_0_Noise.GetSimplexFractal_2D(Variable_43, BufferXY.Variable_44, v_flt(0.02f), _2D_Simplex_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_40 = FMath::Clamp<v_flt>(Variable_40, -0.802678, 0.846347);
			
			// 2D Simplex Noise
			v_flt Variable_62; // 2D Simplex Noise output 0
			Variable_62 = _2D_Simplex_Noise_1_Noise.GetSimplex_2D(Variable_58, BufferXY.Variable_59, v_flt(0.02f));
			Variable_62 = FMath::Clamp<v_flt>(Variable_62, -0.997652, 0.996970);
			
			// 2D Perlin Noise
			v_flt Variable_53; // 2D Perlin Noise output 0
			Variable_53 = _2D_Perlin_Noise_0_Noise.GetPerlin_2D(Variable_49, BufferXY.Variable_50, v_flt(0.1f));
			Variable_53 = FMath::Clamp<v_flt>(Variable_53, -0.888317, 0.811183);
			
			// 2D Simplex Noise Fractal
			v_flt Variable_46; // 2D Simplex Noise Fractal output 0
			Variable_46 = _2D_Simplex_Noise_Fractal_1_Noise.GetSimplexFractal_2D(Variable_49, BufferXY.Variable_50, v_flt(0.02f), _2D_Simplex_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_46 = FMath::Clamp<v_flt>(Variable_46, -0.780446, 0.760988);
			
			// /
			v_flt Variable_69; // / output 0
			Variable_69 = Variable_72 / v_flt(3.141593f);
			
			// /
			v_flt Variable_34; // / output 0
			Variable_34 = Variable_15 / BufferConstant.Variable_36;
			
			// 1 - X
			v_flt Variable_67; // 1 - X output 0
			Variable_67 = 1 - Variable_69;
			
			// *
			v_flt Variable_41; // * output 0
			Variable_41 = Variable_40 * BufferConstant.Variable_42;
			
			// 1 - X
			v_flt Variable_33; // 1 - X output 0
			Variable_33 = 1 - Variable_34;
			
			// Max (float)
			v_flt Variable_18; // Max (float) output 0
			Variable_18 = FVoxelNodeFunctions::Max<v_flt>(Variable_34, v_flt(0.0f));
			
			// Min (float)
			v_flt Variable_66; // Min (float) output 0
			Variable_66 = FVoxelNodeFunctions::Min<v_flt>(Variable_69, Variable_67);
			
			// Float Curve: PlainsNoiseStrengthCurve
			v_flt Variable_61; // Float Curve: PlainsNoiseStrengthCurve output 0
			Variable_61 = FVoxelNodeFunctions::GetCurveValue(Params.PlainsNoiseStrengthCurve, Variable_33);
			
			// Float Curve: RingMainShapeCurve
			v_flt Variable_37; // Float Curve: RingMainShapeCurve output 0
			Variable_37 = FVoxelNodeFunctions::GetCurveValue(Params.RingMainShapeCurve, Variable_33);
			
			// Float Curve: MoutainsMaskCurve
			v_flt Variable_52; // Float Curve: MoutainsMaskCurve output 0
			Variable_52 = FVoxelNodeFunctions::GetCurveValue(Params.MoutainsMaskCurve, Variable_33);
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = Variable_18 * BufferConstant.Variable_22;
			
			// /
			v_flt Variable_68; // / output 0
			Variable_68 = Variable_66 / v_flt(0.5f);
			
			// *
			v_flt Variable_54; // * output 0
			Variable_54 = BufferConstant.Variable_48 * Variable_52 * Variable_53 * v_flt(0.1f);
			
			// *
			v_flt Variable_38; // * output 0
			Variable_38 = Variable_37 * BufferConstant.Variable_39;
			
			// *
			v_flt Variable_47; // * output 0
			Variable_47 = Variable_46 * BufferConstant.Variable_48 * Variable_52;
			
			// Min (float)
			v_flt Variable_21; // Min (float) output 0
			Variable_21 = FVoxelNodeFunctions::Min<v_flt>(Variable_20, v_flt(1.0f));
			
			// 1 - X
			v_flt Variable_75; // 1 - X output 0
			Variable_75 = 1 - Variable_68;
			
			// 1 - X
			v_flt Variable_55; // 1 - X output 0
			Variable_55 = 1 - Variable_52;
			
			// *
			v_flt Variable_76; // * output 0
			Variable_76 = Variable_75 * v_flt(20.0f);
			
			// /
			v_flt Variable_88; // / output 0
			Variable_88 = Variable_76 / BufferConstant.Variable_89;
			
			// Float Curve: RiverDepthCurve
			v_flt Variable_63; // Float Curve: RiverDepthCurve output 0
			Variable_63 = FVoxelNodeFunctions::GetCurveValue(Params.RiverDepthCurve, Variable_88);
			
			// Max (float)
			v_flt Variable_78; // Max (float) output 0
			Variable_78 = FVoxelNodeFunctions::Max<v_flt>(Variable_63, v_flt(0.0f));
			
			// Min (float)
			v_flt Variable_79; // Min (float) output 0
			Variable_79 = FVoxelNodeFunctions::Min<v_flt>(Variable_63, v_flt(0.0f));
			
			// *
			v_flt Variable_64; // * output 0
			Variable_64 = Variable_78 * v_flt(-1.0f) * BufferConstant.Variable_65;
			
			// *
			v_flt Variable_80; // * output 0
			Variable_80 = Variable_79 * v_flt(-1.0f);
			
			// 1 - X
			v_flt Variable_82; // 1 - X output 0
			Variable_82 = 1 - Variable_80;
			
			// *
			v_flt Variable_81; // * output 0
			Variable_81 = Variable_62 * Variable_80;
			
			// -
			v_flt Variable_83; // - output 0
			Variable_83 = Variable_81 - Variable_82;
			
			// *
			v_flt Variable_56; // * output 0
			Variable_56 = Variable_83 * BufferConstant.Variable_57 * Variable_55 * Variable_61;
			
			// +
			v_flt Variable_32; // + output 0
			Variable_32 = Variable_38 + Variable_41 + Variable_47 + Variable_54 + Variable_56 + Variable_64;
			
			// *
			v_flt Variable_87; // * output 0
			Variable_87 = Variable_32 * BufferConstant.Variable_84;
			
			// -
			v_flt Variable_31; // - output 0
			Variable_31 = BufferConstant.Variable_85 - Variable_87;
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_31 - Variable_29;
			
			// Max (float)
			v_flt Variable_8; // Max (float) output 0
			Variable_8 = FVoxelNodeFunctions::Max<v_flt>(Variable_4, Variable_7);
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_8 - v_flt(1.0f);
			
			// *
			v_flt Variable_16; // * output 0
			Variable_16 = Variable_19 * Variable_21;
			
			// +
			v_flt Variable_17; // + output 0
			Variable_17 = Variable_16 + v_flt(1.0f);
			
			Outputs.Value = Variable_17;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_70; // Z output 0
			Variable_70 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// X
			v_flt Variable_24; // X output 0
			Variable_24 = Context.GetLocalX();
			
			// Z
			v_flt Variable_12; // Z output 0
			Variable_12 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_28; // Y output 0
			Variable_28 = Context.GetLocalY();
			
			// Atan2
			v_flt Variable_23; // Atan2 output 0
			Variable_23 = FVoxelNodeFunctions::Atan2(Variable_28, Variable_24);
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// /
			v_flt Variable_25; // / output 0
			Variable_25 = Variable_23 / v_flt(3.141593f);
			
			// +
			v_flt Variable_29; // + output 0
			Variable_29 = Variable_3 + v_flt(0.001f);
			
			// /
			v_flt Variable_13; // / output 0
			Variable_13 = Variable_12 / Variable_29;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_29 - BufferConstant.Variable_6;
			
			// /
			v_flt Variable_71; // / output 0
			Variable_71 = Variable_70 / Variable_29;
			
			// *
			v_flt Variable_27; // * output 0
			Variable_27 = BufferConstant.Variable_86 * Variable_25;
			
			// ACOS
			v_flt Variable_14; // ACOS output 0
			Variable_14 = FVoxelNodeFunctions::Acos(Variable_13);
			
			// *
			v_flt Variable_44; // * output 0
			Variable_44 = Variable_27 * BufferConstant.Variable_45;
			
			// *
			v_flt Variable_59; // * output 0
			Variable_59 = Variable_27 * BufferConstant.Variable_60;
			
			// *
			v_flt Variable_50; // * output 0
			Variable_50 = Variable_27 * BufferConstant.Variable_51;
			
			// 2D Simplex Noise
			v_flt Variable_77; // 2D Simplex Noise output 0
			Variable_77 = _2D_Simplex_Noise_0_Noise.GetSimplex_2D(Variable_27, Variable_27, v_flt(0.001f));
			Variable_77 = FMath::Clamp<v_flt>(Variable_77, -0.997888, 0.997883);
			
			// *
			v_flt Variable_74; // * output 0
			Variable_74 = Variable_77 * v_flt(0.1f);
			
			// /
			v_flt Variable_10; // / output 0
			Variable_10 = Variable_14 / v_flt(3.141593f);
			
			// 1 - X
			v_flt Variable_11; // 1 - X output 0
			Variable_11 = 1 - Variable_10;
			
			// *
			v_flt Variable_26; // * output 0
			Variable_26 = BufferConstant.Variable_86 * Variable_10;
			
			// +
			v_flt Variable_73; // + output 0
			Variable_73 = Variable_71 + Variable_74;
			
			// Min (float)
			v_flt Variable_9; // Min (float) output 0
			Variable_9 = FVoxelNodeFunctions::Min<v_flt>(Variable_10, Variable_11);
			
			// *
			v_flt Variable_43; // * output 0
			Variable_43 = Variable_26 * BufferConstant.Variable_45;
			
			// *
			v_flt Variable_49; // * output 0
			Variable_49 = Variable_26 * BufferConstant.Variable_51;
			
			// ACOS
			v_flt Variable_72; // ACOS output 0
			Variable_72 = FVoxelNodeFunctions::Acos(Variable_73);
			
			// *
			v_flt Variable_58; // * output 0
			Variable_58 = Variable_26 * BufferConstant.Variable_60;
			
			// -
			v_flt Variable_15; // - output 0
			Variable_15 = Variable_9 - BufferConstant.Variable_35;
			
			// 2D Simplex Noise Fractal
			v_flt Variable_40; // 2D Simplex Noise Fractal output 0
			Variable_40 = _2D_Simplex_Noise_Fractal_0_Noise.GetSimplexFractal_2D(Variable_43, Variable_44, v_flt(0.02f), _2D_Simplex_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_40 = FMath::Clamp<v_flt>(Variable_40, -0.802678, 0.846347);
			
			// 2D Simplex Noise
			v_flt Variable_62; // 2D Simplex Noise output 0
			Variable_62 = _2D_Simplex_Noise_1_Noise.GetSimplex_2D(Variable_58, Variable_59, v_flt(0.02f));
			Variable_62 = FMath::Clamp<v_flt>(Variable_62, -0.997652, 0.996970);
			
			// 2D Perlin Noise
			v_flt Variable_53; // 2D Perlin Noise output 0
			Variable_53 = _2D_Perlin_Noise_0_Noise.GetPerlin_2D(Variable_49, Variable_50, v_flt(0.1f));
			Variable_53 = FMath::Clamp<v_flt>(Variable_53, -0.888317, 0.811183);
			
			// 2D Simplex Noise Fractal
			v_flt Variable_46; // 2D Simplex Noise Fractal output 0
			Variable_46 = _2D_Simplex_Noise_Fractal_1_Noise.GetSimplexFractal_2D(Variable_49, Variable_50, v_flt(0.02f), _2D_Simplex_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_46 = FMath::Clamp<v_flt>(Variable_46, -0.780446, 0.760988);
			
			// /
			v_flt Variable_69; // / output 0
			Variable_69 = Variable_72 / v_flt(3.141593f);
			
			// /
			v_flt Variable_34; // / output 0
			Variable_34 = Variable_15 / BufferConstant.Variable_36;
			
			// 1 - X
			v_flt Variable_67; // 1 - X output 0
			Variable_67 = 1 - Variable_69;
			
			// *
			v_flt Variable_41; // * output 0
			Variable_41 = Variable_40 * BufferConstant.Variable_42;
			
			// 1 - X
			v_flt Variable_33; // 1 - X output 0
			Variable_33 = 1 - Variable_34;
			
			// Max (float)
			v_flt Variable_18; // Max (float) output 0
			Variable_18 = FVoxelNodeFunctions::Max<v_flt>(Variable_34, v_flt(0.0f));
			
			// Min (float)
			v_flt Variable_66; // Min (float) output 0
			Variable_66 = FVoxelNodeFunctions::Min<v_flt>(Variable_69, Variable_67);
			
			// Float Curve: PlainsNoiseStrengthCurve
			v_flt Variable_61; // Float Curve: PlainsNoiseStrengthCurve output 0
			Variable_61 = FVoxelNodeFunctions::GetCurveValue(Params.PlainsNoiseStrengthCurve, Variable_33);
			
			// Float Curve: RingMainShapeCurve
			v_flt Variable_37; // Float Curve: RingMainShapeCurve output 0
			Variable_37 = FVoxelNodeFunctions::GetCurveValue(Params.RingMainShapeCurve, Variable_33);
			
			// Float Curve: MoutainsMaskCurve
			v_flt Variable_52; // Float Curve: MoutainsMaskCurve output 0
			Variable_52 = FVoxelNodeFunctions::GetCurveValue(Params.MoutainsMaskCurve, Variable_33);
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = Variable_18 * BufferConstant.Variable_22;
			
			// /
			v_flt Variable_68; // / output 0
			Variable_68 = Variable_66 / v_flt(0.5f);
			
			// *
			v_flt Variable_54; // * output 0
			Variable_54 = BufferConstant.Variable_48 * Variable_52 * Variable_53 * v_flt(0.1f);
			
			// *
			v_flt Variable_38; // * output 0
			Variable_38 = Variable_37 * BufferConstant.Variable_39;
			
			// *
			v_flt Variable_47; // * output 0
			Variable_47 = Variable_46 * BufferConstant.Variable_48 * Variable_52;
			
			// Min (float)
			v_flt Variable_21; // Min (float) output 0
			Variable_21 = FVoxelNodeFunctions::Min<v_flt>(Variable_20, v_flt(1.0f));
			
			// 1 - X
			v_flt Variable_75; // 1 - X output 0
			Variable_75 = 1 - Variable_68;
			
			// 1 - X
			v_flt Variable_55; // 1 - X output 0
			Variable_55 = 1 - Variable_52;
			
			// *
			v_flt Variable_76; // * output 0
			Variable_76 = Variable_75 * v_flt(20.0f);
			
			// /
			v_flt Variable_88; // / output 0
			Variable_88 = Variable_76 / BufferConstant.Variable_89;
			
			// Float Curve: RiverDepthCurve
			v_flt Variable_63; // Float Curve: RiverDepthCurve output 0
			Variable_63 = FVoxelNodeFunctions::GetCurveValue(Params.RiverDepthCurve, Variable_88);
			
			// Max (float)
			v_flt Variable_78; // Max (float) output 0
			Variable_78 = FVoxelNodeFunctions::Max<v_flt>(Variable_63, v_flt(0.0f));
			
			// Min (float)
			v_flt Variable_79; // Min (float) output 0
			Variable_79 = FVoxelNodeFunctions::Min<v_flt>(Variable_63, v_flt(0.0f));
			
			// *
			v_flt Variable_64; // * output 0
			Variable_64 = Variable_78 * v_flt(-1.0f) * BufferConstant.Variable_65;
			
			// *
			v_flt Variable_80; // * output 0
			Variable_80 = Variable_79 * v_flt(-1.0f);
			
			// 1 - X
			v_flt Variable_82; // 1 - X output 0
			Variable_82 = 1 - Variable_80;
			
			// *
			v_flt Variable_81; // * output 0
			Variable_81 = Variable_62 * Variable_80;
			
			// -
			v_flt Variable_83; // - output 0
			Variable_83 = Variable_81 - Variable_82;
			
			// *
			v_flt Variable_56; // * output 0
			Variable_56 = Variable_83 * BufferConstant.Variable_57 * Variable_55 * Variable_61;
			
			// +
			v_flt Variable_32; // + output 0
			Variable_32 = Variable_38 + Variable_41 + Variable_47 + Variable_54 + Variable_56 + Variable_64;
			
			// *
			v_flt Variable_87; // * output 0
			Variable_87 = Variable_32 * BufferConstant.Variable_84;
			
			// -
			v_flt Variable_31; // - output 0
			Variable_31 = BufferConstant.Variable_85 - Variable_87;
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_31 - Variable_29;
			
			// Max (float)
			v_flt Variable_8; // Max (float) output 0
			Variable_8 = FVoxelNodeFunctions::Max<v_flt>(Variable_4, Variable_7);
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_8 - v_flt(1.0f);
			
			// *
			v_flt Variable_16; // * output 0
			Variable_16 = Variable_19 * Variable_21;
			
			// +
			v_flt Variable_17; // + output 0
			Variable_17 = Variable_16 + v_flt(1.0f);
			
			Outputs.Value = Variable_17;
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
			
			v_flt Variable_15; // RingEdgesHardness = 10.0 output 0
			v_flt Variable_31; // MountainsNoiseFrequency = 0.2 output 0
			v_flt Variable_72; // RiverWidth = 1.0 output 0
			v_flt Variable_39; // PlainsNoiseFrequency = 0.2 output 0
			v_flt Variable_27; // / output 0
			v_flt Variable_117; // Lerp Colors.Break Color output 0
			v_flt Variable_118; // Lerp Colors.Break Color output 1
			v_flt Variable_119; // Lerp Colors.Break Color output 2
			v_flt Variable_120; // Lerp Colors.Break Color output 3
			v_flt Variable_95; // Lerp Colors.Break Color output 0
			v_flt Variable_96; // Lerp Colors.Break Color output 1
			v_flt Variable_97; // Lerp Colors.Break Color output 2
			v_flt Variable_98; // Lerp Colors.Break Color output 3
			v_flt Variable_91; // Lerp Colors.Break Color output 0
			v_flt Variable_92; // Lerp Colors.Break Color output 1
			v_flt Variable_93; // Lerp Colors.Break Color output 2
			v_flt Variable_94; // Lerp Colors.Break Color output 3
			v_flt Variable_147; // Lerp Colors.Break Color output 0
			v_flt Variable_148; // Lerp Colors.Break Color output 1
			v_flt Variable_149; // Lerp Colors.Break Color output 2
			v_flt Variable_150; // Lerp Colors.Break Color output 3
			v_flt Variable_169; // Lerp Colors.Break Color output 0
			v_flt Variable_170; // Lerp Colors.Break Color output 1
			v_flt Variable_171; // Lerp Colors.Break Color output 2
			v_flt Variable_172; // Lerp Colors.Break Color output 3
			v_flt Variable_121; // Lerp Colors.Break Color output 0
			v_flt Variable_122; // Lerp Colors.Break Color output 1
			v_flt Variable_123; // Lerp Colors.Break Color output 2
			v_flt Variable_124; // Lerp Colors.Break Color output 3
			v_flt Variable_130; // Lerp Colors.Break Color output 0
			v_flt Variable_131; // Lerp Colors.Break Color output 1
			v_flt Variable_132; // Lerp Colors.Break Color output 2
			v_flt Variable_133; // Lerp Colors.Break Color output 3
			v_flt Variable_134; // Lerp Colors.Break Color output 0
			v_flt Variable_135; // Lerp Colors.Break Color output 1
			v_flt Variable_136; // Lerp Colors.Break Color output 2
			v_flt Variable_137; // Lerp Colors.Break Color output 3
			v_flt Variable_26; // - output 0
			v_flt Variable_70; // / output 0
			v_flt Variable_75; // + output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_0; // X output 0
			v_flt Variable_17; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_1; // Y output 0
			v_flt Variable_30; // * output 0
			v_flt Variable_38; // * output 0
			v_flt Variable_56; // * output 0
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
				// MountainsColorLowHigh
				FColor Variable_36; // MountainsColorLowHigh output 0
				Variable_36 = Params.MountainsColorLowHigh;
				
				// RingEdgesHardness = 10.0
				BufferConstant.Variable_15 = Params.RingEdgesHardness;
				
				// Width in Degrees = 50.0
				v_flt Variable_23; // Width in Degrees = 50.0 output 0
				Variable_23 = Params.Width_in_Degrees;
				
				// RingOuterColor
				FColor Variable_80; // RingOuterColor output 0
				Variable_80 = Params.RingOuterColor;
				
				// PlainsColorHigh
				FColor Variable_40; // PlainsColorHigh output 0
				Variable_40 = Params.PlainsColorHigh;
				
				// MountainsNoiseFrequency = 0.2
				BufferConstant.Variable_31 = Params.MountainsNoiseFrequency;
				
				// MountainsColorHigh
				FColor Variable_33; // MountainsColorHigh output 0
				Variable_33 = Params.MountainsColorHigh;
				
				// MountainsColorLowLow
				FColor Variable_34; // MountainsColorLowLow output 0
				Variable_34 = Params.MountainsColorLowLow;
				
				// RiverWidth = 1.0
				BufferConstant.Variable_72 = Params.RiverWidth;
				
				// PlainsNoiseFrequency = 0.2
				BufferConstant.Variable_39 = Params.PlainsNoiseFrequency;
				
				// PlainsColorLow
				FColor Variable_43; // PlainsColorLow output 0
				Variable_43 = Params.PlainsColorLow;
				
				// RiverColor
				FColor Variable_47; // RiverColor output 0
				Variable_47 = Params.RiverColor;
				
				// BeachColor
				FColor Variable_66; // BeachColor output 0
				Variable_66 = Params.BeachColor;
				
				// Scale = 10.0
				v_flt Variable_68; // Scale = 10.0 output 0
				Variable_68 = Params.Scale;
				
				// Radius = 7000.0
				v_flt Variable_4; // Radius = 7000.0 output 0
				Variable_4 = Params.Radius;
				
				// Thickness = 500.0
				v_flt Variable_76; // Thickness = 500.0 output 0
				Variable_76 = Params.Thickness;
				
				// /
				BufferConstant.Variable_27 = Variable_23 / v_flt(360.0f);
				
				// *
				v_flt Variable_69; // * output 0
				Variable_69 = Variable_4 * Variable_68;
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_66, BufferConstant.Variable_117, BufferConstant.Variable_118, BufferConstant.Variable_119, BufferConstant.Variable_120);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_40, BufferConstant.Variable_95, BufferConstant.Variable_96, BufferConstant.Variable_97, BufferConstant.Variable_98);
				
				// *
				v_flt Variable_77; // * output 0
				Variable_77 = Variable_76 * Variable_68;
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_43, BufferConstant.Variable_91, BufferConstant.Variable_92, BufferConstant.Variable_93, BufferConstant.Variable_94);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_33, BufferConstant.Variable_147, BufferConstant.Variable_148, BufferConstant.Variable_149, BufferConstant.Variable_150);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_80, BufferConstant.Variable_169, BufferConstant.Variable_170, BufferConstant.Variable_171, BufferConstant.Variable_172);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_47, BufferConstant.Variable_121, BufferConstant.Variable_122, BufferConstant.Variable_123, BufferConstant.Variable_124);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_34, BufferConstant.Variable_130, BufferConstant.Variable_131, BufferConstant.Variable_132, BufferConstant.Variable_133);
				
				// Lerp Colors.Break Color
				FVoxelNodeFunctions::BreakColorFloat(Variable_36, BufferConstant.Variable_134, BufferConstant.Variable_135, BufferConstant.Variable_136, BufferConstant.Variable_137);
				
				// -
				BufferConstant.Variable_26 = v_flt(0.5f) - BufferConstant.Variable_27;
				
				// /
				BufferConstant.Variable_70 = Variable_69 / Variable_68;
				
				// +
				BufferConstant.Variable_75 = Variable_69 + Variable_77 + v_flt(-5.0f);
				
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
		
		FVoxelFastNoise _2D_Simplex_Noise_2_Noise;
		FVoxelFastNoise _2D_Perlin_Noise_1_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_3_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_Fractal_2_Noise;
		TStaticArray<uint8, 32> _2D_Simplex_Noise_Fractal_2_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 2D Simplex Noise
			_2D_Simplex_Noise_2_Noise.SetSeed(FVoxelGraphSeed(1000));
			_2D_Simplex_Noise_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Perlin Noise
			_2D_Perlin_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1338));
			_2D_Perlin_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Simplex Noise
			_2D_Simplex_Noise_3_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Simplex_Noise_3_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 2D Simplex Noise Fractal
			_2D_Simplex_Noise_Fractal_2_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Simplex_Noise_Fractal_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Simplex_Noise_Fractal_2_Noise.SetFractalOctavesAndGain(8, 0.5);
			_2D_Simplex_Noise_Fractal_2_Noise.SetFractalLacunarity(2.0);
			_2D_Simplex_Noise_Fractal_2_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[0] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[1] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[2] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[3] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[4] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[5] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[6] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[7] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[8] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[9] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[10] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[11] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[12] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[13] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[14] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[15] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[16] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[17] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[18] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[19] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[20] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[21] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[22] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[23] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[24] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[25] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[26] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[27] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[28] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[29] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[30] = 8;
			_2D_Simplex_Noise_Fractal_2_LODToOctaves[31] = 8;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_17 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			v_flt Variable_21; // Y output 0
			Variable_21 = Context.GetLocalY();
			
			// Atan2
			v_flt Variable_16; // Atan2 output 0
			Variable_16 = FVoxelNodeFunctions::Atan2(Variable_21, BufferX.Variable_17);
			
			// /
			v_flt Variable_18; // / output 0
			Variable_18 = Variable_16 / v_flt(3.141593f);
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = BufferConstant.Variable_70 * Variable_18;
			
			// *
			BufferXY.Variable_30 = Variable_20 * BufferConstant.Variable_31;
			
			// 2D Simplex Noise
			v_flt Variable_59; // 2D Simplex Noise output 0
			Variable_59 = _2D_Simplex_Noise_2_Noise.GetSimplex_2D(Variable_20, Variable_20, v_flt(0.001f));
			Variable_59 = FMath::Clamp<v_flt>(Variable_59, -0.997888, 0.997883);
			
			// *
			BufferXY.Variable_38 = Variable_20 * BufferConstant.Variable_39;
			
			// *
			BufferXY.Variable_56 = Variable_59 * v_flt(0.1f);
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// X
			BufferX.Variable_17 = Context.GetLocalX();
			
			// Y
			v_flt Variable_21; // Y output 0
			Variable_21 = Context.GetLocalY();
			
			// Atan2
			v_flt Variable_16; // Atan2 output 0
			Variable_16 = FVoxelNodeFunctions::Atan2(Variable_21, BufferX.Variable_17);
			
			// /
			v_flt Variable_18; // / output 0
			Variable_18 = Variable_16 / v_flt(3.141593f);
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = BufferConstant.Variable_70 * Variable_18;
			
			// *
			BufferXY.Variable_30 = Variable_20 * BufferConstant.Variable_31;
			
			// 2D Simplex Noise
			v_flt Variable_59; // 2D Simplex Noise output 0
			Variable_59 = _2D_Simplex_Noise_2_Noise.GetSimplex_2D(Variable_20, Variable_20, v_flt(0.001f));
			Variable_59 = FMath::Clamp<v_flt>(Variable_59, -0.997888, 0.997883);
			
			// *
			BufferXY.Variable_38 = Variable_20 * BufferConstant.Variable_39;
			
			// *
			BufferXY.Variable_56 = Variable_59 * v_flt(0.1f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_52; // Z output 0
			Variable_52 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferXY.Variable_1, Variable_2);
			
			// +
			v_flt Variable_22; // + output 0
			Variable_22 = Variable_3 + v_flt(0.001f);
			
			// /
			v_flt Variable_9; // / output 0
			Variable_9 = Variable_8 / Variable_22;
			
			// -
			v_flt Variable_79; // - output 0
			Variable_79 = BufferConstant.Variable_75 - Variable_22;
			
			// /
			v_flt Variable_53; // / output 0
			Variable_53 = Variable_52 / Variable_22;
			
			// Clamp
			v_flt Variable_73; // Clamp output 0
			Variable_73 = FVoxelNodeFunctions::Clamp(Variable_79, v_flt(0.0f), v_flt(1.0f));
			
			// ACOS
			v_flt Variable_10; // ACOS output 0
			Variable_10 = FVoxelNodeFunctions::Acos(Variable_9);
			
			// /
			v_flt Variable_6; // / output 0
			Variable_6 = Variable_10 / v_flt(3.141593f);
			
			// +
			v_flt Variable_55; // + output 0
			Variable_55 = Variable_53 + BufferXY.Variable_56;
			
			// *
			v_flt Variable_19; // * output 0
			Variable_19 = BufferConstant.Variable_70 * Variable_6;
			
			// 1 - X
			v_flt Variable_7; // 1 - X output 0
			Variable_7 = 1 - Variable_6;
			
			// *
			v_flt Variable_29; // * output 0
			Variable_29 = Variable_19 * BufferConstant.Variable_31;
			
			// *
			v_flt Variable_37; // * output 0
			Variable_37 = Variable_19 * BufferConstant.Variable_39;
			
			// ACOS
			v_flt Variable_54; // ACOS output 0
			Variable_54 = FVoxelNodeFunctions::Acos(Variable_55);
			
			// Min (float)
			v_flt Variable_5; // Min (float) output 0
			Variable_5 = FVoxelNodeFunctions::Min<v_flt>(Variable_6, Variable_7);
			
			// 2D Perlin Noise
			v_flt Variable_35; // 2D Perlin Noise output 0
			Variable_35 = _2D_Perlin_Noise_1_Noise.GetPerlin_2D(Variable_29, BufferXY.Variable_30, v_flt(0.1f));
			Variable_35 = FMath::Clamp<v_flt>(Variable_35, -0.888317, 0.811183);
			
			// 2D Simplex Noise
			v_flt Variable_41; // 2D Simplex Noise output 0
			Variable_41 = _2D_Simplex_Noise_3_Noise.GetSimplex_2D(Variable_37, BufferXY.Variable_38, v_flt(0.02f));
			Variable_41 = FMath::Clamp<v_flt>(Variable_41, -0.997652, 0.996970);
			
			// /
			v_flt Variable_51; // / output 0
			Variable_51 = Variable_54 / v_flt(3.141593f);
			
			// -
			v_flt Variable_11; // - output 0
			Variable_11 = Variable_5 - BufferConstant.Variable_26;
			
			// 2D Simplex Noise Fractal
			v_flt Variable_28; // 2D Simplex Noise Fractal output 0
			Variable_28 = _2D_Simplex_Noise_Fractal_2_Noise.GetSimplexFractal_2D(Variable_29, BufferXY.Variable_30, v_flt(0.02f), _2D_Simplex_Noise_Fractal_2_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_28 = FMath::Clamp<v_flt>(Variable_28, -0.780446, 0.760988);
			
			// Lerp Colors.Clamp
			v_flt Variable_84; // Lerp Colors.Clamp output 0
			Variable_84 = FVoxelNodeFunctions::Clamp(Variable_28, v_flt(0.0f), v_flt(1.0f));
			
			// /
			v_flt Variable_25; // / output 0
			Variable_25 = Variable_11 / BufferConstant.Variable_27;
			
			// Lerp Colors.Clamp
			v_flt Variable_83; // Lerp Colors.Clamp output 0
			Variable_83 = FVoxelNodeFunctions::Clamp(Variable_35, v_flt(0.0f), v_flt(1.0f));
			
			// 1 - X
			v_flt Variable_49; // 1 - X output 0
			Variable_49 = 1 - Variable_51;
			
			// Min (float)
			v_flt Variable_48; // Min (float) output 0
			Variable_48 = FVoxelNodeFunctions::Min<v_flt>(Variable_51, Variable_49);
			
			// 1 - X
			v_flt Variable_24; // 1 - X output 0
			Variable_24 = 1 - Variable_25;
			
			// Lerp Colors.Lerp
			v_flt Variable_126; // Lerp Colors.Lerp output 0
			Variable_126 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_130, BufferConstant.Variable_134, Variable_83);
			
			// Lerp Colors.Lerp
			v_flt Variable_128; // Lerp Colors.Lerp output 0
			Variable_128 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_132, BufferConstant.Variable_136, Variable_83);
			
			// Lerp Colors.Lerp
			v_flt Variable_127; // Lerp Colors.Lerp output 0
			Variable_127 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_131, BufferConstant.Variable_135, Variable_83);
			
			// Max (float)
			v_flt Variable_12; // Max (float) output 0
			Variable_12 = FVoxelNodeFunctions::Max<v_flt>(Variable_25, v_flt(0.0f));
			
			// Lerp Colors.Lerp
			v_flt Variable_129; // Lerp Colors.Lerp output 0
			Variable_129 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_133, BufferConstant.Variable_137, Variable_83);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = Variable_12 * BufferConstant.Variable_15;
			
			// /
			v_flt Variable_50; // / output 0
			Variable_50 = Variable_48 / v_flt(0.5f);
			
			// Lerp Colors.Make Color
			FColor Variable_138; // Lerp Colors.Make Color output 0
			Variable_138 = FVoxelNodeFunctions::MakeColorFloat(Variable_126, Variable_127, Variable_128, Variable_129);
			
			// Float Curve: MoutainsMaskCurve
			v_flt Variable_32; // Float Curve: MoutainsMaskCurve output 0
			Variable_32 = FVoxelNodeFunctions::GetCurveValue(Params.MoutainsMaskCurve, Variable_24);
			
			// Min (float)
			v_flt Variable_14; // Min (float) output 0
			Variable_14 = FVoxelNodeFunctions::Min<v_flt>(Variable_13, v_flt(1.0f));
			
			// *
			v_flt Variable_42; // * output 0
			Variable_42 = Variable_32 * v_flt(3.0f);
			
			// 1 - X
			v_flt Variable_57; // 1 - X output 0
			Variable_57 = 1 - Variable_50;
			
			// Lerp Colors.Break Color
			v_flt Variable_143; // Lerp Colors.Break Color output 0
			v_flt Variable_144; // Lerp Colors.Break Color output 1
			v_flt Variable_145; // Lerp Colors.Break Color output 2
			v_flt Variable_146; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_138, Variable_143, Variable_144, Variable_145, Variable_146);
			
			// *
			v_flt Variable_58; // * output 0
			Variable_58 = Variable_57 * v_flt(20.0f);
			
			// Lerp Colors.Lerp
			v_flt Variable_141; // Lerp Colors.Lerp output 0
			Variable_141 = FVoxelNodeFunctions::Lerp(Variable_145, BufferConstant.Variable_149, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_142; // Lerp Colors.Lerp output 0
			Variable_142 = FVoxelNodeFunctions::Lerp(Variable_146, BufferConstant.Variable_150, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_140; // Lerp Colors.Lerp output 0
			Variable_140 = FVoxelNodeFunctions::Lerp(Variable_144, BufferConstant.Variable_148, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_139; // Lerp Colors.Lerp output 0
			Variable_139 = FVoxelNodeFunctions::Lerp(Variable_143, BufferConstant.Variable_147, Variable_84);
			
			// Min (float)
			v_flt Variable_74; // Min (float) output 0
			Variable_74 = FVoxelNodeFunctions::Min<v_flt>(Variable_14, Variable_73);
			
			// Lerp Colors.Make Color
			FColor Variable_151; // Lerp Colors.Make Color output 0
			Variable_151 = FVoxelNodeFunctions::MakeColorFloat(Variable_139, Variable_140, Variable_141, Variable_142);
			
			// Lerp Colors.Clamp
			v_flt Variable_86; // Lerp Colors.Clamp output 0
			Variable_86 = FVoxelNodeFunctions::Clamp(Variable_74, v_flt(0.0f), v_flt(1.0f));
			
			// /
			v_flt Variable_71; // / output 0
			Variable_71 = Variable_58 / BufferConstant.Variable_72;
			
			// Lerp Colors.Break Color
			v_flt Variable_160; // Lerp Colors.Break Color output 0
			v_flt Variable_161; // Lerp Colors.Break Color output 1
			v_flt Variable_162; // Lerp Colors.Break Color output 2
			v_flt Variable_163; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_151, Variable_160, Variable_161, Variable_162, Variable_163);
			
			// Float Curve: RiverDepthCurve
			v_flt Variable_46; // Float Curve: RiverDepthCurve output 0
			Variable_46 = FVoxelNodeFunctions::GetCurveValue(Params.RiverDepthCurve, Variable_71);
			
			// Min (float)
			v_flt Variable_60; // Min (float) output 0
			Variable_60 = FVoxelNodeFunctions::Min<v_flt>(Variable_46, v_flt(0.0f));
			
			// +
			v_flt Variable_65; // + output 0
			Variable_65 = Variable_46 + v_flt(0.3f);
			
			// *
			v_flt Variable_67; // * output 0
			Variable_67 = Variable_46 * v_flt(10.0f);
			
			// Lerp Colors.Clamp
			v_flt Variable_82; // Lerp Colors.Clamp output 0
			Variable_82 = FVoxelNodeFunctions::Clamp(Variable_67, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Clamp
			v_flt Variable_81; // Lerp Colors.Clamp output 0
			Variable_81 = FVoxelNodeFunctions::Clamp(Variable_65, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_61; // * output 0
			Variable_61 = Variable_60 * v_flt(-1.0f);
			
			// Lerp Colors.Lerp
			v_flt Variable_113; // Lerp Colors.Lerp output 0
			Variable_113 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_117, BufferConstant.Variable_121, Variable_82);
			
			// Lerp Colors.Lerp
			v_flt Variable_115; // Lerp Colors.Lerp output 0
			Variable_115 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_119, BufferConstant.Variable_123, Variable_82);
			
			// *
			v_flt Variable_62; // * output 0
			Variable_62 = Variable_41 * Variable_61;
			
			// Lerp Colors.Lerp
			v_flt Variable_116; // Lerp Colors.Lerp output 0
			Variable_116 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_120, BufferConstant.Variable_124, Variable_82);
			
			// Lerp Colors.Lerp
			v_flt Variable_114; // Lerp Colors.Lerp output 0
			Variable_114 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_118, BufferConstant.Variable_122, Variable_82);
			
			// 1 - X
			v_flt Variable_63; // 1 - X output 0
			Variable_63 = 1 - Variable_61;
			
			// -
			v_flt Variable_64; // - output 0
			Variable_64 = Variable_62 - Variable_63;
			
			// Lerp Colors.Make Color
			FColor Variable_125; // Lerp Colors.Make Color output 0
			Variable_125 = FVoxelNodeFunctions::MakeColorFloat(Variable_113, Variable_114, Variable_115, Variable_116);
			
			// Max (float)
			v_flt Variable_44; // Max (float) output 0
			Variable_44 = FVoxelNodeFunctions::Max<v_flt>(Variable_64, v_flt(0.0f));
			
			// Lerp Colors.Clamp
			v_flt Variable_78; // Lerp Colors.Clamp output 0
			Variable_78 = FVoxelNodeFunctions::Clamp(Variable_64, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Break Color
			v_flt Variable_108; // Lerp Colors.Break Color output 0
			v_flt Variable_109; // Lerp Colors.Break Color output 1
			v_flt Variable_110; // Lerp Colors.Break Color output 2
			v_flt Variable_111; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_125, Variable_108, Variable_109, Variable_110, Variable_111);
			
			// -
			v_flt Variable_45; // - output 0
			Variable_45 = Variable_42 - Variable_44;
			
			// Lerp Colors.Lerp
			v_flt Variable_88; // Lerp Colors.Lerp output 0
			Variable_88 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_92, BufferConstant.Variable_96, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_89; // Lerp Colors.Lerp output 0
			Variable_89 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_93, BufferConstant.Variable_97, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_90; // Lerp Colors.Lerp output 0
			Variable_90 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_94, BufferConstant.Variable_98, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_87; // Lerp Colors.Lerp output 0
			Variable_87 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_91, BufferConstant.Variable_95, Variable_78);
			
			// Lerp Colors.Make Color
			FColor Variable_99; // Lerp Colors.Make Color output 0
			Variable_99 = FVoxelNodeFunctions::MakeColorFloat(Variable_87, Variable_88, Variable_89, Variable_90);
			
			// Lerp Colors.Clamp
			v_flt Variable_85; // Lerp Colors.Clamp output 0
			Variable_85 = FVoxelNodeFunctions::Clamp(Variable_45, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Break Color
			v_flt Variable_104; // Lerp Colors.Break Color output 0
			v_flt Variable_105; // Lerp Colors.Break Color output 1
			v_flt Variable_106; // Lerp Colors.Break Color output 2
			v_flt Variable_107; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_99, Variable_104, Variable_105, Variable_106, Variable_107);
			
			// Lerp Colors.Lerp
			v_flt Variable_101; // Lerp Colors.Lerp output 0
			Variable_101 = FVoxelNodeFunctions::Lerp(Variable_105, Variable_109, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_103; // Lerp Colors.Lerp output 0
			Variable_103 = FVoxelNodeFunctions::Lerp(Variable_107, Variable_111, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_102; // Lerp Colors.Lerp output 0
			Variable_102 = FVoxelNodeFunctions::Lerp(Variable_106, Variable_110, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_100; // Lerp Colors.Lerp output 0
			Variable_100 = FVoxelNodeFunctions::Lerp(Variable_104, Variable_108, Variable_81);
			
			// Lerp Colors.Make Color
			FColor Variable_112; // Lerp Colors.Make Color output 0
			Variable_112 = FVoxelNodeFunctions::MakeColorFloat(Variable_100, Variable_101, Variable_102, Variable_103);
			
			// Lerp Colors.Break Color
			v_flt Variable_156; // Lerp Colors.Break Color output 0
			v_flt Variable_157; // Lerp Colors.Break Color output 1
			v_flt Variable_158; // Lerp Colors.Break Color output 2
			v_flt Variable_159; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_112, Variable_156, Variable_157, Variable_158, Variable_159);
			
			// Lerp Colors.Lerp
			v_flt Variable_154; // Lerp Colors.Lerp output 0
			Variable_154 = FVoxelNodeFunctions::Lerp(Variable_158, Variable_162, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_153; // Lerp Colors.Lerp output 0
			Variable_153 = FVoxelNodeFunctions::Lerp(Variable_157, Variable_161, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_155; // Lerp Colors.Lerp output 0
			Variable_155 = FVoxelNodeFunctions::Lerp(Variable_159, Variable_163, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_152; // Lerp Colors.Lerp output 0
			Variable_152 = FVoxelNodeFunctions::Lerp(Variable_156, Variable_160, Variable_85);
			
			// Lerp Colors.Make Color
			FColor Variable_164; // Lerp Colors.Make Color output 0
			Variable_164 = FVoxelNodeFunctions::MakeColorFloat(Variable_152, Variable_153, Variable_154, Variable_155);
			
			// Lerp Colors.Break Color
			v_flt Variable_173; // Lerp Colors.Break Color output 0
			v_flt Variable_174; // Lerp Colors.Break Color output 1
			v_flt Variable_175; // Lerp Colors.Break Color output 2
			v_flt Variable_176; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_164, Variable_173, Variable_174, Variable_175, Variable_176);
			
			// Lerp Colors.Lerp
			v_flt Variable_165; // Lerp Colors.Lerp output 0
			Variable_165 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_169, Variable_173, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_166; // Lerp Colors.Lerp output 0
			Variable_166 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_170, Variable_174, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_167; // Lerp Colors.Lerp output 0
			Variable_167 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_171, Variable_175, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_168; // Lerp Colors.Lerp output 0
			Variable_168 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_172, Variable_176, Variable_86);
			
			// Lerp Colors.Make Color
			FColor Variable_177; // Lerp Colors.Make Color output 0
			Variable_177 = FVoxelNodeFunctions::MakeColorFloat(Variable_165, Variable_166, Variable_167, Variable_168);
			
			Outputs.MaterialBuilder.SetColor(Variable_177);
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_52; // Z output 0
			Variable_52 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// X
			v_flt Variable_17; // X output 0
			Variable_17 = Context.GetLocalX();
			
			// Y
			v_flt Variable_21; // Y output 0
			Variable_21 = Context.GetLocalY();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Atan2
			v_flt Variable_16; // Atan2 output 0
			Variable_16 = FVoxelNodeFunctions::Atan2(Variable_21, Variable_17);
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// /
			v_flt Variable_18; // / output 0
			Variable_18 = Variable_16 / v_flt(3.141593f);
			
			// +
			v_flt Variable_22; // + output 0
			Variable_22 = Variable_3 + v_flt(0.001f);
			
			// /
			v_flt Variable_9; // / output 0
			Variable_9 = Variable_8 / Variable_22;
			
			// -
			v_flt Variable_79; // - output 0
			Variable_79 = BufferConstant.Variable_75 - Variable_22;
			
			// /
			v_flt Variable_53; // / output 0
			Variable_53 = Variable_52 / Variable_22;
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = BufferConstant.Variable_70 * Variable_18;
			
			// Clamp
			v_flt Variable_73; // Clamp output 0
			Variable_73 = FVoxelNodeFunctions::Clamp(Variable_79, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_30; // * output 0
			Variable_30 = Variable_20 * BufferConstant.Variable_31;
			
			// 2D Simplex Noise
			v_flt Variable_59; // 2D Simplex Noise output 0
			Variable_59 = _2D_Simplex_Noise_2_Noise.GetSimplex_2D(Variable_20, Variable_20, v_flt(0.001f));
			Variable_59 = FMath::Clamp<v_flt>(Variable_59, -0.997888, 0.997883);
			
			// ACOS
			v_flt Variable_10; // ACOS output 0
			Variable_10 = FVoxelNodeFunctions::Acos(Variable_9);
			
			// *
			v_flt Variable_38; // * output 0
			Variable_38 = Variable_20 * BufferConstant.Variable_39;
			
			// *
			v_flt Variable_56; // * output 0
			Variable_56 = Variable_59 * v_flt(0.1f);
			
			// /
			v_flt Variable_6; // / output 0
			Variable_6 = Variable_10 / v_flt(3.141593f);
			
			// +
			v_flt Variable_55; // + output 0
			Variable_55 = Variable_53 + Variable_56;
			
			// *
			v_flt Variable_19; // * output 0
			Variable_19 = BufferConstant.Variable_70 * Variable_6;
			
			// 1 - X
			v_flt Variable_7; // 1 - X output 0
			Variable_7 = 1 - Variable_6;
			
			// *
			v_flt Variable_29; // * output 0
			Variable_29 = Variable_19 * BufferConstant.Variable_31;
			
			// *
			v_flt Variable_37; // * output 0
			Variable_37 = Variable_19 * BufferConstant.Variable_39;
			
			// ACOS
			v_flt Variable_54; // ACOS output 0
			Variable_54 = FVoxelNodeFunctions::Acos(Variable_55);
			
			// Min (float)
			v_flt Variable_5; // Min (float) output 0
			Variable_5 = FVoxelNodeFunctions::Min<v_flt>(Variable_6, Variable_7);
			
			// 2D Perlin Noise
			v_flt Variable_35; // 2D Perlin Noise output 0
			Variable_35 = _2D_Perlin_Noise_1_Noise.GetPerlin_2D(Variable_29, Variable_30, v_flt(0.1f));
			Variable_35 = FMath::Clamp<v_flt>(Variable_35, -0.888317, 0.811183);
			
			// 2D Simplex Noise
			v_flt Variable_41; // 2D Simplex Noise output 0
			Variable_41 = _2D_Simplex_Noise_3_Noise.GetSimplex_2D(Variable_37, Variable_38, v_flt(0.02f));
			Variable_41 = FMath::Clamp<v_flt>(Variable_41, -0.997652, 0.996970);
			
			// /
			v_flt Variable_51; // / output 0
			Variable_51 = Variable_54 / v_flt(3.141593f);
			
			// -
			v_flt Variable_11; // - output 0
			Variable_11 = Variable_5 - BufferConstant.Variable_26;
			
			// 2D Simplex Noise Fractal
			v_flt Variable_28; // 2D Simplex Noise Fractal output 0
			Variable_28 = _2D_Simplex_Noise_Fractal_2_Noise.GetSimplexFractal_2D(Variable_29, Variable_30, v_flt(0.02f), _2D_Simplex_Noise_Fractal_2_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_28 = FMath::Clamp<v_flt>(Variable_28, -0.780446, 0.760988);
			
			// Lerp Colors.Clamp
			v_flt Variable_84; // Lerp Colors.Clamp output 0
			Variable_84 = FVoxelNodeFunctions::Clamp(Variable_28, v_flt(0.0f), v_flt(1.0f));
			
			// /
			v_flt Variable_25; // / output 0
			Variable_25 = Variable_11 / BufferConstant.Variable_27;
			
			// Lerp Colors.Clamp
			v_flt Variable_83; // Lerp Colors.Clamp output 0
			Variable_83 = FVoxelNodeFunctions::Clamp(Variable_35, v_flt(0.0f), v_flt(1.0f));
			
			// 1 - X
			v_flt Variable_49; // 1 - X output 0
			Variable_49 = 1 - Variable_51;
			
			// Min (float)
			v_flt Variable_48; // Min (float) output 0
			Variable_48 = FVoxelNodeFunctions::Min<v_flt>(Variable_51, Variable_49);
			
			// 1 - X
			v_flt Variable_24; // 1 - X output 0
			Variable_24 = 1 - Variable_25;
			
			// Lerp Colors.Lerp
			v_flt Variable_126; // Lerp Colors.Lerp output 0
			Variable_126 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_130, BufferConstant.Variable_134, Variable_83);
			
			// Lerp Colors.Lerp
			v_flt Variable_128; // Lerp Colors.Lerp output 0
			Variable_128 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_132, BufferConstant.Variable_136, Variable_83);
			
			// Lerp Colors.Lerp
			v_flt Variable_127; // Lerp Colors.Lerp output 0
			Variable_127 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_131, BufferConstant.Variable_135, Variable_83);
			
			// Max (float)
			v_flt Variable_12; // Max (float) output 0
			Variable_12 = FVoxelNodeFunctions::Max<v_flt>(Variable_25, v_flt(0.0f));
			
			// Lerp Colors.Lerp
			v_flt Variable_129; // Lerp Colors.Lerp output 0
			Variable_129 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_133, BufferConstant.Variable_137, Variable_83);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = Variable_12 * BufferConstant.Variable_15;
			
			// /
			v_flt Variable_50; // / output 0
			Variable_50 = Variable_48 / v_flt(0.5f);
			
			// Lerp Colors.Make Color
			FColor Variable_138; // Lerp Colors.Make Color output 0
			Variable_138 = FVoxelNodeFunctions::MakeColorFloat(Variable_126, Variable_127, Variable_128, Variable_129);
			
			// Float Curve: MoutainsMaskCurve
			v_flt Variable_32; // Float Curve: MoutainsMaskCurve output 0
			Variable_32 = FVoxelNodeFunctions::GetCurveValue(Params.MoutainsMaskCurve, Variable_24);
			
			// Min (float)
			v_flt Variable_14; // Min (float) output 0
			Variable_14 = FVoxelNodeFunctions::Min<v_flt>(Variable_13, v_flt(1.0f));
			
			// *
			v_flt Variable_42; // * output 0
			Variable_42 = Variable_32 * v_flt(3.0f);
			
			// 1 - X
			v_flt Variable_57; // 1 - X output 0
			Variable_57 = 1 - Variable_50;
			
			// Lerp Colors.Break Color
			v_flt Variable_143; // Lerp Colors.Break Color output 0
			v_flt Variable_144; // Lerp Colors.Break Color output 1
			v_flt Variable_145; // Lerp Colors.Break Color output 2
			v_flt Variable_146; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_138, Variable_143, Variable_144, Variable_145, Variable_146);
			
			// *
			v_flt Variable_58; // * output 0
			Variable_58 = Variable_57 * v_flt(20.0f);
			
			// Lerp Colors.Lerp
			v_flt Variable_141; // Lerp Colors.Lerp output 0
			Variable_141 = FVoxelNodeFunctions::Lerp(Variable_145, BufferConstant.Variable_149, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_142; // Lerp Colors.Lerp output 0
			Variable_142 = FVoxelNodeFunctions::Lerp(Variable_146, BufferConstant.Variable_150, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_140; // Lerp Colors.Lerp output 0
			Variable_140 = FVoxelNodeFunctions::Lerp(Variable_144, BufferConstant.Variable_148, Variable_84);
			
			// Lerp Colors.Lerp
			v_flt Variable_139; // Lerp Colors.Lerp output 0
			Variable_139 = FVoxelNodeFunctions::Lerp(Variable_143, BufferConstant.Variable_147, Variable_84);
			
			// Min (float)
			v_flt Variable_74; // Min (float) output 0
			Variable_74 = FVoxelNodeFunctions::Min<v_flt>(Variable_14, Variable_73);
			
			// Lerp Colors.Make Color
			FColor Variable_151; // Lerp Colors.Make Color output 0
			Variable_151 = FVoxelNodeFunctions::MakeColorFloat(Variable_139, Variable_140, Variable_141, Variable_142);
			
			// Lerp Colors.Clamp
			v_flt Variable_86; // Lerp Colors.Clamp output 0
			Variable_86 = FVoxelNodeFunctions::Clamp(Variable_74, v_flt(0.0f), v_flt(1.0f));
			
			// /
			v_flt Variable_71; // / output 0
			Variable_71 = Variable_58 / BufferConstant.Variable_72;
			
			// Lerp Colors.Break Color
			v_flt Variable_160; // Lerp Colors.Break Color output 0
			v_flt Variable_161; // Lerp Colors.Break Color output 1
			v_flt Variable_162; // Lerp Colors.Break Color output 2
			v_flt Variable_163; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_151, Variable_160, Variable_161, Variable_162, Variable_163);
			
			// Float Curve: RiverDepthCurve
			v_flt Variable_46; // Float Curve: RiverDepthCurve output 0
			Variable_46 = FVoxelNodeFunctions::GetCurveValue(Params.RiverDepthCurve, Variable_71);
			
			// Min (float)
			v_flt Variable_60; // Min (float) output 0
			Variable_60 = FVoxelNodeFunctions::Min<v_flt>(Variable_46, v_flt(0.0f));
			
			// +
			v_flt Variable_65; // + output 0
			Variable_65 = Variable_46 + v_flt(0.3f);
			
			// *
			v_flt Variable_67; // * output 0
			Variable_67 = Variable_46 * v_flt(10.0f);
			
			// Lerp Colors.Clamp
			v_flt Variable_82; // Lerp Colors.Clamp output 0
			Variable_82 = FVoxelNodeFunctions::Clamp(Variable_67, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Clamp
			v_flt Variable_81; // Lerp Colors.Clamp output 0
			Variable_81 = FVoxelNodeFunctions::Clamp(Variable_65, v_flt(0.0f), v_flt(1.0f));
			
			// *
			v_flt Variable_61; // * output 0
			Variable_61 = Variable_60 * v_flt(-1.0f);
			
			// Lerp Colors.Lerp
			v_flt Variable_113; // Lerp Colors.Lerp output 0
			Variable_113 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_117, BufferConstant.Variable_121, Variable_82);
			
			// Lerp Colors.Lerp
			v_flt Variable_115; // Lerp Colors.Lerp output 0
			Variable_115 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_119, BufferConstant.Variable_123, Variable_82);
			
			// *
			v_flt Variable_62; // * output 0
			Variable_62 = Variable_41 * Variable_61;
			
			// Lerp Colors.Lerp
			v_flt Variable_116; // Lerp Colors.Lerp output 0
			Variable_116 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_120, BufferConstant.Variable_124, Variable_82);
			
			// Lerp Colors.Lerp
			v_flt Variable_114; // Lerp Colors.Lerp output 0
			Variable_114 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_118, BufferConstant.Variable_122, Variable_82);
			
			// 1 - X
			v_flt Variable_63; // 1 - X output 0
			Variable_63 = 1 - Variable_61;
			
			// -
			v_flt Variable_64; // - output 0
			Variable_64 = Variable_62 - Variable_63;
			
			// Lerp Colors.Make Color
			FColor Variable_125; // Lerp Colors.Make Color output 0
			Variable_125 = FVoxelNodeFunctions::MakeColorFloat(Variable_113, Variable_114, Variable_115, Variable_116);
			
			// Max (float)
			v_flt Variable_44; // Max (float) output 0
			Variable_44 = FVoxelNodeFunctions::Max<v_flt>(Variable_64, v_flt(0.0f));
			
			// Lerp Colors.Clamp
			v_flt Variable_78; // Lerp Colors.Clamp output 0
			Variable_78 = FVoxelNodeFunctions::Clamp(Variable_64, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Break Color
			v_flt Variable_108; // Lerp Colors.Break Color output 0
			v_flt Variable_109; // Lerp Colors.Break Color output 1
			v_flt Variable_110; // Lerp Colors.Break Color output 2
			v_flt Variable_111; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_125, Variable_108, Variable_109, Variable_110, Variable_111);
			
			// -
			v_flt Variable_45; // - output 0
			Variable_45 = Variable_42 - Variable_44;
			
			// Lerp Colors.Lerp
			v_flt Variable_88; // Lerp Colors.Lerp output 0
			Variable_88 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_92, BufferConstant.Variable_96, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_89; // Lerp Colors.Lerp output 0
			Variable_89 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_93, BufferConstant.Variable_97, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_90; // Lerp Colors.Lerp output 0
			Variable_90 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_94, BufferConstant.Variable_98, Variable_78);
			
			// Lerp Colors.Lerp
			v_flt Variable_87; // Lerp Colors.Lerp output 0
			Variable_87 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_91, BufferConstant.Variable_95, Variable_78);
			
			// Lerp Colors.Make Color
			FColor Variable_99; // Lerp Colors.Make Color output 0
			Variable_99 = FVoxelNodeFunctions::MakeColorFloat(Variable_87, Variable_88, Variable_89, Variable_90);
			
			// Lerp Colors.Clamp
			v_flt Variable_85; // Lerp Colors.Clamp output 0
			Variable_85 = FVoxelNodeFunctions::Clamp(Variable_45, v_flt(0.0f), v_flt(1.0f));
			
			// Lerp Colors.Break Color
			v_flt Variable_104; // Lerp Colors.Break Color output 0
			v_flt Variable_105; // Lerp Colors.Break Color output 1
			v_flt Variable_106; // Lerp Colors.Break Color output 2
			v_flt Variable_107; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_99, Variable_104, Variable_105, Variable_106, Variable_107);
			
			// Lerp Colors.Lerp
			v_flt Variable_101; // Lerp Colors.Lerp output 0
			Variable_101 = FVoxelNodeFunctions::Lerp(Variable_105, Variable_109, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_103; // Lerp Colors.Lerp output 0
			Variable_103 = FVoxelNodeFunctions::Lerp(Variable_107, Variable_111, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_102; // Lerp Colors.Lerp output 0
			Variable_102 = FVoxelNodeFunctions::Lerp(Variable_106, Variable_110, Variable_81);
			
			// Lerp Colors.Lerp
			v_flt Variable_100; // Lerp Colors.Lerp output 0
			Variable_100 = FVoxelNodeFunctions::Lerp(Variable_104, Variable_108, Variable_81);
			
			// Lerp Colors.Make Color
			FColor Variable_112; // Lerp Colors.Make Color output 0
			Variable_112 = FVoxelNodeFunctions::MakeColorFloat(Variable_100, Variable_101, Variable_102, Variable_103);
			
			// Lerp Colors.Break Color
			v_flt Variable_156; // Lerp Colors.Break Color output 0
			v_flt Variable_157; // Lerp Colors.Break Color output 1
			v_flt Variable_158; // Lerp Colors.Break Color output 2
			v_flt Variable_159; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_112, Variable_156, Variable_157, Variable_158, Variable_159);
			
			// Lerp Colors.Lerp
			v_flt Variable_154; // Lerp Colors.Lerp output 0
			Variable_154 = FVoxelNodeFunctions::Lerp(Variable_158, Variable_162, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_153; // Lerp Colors.Lerp output 0
			Variable_153 = FVoxelNodeFunctions::Lerp(Variable_157, Variable_161, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_155; // Lerp Colors.Lerp output 0
			Variable_155 = FVoxelNodeFunctions::Lerp(Variable_159, Variable_163, Variable_85);
			
			// Lerp Colors.Lerp
			v_flt Variable_152; // Lerp Colors.Lerp output 0
			Variable_152 = FVoxelNodeFunctions::Lerp(Variable_156, Variable_160, Variable_85);
			
			// Lerp Colors.Make Color
			FColor Variable_164; // Lerp Colors.Make Color output 0
			Variable_164 = FVoxelNodeFunctions::MakeColorFloat(Variable_152, Variable_153, Variable_154, Variable_155);
			
			// Lerp Colors.Break Color
			v_flt Variable_173; // Lerp Colors.Break Color output 0
			v_flt Variable_174; // Lerp Colors.Break Color output 1
			v_flt Variable_175; // Lerp Colors.Break Color output 2
			v_flt Variable_176; // Lerp Colors.Break Color output 3
			FVoxelNodeFunctions::BreakColorFloat(Variable_164, Variable_173, Variable_174, Variable_175, Variable_176);
			
			// Lerp Colors.Lerp
			v_flt Variable_165; // Lerp Colors.Lerp output 0
			Variable_165 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_169, Variable_173, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_166; // Lerp Colors.Lerp output 0
			Variable_166 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_170, Variable_174, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_167; // Lerp Colors.Lerp output 0
			Variable_167 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_171, Variable_175, Variable_86);
			
			// Lerp Colors.Lerp
			v_flt Variable_168; // Lerp Colors.Lerp output 0
			Variable_168 = FVoxelNodeFunctions::Lerp(BufferConstant.Variable_172, Variable_176, Variable_86);
			
			// Lerp Colors.Make Color
			FColor Variable_177; // Lerp Colors.Make Color output 0
			Variable_177 = FVoxelNodeFunctions::MakeColorFloat(Variable_165, Variable_166, Variable_167, Variable_168);
			
			Outputs.MaterialBuilder.SetColor(Variable_177);
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
			
			TVoxelRange<v_flt> Variable_39; // MountainsNoiseHeight = 500.0 output 0
			TVoxelRange<v_flt> Variable_41; // 2D Perlin Noise output 0
			TVoxelRange<v_flt> Variable_22; // RingEdgesHardness = 10.0 output 0
			TVoxelRange<v_flt> Variable_45; // PlainsNoiseHeight = 250.0 output 0
			TVoxelRange<v_flt> Variable_47; // 2D Simplex Noise output 0
			TVoxelRange<v_flt> Variable_33; // BaseHeight = 1000.0 output 0
			TVoxelRange<v_flt> Variable_50; // RiverDepth = 100.0 output 0
			TVoxelRange<v_flt> Variable_37; // 2D Simplex Noise Fractal output 0
			TVoxelRange<v_flt> Variable_73; // RiverWidth = 1.0 output 0
			TVoxelRange<v_flt> Variable_69; // Scale = 10.0 output 0
			TVoxelRange<v_flt> Variable_59; // * output 0
			TVoxelRange<v_flt> Variable_30; // / output 0
			TVoxelRange<v_flt> Variable_70; // * output 0
			TVoxelRange<v_flt> Variable_35; // * output 0
			TVoxelRange<v_flt> Variable_6; // + output 0
			TVoxelRange<v_flt> Variable_29; // - output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_0; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_1; // Y output 0
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
					
					// Init of 2D Perlin Noise
					_2D_Perlin_Noise_2_Noise.SetSeed(FVoxelGraphSeed(1338));
					_2D_Perlin_Noise_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 2D Simplex Noise
					_2D_Simplex_Noise_4_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Simplex_Noise_4_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 2D Simplex Noise Fractal
					_2D_Simplex_Noise_Fractal_3_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Simplex_Noise_Fractal_3_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Simplex_Noise_Fractal_3_Noise.SetFractalOctavesAndGain(5, 0.5);
					_2D_Simplex_Noise_Fractal_3_Noise.SetFractalLacunarity(2.0);
					_2D_Simplex_Noise_Fractal_3_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[0] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[1] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[2] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[3] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[4] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[5] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[6] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[7] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[8] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[9] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[10] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[11] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[12] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[13] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[14] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[15] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[16] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[17] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[18] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[19] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[20] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[21] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[22] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[23] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[24] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[25] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[26] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[27] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[28] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[29] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[30] = 5;
					_2D_Simplex_Noise_Fractal_3_LODToOctaves[31] = 5;
					
					// Init of 2D Simplex Noise Fractal
					_2D_Simplex_Noise_Fractal_4_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Simplex_Noise_Fractal_4_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Simplex_Noise_Fractal_4_Noise.SetFractalOctavesAndGain(8, 0.5);
					_2D_Simplex_Noise_Fractal_4_Noise.SetFractalLacunarity(2.0);
					_2D_Simplex_Noise_Fractal_4_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[0] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[1] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[2] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[3] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[4] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[5] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[6] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[7] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[8] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[9] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[10] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[11] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[12] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[13] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[14] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[15] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[16] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[17] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[18] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[19] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[20] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[21] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[22] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[23] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[24] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[25] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[26] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[27] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[28] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[29] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[30] = 8;
					_2D_Simplex_Noise_Fractal_4_LODToOctaves[31] = 8;
					
					// Init of 2D Simplex Noise
					_2D_Simplex_Noise_5_Noise.SetSeed(FVoxelGraphSeed(1000));
					_2D_Simplex_Noise_5_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
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
				// MountainsNoiseHeight = 500.0
				BufferConstant.Variable_39 = Params.MountainsNoiseHeight;
				
				// 2D Perlin Noise
				BufferConstant.Variable_41 = { -0.888317f, 0.811183f };
				
				// RingEdgesHardness = 10.0
				BufferConstant.Variable_22 = Params.RingEdgesHardness;
				
				// Width in Degrees = 50.0
				TVoxelRange<v_flt> Variable_24; // Width in Degrees = 50.0 output 0
				Variable_24 = Params.Width_in_Degrees;
				
				// PlainsNoiseHeight = 250.0
				BufferConstant.Variable_45 = Params.PlainsNoiseHeight;
				
				// 2D Simplex Noise
				BufferConstant.Variable_47 = { -0.997652f, 0.996970f };
				
				// BaseHeight = 1000.0
				BufferConstant.Variable_33 = Params.BaseHeight;
				
				// 2D Simplex Noise Fractal
				TVoxelRange<v_flt> Variable_34; // 2D Simplex Noise Fractal output 0
				Variable_34 = { -0.802678f, 0.846347f };
				
				// RiverDepth = 100.0
				BufferConstant.Variable_50 = Params.RiverDepth;
				
				// BaseNoiseHeight = 250.0
				TVoxelRange<v_flt> Variable_36; // BaseNoiseHeight = 250.0 output 0
				Variable_36 = Params.BaseNoiseHeight;
				
				// 2D Simplex Noise Fractal
				BufferConstant.Variable_37 = { -0.780446f, 0.760988f };
				
				// Radius = 7000.0
				TVoxelRange<v_flt> Variable_5; // Radius = 7000.0 output 0
				Variable_5 = Params.Radius;
				
				// 2D Simplex Noise
				TVoxelRange<v_flt> Variable_62; // 2D Simplex Noise output 0
				Variable_62 = { -0.997888f, 0.997883f };
				
				// Thickness = 500.0
				TVoxelRange<v_flt> Variable_74; // Thickness = 500.0 output 0
				Variable_74 = Params.Thickness;
				
				// RiverWidth = 1.0
				BufferConstant.Variable_73 = Params.RiverWidth;
				
				// Scale = 10.0
				BufferConstant.Variable_69 = Params.Scale;
				
				// *
				BufferConstant.Variable_59 = Variable_62 * TVoxelRange<v_flt>(0.1f);
				
				// /
				BufferConstant.Variable_30 = Variable_24 / TVoxelRange<v_flt>(360.0f);
				
				// *
				TVoxelRange<v_flt> Variable_75; // * output 0
				Variable_75 = Variable_74 * BufferConstant.Variable_69;
				
				// *
				BufferConstant.Variable_70 = Variable_5 * BufferConstant.Variable_69;
				
				// *
				BufferConstant.Variable_35 = Variable_34 * Variable_36;
				
				// +
				BufferConstant.Variable_6 = BufferConstant.Variable_70 + Variable_75;
				
				// -
				BufferConstant.Variable_29 = TVoxelRange<v_flt>(0.5f) - BufferConstant.Variable_30;
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_2_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_4_Noise;
		FVoxelFastNoise _2D_Simplex_Noise_Fractal_3_Noise;
		TStaticArray<uint8, 32> _2D_Simplex_Noise_Fractal_3_LODToOctaves;
		FVoxelFastNoise _2D_Simplex_Noise_Fractal_4_Noise;
		TStaticArray<uint8, 32> _2D_Simplex_Noise_Fractal_4_LODToOctaves;
		FVoxelFastNoise _2D_Simplex_Noise_5_Noise;
		
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
			TVoxelRange<v_flt> Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Z
			TVoxelRange<v_flt> Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_12; // Z output 0
			Variable_12 = Context.GetLocalZ();
			
			// Z
			TVoxelRange<v_flt> Variable_55; // Z output 0
			Variable_55 = Context.GetLocalZ();
			
			// Vector Length
			TVoxelRange<v_flt> Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// +
			TVoxelRange<v_flt> Variable_23; // + output 0
			Variable_23 = Variable_3 + TVoxelRange<v_flt>(0.001f);
			
			// /
			TVoxelRange<v_flt> Variable_56; // / output 0
			Variable_56 = Variable_55 / Variable_23;
			
			// -
			TVoxelRange<v_flt> Variable_7; // - output 0
			Variable_7 = Variable_23 - BufferConstant.Variable_6;
			
			// /
			TVoxelRange<v_flt> Variable_13; // / output 0
			Variable_13 = Variable_12 / Variable_23;
			
			// +
			TVoxelRange<v_flt> Variable_58; // + output 0
			Variable_58 = Variable_56 + BufferConstant.Variable_59;
			
			// ACOS
			TVoxelRange<v_flt> Variable_14; // ACOS output 0
			Variable_14 = FVoxelNodeFunctions::Acos(Variable_13);
			
			// ACOS
			TVoxelRange<v_flt> Variable_57; // ACOS output 0
			Variable_57 = FVoxelNodeFunctions::Acos(Variable_58);
			
			// /
			TVoxelRange<v_flt> Variable_10; // / output 0
			Variable_10 = Variable_14 / TVoxelRange<v_flt>(3.141593f);
			
			// /
			TVoxelRange<v_flt> Variable_54; // / output 0
			Variable_54 = Variable_57 / TVoxelRange<v_flt>(3.141593f);
			
			// 1 - X
			TVoxelRange<v_flt> Variable_11; // 1 - X output 0
			Variable_11 = 1 - Variable_10;
			
			// Min (float)
			TVoxelRange<v_flt> Variable_9; // Min (float) output 0
			Variable_9 = FVoxelNodeFunctions::Min<v_flt>(Variable_10, Variable_11);
			
			// 1 - X
			TVoxelRange<v_flt> Variable_52; // 1 - X output 0
			Variable_52 = 1 - Variable_54;
			
			// Min (float)
			TVoxelRange<v_flt> Variable_51; // Min (float) output 0
			Variable_51 = FVoxelNodeFunctions::Min<v_flt>(Variable_54, Variable_52);
			
			// -
			TVoxelRange<v_flt> Variable_15; // - output 0
			Variable_15 = Variable_9 - BufferConstant.Variable_29;
			
			// /
			TVoxelRange<v_flt> Variable_28; // / output 0
			Variable_28 = Variable_15 / BufferConstant.Variable_30;
			
			// /
			TVoxelRange<v_flt> Variable_53; // / output 0
			Variable_53 = Variable_51 / TVoxelRange<v_flt>(0.5f);
			
			// Max (float)
			TVoxelRange<v_flt> Variable_18; // Max (float) output 0
			Variable_18 = FVoxelNodeFunctions::Max<v_flt>(Variable_28, TVoxelRange<v_flt>(0.0f));
			
			// 1 - X
			TVoxelRange<v_flt> Variable_27; // 1 - X output 0
			Variable_27 = 1 - Variable_28;
			
			// 1 - X
			TVoxelRange<v_flt> Variable_60; // 1 - X output 0
			Variable_60 = 1 - Variable_53;
			
			// *
			TVoxelRange<v_flt> Variable_61; // * output 0
			Variable_61 = Variable_60 * TVoxelRange<v_flt>(20.0f);
			
			// Float Curve: PlainsNoiseStrengthCurve
			TVoxelRange<v_flt> Variable_46; // Float Curve: PlainsNoiseStrengthCurve output 0
			Variable_46 = FVoxelNodeFunctions::GetCurveValue(Params.PlainsNoiseStrengthCurve, Variable_27);
			
			// Float Curve: MoutainsMaskCurve
			TVoxelRange<v_flt> Variable_40; // Float Curve: MoutainsMaskCurve output 0
			Variable_40 = FVoxelNodeFunctions::GetCurveValue(Params.MoutainsMaskCurve, Variable_27);
			
			// Float Curve: RingMainShapeCurve
			TVoxelRange<v_flt> Variable_31; // Float Curve: RingMainShapeCurve output 0
			Variable_31 = FVoxelNodeFunctions::GetCurveValue(Params.RingMainShapeCurve, Variable_27);
			
			// *
			TVoxelRange<v_flt> Variable_20; // * output 0
			Variable_20 = Variable_18 * BufferConstant.Variable_22;
			
			// *
			TVoxelRange<v_flt> Variable_32; // * output 0
			Variable_32 = Variable_31 * BufferConstant.Variable_33;
			
			// *
			TVoxelRange<v_flt> Variable_38; // * output 0
			Variable_38 = BufferConstant.Variable_37 * BufferConstant.Variable_39 * Variable_40;
			
			// *
			TVoxelRange<v_flt> Variable_42; // * output 0
			Variable_42 = BufferConstant.Variable_39 * Variable_40 * BufferConstant.Variable_41 * TVoxelRange<v_flt>(0.1f);
			
			// /
			TVoxelRange<v_flt> Variable_72; // / output 0
			Variable_72 = Variable_61 / BufferConstant.Variable_73;
			
			// Min (float)
			TVoxelRange<v_flt> Variable_21; // Min (float) output 0
			Variable_21 = FVoxelNodeFunctions::Min<v_flt>(Variable_20, TVoxelRange<v_flt>(1.0f));
			
			// 1 - X
			TVoxelRange<v_flt> Variable_43; // 1 - X output 0
			Variable_43 = 1 - Variable_40;
			
			// Float Curve: RiverDepthCurve
			TVoxelRange<v_flt> Variable_48; // Float Curve: RiverDepthCurve output 0
			Variable_48 = FVoxelNodeFunctions::GetCurveValue(Params.RiverDepthCurve, Variable_72);
			
			// Max (float)
			TVoxelRange<v_flt> Variable_63; // Max (float) output 0
			Variable_63 = FVoxelNodeFunctions::Max<v_flt>(Variable_48, TVoxelRange<v_flt>(0.0f));
			
			// Min (float)
			TVoxelRange<v_flt> Variable_64; // Min (float) output 0
			Variable_64 = FVoxelNodeFunctions::Min<v_flt>(Variable_48, TVoxelRange<v_flt>(0.0f));
			
			// *
			TVoxelRange<v_flt> Variable_65; // * output 0
			Variable_65 = Variable_64 * TVoxelRange<v_flt>(-1.0f);
			
			// *
			TVoxelRange<v_flt> Variable_49; // * output 0
			Variable_49 = Variable_63 * TVoxelRange<v_flt>(-1.0f) * BufferConstant.Variable_50;
			
			// *
			TVoxelRange<v_flt> Variable_66; // * output 0
			Variable_66 = BufferConstant.Variable_47 * Variable_65;
			
			// 1 - X
			TVoxelRange<v_flt> Variable_67; // 1 - X output 0
			Variable_67 = 1 - Variable_65;
			
			// -
			TVoxelRange<v_flt> Variable_68; // - output 0
			Variable_68 = Variable_66 - Variable_67;
			
			// *
			TVoxelRange<v_flt> Variable_44; // * output 0
			Variable_44 = Variable_68 * BufferConstant.Variable_45 * Variable_43 * Variable_46;
			
			// +
			TVoxelRange<v_flt> Variable_26; // + output 0
			Variable_26 = Variable_32 + BufferConstant.Variable_35 + Variable_38 + Variable_42 + Variable_44 + Variable_49;
			
			// *
			TVoxelRange<v_flt> Variable_71; // * output 0
			Variable_71 = Variable_26 * BufferConstant.Variable_69;
			
			// -
			TVoxelRange<v_flt> Variable_25; // - output 0
			Variable_25 = BufferConstant.Variable_70 - Variable_71;
			
			// -
			TVoxelRange<v_flt> Variable_4; // - output 0
			Variable_4 = Variable_25 - Variable_23;
			
			// Max (float)
			TVoxelRange<v_flt> Variable_8; // Max (float) output 0
			Variable_8 = FVoxelNodeFunctions::Max<v_flt>(Variable_4, Variable_7);
			
			// -
			TVoxelRange<v_flt> Variable_19; // - output 0
			Variable_19 = Variable_8 - TVoxelRange<v_flt>(1.0f);
			
			// *
			TVoxelRange<v_flt> Variable_16; // * output 0
			Variable_16 = Variable_19 * Variable_21;
			
			// +
			TVoxelRange<v_flt> Variable_17; // + output 0
			Variable_17 = Variable_16 + TVoxelRange<v_flt>(1.0f);
			
			Outputs.Value = Variable_17;
		}
		
	};
	
	FVoxelExample_RingWorldInstance(UVoxelExample_RingWorld& Object)
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
			Object.Radius,
			Object.RingEdgesHardness,
			Object.Scale,
			Object.Thickness,
			Object.Width_in_Degrees,
			Object.RiverDepth,
			Object.RiverWidth,
			Object.BeachColor,
			Object.MountainsColorHigh,
			Object.MountainsColorLowHigh,
			Object.MountainsColorLowLow,
			FVoxelRichCurve(Object.MoutainsMaskCurve.LoadSynchronous()),
			Object.PlainsColorHigh,
			Object.PlainsColorLow,
			Object.PlainsNoiseFrequency,
			Object.PlainsNoiseHeight,
			FVoxelRichCurve(Object.PlainsNoiseStrengthCurve.LoadSynchronous()),
			FVoxelRichCurve(Object.RingMainShapeCurve.LoadSynchronous()),
			Object.RingOuterColor,
			Object.RiverColor,
			FVoxelRichCurve(Object.RiverDepthCurve.LoadSynchronous()),
			Object.MountainsNoiseFrequency,
			Object.MountainsNoiseHeight,
			Object.BaseNoiseFrquency,
			Object.BaseNoiseHeight,
			Object.BaseHeight
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
inline v_flt FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_RingWorldInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_RingWorldInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_RingWorldInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_RingWorldInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_RingWorldInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_RingWorld::UVoxelExample_RingWorld()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_RingWorld::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_RingWorldInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_RingWorld. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_RingWorld. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_RingWorld. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_RingWorld. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
