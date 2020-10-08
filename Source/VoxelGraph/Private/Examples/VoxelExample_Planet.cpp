// Copyright 2020 Phyronnaz

#include "VoxelExample_Planet.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_PlanetInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_PlanetInstance, UVoxelExample_Planet>
{
public:
	struct FParams
	{
		const float Frequency;
		const int32 Noise_Seed;
		const float Noise_Strength;
		const FVoxelColorRichCurve PlanetColorCurve;
		const FVoxelRichCurve PlanetCurve;
		const float Radius;
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
			
			v_flt Variable_8; // Noise Strength = 0.02 output 0
			v_flt Variable_10; // Frequency = 2.0 output 0
			v_flt Variable_5; // Radius = 1000.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_0; // X output 0
			v_flt Variable_16; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_1; // Y output 0
			v_flt Variable_17; // Y output 0
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
					
					// Init of Noise Seed = 1443
					FVoxelGraphSeed Variable_20; // Noise Seed = 1443 output 0
					Variable_20 = Params.Noise_Seed;
					
					
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
				// Noise Strength = 0.02
				BufferConstant.Variable_8 = Params.Noise_Strength;
				
				// Frequency = 2.0
				BufferConstant.Variable_10 = Params.Frequency;
				
				// Radius = 1000.0
				BufferConstant.Variable_5 = Params.Radius;
				
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
		
		FVoxelFastNoise _3D_Gradient_Perturb_0_Noise;
		FVoxelFastNoise _3D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _3D_IQ_Noise_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Noise Seed = 1443
			FVoxelGraphSeed Variable_20; // Noise Seed = 1443 output 0
			Variable_20 = Params.Noise_Seed;
			
			// Init of 3D Gradient Perturb
			_3D_Gradient_Perturb_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D IQ Noise
			_3D_IQ_Noise_0_Noise.SetSeed(Variable_20);
			_3D_IQ_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.6);
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
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// X
			BufferX.Variable_16 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_17 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_17 = Context.GetLocalY();
			
			// X
			BufferX.Variable_16 = Context.GetLocalX();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_18; // Z output 0
			Variable_18 = Context.GetLocalZ();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferXY.Variable_1, Variable_2);
			
			// Sphere Normalize with Preview.Normalize.Vector Length
			v_flt Variable_21; // Sphere Normalize with Preview.Normalize.Vector Length output 0
			Variable_21 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_16, BufferXY.Variable_17, Variable_18);
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_23; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_23 = BufferXY.Variable_17 / Variable_21;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_24; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_24 = Variable_18 / Variable_21;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_22; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_22 = BufferX.Variable_16 / Variable_21;
			
			// 3D Gradient Perturb
			v_flt Variable_13; // 3D Gradient Perturb output 0
			v_flt Variable_14; // 3D Gradient Perturb output 1
			v_flt Variable_15; // 3D Gradient Perturb output 2
			Variable_13 = Variable_22;
			Variable_14 = Variable_23;
			Variable_15 = Variable_24;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_13, Variable_14, Variable_15, v_flt(0.02f), v_flt(0.01f));
			
			// 3D IQ Noise
			v_flt Variable_6; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_0_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_0_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_0_Temp_3; // 3D IQ Noise output 3
			Variable_6 = _3D_IQ_Noise_0_Noise.IQNoise_3D_Deriv(Variable_13, Variable_14, Variable_15, BufferConstant.Variable_10, _3D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_0_Temp_1,_3D_IQ_Noise_0_Temp_2,_3D_IQ_Noise_0_Temp_3);
			Variable_6 = FMath::Clamp<v_flt>(Variable_6, -0.653693, 0.750231);
			_3D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_1, -1.536367, 1.653675);
			_3D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_2, -1.654880, 1.681203);
			_3D_IQ_Noise_0_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_3, -1.580102, 1.625968);
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_6 - v_flt(0.1f);
			
			// /
			v_flt Variable_12; // / output 0
			Variable_12 = Variable_19 / v_flt(0.5f);
			
			// Float Curve: PlanetCurve
			v_flt Variable_11; // Float Curve: PlanetCurve output 0
			Variable_11 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetCurve, Variable_12);
			
			// *
			v_flt Variable_7; // * output 0
			Variable_7 = BufferConstant.Variable_5 * Variable_11 * BufferConstant.Variable_8;
			
			// +
			v_flt Variable_9; // + output 0
			Variable_9 = BufferConstant.Variable_5 + Variable_7;
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_3 - Variable_9;
			
			Outputs.Value = Variable_4;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_17; // Y output 0
			Variable_17 = Context.GetLocalY();
			
			// X
			v_flt Variable_16; // X output 0
			Variable_16 = Context.GetLocalX();
			
			// Z
			v_flt Variable_18; // Z output 0
			Variable_18 = Context.GetLocalZ();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// Sphere Normalize with Preview.Normalize.Vector Length
			v_flt Variable_21; // Sphere Normalize with Preview.Normalize.Vector Length output 0
			Variable_21 = FVoxelNodeFunctions::VectorLength(Variable_16, Variable_17, Variable_18);
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_23; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_23 = Variable_17 / Variable_21;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_24; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_24 = Variable_18 / Variable_21;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_22; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_22 = Variable_16 / Variable_21;
			
			// 3D Gradient Perturb
			v_flt Variable_13; // 3D Gradient Perturb output 0
			v_flt Variable_14; // 3D Gradient Perturb output 1
			v_flt Variable_15; // 3D Gradient Perturb output 2
			Variable_13 = Variable_22;
			Variable_14 = Variable_23;
			Variable_15 = Variable_24;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_13, Variable_14, Variable_15, v_flt(0.02f), v_flt(0.01f));
			
			// 3D IQ Noise
			v_flt Variable_6; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_0_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_0_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_0_Temp_3; // 3D IQ Noise output 3
			Variable_6 = _3D_IQ_Noise_0_Noise.IQNoise_3D_Deriv(Variable_13, Variable_14, Variable_15, BufferConstant.Variable_10, _3D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_0_Temp_1,_3D_IQ_Noise_0_Temp_2,_3D_IQ_Noise_0_Temp_3);
			Variable_6 = FMath::Clamp<v_flt>(Variable_6, -0.653693, 0.750231);
			_3D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_1, -1.536367, 1.653675);
			_3D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_2, -1.654880, 1.681203);
			_3D_IQ_Noise_0_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_0_Temp_3, -1.580102, 1.625968);
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_6 - v_flt(0.1f);
			
			// /
			v_flt Variable_12; // / output 0
			Variable_12 = Variable_19 / v_flt(0.5f);
			
			// Float Curve: PlanetCurve
			v_flt Variable_11; // Float Curve: PlanetCurve output 0
			Variable_11 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetCurve, Variable_12);
			
			// *
			v_flt Variable_7; // * output 0
			Variable_7 = BufferConstant.Variable_5 * Variable_11 * BufferConstant.Variable_8;
			
			// +
			v_flt Variable_9; // + output 0
			Variable_9 = BufferConstant.Variable_5 + Variable_7;
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_3 - Variable_9;
			
			Outputs.Value = Variable_4;
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
			
			v_flt Variable_1; // Frequency = 2.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_10; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_11; // Y output 0
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
					
					// Init of Noise Seed = 1443
					FVoxelGraphSeed Variable_15; // Noise Seed = 1443 output 0
					Variable_15 = Params.Noise_Seed;
					
					
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
				// Frequency = 2.0
				BufferConstant.Variable_1 = Params.Frequency;
				
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
		
		FVoxelFastNoise _3D_Gradient_Perturb_1_Noise;
		FVoxelFastNoise _3D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _3D_IQ_Noise_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Noise Seed = 1443
			FVoxelGraphSeed Variable_15; // Noise Seed = 1443 output 0
			Variable_15 = Params.Noise_Seed;
			
			// Init of 3D Gradient Perturb
			_3D_Gradient_Perturb_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D IQ Noise
			_3D_IQ_Noise_1_Noise.SetSeed(Variable_15);
			_3D_IQ_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.6);
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
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_10 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_11 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_10 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_11 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_12; // Z output 0
			Variable_12 = Context.GetLocalZ();
			
			// Sphere Normalize with Preview.Normalize.Vector Length
			v_flt Variable_16; // Sphere Normalize with Preview.Normalize.Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_10, BufferXY.Variable_11, Variable_12);
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_17; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_17 = BufferX.Variable_10 / Variable_16;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_19; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_19 = Variable_12 / Variable_16;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_18; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_18 = BufferXY.Variable_11 / Variable_16;
			
			// 3D Gradient Perturb
			v_flt Variable_7; // 3D Gradient Perturb output 0
			v_flt Variable_8; // 3D Gradient Perturb output 1
			v_flt Variable_9; // 3D Gradient Perturb output 2
			Variable_7 = Variable_17;
			Variable_8 = Variable_18;
			Variable_9 = Variable_19;
			_3D_Gradient_Perturb_1_Noise.GradientPerturb_3D(Variable_7, Variable_8, Variable_9, v_flt(0.02f), v_flt(0.01f));
			
			// 3D IQ Noise
			v_flt Variable_0; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_1_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_1_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_1_Temp_3; // 3D IQ Noise output 3
			Variable_0 = _3D_IQ_Noise_1_Noise.IQNoise_3D_Deriv(Variable_7, Variable_8, Variable_9, BufferConstant.Variable_1, _3D_IQ_Noise_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_1_Temp_1,_3D_IQ_Noise_1_Temp_2,_3D_IQ_Noise_1_Temp_3);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -0.653693, 0.750231);
			_3D_IQ_Noise_1_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_1, -1.536367, 1.653675);
			_3D_IQ_Noise_1_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_2, -1.654880, 1.681203);
			_3D_IQ_Noise_1_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_3, -1.580102, 1.625968);
			
			// -
			v_flt Variable_13; // - output 0
			Variable_13 = Variable_0 - v_flt(0.1f);
			
			// /
			v_flt Variable_2; // / output 0
			Variable_2 = Variable_13 / v_flt(0.5f);
			
			// Color Curve: PlanetColorCurve
			v_flt Variable_3; // Color Curve: PlanetColorCurve output 0
			v_flt Variable_4; // Color Curve: PlanetColorCurve output 1
			v_flt Variable_5; // Color Curve: PlanetColorCurve output 2
			v_flt Variable_6; // Color Curve: PlanetColorCurve output 3
			Variable_3 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[0], Variable_2);
			Variable_4 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[1], Variable_2);
			Variable_5 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[2], Variable_2);
			Variable_6 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[3], Variable_2);
			
			// Make Color
			FColor Variable_14; // Make Color output 0
			Variable_14 = FVoxelNodeFunctions::MakeColorFloat(Variable_3, Variable_4, Variable_5, Variable_6);
			
			Outputs.MaterialBuilder.SetColor(Variable_14);
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_10; // X output 0
			Variable_10 = Context.GetLocalX();
			
			// Z
			v_flt Variable_12; // Z output 0
			Variable_12 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_11; // Y output 0
			Variable_11 = Context.GetLocalY();
			
			// Sphere Normalize with Preview.Normalize.Vector Length
			v_flt Variable_16; // Sphere Normalize with Preview.Normalize.Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(Variable_10, Variable_11, Variable_12);
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_17; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_17 = Variable_10 / Variable_16;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_19; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_19 = Variable_12 / Variable_16;
			
			// Sphere Normalize with Preview.Normalize./
			v_flt Variable_18; // Sphere Normalize with Preview.Normalize./ output 0
			Variable_18 = Variable_11 / Variable_16;
			
			// 3D Gradient Perturb
			v_flt Variable_7; // 3D Gradient Perturb output 0
			v_flt Variable_8; // 3D Gradient Perturb output 1
			v_flt Variable_9; // 3D Gradient Perturb output 2
			Variable_7 = Variable_17;
			Variable_8 = Variable_18;
			Variable_9 = Variable_19;
			_3D_Gradient_Perturb_1_Noise.GradientPerturb_3D(Variable_7, Variable_8, Variable_9, v_flt(0.02f), v_flt(0.01f));
			
			// 3D IQ Noise
			v_flt Variable_0; // 3D IQ Noise output 0
			v_flt _3D_IQ_Noise_1_Temp_1; // 3D IQ Noise output 1
			v_flt _3D_IQ_Noise_1_Temp_2; // 3D IQ Noise output 2
			v_flt _3D_IQ_Noise_1_Temp_3; // 3D IQ Noise output 3
			Variable_0 = _3D_IQ_Noise_1_Noise.IQNoise_3D_Deriv(Variable_7, Variable_8, Variable_9, BufferConstant.Variable_1, _3D_IQ_Noise_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_3D_IQ_Noise_1_Temp_1,_3D_IQ_Noise_1_Temp_2,_3D_IQ_Noise_1_Temp_3);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -0.653693, 0.750231);
			_3D_IQ_Noise_1_Temp_1 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_1, -1.536367, 1.653675);
			_3D_IQ_Noise_1_Temp_2 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_2, -1.654880, 1.681203);
			_3D_IQ_Noise_1_Temp_3 = FMath::Clamp<v_flt>(_3D_IQ_Noise_1_Temp_3, -1.580102, 1.625968);
			
			// -
			v_flt Variable_13; // - output 0
			Variable_13 = Variable_0 - v_flt(0.1f);
			
			// /
			v_flt Variable_2; // / output 0
			Variable_2 = Variable_13 / v_flt(0.5f);
			
			// Color Curve: PlanetColorCurve
			v_flt Variable_3; // Color Curve: PlanetColorCurve output 0
			v_flt Variable_4; // Color Curve: PlanetColorCurve output 1
			v_flt Variable_5; // Color Curve: PlanetColorCurve output 2
			v_flt Variable_6; // Color Curve: PlanetColorCurve output 3
			Variable_3 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[0], Variable_2);
			Variable_4 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[1], Variable_2);
			Variable_5 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[2], Variable_2);
			Variable_6 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetColorCurve.Curves[3], Variable_2);
			
			// Make Color
			FColor Variable_14; // Make Color output 0
			Variable_14 = FVoxelNodeFunctions::MakeColorFloat(Variable_3, Variable_4, Variable_5, Variable_6);
			
			Outputs.MaterialBuilder.SetColor(Variable_14);
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
			
			v_flt Variable_1; // Set Sphere Up Vector.X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_2; // Set Sphere Up Vector.Y output 0
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
			// Set Sphere Up Vector.X
			BufferX.Variable_1 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Set Sphere Up Vector.Y
			BufferXY.Variable_2 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Set Sphere Up Vector.X
			BufferX.Variable_1 = Context.GetLocalX();
			
			// Set Sphere Up Vector.Y
			BufferXY.Variable_2 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Set Sphere Up Vector.Z
			v_flt Variable_0; // Set Sphere Up Vector.Z output 0
			Variable_0 = Context.GetLocalZ();
			
			Outputs.UpVectorX = BufferX.Variable_1;
			Outputs.UpVectorY = BufferXY.Variable_2;
			Outputs.UpVectorZ = Variable_0;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Set Sphere Up Vector.X
			v_flt Variable_1; // Set Sphere Up Vector.X output 0
			Variable_1 = Context.GetLocalX();
			
			// Set Sphere Up Vector.Y
			v_flt Variable_2; // Set Sphere Up Vector.Y output 0
			Variable_2 = Context.GetLocalY();
			
			// Set Sphere Up Vector.Z
			v_flt Variable_0; // Set Sphere Up Vector.Z output 0
			Variable_0 = Context.GetLocalZ();
			
			Outputs.UpVectorX = Variable_1;
			Outputs.UpVectorY = Variable_2;
			Outputs.UpVectorZ = Variable_0;
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
			
			TVoxelRange<v_flt> Variable_9; // + output 0
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
					
					// Init of 3D IQ Noise
					_3D_IQ_Noise_2_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_IQ_Noise_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_3D_IQ_Noise_2_Noise.SetFractalOctavesAndGain(15, 0.6);
					_3D_IQ_Noise_2_Noise.SetFractalLacunarity(2.0);
					_3D_IQ_Noise_2_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_IQ_Noise_2_Noise.SetMatrixFromRotation_3D(FRotator(40.000000, 45.000000, 50.000000));
					_3D_IQ_Noise_2_LODToOctaves[0] = 15;
					_3D_IQ_Noise_2_LODToOctaves[1] = 15;
					_3D_IQ_Noise_2_LODToOctaves[2] = 15;
					_3D_IQ_Noise_2_LODToOctaves[3] = 15;
					_3D_IQ_Noise_2_LODToOctaves[4] = 15;
					_3D_IQ_Noise_2_LODToOctaves[5] = 15;
					_3D_IQ_Noise_2_LODToOctaves[6] = 15;
					_3D_IQ_Noise_2_LODToOctaves[7] = 15;
					_3D_IQ_Noise_2_LODToOctaves[8] = 15;
					_3D_IQ_Noise_2_LODToOctaves[9] = 15;
					_3D_IQ_Noise_2_LODToOctaves[10] = 15;
					_3D_IQ_Noise_2_LODToOctaves[11] = 15;
					_3D_IQ_Noise_2_LODToOctaves[12] = 15;
					_3D_IQ_Noise_2_LODToOctaves[13] = 15;
					_3D_IQ_Noise_2_LODToOctaves[14] = 15;
					_3D_IQ_Noise_2_LODToOctaves[15] = 15;
					_3D_IQ_Noise_2_LODToOctaves[16] = 15;
					_3D_IQ_Noise_2_LODToOctaves[17] = 15;
					_3D_IQ_Noise_2_LODToOctaves[18] = 15;
					_3D_IQ_Noise_2_LODToOctaves[19] = 15;
					_3D_IQ_Noise_2_LODToOctaves[20] = 15;
					_3D_IQ_Noise_2_LODToOctaves[21] = 15;
					_3D_IQ_Noise_2_LODToOctaves[22] = 15;
					_3D_IQ_Noise_2_LODToOctaves[23] = 15;
					_3D_IQ_Noise_2_LODToOctaves[24] = 15;
					_3D_IQ_Noise_2_LODToOctaves[25] = 15;
					_3D_IQ_Noise_2_LODToOctaves[26] = 15;
					_3D_IQ_Noise_2_LODToOctaves[27] = 15;
					_3D_IQ_Noise_2_LODToOctaves[28] = 15;
					_3D_IQ_Noise_2_LODToOctaves[29] = 15;
					_3D_IQ_Noise_2_LODToOctaves[30] = 15;
					_3D_IQ_Noise_2_LODToOctaves[31] = 15;
					
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
				// 3D IQ Noise
				TVoxelRange<v_flt> Variable_6; // 3D IQ Noise output 0
				TVoxelRange<v_flt> _3D_IQ_Noise_2_Temp_1; // 3D IQ Noise output 1
				TVoxelRange<v_flt> _3D_IQ_Noise_2_Temp_2; // 3D IQ Noise output 2
				TVoxelRange<v_flt> _3D_IQ_Noise_2_Temp_3; // 3D IQ Noise output 3
				Variable_6 = { -0.653693f, 0.750231f };
				_3D_IQ_Noise_2_Temp_1 = { -1.536367f, 1.653675f };
				_3D_IQ_Noise_2_Temp_2 = { -1.654880f, 1.681203f };
				_3D_IQ_Noise_2_Temp_3 = { -1.580102f, 1.625968f };
				
				// Noise Strength = 0.02
				TVoxelRange<v_flt> Variable_8; // Noise Strength = 0.02 output 0
				Variable_8 = Params.Noise_Strength;
				
				// Radius = 1000.0
				TVoxelRange<v_flt> Variable_5; // Radius = 1000.0 output 0
				Variable_5 = Params.Radius;
				
				// -
				TVoxelRange<v_flt> Variable_12; // - output 0
				Variable_12 = Variable_6 - TVoxelRange<v_flt>(0.1f);
				
				// /
				TVoxelRange<v_flt> Variable_11; // / output 0
				Variable_11 = Variable_12 / TVoxelRange<v_flt>(0.5f);
				
				// Float Curve: PlanetCurve
				TVoxelRange<v_flt> Variable_10; // Float Curve: PlanetCurve output 0
				Variable_10 = FVoxelNodeFunctions::GetCurveValue(Params.PlanetCurve, Variable_11);
				
				// *
				TVoxelRange<v_flt> Variable_7; // * output 0
				Variable_7 = Variable_5 * Variable_10 * Variable_8;
				
				// +
				BufferConstant.Variable_9 = Variable_5 + Variable_7;
				
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
		
		FVoxelFastNoise _3D_IQ_Noise_2_Noise;
		TStaticArray<uint8, 32> _3D_IQ_Noise_2_LODToOctaves;
		
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
			// Y
			TVoxelRange<v_flt> Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// X
			TVoxelRange<v_flt> Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Z
			TVoxelRange<v_flt> Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Vector Length
			TVoxelRange<v_flt> Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// -
			TVoxelRange<v_flt> Variable_4; // - output 0
			Variable_4 = Variable_3 - BufferConstant.Variable_9;
			
			Outputs.Value = Variable_4;
		}
		
	};
	
	FVoxelExample_PlanetInstance(UVoxelExample_Planet& Object)
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
			Object.Frequency,
			Object.Noise_Seed,
			Object.Noise_Strength,
			FVoxelColorRichCurve(Object.PlanetColorCurve.LoadSynchronous()),
			FVoxelRichCurve(Object.PlanetCurve.LoadSynchronous()),
			Object.Radius
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
inline v_flt FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_PlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_PlanetInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_PlanetInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_PlanetInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_PlanetInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Planet::UVoxelExample_Planet()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_Planet::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_PlanetInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_Planet. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_Planet. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Planet. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Planet. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
