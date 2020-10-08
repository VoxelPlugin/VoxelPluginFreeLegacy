// Copyright 2020 Phyronnaz

#include "VoxelExample_Ravines.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_RavinesInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_RavinesInstance, UVoxelExample_Ravines>
{
public:
	struct FParams
	{
		const float _3D_Noise_Frequency;
		const int32 _3D_Noise_Seed;
		const float Bottom_Transition_Smoothness;
		const float Height;
		const float Top_Transition_Smoothness;
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
			
			v_flt Variable_9; // Top Transition Smoothness = 5.0 output 0
			v_flt Variable_10; // Bottom Transition Smoothness = 5.0 output 0
			v_flt Variable_11; //  3D Noise Frequency = 0.02 output 0
			v_flt Variable_5; // Height = 50.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_0; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
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
					
					// Init of 3D Noise Seed = 1443
					FVoxelGraphSeed Variable_12; // 3D Noise Seed = 1443 output 0
					Variable_12 = Params._3D_Noise_Seed;
					
					
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
				// Top Transition Smoothness = 5.0
				BufferConstant.Variable_9 = Params.Top_Transition_Smoothness;
				
				// Bottom Transition Smoothness = 5.0
				BufferConstant.Variable_10 = Params.Bottom_Transition_Smoothness;
				
				//  3D Noise Frequency = 0.02
				BufferConstant.Variable_11 = Params._3D_Noise_Frequency;
				
				// Height = 50.0
				BufferConstant.Variable_5 = Params.Height;
				
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
		
		FVoxelFastNoise _3D_Perlin_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _3D_Perlin_Noise_Fractal_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 3D Noise Seed = 1443
			FVoxelGraphSeed Variable_12; // 3D Noise Seed = 1443 output 0
			Variable_12 = Params._3D_Noise_Seed;
			
			// Init of 3D Perlin Noise Fractal
			_3D_Perlin_Noise_Fractal_0_Noise.SetSeed(Variable_12);
			_3D_Perlin_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(1, 0.5);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[0] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[1] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[2] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[3] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[4] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[5] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[6] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[7] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[8] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[9] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[10] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[11] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[12] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[13] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[14] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[15] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[16] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[17] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[18] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[19] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[20] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[21] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[22] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[23] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[24] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[25] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[26] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[27] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[28] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[29] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[30] = 1;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[31] = 1;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// +
			v_flt Variable_7; // + output 0
			Variable_7 = Variable_8 + BufferConstant.Variable_5;
			
			// 3D Perlin Noise Fractal
			v_flt Variable_3; // 3D Perlin Noise Fractal output 0
			Variable_3 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(BufferX.Variable_0, BufferXY.Variable_1, Variable_2, BufferConstant.Variable_11, _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_3 = FMath::Clamp<v_flt>(Variable_3, -0.852398, 0.865098);
			
			// *
			v_flt Variable_4; // * output 0
			Variable_4 = Variable_3 * v_flt(5.0f);
			
			// Smooth Intersection.-
			v_flt Variable_20; // Smooth Intersection.- output 0
			Variable_20 = Variable_6 - Variable_4;
			
			// Smooth Intersection./
			v_flt Variable_13; // Smooth Intersection./ output 0
			Variable_13 = Variable_20 / BufferConstant.Variable_9;
			
			// Smooth Intersection.*
			v_flt Variable_14; // Smooth Intersection.* output 0
			Variable_14 = Variable_13 * v_flt(0.5f);
			
			// Smooth Intersection.-
			v_flt Variable_21; // Smooth Intersection.- output 0
			Variable_21 = v_flt(0.5f) - Variable_14;
			
			// Smooth Intersection.Clamp
			v_flt Variable_15; // Smooth Intersection.Clamp output 0
			Variable_15 = FVoxelNodeFunctions::Clamp(Variable_21, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Intersection.Lerp
			v_flt Variable_16; // Smooth Intersection.Lerp output 0
			Variable_16 = FVoxelNodeFunctions::Lerp(Variable_6, Variable_4, Variable_15);
			
			// Smooth Intersection.1 - X
			v_flt Variable_18; // Smooth Intersection.1 - X output 0
			Variable_18 = 1 - Variable_15;
			
			// Smooth Intersection.*
			v_flt Variable_17; // Smooth Intersection.* output 0
			Variable_17 = BufferConstant.Variable_9 * Variable_15 * Variable_18;
			
			// Smooth Intersection.+
			v_flt Variable_19; // Smooth Intersection.+ output 0
			Variable_19 = Variable_16 + Variable_17;
			
			// Smooth Union.-
			v_flt Variable_22; // Smooth Union.- output 0
			Variable_22 = Variable_7 - Variable_19;
			
			// Smooth Union./
			v_flt Variable_23; // Smooth Union./ output 0
			Variable_23 = Variable_22 / BufferConstant.Variable_10;
			
			// Smooth Union.*
			v_flt Variable_24; // Smooth Union.* output 0
			Variable_24 = Variable_23 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_25; // Smooth Union.+ output 0
			Variable_25 = Variable_24 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_26; // Smooth Union.Clamp output 0
			Variable_26 = FVoxelNodeFunctions::Clamp(Variable_25, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.Lerp
			v_flt Variable_27; // Smooth Union.Lerp output 0
			Variable_27 = FVoxelNodeFunctions::Lerp(Variable_7, Variable_19, Variable_26);
			
			// Smooth Union.1 - X
			v_flt Variable_30; // Smooth Union.1 - X output 0
			Variable_30 = 1 - Variable_26;
			
			// Smooth Union.*
			v_flt Variable_29; // Smooth Union.* output 0
			Variable_29 = BufferConstant.Variable_10 * Variable_26 * Variable_30;
			
			// Smooth Union.-
			v_flt Variable_28; // Smooth Union.- output 0
			Variable_28 = Variable_27 - Variable_29;
			
			Outputs.Value = Variable_28;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			v_flt Variable_8; // Z output 0
			Variable_8 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// +
			v_flt Variable_7; // + output 0
			Variable_7 = Variable_8 + BufferConstant.Variable_5;
			
			// 3D Perlin Noise Fractal
			v_flt Variable_3; // 3D Perlin Noise Fractal output 0
			Variable_3 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(Variable_0, Variable_1, Variable_2, BufferConstant.Variable_11, _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_3 = FMath::Clamp<v_flt>(Variable_3, -0.852398, 0.865098);
			
			// *
			v_flt Variable_4; // * output 0
			Variable_4 = Variable_3 * v_flt(5.0f);
			
			// Smooth Intersection.-
			v_flt Variable_20; // Smooth Intersection.- output 0
			Variable_20 = Variable_6 - Variable_4;
			
			// Smooth Intersection./
			v_flt Variable_13; // Smooth Intersection./ output 0
			Variable_13 = Variable_20 / BufferConstant.Variable_9;
			
			// Smooth Intersection.*
			v_flt Variable_14; // Smooth Intersection.* output 0
			Variable_14 = Variable_13 * v_flt(0.5f);
			
			// Smooth Intersection.-
			v_flt Variable_21; // Smooth Intersection.- output 0
			Variable_21 = v_flt(0.5f) - Variable_14;
			
			// Smooth Intersection.Clamp
			v_flt Variable_15; // Smooth Intersection.Clamp output 0
			Variable_15 = FVoxelNodeFunctions::Clamp(Variable_21, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Intersection.Lerp
			v_flt Variable_16; // Smooth Intersection.Lerp output 0
			Variable_16 = FVoxelNodeFunctions::Lerp(Variable_6, Variable_4, Variable_15);
			
			// Smooth Intersection.1 - X
			v_flt Variable_18; // Smooth Intersection.1 - X output 0
			Variable_18 = 1 - Variable_15;
			
			// Smooth Intersection.*
			v_flt Variable_17; // Smooth Intersection.* output 0
			Variable_17 = BufferConstant.Variable_9 * Variable_15 * Variable_18;
			
			// Smooth Intersection.+
			v_flt Variable_19; // Smooth Intersection.+ output 0
			Variable_19 = Variable_16 + Variable_17;
			
			// Smooth Union.-
			v_flt Variable_22; // Smooth Union.- output 0
			Variable_22 = Variable_7 - Variable_19;
			
			// Smooth Union./
			v_flt Variable_23; // Smooth Union./ output 0
			Variable_23 = Variable_22 / BufferConstant.Variable_10;
			
			// Smooth Union.*
			v_flt Variable_24; // Smooth Union.* output 0
			Variable_24 = Variable_23 * v_flt(0.5f);
			
			// Smooth Union.+
			v_flt Variable_25; // Smooth Union.+ output 0
			Variable_25 = Variable_24 + v_flt(0.5f);
			
			// Smooth Union.Clamp
			v_flt Variable_26; // Smooth Union.Clamp output 0
			Variable_26 = FVoxelNodeFunctions::Clamp(Variable_25, v_flt(0.0f), v_flt(1.0f));
			
			// Smooth Union.Lerp
			v_flt Variable_27; // Smooth Union.Lerp output 0
			Variable_27 = FVoxelNodeFunctions::Lerp(Variable_7, Variable_19, Variable_26);
			
			// Smooth Union.1 - X
			v_flt Variable_30; // Smooth Union.1 - X output 0
			Variable_30 = 1 - Variable_26;
			
			// Smooth Union.*
			v_flt Variable_29; // Smooth Union.* output 0
			Variable_29 = BufferConstant.Variable_10 * Variable_26 * Variable_30;
			
			// Smooth Union.-
			v_flt Variable_28; // Smooth Union.- output 0
			Variable_28 = Variable_27 - Variable_29;
			
			Outputs.Value = Variable_28;
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
			
			TVoxelRange<v_flt> Variable_7; // Bottom Transition Smoothness = 5.0 output 0
			TVoxelRange<v_flt> Variable_2; // Height = 50.0 output 0
			TVoxelRange<v_flt> Variable_6; // Top Transition Smoothness = 5.0 output 0
			TVoxelRange<v_flt> Variable_1; // * output 0
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
					
					// Init of 3D Perlin Noise Fractal
					_3D_Perlin_Noise_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_Perlin_Noise_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(1, 0.5);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[0] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[1] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[2] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[3] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[4] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[5] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[6] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[7] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[8] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[9] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[10] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[11] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[12] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[13] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[14] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[15] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[16] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[17] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[18] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[19] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[20] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[21] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[22] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[23] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[24] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[25] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[26] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[27] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[28] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[29] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[30] = 1;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[31] = 1;
					
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
				// Bottom Transition Smoothness = 5.0
				BufferConstant.Variable_7 = Params.Bottom_Transition_Smoothness;
				
				// Height = 50.0
				BufferConstant.Variable_2 = Params.Height;
				
				// Top Transition Smoothness = 5.0
				BufferConstant.Variable_6 = Params.Top_Transition_Smoothness;
				
				// 3D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_0; // 3D Perlin Noise Fractal output 0
				Variable_0 = { -0.852398f, 0.865098f };
				
				// *
				BufferConstant.Variable_1 = Variable_0 * TVoxelRange<v_flt>(5.0f);
				
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
			TVoxelRange<v_flt> Variable_3; // Z output 0
			Variable_3 = Context.GetLocalZ();
			
			// +
			TVoxelRange<v_flt> Variable_4; // + output 0
			Variable_4 = Variable_5 + BufferConstant.Variable_2;
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_15; // Smooth Intersection.- output 0
			Variable_15 = Variable_3 - BufferConstant.Variable_1;
			
			// Smooth Intersection./
			TVoxelRange<v_flt> Variable_8; // Smooth Intersection./ output 0
			Variable_8 = Variable_15 / BufferConstant.Variable_6;
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_9; // Smooth Intersection.* output 0
			Variable_9 = Variable_8 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Intersection.-
			TVoxelRange<v_flt> Variable_16; // Smooth Intersection.- output 0
			Variable_16 = TVoxelRange<v_flt>(0.5f) - Variable_9;
			
			// Smooth Intersection.Clamp
			TVoxelRange<v_flt> Variable_10; // Smooth Intersection.Clamp output 0
			Variable_10 = FVoxelNodeFunctions::Clamp(Variable_16, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Intersection.1 - X
			TVoxelRange<v_flt> Variable_13; // Smooth Intersection.1 - X output 0
			Variable_13 = 1 - Variable_10;
			
			// Smooth Intersection.Lerp
			TVoxelRange<v_flt> Variable_11; // Smooth Intersection.Lerp output 0
			Variable_11 = FVoxelNodeFunctions::Lerp(Variable_3, BufferConstant.Variable_1, Variable_10);
			
			// Smooth Intersection.*
			TVoxelRange<v_flt> Variable_12; // Smooth Intersection.* output 0
			Variable_12 = BufferConstant.Variable_6 * Variable_10 * Variable_13;
			
			// Smooth Intersection.+
			TVoxelRange<v_flt> Variable_14; // Smooth Intersection.+ output 0
			Variable_14 = Variable_11 + Variable_12;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_17; // Smooth Union.- output 0
			Variable_17 = Variable_4 - Variable_14;
			
			// Smooth Union./
			TVoxelRange<v_flt> Variable_18; // Smooth Union./ output 0
			Variable_18 = Variable_17 / BufferConstant.Variable_7;
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_19; // Smooth Union.* output 0
			Variable_19 = Variable_18 * TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.+
			TVoxelRange<v_flt> Variable_20; // Smooth Union.+ output 0
			Variable_20 = Variable_19 + TVoxelRange<v_flt>(0.5f);
			
			// Smooth Union.Clamp
			TVoxelRange<v_flt> Variable_21; // Smooth Union.Clamp output 0
			Variable_21 = FVoxelNodeFunctions::Clamp(Variable_20, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f));
			
			// Smooth Union.1 - X
			TVoxelRange<v_flt> Variable_25; // Smooth Union.1 - X output 0
			Variable_25 = 1 - Variable_21;
			
			// Smooth Union.Lerp
			TVoxelRange<v_flt> Variable_22; // Smooth Union.Lerp output 0
			Variable_22 = FVoxelNodeFunctions::Lerp(Variable_4, Variable_14, Variable_21);
			
			// Smooth Union.*
			TVoxelRange<v_flt> Variable_24; // Smooth Union.* output 0
			Variable_24 = BufferConstant.Variable_7 * Variable_21 * Variable_25;
			
			// Smooth Union.-
			TVoxelRange<v_flt> Variable_23; // Smooth Union.- output 0
			Variable_23 = Variable_22 - Variable_24;
			
			Outputs.Value = Variable_23;
		}
		
	};
	
	FVoxelExample_RavinesInstance(UVoxelExample_Ravines& Object)
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
			Object._3D_Noise_Frequency,
			Object._3D_Noise_Seed,
			Object.Bottom_Transition_Smoothness,
			Object.Height,
			Object.Top_Transition_Smoothness
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
inline v_flt FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_RavinesInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_RavinesInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_RavinesInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_RavinesInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_RavinesInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Ravines::UVoxelExample_Ravines()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_Ravines::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_RavinesInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_Ravines. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_Ravines. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Ravines. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Ravines. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
