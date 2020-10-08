// Copyright 2020 Phyronnaz

#include "VG_Example_Dunes.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVG_Example_DunesInstance : public TVoxelGraphGeneratorInstanceHelper<FVG_Example_DunesInstance, UVG_Example_Dunes>
{
public:
	struct FParams
	{
		const float Direction_X;
		const float Direction_Y;
		const float Dune_Frequency;
		const float Height;
		const float Noise_Frequency;
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
			
			v_flt Variable_17; // Dune Frequency = 0.002 output 0
			v_flt Variable_18; // Noise Frequency = 0.001 output 0
			v_flt Variable_19; // Height = 75.0 output 0
			v_flt Variable_23; // Normalize./ output 0
			v_flt Variable_22; // Normalize./ output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_4; // X output 0
			v_flt Variable_27; // vector2 * float.* output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_2; // * output 0
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
				// Direction Y = 1.0
				v_flt Variable_14; // Direction Y = 1.0 output 0
				Variable_14 = Params.Direction_Y;
				
				// Dune Frequency = 0.002
				BufferConstant.Variable_17 = Params.Dune_Frequency;
				
				// Noise Frequency = 0.001
				BufferConstant.Variable_18 = Params.Noise_Frequency;
				
				// Height = 75.0
				BufferConstant.Variable_19 = Params.Height;
				
				// Direction X = 0.4
				v_flt Variable_13; // Direction X = 0.4 output 0
				Variable_13 = Params.Direction_X;
				
				// Normalize.Vector Length
				v_flt Variable_21; // Normalize.Vector Length output 0
				Variable_21 = FVoxelNodeFunctions::VectorLength(Variable_13, Variable_14, v_flt(0.0f));
				
				// Normalize.==
				bool Variable_24; // Normalize.== output 0
				Variable_24 = Variable_21 == v_flt(0.0f);
				
				// Normalize.Switch (float)
				v_flt Variable_25; // Normalize.Switch (float) output 0
				Variable_25 = FVoxelNodeFunctions::Switch(v_flt(1.0f), Variable_21, Variable_24);
				
				// Normalize./
				BufferConstant.Variable_23 = Variable_14 / Variable_25;
				
				// Normalize./
				BufferConstant.Variable_22 = Variable_13 / Variable_25;
				
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
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 2D Perlin Noise Fractal
			_2D_Perlin_Noise_Fractal_0_Noise.SetSeed(FVoxelGraphSeed(1337));
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
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			v_flt Variable_11; // X output 0
			Variable_11 = Context.GetLocalX();
			
			// X
			BufferX.Variable_4 = Context.GetLocalX();
			
			// vector2 * vector2.*
			v_flt Variable_26; // vector2 * vector2.* output 0
			Variable_26 = Variable_11 * BufferConstant.Variable_22;
			
			// vector2 * float.*
			BufferX.Variable_27 = Variable_26 * BufferConstant.Variable_17;
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_9; // 2D Perlin Noise Fractal output 0
			Variable_9 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_4, Variable_3, BufferConstant.Variable_18, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_9 = FMath::Clamp<v_flt>(Variable_9, -0.663838, 0.649431);
			
			// vector2 * vector2.*
			v_flt Variable_15; // vector2 * vector2.* output 0
			Variable_15 = Variable_12 * BufferConstant.Variable_23;
			
			// vector2 * float.*
			v_flt Variable_16; // vector2 * float.* output 0
			Variable_16 = Variable_15 * BufferConstant.Variable_17;
			
			// +
			v_flt Variable_10; // + output 0
			Variable_10 = Variable_9 + BufferX.Variable_27 + Variable_16;
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = v_flt(3.141593f) * Variable_10;
			
			// SIN
			v_flt Variable_5; // SIN output 0
			Variable_5 = FVoxelNodeFunctions::Sin(Variable_8);
			
			// ABS
			v_flt Variable_6; // ABS output 0
			Variable_6 = FVoxelNodeFunctions::Abs(Variable_5);
			
			// * -1
			v_flt Variable_7; // * -1 output 0
			Variable_7 = Variable_6 * -1;
			
			// *
			BufferXY.Variable_2 = Variable_7 * BufferConstant.Variable_19;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// X
			v_flt Variable_11; // X output 0
			Variable_11 = Context.GetLocalX();
			
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// X
			BufferX.Variable_4 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_9; // 2D Perlin Noise Fractal output 0
			Variable_9 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(BufferX.Variable_4, Variable_3, BufferConstant.Variable_18, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_9 = FMath::Clamp<v_flt>(Variable_9, -0.663838, 0.649431);
			
			// vector2 * vector2.*
			v_flt Variable_15; // vector2 * vector2.* output 0
			Variable_15 = Variable_12 * BufferConstant.Variable_23;
			
			// vector2 * vector2.*
			v_flt Variable_26; // vector2 * vector2.* output 0
			Variable_26 = Variable_11 * BufferConstant.Variable_22;
			
			// vector2 * float.*
			BufferX.Variable_27 = Variable_26 * BufferConstant.Variable_17;
			
			// vector2 * float.*
			v_flt Variable_16; // vector2 * float.* output 0
			Variable_16 = Variable_15 * BufferConstant.Variable_17;
			
			// +
			v_flt Variable_10; // + output 0
			Variable_10 = Variable_9 + BufferX.Variable_27 + Variable_16;
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = v_flt(3.141593f) * Variable_10;
			
			// SIN
			v_flt Variable_5; // SIN output 0
			Variable_5 = FVoxelNodeFunctions::Sin(Variable_8);
			
			// ABS
			v_flt Variable_6; // ABS output 0
			Variable_6 = FVoxelNodeFunctions::Abs(Variable_5);
			
			// * -1
			v_flt Variable_7; // * -1 output 0
			Variable_7 = Variable_6 * -1;
			
			// *
			BufferXY.Variable_2 = Variable_7 * BufferConstant.Variable_19;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// -
			v_flt Variable_1; // - output 0
			Variable_1 = Variable_0 - BufferXY.Variable_2;
			
			// Set High Quality Value.*
			v_flt Variable_20; // Set High Quality Value.* output 0
			Variable_20 = Variable_1 * v_flt(0.2f);
			
			Outputs.Value = Variable_20;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// X
			v_flt Variable_11; // X output 0
			Variable_11 = Context.GetLocalX();
			
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_9; // 2D Perlin Noise Fractal output 0
			Variable_9 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D(Variable_4, Variable_3, BufferConstant.Variable_18, _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_9 = FMath::Clamp<v_flt>(Variable_9, -0.663838, 0.649431);
			
			// vector2 * vector2.*
			v_flt Variable_15; // vector2 * vector2.* output 0
			Variable_15 = Variable_12 * BufferConstant.Variable_23;
			
			// vector2 * vector2.*
			v_flt Variable_26; // vector2 * vector2.* output 0
			Variable_26 = Variable_11 * BufferConstant.Variable_22;
			
			// vector2 * float.*
			v_flt Variable_27; // vector2 * float.* output 0
			Variable_27 = Variable_26 * BufferConstant.Variable_17;
			
			// vector2 * float.*
			v_flt Variable_16; // vector2 * float.* output 0
			Variable_16 = Variable_15 * BufferConstant.Variable_17;
			
			// +
			v_flt Variable_10; // + output 0
			Variable_10 = Variable_9 + Variable_27 + Variable_16;
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = v_flt(3.141593f) * Variable_10;
			
			// SIN
			v_flt Variable_5; // SIN output 0
			Variable_5 = FVoxelNodeFunctions::Sin(Variable_8);
			
			// ABS
			v_flt Variable_6; // ABS output 0
			Variable_6 = FVoxelNodeFunctions::Abs(Variable_5);
			
			// * -1
			v_flt Variable_7; // * -1 output 0
			Variable_7 = Variable_6 * -1;
			
			// *
			v_flt Variable_2; // * output 0
			Variable_2 = Variable_7 * BufferConstant.Variable_19;
			
			// -
			v_flt Variable_1; // - output 0
			Variable_1 = Variable_0 - Variable_2;
			
			// Set High Quality Value.*
			v_flt Variable_20; // Set High Quality Value.* output 0
			Variable_20 = Variable_1 * v_flt(0.2f);
			
			Outputs.Value = Variable_20;
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
			
			TVoxelRange<v_flt> Variable_13; // Dune Frequency = 0.002 output 0
			TVoxelRange<v_flt> Variable_14; // Height = 75.0 output 0
			TVoxelRange<v_flt> Variable_16; // Normalize.Range Union output 0
			TVoxelRange<v_flt> Variable_7; // 2D Perlin Noise Fractal output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_18; // vector2 * float.* output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_2; // * output 0
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
					_2D_Perlin_Noise_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
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
				
				////////////////////////////////////////////////////
				//////////////////// Other inits ///////////////////
				////////////////////////////////////////////////////
				Function0_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Dune Frequency = 0.002
				BufferConstant.Variable_13 = Params.Dune_Frequency;
				
				// Height = 75.0
				BufferConstant.Variable_14 = Params.Height;
				
				// Normalize.Range Union
				BufferConstant.Variable_16 = FVoxelNodeFunctions::Union(TVoxelRange<v_flt>(-1.0f), TVoxelRange<v_flt>(1.0f));
				
				// 2D Perlin Noise Fractal
				BufferConstant.Variable_7 = { -0.663838f, 0.649431f };
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_1_LODToOctaves;
		
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
			TVoxelRange<v_flt> Variable_10; // Y output 0
			Variable_10 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_9; // X output 0
			Variable_9 = Context.GetLocalX();
			
			// vector2 * vector2.*
			TVoxelRange<v_flt> Variable_11; // vector2 * vector2.* output 0
			Variable_11 = Variable_10 * BufferConstant.Variable_16;
			
			// vector2 * vector2.*
			TVoxelRange<v_flt> Variable_17; // vector2 * vector2.* output 0
			Variable_17 = Variable_9 * BufferConstant.Variable_16;
			
			// vector2 * float.*
			TVoxelRange<v_flt> Variable_12; // vector2 * float.* output 0
			Variable_12 = Variable_11 * BufferConstant.Variable_13;
			
			// vector2 * float.*
			TVoxelRange<v_flt> Variable_18; // vector2 * float.* output 0
			Variable_18 = Variable_17 * BufferConstant.Variable_13;
			
			// +
			TVoxelRange<v_flt> Variable_8; // + output 0
			Variable_8 = BufferConstant.Variable_7 + Variable_18 + Variable_12;
			
			// *
			TVoxelRange<v_flt> Variable_6; // * output 0
			Variable_6 = TVoxelRange<v_flt>(3.141593f) * Variable_8;
			
			// SIN
			TVoxelRange<v_flt> Variable_3; // SIN output 0
			Variable_3 = FVoxelNodeFunctions::Sin(Variable_6);
			
			// ABS
			TVoxelRange<v_flt> Variable_4; // ABS output 0
			Variable_4 = FVoxelNodeFunctions::Abs(Variable_3);
			
			// * -1
			TVoxelRange<v_flt> Variable_5; // * -1 output 0
			Variable_5 = Variable_4 * -1;
			
			// *
			TVoxelRange<v_flt> Variable_2; // * output 0
			Variable_2 = Variable_5 * BufferConstant.Variable_14;
			
			// -
			TVoxelRange<v_flt> Variable_1; // - output 0
			Variable_1 = Variable_0 - Variable_2;
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_15; // Set High Quality Value.* output 0
			Variable_15 = Variable_1 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_15;
		}
		
	};
	
	FVG_Example_DunesInstance(UVG_Example_Dunes& Object)
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
			Object.Direction_X,
			Object.Direction_Y,
			Object.Dune_Frequency,
			Object.Height,
			Object.Noise_Frequency
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
inline v_flt FVG_Example_DunesInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVG_Example_DunesInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVG_Example_DunesInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_DunesInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVG_Example_DunesInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVG_Example_DunesInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVG_Example_DunesInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVG_Example_DunesInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVG_Example_Dunes::UVG_Example_Dunes()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVG_Example_Dunes::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVG_Example_DunesInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VG_Example_Dunes. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VG_Example_Dunes. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Dunes. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Dunes. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
