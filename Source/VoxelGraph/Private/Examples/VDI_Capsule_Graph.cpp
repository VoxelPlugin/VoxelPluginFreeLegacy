// Copyright 2020 Phyronnaz

#include "VDI_Capsule_Graph.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVDI_Capsule_GraphInstance : public TVoxelGraphGeneratorInstanceHelper<FVDI_Capsule_GraphInstance, UVDI_Capsule_Graph>
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
			
			v_flt Variable_8; // X output 0
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			v_flt Variable_6; // Data Item Parameters output 6
			v_flt Variable_15; // * output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_9; // Y output 0
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
			_3D_Gradient_Perturb_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// Data Item Parameters
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				BufferX.Variable_0 = (*Context.Items.CustomData).GetData()[0];
				BufferX.Variable_1 = (*Context.Items.CustomData).GetData()[1];
				BufferX.Variable_2 = (*Context.Items.CustomData).GetData()[2];
				BufferX.Variable_3 = (*Context.Items.CustomData).GetData()[3];
				BufferX.Variable_4 = (*Context.Items.CustomData).GetData()[4];
				BufferX.Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				BufferX.Variable_0 = 57.599049;
				BufferX.Variable_1 = 0.000000;
				BufferX.Variable_2 = 0.000000;
				BufferX.Variable_3 = 0.000000;
				BufferX.Variable_4 = 100.000000;
				BufferX.Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// 1 / X
			v_flt Variable_14; // 1 / X output 0
			Variable_14 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// *
			BufferX.Variable_15 = Variable_14 * v_flt(0.2f);
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
			// Data Item Parameters
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 7)
			{
				BufferX.Variable_0 = (*Context.Items.CustomData).GetData()[0];
				BufferX.Variable_1 = (*Context.Items.CustomData).GetData()[1];
				BufferX.Variable_2 = (*Context.Items.CustomData).GetData()[2];
				BufferX.Variable_3 = (*Context.Items.CustomData).GetData()[3];
				BufferX.Variable_4 = (*Context.Items.CustomData).GetData()[4];
				BufferX.Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
			}
			else
			{
				BufferX.Variable_0 = 57.599049;
				BufferX.Variable_1 = 0.000000;
				BufferX.Variable_2 = 0.000000;
				BufferX.Variable_3 = 0.000000;
				BufferX.Variable_4 = 100.000000;
				BufferX.Variable_5 = 0.000000;
				BufferX.Variable_6 = 40.000000;
			}
			
			// 1 / X
			v_flt Variable_14; // 1 / X output 0
			Variable_14 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// *
			BufferX.Variable_15 = Variable_14 * v_flt(0.2f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// 3D Gradient Perturb
			v_flt Variable_11; // 3D Gradient Perturb output 0
			v_flt Variable_12; // 3D Gradient Perturb output 1
			v_flt Variable_13; // 3D Gradient Perturb output 2
			Variable_11 = BufferX.Variable_8;
			Variable_12 = BufferXY.Variable_9;
			Variable_13 = Variable_10;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_11, Variable_12, Variable_13, BufferX.Variable_15, BufferX.Variable_6);
			
			// Capsule SDF
			v_flt Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_11, Variable_12, Variable_13, BufferX.Variable_0, BufferX.Variable_1, BufferX.Variable_2, BufferX.Variable_3, BufferX.Variable_4, BufferX.Variable_5, BufferX.Variable_6);
			
			Outputs.Value = Variable_7;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// X
			v_flt Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// Y
			v_flt Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
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
				Variable_0 = 57.599049;
				Variable_1 = 0.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 100.000000;
				Variable_5 = 0.000000;
				Variable_6 = 40.000000;
			}
			
			// 1 / X
			v_flt Variable_14; // 1 / X output 0
			Variable_14 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// *
			v_flt Variable_15; // * output 0
			Variable_15 = Variable_14 * v_flt(0.2f);
			
			// 3D Gradient Perturb
			v_flt Variable_11; // 3D Gradient Perturb output 0
			v_flt Variable_12; // 3D Gradient Perturb output 1
			v_flt Variable_13; // 3D Gradient Perturb output 2
			Variable_11 = Variable_8;
			Variable_12 = Variable_9;
			Variable_13 = Variable_10;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_11, Variable_12, Variable_13, Variable_15, Variable_6);
			
			// Capsule SDF
			v_flt Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_11, Variable_12, Variable_13, Variable_0, Variable_1, Variable_2, Variable_3, Variable_4, Variable_5, Variable_6);
			
			Outputs.Value = Variable_7;
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
			
			TVoxelRange<v_flt> Variable_8; // X output 0
			TVoxelRange<v_flt> Variable_0; // Data Item Parameters output 0
			TVoxelRange<v_flt> Variable_1; // Data Item Parameters output 1
			TVoxelRange<v_flt> Variable_2; // Data Item Parameters output 2
			TVoxelRange<v_flt> Variable_3; // Data Item Parameters output 3
			TVoxelRange<v_flt> Variable_4; // Data Item Parameters output 4
			TVoxelRange<v_flt> Variable_5; // Data Item Parameters output 5
			TVoxelRange<v_flt> Variable_6; // Data Item Parameters output 6
			TVoxelRange<v_flt> Variable_15; // * output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_9; // Y output 0
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
			_3D_Gradient_Perturb_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Z
			TVoxelRange<v_flt> Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
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
				Variable_0 = 57.599049;
				Variable_1 = 0.000000;
				Variable_2 = 0.000000;
				Variable_3 = 0.000000;
				Variable_4 = 100.000000;
				Variable_5 = 0.000000;
				Variable_6 = 40.000000;
			}
			
			// 1 / X
			TVoxelRange<v_flt> Variable_14; // 1 / X output 0
			Variable_14 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// *
			TVoxelRange<v_flt> Variable_15; // * output 0
			Variable_15 = Variable_14 * TVoxelRange<v_flt>(0.2f);
			
			// 3D Gradient Perturb
			TVoxelRange<v_flt> Variable_11; // 3D Gradient Perturb output 0
			TVoxelRange<v_flt> Variable_12; // 3D Gradient Perturb output 1
			TVoxelRange<v_flt> Variable_13; // 3D Gradient Perturb output 2
			Variable_11 = TVoxelRange<v_flt>::FromList(Variable_8.Min - 2 * Variable_6.Max, Variable_8.Max + 2 * Variable_6.Max);
			Variable_12 = TVoxelRange<v_flt>::FromList(Variable_9.Min - 2 * Variable_6.Max, Variable_9.Max + 2 * Variable_6.Max);
			Variable_13 = TVoxelRange<v_flt>::FromList(Variable_10.Min - 2 * Variable_6.Max, Variable_10.Max + 2 * Variable_6.Max);
			
			// Capsule SDF
			TVoxelRange<v_flt> Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_11, Variable_12, Variable_13, Variable_0, Variable_1, Variable_2, Variable_3, Variable_4, Variable_5, Variable_6);
			
			Outputs.Value = Variable_7;
		}
		
	};
	
	FVDI_Capsule_GraphInstance(UVDI_Capsule_Graph& Object)
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
inline v_flt FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Capsule_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVDI_Capsule_GraphInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVDI_Capsule_GraphInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVDI_Capsule_GraphInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVDI_Capsule_GraphInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVDI_Capsule_Graph::UVDI_Capsule_Graph()
{
	bEnableRangeAnalysis = true;
}

TMap<FName, int32> UVDI_Capsule_Graph::GetDefaultSeeds() const
{
	return {
		};
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVDI_Capsule_Graph::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVDI_Capsule_GraphInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VDI_Capsule_Graph. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VDI_Capsule_Graph. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Capsule_Graph. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Capsule_Graph. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
