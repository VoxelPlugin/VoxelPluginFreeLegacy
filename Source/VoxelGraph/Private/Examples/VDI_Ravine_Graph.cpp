// Copyright 2020 Phyronnaz

#include "VDI_Ravine_Graph.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVDI_Ravine_GraphInstance : public TVoxelGraphGeneratorInstanceHelper<FVDI_Ravine_GraphInstance, UVDI_Ravine_Graph>
{
public:
	struct FParams
	{
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
			
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_6; // Data Item Parameters output 6
			v_flt Variable_7; // X output 0
			v_flt Variable_28; // vector - vector.- output 0
			v_flt Variable_27; // vector - vector.- output 0
			v_flt Variable_21; // vector - vector.- output 0
			v_flt Variable_34; // vector / float./ output 0
			v_flt Variable_16; // vector / float./ output 0
			v_flt Variable_33; // vector / float./ output 0
			v_flt Variable_20; // + output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_8; // Y output 0
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
		
		FVoxelFastNoise _3D_Gradient_Perturb_Fractal_0_Noise;
		TStaticArray<uint8, 32> _3D_Gradient_Perturb_Fractal_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 3D Gradient Perturb Fractal
			_3D_Gradient_Perturb_Fractal_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Gradient_Perturb_Fractal_0_Noise.SetFractalOctavesAndGain(10, 0.5);
			_3D_Gradient_Perturb_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_3D_Gradient_Perturb_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[0] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[1] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[2] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[3] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[4] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[5] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[6] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[7] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[8] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[9] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[10] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[11] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[12] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[13] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[14] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[15] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[16] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[17] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[18] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[19] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[20] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[21] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[22] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[23] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[24] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[25] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[26] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[27] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[28] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[29] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[30] = 10;
			_3D_Gradient_Perturb_Fractal_0_LODToOctaves[31] = 10;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// Data Item Parameters
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				Variable_0 = 0.000000;
				Variable_1 = 100.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// X
			BufferX.Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			BufferX.Variable_28 = Variable_4 - Variable_1;
			
			// vector - vector.-
			v_flt Variable_35; // vector - vector.- output 0
			Variable_35 = Variable_0 - Variable_3;
			
			// vector - vector.-
			BufferX.Variable_27 = Variable_3 - Variable_0;
			
			// vector - vector.-
			BufferX.Variable_21 = Variable_5 - Variable_2;
			
			// /
			v_flt Variable_22; // / output 0
			Variable_22 = BufferX.Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_15; // vector + vector.+ output 0
			Variable_15 = Variable_2 + Variable_5;
			
			// vector + vector.+
			v_flt Variable_31; // vector + vector.+ output 0
			Variable_31 = Variable_0 + Variable_3;
			
			// vector - vector.-
			v_flt Variable_17; // vector - vector.- output 0
			Variable_17 = Variable_2 - Variable_5;
			
			// vector + vector.+
			v_flt Variable_32; // vector + vector.+ output 0
			Variable_32 = Variable_1 + Variable_4;
			
			// vector - vector.-
			v_flt Variable_36; // vector - vector.- output 0
			Variable_36 = Variable_1 - Variable_4;
			
			// vector / float./
			BufferX.Variable_34 = Variable_32 / v_flt(2.0f);
			
			// vector / float./
			BufferX.Variable_16 = Variable_15 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_18; // Vector Length output 0
			Variable_18 = FVoxelNodeFunctions::VectorLength(Variable_35, Variable_36, Variable_17);
			
			// vector / float./
			BufferX.Variable_33 = Variable_31 / v_flt(2.0f);
			
			// /
			v_flt Variable_19; // / output 0
			Variable_19 = Variable_18 / v_flt(2.0f);
			
			// +
			BufferX.Variable_20 = Variable_19 + Variable_22;
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_8 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_8 = Context.GetLocalY();
			
			// Data Item Parameters
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				Variable_0 = 0.000000;
				Variable_1 = 100.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// X
			BufferX.Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			BufferX.Variable_28 = Variable_4 - Variable_1;
			
			// vector - vector.-
			v_flt Variable_35; // vector - vector.- output 0
			Variable_35 = Variable_0 - Variable_3;
			
			// vector - vector.-
			BufferX.Variable_27 = Variable_3 - Variable_0;
			
			// vector - vector.-
			BufferX.Variable_21 = Variable_5 - Variable_2;
			
			// /
			v_flt Variable_22; // / output 0
			Variable_22 = BufferX.Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_15; // vector + vector.+ output 0
			Variable_15 = Variable_2 + Variable_5;
			
			// vector + vector.+
			v_flt Variable_31; // vector + vector.+ output 0
			Variable_31 = Variable_0 + Variable_3;
			
			// vector - vector.-
			v_flt Variable_17; // vector - vector.- output 0
			Variable_17 = Variable_2 - Variable_5;
			
			// vector + vector.+
			v_flt Variable_32; // vector + vector.+ output 0
			Variable_32 = Variable_1 + Variable_4;
			
			// vector - vector.-
			v_flt Variable_36; // vector - vector.- output 0
			Variable_36 = Variable_1 - Variable_4;
			
			// vector / float./
			BufferX.Variable_34 = Variable_32 / v_flt(2.0f);
			
			// vector / float./
			BufferX.Variable_16 = Variable_15 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_18; // Vector Length output 0
			Variable_18 = FVoxelNodeFunctions::VectorLength(Variable_35, Variable_36, Variable_17);
			
			// vector / float./
			BufferX.Variable_33 = Variable_31 / v_flt(2.0f);
			
			// /
			v_flt Variable_19; // / output 0
			Variable_19 = Variable_18 / v_flt(2.0f);
			
			// +
			BufferX.Variable_20 = Variable_19 + Variable_22;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// 3D Gradient Perturb Fractal
			v_flt Variable_23; // 3D Gradient Perturb Fractal output 0
			v_flt Variable_24; // 3D Gradient Perturb Fractal output 1
			v_flt Variable_25; // 3D Gradient Perturb Fractal output 2
			Variable_23 = BufferX.Variable_7;
			Variable_24 = BufferXY.Variable_8;
			Variable_25 = Variable_9;
			_3D_Gradient_Perturb_Fractal_0_Noise.GradientPerturbFractal_3D(Variable_23, Variable_24, Variable_25, v_flt(0.001f), _3D_Gradient_Perturb_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], v_flt(200.0f));
			
			// vector - vector.-
			v_flt Variable_29; // vector - vector.- output 0
			Variable_29 = Variable_23 - BufferX.Variable_33;
			
			// vector - vector.-
			v_flt Variable_14; // vector - vector.- output 0
			Variable_14 = Variable_25 - BufferX.Variable_16;
			
			// vector - vector.-
			v_flt Variable_30; // vector - vector.- output 0
			Variable_30 = Variable_24 - BufferX.Variable_34;
			
			// Inverse Transform Position XZ
			v_flt Variable_11; // Inverse Transform Position XZ output 0
			v_flt Variable_12; // Inverse Transform Position XZ output 1
			v_flt Variable_13; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(BufferX.Variable_27, BufferX.Variable_28, BufferX.Variable_21, v_flt(0.0f), v_flt(0.0f), v_flt(1.0f), Variable_29, Variable_30, Variable_14, Variable_11, Variable_12, Variable_13);
			
			// *
			v_flt Variable_26; // * output 0
			Variable_26 = Variable_13 * v_flt(2.0f);
			
			// Triangular Prism SDF
			v_flt Variable_10; // Triangular Prism SDF output 0
			Variable_10 = FVoxelSDFNodeFunctions::TriPrism(Variable_12, Variable_11, Variable_26, BufferX.Variable_6, BufferX.Variable_20);
			
			Outputs.Value = Variable_10;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_8; // Y output 0
			Variable_8 = Context.GetLocalY();
			
			// Z
			v_flt Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// Data Item Parameters
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			v_flt Variable_6; // Data Item Parameters output 6
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				Variable_0 = 0.000000;
				Variable_1 = 100.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				Variable_6 = 40.000000;
			}
			
			// X
			v_flt Variable_7; // X output 0
			Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			v_flt Variable_28; // vector - vector.- output 0
			Variable_28 = Variable_4 - Variable_1;
			
			// vector - vector.-
			v_flt Variable_35; // vector - vector.- output 0
			Variable_35 = Variable_0 - Variable_3;
			
			// 3D Gradient Perturb Fractal
			v_flt Variable_23; // 3D Gradient Perturb Fractal output 0
			v_flt Variable_24; // 3D Gradient Perturb Fractal output 1
			v_flt Variable_25; // 3D Gradient Perturb Fractal output 2
			Variable_23 = Variable_7;
			Variable_24 = Variable_8;
			Variable_25 = Variable_9;
			_3D_Gradient_Perturb_Fractal_0_Noise.GradientPerturbFractal_3D(Variable_23, Variable_24, Variable_25, v_flt(0.001f), _3D_Gradient_Perturb_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], v_flt(200.0f));
			
			// vector - vector.-
			v_flt Variable_27; // vector - vector.- output 0
			Variable_27 = Variable_3 - Variable_0;
			
			// vector - vector.-
			v_flt Variable_21; // vector - vector.- output 0
			Variable_21 = Variable_5 - Variable_2;
			
			// /
			v_flt Variable_22; // / output 0
			Variable_22 = Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_15; // vector + vector.+ output 0
			Variable_15 = Variable_2 + Variable_5;
			
			// vector + vector.+
			v_flt Variable_31; // vector + vector.+ output 0
			Variable_31 = Variable_0 + Variable_3;
			
			// vector - vector.-
			v_flt Variable_17; // vector - vector.- output 0
			Variable_17 = Variable_2 - Variable_5;
			
			// vector + vector.+
			v_flt Variable_32; // vector + vector.+ output 0
			Variable_32 = Variable_1 + Variable_4;
			
			// vector - vector.-
			v_flt Variable_36; // vector - vector.- output 0
			Variable_36 = Variable_1 - Variable_4;
			
			// vector / float./
			v_flt Variable_34; // vector / float./ output 0
			Variable_34 = Variable_32 / v_flt(2.0f);
			
			// vector / float./
			v_flt Variable_16; // vector / float./ output 0
			Variable_16 = Variable_15 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_18; // Vector Length output 0
			Variable_18 = FVoxelNodeFunctions::VectorLength(Variable_35, Variable_36, Variable_17);
			
			// vector / float./
			v_flt Variable_33; // vector / float./ output 0
			Variable_33 = Variable_31 / v_flt(2.0f);
			
			// vector - vector.-
			v_flt Variable_29; // vector - vector.- output 0
			Variable_29 = Variable_23 - Variable_33;
			
			// /
			v_flt Variable_19; // / output 0
			Variable_19 = Variable_18 / v_flt(2.0f);
			
			// vector - vector.-
			v_flt Variable_14; // vector - vector.- output 0
			Variable_14 = Variable_25 - Variable_16;
			
			// vector - vector.-
			v_flt Variable_30; // vector - vector.- output 0
			Variable_30 = Variable_24 - Variable_34;
			
			// Inverse Transform Position XZ
			v_flt Variable_11; // Inverse Transform Position XZ output 0
			v_flt Variable_12; // Inverse Transform Position XZ output 1
			v_flt Variable_13; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(Variable_27, Variable_28, Variable_21, v_flt(0.0f), v_flt(0.0f), v_flt(1.0f), Variable_29, Variable_30, Variable_14, Variable_11, Variable_12, Variable_13);
			
			// +
			v_flt Variable_20; // + output 0
			Variable_20 = Variable_19 + Variable_22;
			
			// *
			v_flt Variable_26; // * output 0
			Variable_26 = Variable_13 * v_flt(2.0f);
			
			// Triangular Prism SDF
			v_flt Variable_10; // Triangular Prism SDF output 0
			Variable_10 = FVoxelSDFNodeFunctions::TriPrism(Variable_12, Variable_11, Variable_26, Variable_6, Variable_20);
			
			Outputs.Value = Variable_10;
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
			
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_6; // Data Item Parameters output 6
			TVoxelRange<v_flt> Variable_7; // X output 0
			TVoxelRange<v_flt> Variable_28; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_27; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_21; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_34; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_16; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_33; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_20; // + output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_8; // Y output 0
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
		
		FVoxelFastNoise _3D_Gradient_Perturb_Fractal_1_Noise;
		TStaticArray<uint8, 32> _3D_Gradient_Perturb_Fractal_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 3D Gradient Perturb Fractal
			_3D_Gradient_Perturb_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Gradient_Perturb_Fractal_1_Noise.SetFractalOctavesAndGain(10, 0.5);
			_3D_Gradient_Perturb_Fractal_1_Noise.SetFractalLacunarity(2.0);
			_3D_Gradient_Perturb_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[0] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[1] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[2] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[3] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[4] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[5] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[6] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[7] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[8] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[9] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[10] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[11] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[12] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[13] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[14] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[15] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[16] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[17] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[18] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[19] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[20] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[21] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[22] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[23] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[24] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[25] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[26] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[27] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[28] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[29] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[30] = 10;
			_3D_Gradient_Perturb_Fractal_1_LODToOctaves[31] = 10;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Y
			TVoxelRange<v_flt> Variable_8; // Y output 0
			Variable_8 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// Data Item Parameters
			TVoxelRange<v_flt> Variable_0; // Data Item Parameters output 0
			TVoxelRange<v_flt> Variable_1; // Data Item Parameters output 1
			TVoxelRange<v_flt> Variable_2; // Data Item Parameters output 2
			TVoxelRange<v_flt> Variable_3; // Data Item Parameters output 3
			TVoxelRange<v_flt> Variable_4; // Data Item Parameters output 4
			TVoxelRange<v_flt> Variable_5; // Data Item Parameters output 5
			TVoxelRange<v_flt> Variable_6; // Data Item Parameters output 6
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				Variable_0 = 0.000000;
				Variable_1 = 100.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				Variable_6 = 40.000000;
			}
			
			// X
			TVoxelRange<v_flt> Variable_7; // X output 0
			Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_28; // vector - vector.- output 0
			Variable_28 = Variable_4 - Variable_1;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_35; // vector - vector.- output 0
			Variable_35 = Variable_0 - Variable_3;
			
			// 3D Gradient Perturb Fractal
			TVoxelRange<v_flt> Variable_23; // 3D Gradient Perturb Fractal output 0
			TVoxelRange<v_flt> Variable_24; // 3D Gradient Perturb Fractal output 1
			TVoxelRange<v_flt> Variable_25; // 3D Gradient Perturb Fractal output 2
			Variable_23 = TVoxelRange<v_flt>::FromList(Variable_7.Min - 2 * TVoxelRange<v_flt>(200.0f).Max, Variable_7.Max + 2 * TVoxelRange<v_flt>(200.0f).Max);
			Variable_24 = TVoxelRange<v_flt>::FromList(Variable_8.Min - 2 * TVoxelRange<v_flt>(200.0f).Max, Variable_8.Max + 2 * TVoxelRange<v_flt>(200.0f).Max);
			Variable_25 = TVoxelRange<v_flt>::FromList(Variable_9.Min - 2 * TVoxelRange<v_flt>(200.0f).Max, Variable_9.Max + 2 * TVoxelRange<v_flt>(200.0f).Max);
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_27; // vector - vector.- output 0
			Variable_27 = Variable_3 - Variable_0;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_21; // vector - vector.- output 0
			Variable_21 = Variable_5 - Variable_2;
			
			// /
			TVoxelRange<v_flt> Variable_22; // / output 0
			Variable_22 = Variable_6 / TVoxelRange<v_flt>(4.0f);
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_15; // vector + vector.+ output 0
			Variable_15 = Variable_2 + Variable_5;
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_31; // vector + vector.+ output 0
			Variable_31 = Variable_0 + Variable_3;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_17; // vector - vector.- output 0
			Variable_17 = Variable_2 - Variable_5;
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_32; // vector + vector.+ output 0
			Variable_32 = Variable_1 + Variable_4;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_36; // vector - vector.- output 0
			Variable_36 = Variable_1 - Variable_4;
			
			// vector / float./
			TVoxelRange<v_flt> Variable_34; // vector / float./ output 0
			Variable_34 = Variable_32 / TVoxelRange<v_flt>(2.0f);
			
			// vector / float./
			TVoxelRange<v_flt> Variable_16; // vector / float./ output 0
			Variable_16 = Variable_15 / TVoxelRange<v_flt>(2.0f);
			
			// Vector Length
			TVoxelRange<v_flt> Variable_18; // Vector Length output 0
			Variable_18 = FVoxelNodeFunctions::VectorLength(Variable_35, Variable_36, Variable_17);
			
			// vector / float./
			TVoxelRange<v_flt> Variable_33; // vector / float./ output 0
			Variable_33 = Variable_31 / TVoxelRange<v_flt>(2.0f);
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_29; // vector - vector.- output 0
			Variable_29 = Variable_23 - Variable_33;
			
			// /
			TVoxelRange<v_flt> Variable_19; // / output 0
			Variable_19 = Variable_18 / TVoxelRange<v_flt>(2.0f);
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_14; // vector - vector.- output 0
			Variable_14 = Variable_25 - Variable_16;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_30; // vector - vector.- output 0
			Variable_30 = Variable_24 - Variable_34;
			
			// Inverse Transform Position XZ
			TVoxelRange<v_flt> Variable_11; // Inverse Transform Position XZ output 0
			TVoxelRange<v_flt> Variable_12; // Inverse Transform Position XZ output 1
			TVoxelRange<v_flt> Variable_13; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(Variable_27, Variable_28, Variable_21, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f), Variable_29, Variable_30, Variable_14, Variable_11, Variable_12, Variable_13);
			
			// +
			TVoxelRange<v_flt> Variable_20; // + output 0
			Variable_20 = Variable_19 + Variable_22;
			
			// *
			TVoxelRange<v_flt> Variable_26; // * output 0
			Variable_26 = Variable_13 * TVoxelRange<v_flt>(2.0f);
			
			// Triangular Prism SDF
			TVoxelRange<v_flt> Variable_10; // Triangular Prism SDF output 0
			Variable_10 = FVoxelSDFNodeFunctions::TriPrism(Variable_12, Variable_11, Variable_26, Variable_6, Variable_20);
			
			Outputs.Value = Variable_10;
		}
		
	};
	
	FVDI_Ravine_GraphInstance(UVDI_Ravine_Graph& Object)
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
inline v_flt FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Ravine_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVDI_Ravine_GraphInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVDI_Ravine_GraphInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVDI_Ravine_GraphInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVDI_Ravine_GraphInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVDI_Ravine_Graph::UVDI_Ravine_Graph()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVDI_Ravine_Graph::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVDI_Ravine_GraphInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VDI_Ravine_Graph. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VDI_Ravine_Graph. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Ravine_Graph. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Ravine_Graph. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
