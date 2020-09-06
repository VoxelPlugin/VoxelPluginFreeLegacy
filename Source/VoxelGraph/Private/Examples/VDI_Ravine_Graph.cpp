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
			v_flt Variable_26; // vector - vector.- output 0
			v_flt Variable_30; // vector - vector.- output 0
			v_flt Variable_29; // vector - vector.- output 0
			v_flt Variable_35; // vector / float./ output 0
			v_flt Variable_14; // * output 0
			v_flt Variable_36; // vector / float./ output 0
			v_flt Variable_21; // vector / float./ output 0
			v_flt Variable_25; // + output 0
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
		
		const FParams& Params;
		
		FVoxelFastNoise _3D_Gradient_Perturb_0_Noise;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of 3D Gradient Perturb
			_3D_Gradient_Perturb_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_0_Noise.SetInterp(FVoxelFastNoise::Quintic);
			
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
				Variable_0 = 132.784760;
				Variable_1 = 62.540493;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// X
			BufferX.Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			BufferX.Variable_26 = Variable_5 - Variable_2;
			
			// vector - vector.-
			BufferX.Variable_30 = Variable_4 - Variable_1;
			
			// vector - vector.-
			BufferX.Variable_29 = Variable_3 - Variable_0;
			
			// vector + vector.+
			v_flt Variable_33; // vector + vector.+ output 0
			Variable_33 = Variable_0 + Variable_3;
			
			// 1 / X
			v_flt Variable_13; // 1 / X output 0
			Variable_13 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// vector - vector.-
			v_flt Variable_38; // vector - vector.- output 0
			Variable_38 = Variable_1 - Variable_4;
			
			// /
			v_flt Variable_27; // / output 0
			Variable_27 = BufferX.Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_34; // vector + vector.+ output 0
			Variable_34 = Variable_1 + Variable_4;
			
			// vector + vector.+
			v_flt Variable_20; // vector + vector.+ output 0
			Variable_20 = Variable_2 + Variable_5;
			
			// vector - vector.-
			v_flt Variable_22; // vector - vector.- output 0
			Variable_22 = Variable_2 - Variable_5;
			
			// vector - vector.-
			v_flt Variable_37; // vector - vector.- output 0
			Variable_37 = Variable_0 - Variable_3;
			
			// vector / float./
			BufferX.Variable_35 = Variable_33 / v_flt(2.0f);
			
			// *
			BufferX.Variable_14 = Variable_13 * v_flt(0.2f);
			
			// vector / float./
			BufferX.Variable_36 = Variable_34 / v_flt(2.0f);
			
			// vector / float./
			BufferX.Variable_21 = Variable_20 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_23; // Vector Length output 0
			Variable_23 = FVoxelNodeFunctions::VectorLength(Variable_37, Variable_38, Variable_22);
			
			// /
			v_flt Variable_24; // / output 0
			Variable_24 = Variable_23 / v_flt(2.0f);
			
			// +
			BufferX.Variable_25 = Variable_24 + Variable_27;
			
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
				Variable_0 = 132.784760;
				Variable_1 = 62.540493;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 0.000000;
				Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// X
			BufferX.Variable_7 = Context.GetLocalX();
			
			// vector - vector.-
			BufferX.Variable_26 = Variable_5 - Variable_2;
			
			// vector - vector.-
			BufferX.Variable_30 = Variable_4 - Variable_1;
			
			// vector - vector.-
			BufferX.Variable_29 = Variable_3 - Variable_0;
			
			// vector + vector.+
			v_flt Variable_33; // vector + vector.+ output 0
			Variable_33 = Variable_0 + Variable_3;
			
			// 1 / X
			v_flt Variable_13; // 1 / X output 0
			Variable_13 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// vector - vector.-
			v_flt Variable_38; // vector - vector.- output 0
			Variable_38 = Variable_1 - Variable_4;
			
			// /
			v_flt Variable_27; // / output 0
			Variable_27 = BufferX.Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_34; // vector + vector.+ output 0
			Variable_34 = Variable_1 + Variable_4;
			
			// vector + vector.+
			v_flt Variable_20; // vector + vector.+ output 0
			Variable_20 = Variable_2 + Variable_5;
			
			// vector - vector.-
			v_flt Variable_22; // vector - vector.- output 0
			Variable_22 = Variable_2 - Variable_5;
			
			// vector - vector.-
			v_flt Variable_37; // vector - vector.- output 0
			Variable_37 = Variable_0 - Variable_3;
			
			// vector / float./
			BufferX.Variable_35 = Variable_33 / v_flt(2.0f);
			
			// *
			BufferX.Variable_14 = Variable_13 * v_flt(0.2f);
			
			// vector / float./
			BufferX.Variable_36 = Variable_34 / v_flt(2.0f);
			
			// vector / float./
			BufferX.Variable_21 = Variable_20 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_23; // Vector Length output 0
			Variable_23 = FVoxelNodeFunctions::VectorLength(Variable_37, Variable_38, Variable_22);
			
			// /
			v_flt Variable_24; // / output 0
			Variable_24 = Variable_23 / v_flt(2.0f);
			
			// +
			BufferX.Variable_25 = Variable_24 + Variable_27;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// 3D Gradient Perturb
			v_flt Variable_10; // 3D Gradient Perturb output 0
			v_flt Variable_11; // 3D Gradient Perturb output 1
			v_flt Variable_12; // 3D Gradient Perturb output 2
			Variable_10 = BufferX.Variable_7;
			Variable_11 = BufferXY.Variable_8;
			Variable_12 = Variable_9;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_10, Variable_11, Variable_12, BufferX.Variable_14, v_flt(0.0f));
			
			// vector - vector.-
			v_flt Variable_31; // vector - vector.- output 0
			Variable_31 = Variable_10 - BufferX.Variable_35;
			
			// vector - vector.-
			v_flt Variable_32; // vector - vector.- output 0
			Variable_32 = Variable_11 - BufferX.Variable_36;
			
			// vector - vector.-
			v_flt Variable_19; // vector - vector.- output 0
			Variable_19 = Variable_12 - BufferX.Variable_21;
			
			// Inverse Transform Position XZ
			v_flt Variable_16; // Inverse Transform Position XZ output 0
			v_flt Variable_17; // Inverse Transform Position XZ output 1
			v_flt Variable_18; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(BufferX.Variable_29, BufferX.Variable_30, BufferX.Variable_26, v_flt(0.0f), v_flt(0.0f), v_flt(1.0f), Variable_31, Variable_32, Variable_19, Variable_16, Variable_17, Variable_18);
			
			// * -1
			v_flt Variable_28; // * -1 output 0
			Variable_28 = Variable_18 * -1;
			
			// Triangular Prism SDF
			v_flt Variable_15; // Triangular Prism SDF output 0
			Variable_15 = FVoxelSDFNodeFunctions::TriPrism(Variable_17, Variable_16, Variable_28, BufferX.Variable_6, BufferX.Variable_25);
			
			Outputs.Value = Variable_15;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_8; // Y output 0
			Variable_8 = Context.GetLocalY();
			
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
				Variable_0 = 132.784760;
				Variable_1 = 62.540493;
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
			v_flt Variable_26; // vector - vector.- output 0
			Variable_26 = Variable_5 - Variable_2;
			
			// vector - vector.-
			v_flt Variable_30; // vector - vector.- output 0
			Variable_30 = Variable_4 - Variable_1;
			
			// vector - vector.-
			v_flt Variable_29; // vector - vector.- output 0
			Variable_29 = Variable_3 - Variable_0;
			
			// vector + vector.+
			v_flt Variable_33; // vector + vector.+ output 0
			Variable_33 = Variable_0 + Variable_3;
			
			// 1 / X
			v_flt Variable_13; // 1 / X output 0
			Variable_13 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// vector - vector.-
			v_flt Variable_38; // vector - vector.- output 0
			Variable_38 = Variable_1 - Variable_4;
			
			// /
			v_flt Variable_27; // / output 0
			Variable_27 = Variable_6 / v_flt(4.0f);
			
			// vector + vector.+
			v_flt Variable_34; // vector + vector.+ output 0
			Variable_34 = Variable_1 + Variable_4;
			
			// vector + vector.+
			v_flt Variable_20; // vector + vector.+ output 0
			Variable_20 = Variable_2 + Variable_5;
			
			// vector - vector.-
			v_flt Variable_22; // vector - vector.- output 0
			Variable_22 = Variable_2 - Variable_5;
			
			// vector - vector.-
			v_flt Variable_37; // vector - vector.- output 0
			Variable_37 = Variable_0 - Variable_3;
			
			// vector / float./
			v_flt Variable_35; // vector / float./ output 0
			Variable_35 = Variable_33 / v_flt(2.0f);
			
			// *
			v_flt Variable_14; // * output 0
			Variable_14 = Variable_13 * v_flt(0.2f);
			
			// vector / float./
			v_flt Variable_36; // vector / float./ output 0
			Variable_36 = Variable_34 / v_flt(2.0f);
			
			// vector / float./
			v_flt Variable_21; // vector / float./ output 0
			Variable_21 = Variable_20 / v_flt(2.0f);
			
			// Vector Length
			v_flt Variable_23; // Vector Length output 0
			Variable_23 = FVoxelNodeFunctions::VectorLength(Variable_37, Variable_38, Variable_22);
			
			// /
			v_flt Variable_24; // / output 0
			Variable_24 = Variable_23 / v_flt(2.0f);
			
			// 3D Gradient Perturb
			v_flt Variable_10; // 3D Gradient Perturb output 0
			v_flt Variable_11; // 3D Gradient Perturb output 1
			v_flt Variable_12; // 3D Gradient Perturb output 2
			Variable_10 = Variable_7;
			Variable_11 = Variable_8;
			Variable_12 = Variable_9;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_10, Variable_11, Variable_12, Variable_14, v_flt(0.0f));
			
			// +
			v_flt Variable_25; // + output 0
			Variable_25 = Variable_24 + Variable_27;
			
			// vector - vector.-
			v_flt Variable_31; // vector - vector.- output 0
			Variable_31 = Variable_10 - Variable_35;
			
			// vector - vector.-
			v_flt Variable_32; // vector - vector.- output 0
			Variable_32 = Variable_11 - Variable_36;
			
			// vector - vector.-
			v_flt Variable_19; // vector - vector.- output 0
			Variable_19 = Variable_12 - Variable_21;
			
			// Inverse Transform Position XZ
			v_flt Variable_16; // Inverse Transform Position XZ output 0
			v_flt Variable_17; // Inverse Transform Position XZ output 1
			v_flt Variable_18; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(Variable_29, Variable_30, Variable_26, v_flt(0.0f), v_flt(0.0f), v_flt(1.0f), Variable_31, Variable_32, Variable_19, Variable_16, Variable_17, Variable_18);
			
			// * -1
			v_flt Variable_28; // * -1 output 0
			Variable_28 = Variable_18 * -1;
			
			// Triangular Prism SDF
			v_flt Variable_15; // Triangular Prism SDF output 0
			Variable_15 = FVoxelSDFNodeFunctions::TriPrism(Variable_17, Variable_16, Variable_28, Variable_6, Variable_25);
			
			Outputs.Value = Variable_15;
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
		
		const FParams& Params;
		
		
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
		
		const FParams& Params;
		
		
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
			TVoxelRange<v_flt> Variable_26; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_30; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_29; // vector - vector.- output 0
			TVoxelRange<v_flt> Variable_35; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_14; // * output 0
			TVoxelRange<v_flt> Variable_36; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_21; // vector / float./ output 0
			TVoxelRange<v_flt> Variable_25; // + output 0
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
		
		FVoxelFastNoise _3D_Gradient_Perturb_1_Noise;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of 3D Gradient Perturb
			_3D_Gradient_Perturb_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Gradient_Perturb_1_Noise.SetInterp(FVoxelFastNoise::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Z
			TVoxelRange<v_flt> Variable_9; // Z output 0
			Variable_9 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_8; // Y output 0
			Variable_8 = Context.GetLocalY();
			
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
				Variable_0 = 132.784760;
				Variable_1 = 62.540493;
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
			TVoxelRange<v_flt> Variable_26; // vector - vector.- output 0
			Variable_26 = Variable_5 - Variable_2;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_30; // vector - vector.- output 0
			Variable_30 = Variable_4 - Variable_1;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_29; // vector - vector.- output 0
			Variable_29 = Variable_3 - Variable_0;
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_33; // vector + vector.+ output 0
			Variable_33 = Variable_0 + Variable_3;
			
			// 1 / X
			TVoxelRange<v_flt> Variable_13; // 1 / X output 0
			Variable_13 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_38; // vector - vector.- output 0
			Variable_38 = Variable_1 - Variable_4;
			
			// /
			TVoxelRange<v_flt> Variable_27; // / output 0
			Variable_27 = Variable_6 / TVoxelRange<v_flt>(4.0f);
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_34; // vector + vector.+ output 0
			Variable_34 = Variable_1 + Variable_4;
			
			// vector + vector.+
			TVoxelRange<v_flt> Variable_20; // vector + vector.+ output 0
			Variable_20 = Variable_2 + Variable_5;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_22; // vector - vector.- output 0
			Variable_22 = Variable_2 - Variable_5;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_37; // vector - vector.- output 0
			Variable_37 = Variable_0 - Variable_3;
			
			// vector / float./
			TVoxelRange<v_flt> Variable_35; // vector / float./ output 0
			Variable_35 = Variable_33 / TVoxelRange<v_flt>(2.0f);
			
			// *
			TVoxelRange<v_flt> Variable_14; // * output 0
			Variable_14 = Variable_13 * TVoxelRange<v_flt>(0.2f);
			
			// vector / float./
			TVoxelRange<v_flt> Variable_36; // vector / float./ output 0
			Variable_36 = Variable_34 / TVoxelRange<v_flt>(2.0f);
			
			// vector / float./
			TVoxelRange<v_flt> Variable_21; // vector / float./ output 0
			Variable_21 = Variable_20 / TVoxelRange<v_flt>(2.0f);
			
			// Vector Length
			TVoxelRange<v_flt> Variable_23; // Vector Length output 0
			Variable_23 = FVoxelNodeFunctions::VectorLength(Variable_37, Variable_38, Variable_22);
			
			// /
			TVoxelRange<v_flt> Variable_24; // / output 0
			Variable_24 = Variable_23 / TVoxelRange<v_flt>(2.0f);
			
			// 3D Gradient Perturb
			TVoxelRange<v_flt> Variable_10; // 3D Gradient Perturb output 0
			TVoxelRange<v_flt> Variable_11; // 3D Gradient Perturb output 1
			TVoxelRange<v_flt> Variable_12; // 3D Gradient Perturb output 2
			Variable_10 = TVoxelRange<v_flt>::FromList(Variable_7.Min - 2 * TVoxelRange<v_flt>(0.0f).Max, Variable_7.Max + 2 * TVoxelRange<v_flt>(0.0f).Max);
			Variable_11 = TVoxelRange<v_flt>::FromList(Variable_8.Min - 2 * TVoxelRange<v_flt>(0.0f).Max, Variable_8.Max + 2 * TVoxelRange<v_flt>(0.0f).Max);
			Variable_12 = TVoxelRange<v_flt>::FromList(Variable_9.Min - 2 * TVoxelRange<v_flt>(0.0f).Max, Variable_9.Max + 2 * TVoxelRange<v_flt>(0.0f).Max);
			
			// +
			TVoxelRange<v_flt> Variable_25; // + output 0
			Variable_25 = Variable_24 + Variable_27;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_31; // vector - vector.- output 0
			Variable_31 = Variable_10 - Variable_35;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_32; // vector - vector.- output 0
			Variable_32 = Variable_11 - Variable_36;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_19; // vector - vector.- output 0
			Variable_19 = Variable_12 - Variable_21;
			
			// Inverse Transform Position XZ
			TVoxelRange<v_flt> Variable_16; // Inverse Transform Position XZ output 0
			TVoxelRange<v_flt> Variable_17; // Inverse Transform Position XZ output 1
			TVoxelRange<v_flt> Variable_18; // Inverse Transform Position XZ output 2
			FVoxelMathNodeFunctions::InverseTransformPositionXZ(Variable_29, Variable_30, Variable_26, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(1.0f), Variable_31, Variable_32, Variable_19, Variable_16, Variable_17, Variable_18);
			
			// * -1
			TVoxelRange<v_flt> Variable_28; // * -1 output 0
			Variable_28 = Variable_18 * -1;
			
			// Triangular Prism SDF
			TVoxelRange<v_flt> Variable_15; // Triangular Prism SDF output 0
			Variable_15 = FVoxelSDFNodeFunctions::TriPrism(Variable_17, Variable_16, Variable_28, Variable_6, Variable_25);
			
			Outputs.Value = Variable_15;
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
	
	virtual void InitGraph(const FVoxelWorldGeneratorInit& InitStruct) override final
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

TMap<FName, int32> UVDI_Ravine_Graph::GetDefaultSeeds() const
{
	return {
		};
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVDI_Ravine_Graph::GetTransformableInstance()
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
	return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
