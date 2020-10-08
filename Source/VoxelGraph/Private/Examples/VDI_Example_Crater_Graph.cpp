// Copyright 2020 Phyronnaz

#include "VDI_Example_Crater_Graph.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVDI_Example_Crater_GraphInstance : public TVoxelGraphGeneratorInstanceHelper<FVDI_Example_Crater_GraphInstance, UVDI_Example_Crater_Graph>
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
			
			v_flt Variable_9; // X output 0
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			v_flt Variable_6; // Data Item Parameters output 6
			v_flt Variable_7; // Data Item Parameters output 7
			v_flt Variable_8; // Data Item Parameters output 8
			v_flt Variable_16; // Vector Length output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_10; // Y output 0
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
			// X
			BufferX.Variable_9 = Context.GetLocalX();
			
			// Data Item Parameters
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 9)
			{
				BufferX.Variable_0 = (*Context.Items.CustomData).GetData()[0];
				BufferX.Variable_1 = (*Context.Items.CustomData).GetData()[1];
				BufferX.Variable_2 = (*Context.Items.CustomData).GetData()[2];
				BufferX.Variable_3 = (*Context.Items.CustomData).GetData()[3];
				BufferX.Variable_4 = (*Context.Items.CustomData).GetData()[4];
				BufferX.Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
				BufferX.Variable_7 = (*Context.Items.CustomData).GetData()[7];
				BufferX.Variable_8 = (*Context.Items.CustomData).GetData()[8];
			}
			else
			{
				BufferX.Variable_0 = 140.000000;
				BufferX.Variable_1 = 140.000000;
				BufferX.Variable_2 = 0.000000;
				BufferX.Variable_3 = 50.000000;
				BufferX.Variable_4 = 2.000000;
				BufferX.Variable_5 = 0.100000;
				BufferX.Variable_6 = -0.800000;
				BufferX.Variable_7 = 0.200000;
				BufferX.Variable_8 = 0.500000;
			}
			
			// Vector Length
			BufferX.Variable_16 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferX.Variable_1, BufferX.Variable_2);
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_10 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_10 = Context.GetLocalY();
			
			// X
			BufferX.Variable_9 = Context.GetLocalX();
			
			// Data Item Parameters
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 9)
			{
				BufferX.Variable_0 = (*Context.Items.CustomData).GetData()[0];
				BufferX.Variable_1 = (*Context.Items.CustomData).GetData()[1];
				BufferX.Variable_2 = (*Context.Items.CustomData).GetData()[2];
				BufferX.Variable_3 = (*Context.Items.CustomData).GetData()[3];
				BufferX.Variable_4 = (*Context.Items.CustomData).GetData()[4];
				BufferX.Variable_5 = (*Context.Items.CustomData).GetData()[5];
				BufferX.Variable_6 = (*Context.Items.CustomData).GetData()[6];
				BufferX.Variable_7 = (*Context.Items.CustomData).GetData()[7];
				BufferX.Variable_8 = (*Context.Items.CustomData).GetData()[8];
			}
			else
			{
				BufferX.Variable_0 = 140.000000;
				BufferX.Variable_1 = 140.000000;
				BufferX.Variable_2 = 0.000000;
				BufferX.Variable_3 = 50.000000;
				BufferX.Variable_4 = 2.000000;
				BufferX.Variable_5 = 0.100000;
				BufferX.Variable_6 = -0.800000;
				BufferX.Variable_7 = 0.200000;
				BufferX.Variable_8 = 0.500000;
			}
			
			// Vector Length
			BufferX.Variable_16 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferX.Variable_1, BufferX.Variable_2);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// Normalize.Vector Length
			v_flt Variable_28; // Normalize.Vector Length output 0
			Variable_28 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_9, BufferXY.Variable_10, Variable_11);
			
			// Normalize./
			v_flt Variable_30; // Normalize./ output 0
			Variable_30 = BufferXY.Variable_10 / Variable_28;
			
			// Normalize./
			v_flt Variable_29; // Normalize./ output 0
			Variable_29 = BufferX.Variable_9 / Variable_28;
			
			// Normalize./
			v_flt Variable_31; // Normalize./ output 0
			Variable_31 = Variable_11 / Variable_28;
			
			// vector * float.*
			v_flt Variable_15; // vector * float.* output 0
			Variable_15 = Variable_31 * BufferX.Variable_16;
			
			// vector * float.*
			v_flt Variable_32; // vector * float.* output 0
			Variable_32 = Variable_29 * BufferX.Variable_16;
			
			// vector * float.*
			v_flt Variable_33; // vector * float.* output 0
			Variable_33 = Variable_30 * BufferX.Variable_16;
			
			// vector - vector.-
			v_flt Variable_25; // vector - vector.- output 0
			Variable_25 = Variable_15 - BufferX.Variable_2;
			
			// vector - vector.-
			v_flt Variable_26; // vector - vector.- output 0
			Variable_26 = Variable_32 - BufferX.Variable_0;
			
			// vector - vector.-
			v_flt Variable_27; // vector - vector.- output 0
			Variable_27 = Variable_33 - BufferX.Variable_1;
			
			// Vector Length
			v_flt Variable_12; // Vector Length output 0
			Variable_12 = FVoxelNodeFunctions::VectorLength(Variable_26, Variable_27, Variable_25);
			
			// /
			v_flt Variable_13; // / output 0
			Variable_13 = Variable_12 / BufferX.Variable_3;
			
			// *
			v_flt Variable_14; // * output 0
			Variable_14 = Variable_13 * Variable_13;
			
			// -
			v_flt Variable_21; // - output 0
			Variable_21 = Variable_14 - v_flt(1.0f);
			
			// -
			v_flt Variable_17; // - output 0
			Variable_17 = Variable_14 - v_flt(1.0f);
			
			// -
			v_flt Variable_18; // - output 0
			Variable_18 = Variable_17 - BufferX.Variable_4;
			
			// Smooth Intersection
			v_flt Variable_23; // Smooth Intersection output 0
			Variable_23 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_21, BufferX.Variable_6, BufferX.Variable_7);
			
			// Min (float)
			v_flt Variable_19; // Min (float) output 0
			Variable_19 = FVoxelNodeFunctions::Min<v_flt>(Variable_18, v_flt(0.0f));
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = Variable_19 * Variable_19 * BufferX.Variable_5;
			
			// Smooth Union
			v_flt Variable_24; // Smooth Union output 0
			Variable_24 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_23, Variable_20, BufferX.Variable_8);
			
			// *
			v_flt Variable_22; // * output 0
			Variable_22 = Variable_24 * BufferX.Variable_3;
			
			Outputs.Value = Variable_22;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_10; // Y output 0
			Variable_10 = Context.GetLocalY();
			
			// X
			v_flt Variable_9; // X output 0
			Variable_9 = Context.GetLocalX();
			
			// Data Item Parameters
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			v_flt Variable_6; // Data Item Parameters output 6
			v_flt Variable_7; // Data Item Parameters output 7
			v_flt Variable_8; // Data Item Parameters output 8
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 9)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				Variable_6 = (*Context.Items.CustomData).GetData()[6];
				Variable_7 = (*Context.Items.CustomData).GetData()[7];
				Variable_8 = (*Context.Items.CustomData).GetData()[8];
			}
			else
			{
				Variable_0 = 140.000000;
				Variable_1 = 140.000000;
				Variable_2 = 0.000000;
				Variable_3 = 50.000000;
				Variable_4 = 2.000000;
				Variable_5 = 0.100000;
				Variable_6 = -0.800000;
				Variable_7 = 0.200000;
				Variable_8 = 0.500000;
			}
			
			// Vector Length
			v_flt Variable_16; // Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// Normalize.Vector Length
			v_flt Variable_28; // Normalize.Vector Length output 0
			Variable_28 = FVoxelNodeFunctions::VectorLength(Variable_9, Variable_10, Variable_11);
			
			// Normalize./
			v_flt Variable_30; // Normalize./ output 0
			Variable_30 = Variable_10 / Variable_28;
			
			// Normalize./
			v_flt Variable_29; // Normalize./ output 0
			Variable_29 = Variable_9 / Variable_28;
			
			// Normalize./
			v_flt Variable_31; // Normalize./ output 0
			Variable_31 = Variable_11 / Variable_28;
			
			// vector * float.*
			v_flt Variable_15; // vector * float.* output 0
			Variable_15 = Variable_31 * Variable_16;
			
			// vector * float.*
			v_flt Variable_32; // vector * float.* output 0
			Variable_32 = Variable_29 * Variable_16;
			
			// vector * float.*
			v_flt Variable_33; // vector * float.* output 0
			Variable_33 = Variable_30 * Variable_16;
			
			// vector - vector.-
			v_flt Variable_25; // vector - vector.- output 0
			Variable_25 = Variable_15 - Variable_2;
			
			// vector - vector.-
			v_flt Variable_26; // vector - vector.- output 0
			Variable_26 = Variable_32 - Variable_0;
			
			// vector - vector.-
			v_flt Variable_27; // vector - vector.- output 0
			Variable_27 = Variable_33 - Variable_1;
			
			// Vector Length
			v_flt Variable_12; // Vector Length output 0
			Variable_12 = FVoxelNodeFunctions::VectorLength(Variable_26, Variable_27, Variable_25);
			
			// /
			v_flt Variable_13; // / output 0
			Variable_13 = Variable_12 / Variable_3;
			
			// *
			v_flt Variable_14; // * output 0
			Variable_14 = Variable_13 * Variable_13;
			
			// -
			v_flt Variable_21; // - output 0
			Variable_21 = Variable_14 - v_flt(1.0f);
			
			// -
			v_flt Variable_17; // - output 0
			Variable_17 = Variable_14 - v_flt(1.0f);
			
			// -
			v_flt Variable_18; // - output 0
			Variable_18 = Variable_17 - Variable_4;
			
			// Smooth Intersection
			v_flt Variable_23; // Smooth Intersection output 0
			Variable_23 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_21, Variable_6, Variable_7);
			
			// Min (float)
			v_flt Variable_19; // Min (float) output 0
			Variable_19 = FVoxelNodeFunctions::Min<v_flt>(Variable_18, v_flt(0.0f));
			
			// *
			v_flt Variable_20; // * output 0
			Variable_20 = Variable_19 * Variable_19 * Variable_5;
			
			// Smooth Union
			v_flt Variable_24; // Smooth Union output 0
			Variable_24 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_23, Variable_20, Variable_8);
			
			// *
			v_flt Variable_22; // * output 0
			Variable_22 = Variable_24 * Variable_3;
			
			Outputs.Value = Variable_22;
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
			
			TVoxelRange<v_flt> Variable_25; // Normalize.Range Union output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_19; // * output 0
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
				// Normalize.Range Union
				BufferConstant.Variable_25 = FVoxelNodeFunctions::Union(TVoxelRange<v_flt>(-1.0f), TVoxelRange<v_flt>(1.0f));
				
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
			// Data Item Parameters
			TVoxelRange<v_flt> Variable_0; // Data Item Parameters output 0
			TVoxelRange<v_flt> Variable_1; // Data Item Parameters output 1
			TVoxelRange<v_flt> Variable_2; // Data Item Parameters output 2
			TVoxelRange<v_flt> Variable_3; // Data Item Parameters output 3
			TVoxelRange<v_flt> Variable_4; // Data Item Parameters output 4
			TVoxelRange<v_flt> Variable_5; // Data Item Parameters output 5
			TVoxelRange<v_flt> Variable_6; // Data Item Parameters output 6
			TVoxelRange<v_flt> Variable_7; // Data Item Parameters output 7
			TVoxelRange<v_flt> Variable_8; // Data Item Parameters output 8
			if (Context.Items.CustomData && Context.Items.CustomData->Num() == 9)
			{
				Variable_0 = (*Context.Items.CustomData).GetData()[0];
				Variable_1 = (*Context.Items.CustomData).GetData()[1];
				Variable_2 = (*Context.Items.CustomData).GetData()[2];
				Variable_3 = (*Context.Items.CustomData).GetData()[3];
				Variable_4 = (*Context.Items.CustomData).GetData()[4];
				Variable_5 = (*Context.Items.CustomData).GetData()[5];
				Variable_6 = (*Context.Items.CustomData).GetData()[6];
				Variable_7 = (*Context.Items.CustomData).GetData()[7];
				Variable_8 = (*Context.Items.CustomData).GetData()[8];
			}
			else
			{
				Variable_0 = 140.000000;
				Variable_1 = 140.000000;
				Variable_2 = 0.000000;
				Variable_3 = 50.000000;
				Variable_4 = 2.000000;
				Variable_5 = 0.100000;
				Variable_6 = -0.800000;
				Variable_7 = 0.200000;
				Variable_8 = 0.500000;
			}
			
			// Vector Length
			TVoxelRange<v_flt> Variable_13; // Vector Length output 0
			Variable_13 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// vector * float.*
			TVoxelRange<v_flt> Variable_12; // vector * float.* output 0
			Variable_12 = BufferConstant.Variable_25 * Variable_13;
			
			// vector * float.*
			TVoxelRange<v_flt> Variable_26; // vector * float.* output 0
			Variable_26 = BufferConstant.Variable_25 * Variable_13;
			
			// vector * float.*
			TVoxelRange<v_flt> Variable_27; // vector * float.* output 0
			Variable_27 = BufferConstant.Variable_25 * Variable_13;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_23; // vector - vector.- output 0
			Variable_23 = Variable_26 - Variable_0;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_24; // vector - vector.- output 0
			Variable_24 = Variable_27 - Variable_1;
			
			// vector - vector.-
			TVoxelRange<v_flt> Variable_22; // vector - vector.- output 0
			Variable_22 = Variable_12 - Variable_2;
			
			// Vector Length
			TVoxelRange<v_flt> Variable_9; // Vector Length output 0
			Variable_9 = FVoxelNodeFunctions::VectorLength(Variable_23, Variable_24, Variable_22);
			
			// /
			TVoxelRange<v_flt> Variable_10; // / output 0
			Variable_10 = Variable_9 / Variable_3;
			
			// *
			TVoxelRange<v_flt> Variable_11; // * output 0
			Variable_11 = Variable_10 * Variable_10;
			
			// -
			TVoxelRange<v_flt> Variable_18; // - output 0
			Variable_18 = Variable_11 - TVoxelRange<v_flt>(1.0f);
			
			// -
			TVoxelRange<v_flt> Variable_14; // - output 0
			Variable_14 = Variable_11 - TVoxelRange<v_flt>(1.0f);
			
			// -
			TVoxelRange<v_flt> Variable_15; // - output 0
			Variable_15 = Variable_14 - Variable_4;
			
			// Smooth Intersection
			TVoxelRange<v_flt> Variable_20; // Smooth Intersection output 0
			Variable_20 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_18, Variable_6, Variable_7);
			
			// Min (float)
			TVoxelRange<v_flt> Variable_16; // Min (float) output 0
			Variable_16 = FVoxelNodeFunctions::Min<v_flt>(Variable_15, TVoxelRange<v_flt>(0.0f));
			
			// *
			TVoxelRange<v_flt> Variable_17; // * output 0
			Variable_17 = Variable_16 * Variable_16 * Variable_5;
			
			// Smooth Union
			TVoxelRange<v_flt> Variable_21; // Smooth Union output 0
			Variable_21 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_20, Variable_17, Variable_8);
			
			// *
			TVoxelRange<v_flt> Variable_19; // * output 0
			Variable_19 = Variable_21 * Variable_3;
			
			Outputs.Value = Variable_19;
		}
		
	};
	
	FVDI_Example_Crater_GraphInstance(UVDI_Example_Crater_Graph& Object)
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
inline v_flt FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVDI_Example_Crater_GraphInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVDI_Example_Crater_GraphInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVDI_Example_Crater_GraphInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVDI_Example_Crater_GraphInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVDI_Example_Crater_GraphInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVDI_Example_Crater_Graph::UVDI_Example_Crater_Graph()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVDI_Example_Crater_Graph::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVDI_Example_Crater_GraphInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VDI_Example_Crater_Graph. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VDI_Example_Crater_Graph. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Example_Crater_Graph. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VDI_Example_Crater_Graph. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
