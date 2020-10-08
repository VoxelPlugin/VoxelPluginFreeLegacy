// Copyright 2020 Phyronnaz

#include "VoxelExample_FloatingIslandOnion.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_FloatingIslandOnionInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_FloatingIslandOnionInstance, UVoxelExample_FloatingIslandOnion>
{
public:
	struct FParams
	{
		const float Height;
		const float Perturb_Amplitude;
		const float Perturb_Frequency;
		const int32 Seed;
		const float Top_Noise_Frequency;
		const float Top_Noise_Height;
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
			
			v_flt Variable_22; // Top Noise Frequency = 0.002 output 0
			v_flt Variable_21; // Perturb Frequency = 0.01 output 0
			v_flt Variable_20; // Perturb Amplitude = 50.0 output 0
			v_flt Variable_3; // Top Noise Height = 500.0 output 0
			v_flt Variable_12; // * -1 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_6; // X output 0
			v_flt Variable_1; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_25; // 2D Noise SDF.+ output 0
			v_flt Variable_27; // Elongate.vector - vector.- output 0
			v_flt Variable_26; // Elongate.vector - vector.- output 0
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
					
					// Init of Seed = 1337
					FVoxelGraphSeed Variable_18; // Seed = 1337 output 0
					Variable_18 = Params.Seed;
					
					// Init of HASH
					FVoxelGraphSeed Variable_19; // HASH output 0
					Variable_19 = FVoxelUtilities::MurmurHash32(Variable_18);
					
					
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
				// Height = 200.0
				v_flt Variable_9; // Height = 200.0 output 0
				Variable_9 = Params.Height;
				
				// Top Noise Frequency = 0.002
				BufferConstant.Variable_22 = Params.Top_Noise_Frequency;
				
				// Perturb Frequency = 0.01
				BufferConstant.Variable_21 = Params.Perturb_Frequency;
				
				// Perturb Amplitude = 50.0
				BufferConstant.Variable_20 = Params.Perturb_Amplitude;
				
				// Top Noise Height = 500.0
				BufferConstant.Variable_3 = Params.Top_Noise_Height;
				
				// * -1
				BufferConstant.Variable_12 = Variable_9 * -1;
				
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
		FVoxelFastNoise _2D_Gradient_Perturb_Fractal_0_Noise;
		TStaticArray<uint8, 32> _2D_Gradient_Perturb_Fractal_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 1337
			FVoxelGraphSeed Variable_18; // Seed = 1337 output 0
			Variable_18 = Params.Seed;
			
			// Init of HASH
			FVoxelGraphSeed Variable_19; // HASH output 0
			Variable_19 = FVoxelUtilities::MurmurHash32(Variable_18);
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_18);
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
			
			// Init of 2D Gradient Perturb Fractal
			_2D_Gradient_Perturb_Fractal_0_Noise.SetSeed(Variable_19);
			_2D_Gradient_Perturb_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Gradient_Perturb_Fractal_0_Noise.SetFractalOctavesAndGain(7, 0.5);
			_2D_Gradient_Perturb_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_2D_Gradient_Perturb_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[0] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[1] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[2] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[3] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[4] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[5] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[6] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[7] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[8] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[9] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[10] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[11] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[12] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[13] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[14] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[15] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[16] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[17] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[18] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[19] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[20] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[21] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[22] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[23] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[24] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[25] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[26] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[27] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[28] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[29] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[30] = 7;
			_2D_Gradient_Perturb_Fractal_0_LODToOctaves[31] = 7;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_6 = Context.GetLocalX();
			
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_7; // Y output 0
			Variable_7 = Context.GetLocalY();
			
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// 2D IQ Noise
			v_flt Variable_0; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_0 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_1, Variable_2, BufferConstant.Variable_22, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -0.631134, 0.652134);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.224071, 1.771704);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.220247, 1.219364);
			
			// 2D Gradient Perturb Fractal
			v_flt Variable_13; // 2D Gradient Perturb Fractal output 0
			v_flt Variable_14; // 2D Gradient Perturb Fractal output 1
			Variable_13 = BufferX.Variable_6;
			Variable_14 = Variable_7;
			_2D_Gradient_Perturb_Fractal_0_Noise.GradientPerturbFractal_2D(Variable_13, Variable_14, BufferConstant.Variable_21, _2D_Gradient_Perturb_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], BufferConstant.Variable_20);
			
			// 2D Noise SDF.*
			v_flt Variable_24; // 2D Noise SDF.* output 0
			Variable_24 = Variable_0 * BufferConstant.Variable_3;
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_28; // Elongate.Clamp Vector.Clamp output 0
			Variable_28 = FVoxelNodeFunctions::Clamp(Variable_13, v_flt(-1.0f), v_flt(1.0f));
			
			// 2D Noise SDF.+
			BufferXY.Variable_25 = Variable_24 + v_flt(0.0f);
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_5; // Elongate.Clamp Vector.Clamp output 0
			Variable_5 = FVoxelNodeFunctions::Clamp(Variable_14, v_flt(-100.0f), v_flt(100.0f));
			
			// Elongate.vector - vector.-
			BufferXY.Variable_27 = Variable_14 - Variable_5;
			
			// Elongate.vector - vector.-
			BufferXY.Variable_26 = Variable_13 - Variable_28;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_6 = Context.GetLocalX();
			
			// Y
			v_flt Variable_7; // Y output 0
			Variable_7 = Context.GetLocalY();
			
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// 2D IQ Noise
			v_flt Variable_0; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_0 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_1, Variable_2, BufferConstant.Variable_22, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -0.631134, 0.652134);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.224071, 1.771704);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.220247, 1.219364);
			
			// 2D Gradient Perturb Fractal
			v_flt Variable_13; // 2D Gradient Perturb Fractal output 0
			v_flt Variable_14; // 2D Gradient Perturb Fractal output 1
			Variable_13 = BufferX.Variable_6;
			Variable_14 = Variable_7;
			_2D_Gradient_Perturb_Fractal_0_Noise.GradientPerturbFractal_2D(Variable_13, Variable_14, BufferConstant.Variable_21, _2D_Gradient_Perturb_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], BufferConstant.Variable_20);
			
			// 2D Noise SDF.*
			v_flt Variable_24; // 2D Noise SDF.* output 0
			Variable_24 = Variable_0 * BufferConstant.Variable_3;
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_28; // Elongate.Clamp Vector.Clamp output 0
			Variable_28 = FVoxelNodeFunctions::Clamp(Variable_13, v_flt(-1.0f), v_flt(1.0f));
			
			// 2D Noise SDF.+
			BufferXY.Variable_25 = Variable_24 + v_flt(0.0f);
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_5; // Elongate.Clamp Vector.Clamp output 0
			Variable_5 = FVoxelNodeFunctions::Clamp(Variable_14, v_flt(-100.0f), v_flt(100.0f));
			
			// Elongate.vector - vector.-
			BufferXY.Variable_27 = Variable_14 - Variable_5;
			
			// Elongate.vector - vector.-
			BufferXY.Variable_26 = Variable_13 - Variable_28;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_4; // Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// 2D Noise SDF.-
			v_flt Variable_23; // 2D Noise SDF.- output 0
			Variable_23 = Variable_4 - BufferXY.Variable_25;
			
			// Round Cone SDF
			v_flt Variable_11; // Round Cone SDF output 0
			Variable_11 = FVoxelSDFNodeFunctions::RoundCone(BufferXY.Variable_26, BufferXY.Variable_27, Variable_8, v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), BufferConstant.Variable_12, v_flt(150.0f), v_flt(50.0f));
			
			// Shell.ABS
			v_flt Variable_29; // Shell.ABS output 0
			Variable_29 = FVoxelNodeFunctions::Abs(Variable_11);
			
			// Shell.-
			v_flt Variable_15; // Shell.- output 0
			Variable_15 = Variable_29 - v_flt(150.0f);
			
			// Shell.ABS
			v_flt Variable_30; // Shell.ABS output 0
			Variable_30 = FVoxelNodeFunctions::Abs(Variable_15);
			
			// Shell.-
			v_flt Variable_16; // Shell.- output 0
			Variable_16 = Variable_30 - v_flt(75.0f);
			
			// Shell.ABS
			v_flt Variable_31; // Shell.ABS output 0
			Variable_31 = FVoxelNodeFunctions::Abs(Variable_16);
			
			// Shell.-
			v_flt Variable_17; // Shell.- output 0
			Variable_17 = Variable_31 - v_flt(37.0f);
			
			// Smooth Intersection
			v_flt Variable_10; // Smooth Intersection output 0
			Variable_10 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_23, Variable_17, v_flt(20.0f));
			
			// Set High Quality Value.*
			v_flt Variable_32; // Set High Quality Value.* output 0
			Variable_32 = Variable_10 * v_flt(0.2f);
			
			Outputs.Value = Variable_32;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// X
			v_flt Variable_6; // X output 0
			Variable_6 = Context.GetLocalX();
			
			// Z
			v_flt Variable_4; // Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_7; // Y output 0
			Variable_7 = Context.GetLocalY();
			
			// X
			v_flt Variable_1; // X output 0
			Variable_1 = Context.GetLocalX();
			
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// 2D IQ Noise
			v_flt Variable_0; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_0 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(Variable_1, Variable_2, BufferConstant.Variable_22, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -0.631134, 0.652134);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.224071, 1.771704);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.220247, 1.219364);
			
			// 2D Gradient Perturb Fractal
			v_flt Variable_13; // 2D Gradient Perturb Fractal output 0
			v_flt Variable_14; // 2D Gradient Perturb Fractal output 1
			Variable_13 = Variable_6;
			Variable_14 = Variable_7;
			_2D_Gradient_Perturb_Fractal_0_Noise.GradientPerturbFractal_2D(Variable_13, Variable_14, BufferConstant.Variable_21, _2D_Gradient_Perturb_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], BufferConstant.Variable_20);
			
			// 2D Noise SDF.*
			v_flt Variable_24; // 2D Noise SDF.* output 0
			Variable_24 = Variable_0 * BufferConstant.Variable_3;
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_28; // Elongate.Clamp Vector.Clamp output 0
			Variable_28 = FVoxelNodeFunctions::Clamp(Variable_13, v_flt(-1.0f), v_flt(1.0f));
			
			// 2D Noise SDF.+
			v_flt Variable_25; // 2D Noise SDF.+ output 0
			Variable_25 = Variable_24 + v_flt(0.0f);
			
			// Elongate.Clamp Vector.Clamp
			v_flt Variable_5; // Elongate.Clamp Vector.Clamp output 0
			Variable_5 = FVoxelNodeFunctions::Clamp(Variable_14, v_flt(-100.0f), v_flt(100.0f));
			
			// Elongate.vector - vector.-
			v_flt Variable_27; // Elongate.vector - vector.- output 0
			Variable_27 = Variable_14 - Variable_5;
			
			// Elongate.vector - vector.-
			v_flt Variable_26; // Elongate.vector - vector.- output 0
			Variable_26 = Variable_13 - Variable_28;
			
			// 2D Noise SDF.-
			v_flt Variable_23; // 2D Noise SDF.- output 0
			Variable_23 = Variable_4 - Variable_25;
			
			// Round Cone SDF
			v_flt Variable_11; // Round Cone SDF output 0
			Variable_11 = FVoxelSDFNodeFunctions::RoundCone(Variable_26, Variable_27, Variable_8, v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), v_flt(0.0f), BufferConstant.Variable_12, v_flt(150.0f), v_flt(50.0f));
			
			// Shell.ABS
			v_flt Variable_29; // Shell.ABS output 0
			Variable_29 = FVoxelNodeFunctions::Abs(Variable_11);
			
			// Shell.-
			v_flt Variable_15; // Shell.- output 0
			Variable_15 = Variable_29 - v_flt(150.0f);
			
			// Shell.ABS
			v_flt Variable_30; // Shell.ABS output 0
			Variable_30 = FVoxelNodeFunctions::Abs(Variable_15);
			
			// Shell.-
			v_flt Variable_16; // Shell.- output 0
			Variable_16 = Variable_30 - v_flt(75.0f);
			
			// Shell.ABS
			v_flt Variable_31; // Shell.ABS output 0
			Variable_31 = FVoxelNodeFunctions::Abs(Variable_16);
			
			// Shell.-
			v_flt Variable_17; // Shell.- output 0
			Variable_17 = Variable_31 - v_flt(37.0f);
			
			// Smooth Intersection
			v_flt Variable_10; // Smooth Intersection output 0
			Variable_10 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_23, Variable_17, v_flt(20.0f));
			
			// Set High Quality Value.*
			v_flt Variable_32; // Set High Quality Value.* output 0
			Variable_32 = Variable_10 * v_flt(0.2f);
			
			Outputs.Value = Variable_32;
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
			
			TVoxelRange<v_flt> Variable_17; // Perturb Frequency = 0.01 output 0
			TVoxelRange<v_flt> Variable_16; // Perturb Amplitude = 50.0 output 0
			TVoxelRange<v_flt> Variable_10; // * -1 output 0
			TVoxelRange<v_flt> Variable_20; // 2D Noise SDF.+ output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_4; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_21; // Elongate.vector - vector.- output 0
			TVoxelRange<v_flt> Variable_22; // Elongate.vector - vector.- output 0
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
				// Top Noise Height = 500.0
				TVoxelRange<v_flt> Variable_1; // Top Noise Height = 500.0 output 0
				Variable_1 = Params.Top_Noise_Height;
				
				// Height = 200.0
				TVoxelRange<v_flt> Variable_7; // Height = 200.0 output 0
				Variable_7 = Params.Height;
				
				// Perturb Frequency = 0.01
				BufferConstant.Variable_17 = Params.Perturb_Frequency;
				
				// Perturb Amplitude = 50.0
				BufferConstant.Variable_16 = Params.Perturb_Amplitude;
				
				// 2D IQ Noise
				TVoxelRange<v_flt> Variable_0; // 2D IQ Noise output 0
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
				Variable_0 = { -0.631134f, 0.652134f };
				_2D_IQ_Noise_1_Temp_1 = { -1.224071f, 1.771704f };
				_2D_IQ_Noise_1_Temp_2 = { -1.220247f, 1.219364f };
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_19; // 2D Noise SDF.* output 0
				Variable_19 = Variable_0 * Variable_1;
				
				// * -1
				BufferConstant.Variable_10 = Variable_7 * -1;
				
				// 2D Noise SDF.+
				BufferConstant.Variable_20 = Variable_19 + TVoxelRange<v_flt>(0.0f);
				
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
		
		FVoxelFastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		FVoxelFastNoise _2D_Gradient_Perturb_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Gradient_Perturb_Fractal_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 2D Gradient Perturb Fractal
			_2D_Gradient_Perturb_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Gradient_Perturb_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Gradient_Perturb_Fractal_1_Noise.SetFractalOctavesAndGain(7, 0.5);
			_2D_Gradient_Perturb_Fractal_1_Noise.SetFractalLacunarity(2.0);
			_2D_Gradient_Perturb_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[0] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[1] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[2] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[3] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[4] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[5] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[6] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[7] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[8] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[9] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[10] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[11] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[12] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[13] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[14] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[15] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[16] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[17] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[18] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[19] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[20] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[21] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[22] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[23] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[24] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[25] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[26] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[27] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[28] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[29] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[30] = 7;
			_2D_Gradient_Perturb_Fractal_1_LODToOctaves[31] = 7;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Z
			TVoxelRange<v_flt> Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// Z
			TVoxelRange<v_flt> Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_5; // Y output 0
			Variable_5 = Context.GetLocalY();
			
			// 2D Gradient Perturb Fractal
			TVoxelRange<v_flt> Variable_11; // 2D Gradient Perturb Fractal output 0
			TVoxelRange<v_flt> Variable_12; // 2D Gradient Perturb Fractal output 1
			Variable_11 = TVoxelRange<v_flt>::FromList(Variable_4.Min - 2 * BufferConstant.Variable_16.Max, Variable_4.Max + 2 * BufferConstant.Variable_16.Max);
			Variable_12 = TVoxelRange<v_flt>::FromList(Variable_5.Min - 2 * BufferConstant.Variable_16.Max, Variable_5.Max + 2 * BufferConstant.Variable_16.Max);
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_18; // 2D Noise SDF.- output 0
			Variable_18 = Variable_2 - BufferConstant.Variable_20;
			
			// Elongate.Clamp Vector.Clamp
			TVoxelRange<v_flt> Variable_23; // Elongate.Clamp Vector.Clamp output 0
			Variable_23 = FVoxelNodeFunctions::Clamp(Variable_11, TVoxelRange<v_flt>(-1.0f), TVoxelRange<v_flt>(1.0f));
			
			// Elongate.Clamp Vector.Clamp
			TVoxelRange<v_flt> Variable_3; // Elongate.Clamp Vector.Clamp output 0
			Variable_3 = FVoxelNodeFunctions::Clamp(Variable_12, TVoxelRange<v_flt>(-100.0f), TVoxelRange<v_flt>(100.0f));
			
			// Elongate.vector - vector.-
			TVoxelRange<v_flt> Variable_21; // Elongate.vector - vector.- output 0
			Variable_21 = Variable_11 - Variable_23;
			
			// Elongate.vector - vector.-
			TVoxelRange<v_flt> Variable_22; // Elongate.vector - vector.- output 0
			Variable_22 = Variable_12 - Variable_3;
			
			// Round Cone SDF
			TVoxelRange<v_flt> Variable_9; // Round Cone SDF output 0
			Variable_9 = FVoxelSDFNodeFunctions::RoundCone(Variable_21, Variable_22, Variable_6, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), BufferConstant.Variable_10, TVoxelRange<v_flt>(150.0f), TVoxelRange<v_flt>(50.0f));
			
			// Shell.ABS
			TVoxelRange<v_flt> Variable_24; // Shell.ABS output 0
			Variable_24 = FVoxelNodeFunctions::Abs(Variable_9);
			
			// Shell.-
			TVoxelRange<v_flt> Variable_13; // Shell.- output 0
			Variable_13 = Variable_24 - TVoxelRange<v_flt>(150.0f);
			
			// Shell.ABS
			TVoxelRange<v_flt> Variable_25; // Shell.ABS output 0
			Variable_25 = FVoxelNodeFunctions::Abs(Variable_13);
			
			// Shell.-
			TVoxelRange<v_flt> Variable_14; // Shell.- output 0
			Variable_14 = Variable_25 - TVoxelRange<v_flt>(75.0f);
			
			// Shell.ABS
			TVoxelRange<v_flt> Variable_26; // Shell.ABS output 0
			Variable_26 = FVoxelNodeFunctions::Abs(Variable_14);
			
			// Shell.-
			TVoxelRange<v_flt> Variable_15; // Shell.- output 0
			Variable_15 = Variable_26 - TVoxelRange<v_flt>(37.0f);
			
			// Smooth Intersection
			TVoxelRange<v_flt> Variable_8; // Smooth Intersection output 0
			Variable_8 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_18, Variable_15, TVoxelRange<v_flt>(20.0f));
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_27; // Set High Quality Value.* output 0
			Variable_27 = Variable_8 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_27;
		}
		
	};
	
	FVoxelExample_FloatingIslandOnionInstance(UVoxelExample_FloatingIslandOnion& Object)
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
			Object.Height,
			Object.Perturb_Amplitude,
			Object.Perturb_Frequency,
			Object.Seed,
			Object.Top_Noise_Frequency,
			Object.Top_Noise_Height
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
inline v_flt FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_FloatingIslandOnionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_FloatingIslandOnionInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_FloatingIslandOnionInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_FloatingIslandOnionInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_FloatingIslandOnionInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_FloatingIslandOnion::UVoxelExample_FloatingIslandOnion()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_FloatingIslandOnion::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_FloatingIslandOnionInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_FloatingIslandOnion. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_FloatingIslandOnion. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_FloatingIslandOnion. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_FloatingIslandOnion. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
