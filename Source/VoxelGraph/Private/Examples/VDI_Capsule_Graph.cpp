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
		const float Noise_Amplitude;
		const int32 Seed;
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
			
			v_flt Variable_14; // Noise Amplitude = 1.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_16; // XYZ.X output 0
			v_flt Variable_0; // Data Item Parameters output 0
			v_flt Variable_1; // Data Item Parameters output 1
			v_flt Variable_2; // Data Item Parameters output 2
			v_flt Variable_3; // Data Item Parameters output 3
			v_flt Variable_4; // Data Item Parameters output 4
			v_flt Variable_5; // Data Item Parameters output 5
			v_flt Variable_6; // Data Item Parameters output 6
			v_flt Variable_13; // * output 0
			v_flt Variable_12; // * output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_17; // XYZ.Y output 0
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
					
					// Init of Seed = 1443
					FVoxelGraphSeed Variable_15; // Seed = 1443 output 0
					Variable_15 = Params.Seed;
					
					
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
				// Noise Amplitude = 1.0
				BufferConstant.Variable_14 = Params.Noise_Amplitude;
				
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
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 1443
			FVoxelGraphSeed Variable_15; // Seed = 1443 output 0
			Variable_15 = Params.Seed;
			
			// Init of 3D Gradient Perturb
			_3D_Gradient_Perturb_0_Noise.SetSeed(Variable_15);
			_3D_Gradient_Perturb_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// XYZ.X
			BufferX.Variable_16 = Context.GetLocalX();
			
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
			v_flt Variable_11; // 1 / X output 0
			Variable_11 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// *
			BufferX.Variable_13 = BufferX.Variable_6 * BufferConstant.Variable_14;
			
			// *
			BufferX.Variable_12 = Variable_11 * v_flt(0.2f);
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// XYZ.Y
			BufferXY.Variable_17 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// XYZ.X
			BufferX.Variable_16 = Context.GetLocalX();
			
			// XYZ.Y
			BufferXY.Variable_17 = Context.GetLocalY();
			
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
			v_flt Variable_11; // 1 / X output 0
			Variable_11 = FVoxelNodeFunctions::OneOverX(BufferX.Variable_6);
			
			// *
			BufferX.Variable_13 = BufferX.Variable_6 * BufferConstant.Variable_14;
			
			// *
			BufferX.Variable_12 = Variable_11 * v_flt(0.2f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// XYZ.Z
			v_flt Variable_18; // XYZ.Z output 0
			Variable_18 = Context.GetLocalZ();
			
			// 3D Gradient Perturb
			v_flt Variable_8; // 3D Gradient Perturb output 0
			v_flt Variable_9; // 3D Gradient Perturb output 1
			v_flt Variable_10; // 3D Gradient Perturb output 2
			Variable_8 = BufferX.Variable_16;
			Variable_9 = BufferXY.Variable_17;
			Variable_10 = Variable_18;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_8, Variable_9, Variable_10, BufferX.Variable_12, BufferX.Variable_13);
			
			// Capsule SDF
			v_flt Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_8, Variable_9, Variable_10, BufferX.Variable_0, BufferX.Variable_1, BufferX.Variable_2, BufferX.Variable_3, BufferX.Variable_4, BufferX.Variable_5, BufferX.Variable_6);
			
			Outputs.Value = Variable_7;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// XYZ.X
			v_flt Variable_16; // XYZ.X output 0
			Variable_16 = Context.GetLocalX();
			
			// XYZ.Y
			v_flt Variable_17; // XYZ.Y output 0
			Variable_17 = Context.GetLocalY();
			
			// XYZ.Z
			v_flt Variable_18; // XYZ.Z output 0
			Variable_18 = Context.GetLocalZ();
			
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
			v_flt Variable_11; // 1 / X output 0
			Variable_11 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = Variable_6 * BufferConstant.Variable_14;
			
			// *
			v_flt Variable_12; // * output 0
			Variable_12 = Variable_11 * v_flt(0.2f);
			
			// 3D Gradient Perturb
			v_flt Variable_8; // 3D Gradient Perturb output 0
			v_flt Variable_9; // 3D Gradient Perturb output 1
			v_flt Variable_10; // 3D Gradient Perturb output 2
			Variable_8 = Variable_16;
			Variable_9 = Variable_17;
			Variable_10 = Variable_18;
			_3D_Gradient_Perturb_0_Noise.GradientPerturb_3D(Variable_8, Variable_9, Variable_10, Variable_12, Variable_13);
			
			// Capsule SDF
			v_flt Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_8, Variable_9, Variable_10, Variable_0, Variable_1, Variable_2, Variable_3, Variable_4, Variable_5, Variable_6);
			
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
			
			TVoxelRange<v_flt> Variable_14; // Noise Amplitude = 1.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_15; // XYZ.X output 0
			TVoxelRange<v_flt> Variable_0; // Data Item Parameters output 0
			TVoxelRange<v_flt> Variable_1; // Data Item Parameters output 1
			TVoxelRange<v_flt> Variable_2; // Data Item Parameters output 2
			TVoxelRange<v_flt> Variable_3; // Data Item Parameters output 3
			TVoxelRange<v_flt> Variable_4; // Data Item Parameters output 4
			TVoxelRange<v_flt> Variable_5; // Data Item Parameters output 5
			TVoxelRange<v_flt> Variable_6; // Data Item Parameters output 6
			TVoxelRange<v_flt> Variable_13; // * output 0
			TVoxelRange<v_flt> Variable_12; // * output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_16; // XYZ.Y output 0
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
				// Noise Amplitude = 1.0
				BufferConstant.Variable_14 = Params.Noise_Amplitude;
				
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
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
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
			// XYZ.Z
			TVoxelRange<v_flt> Variable_17; // XYZ.Z output 0
			Variable_17 = Context.GetLocalZ();
			
			// XYZ.X
			TVoxelRange<v_flt> Variable_15; // XYZ.X output 0
			Variable_15 = Context.GetLocalX();
			
			// XYZ.Y
			TVoxelRange<v_flt> Variable_16; // XYZ.Y output 0
			Variable_16 = Context.GetLocalY();
			
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
			TVoxelRange<v_flt> Variable_11; // 1 / X output 0
			Variable_11 = FVoxelNodeFunctions::OneOverX(Variable_6);
			
			// *
			TVoxelRange<v_flt> Variable_13; // * output 0
			Variable_13 = Variable_6 * BufferConstant.Variable_14;
			
			// *
			TVoxelRange<v_flt> Variable_12; // * output 0
			Variable_12 = Variable_11 * TVoxelRange<v_flt>(0.2f);
			
			// 3D Gradient Perturb
			TVoxelRange<v_flt> Variable_8; // 3D Gradient Perturb output 0
			TVoxelRange<v_flt> Variable_9; // 3D Gradient Perturb output 1
			TVoxelRange<v_flt> Variable_10; // 3D Gradient Perturb output 2
			Variable_8 = TVoxelRange<v_flt>::FromList(Variable_15.Min - 2 * Variable_13.Max, Variable_15.Max + 2 * Variable_13.Max);
			Variable_9 = TVoxelRange<v_flt>::FromList(Variable_16.Min - 2 * Variable_13.Max, Variable_16.Max + 2 * Variable_13.Max);
			Variable_10 = TVoxelRange<v_flt>::FromList(Variable_17.Min - 2 * Variable_13.Max, Variable_17.Max + 2 * Variable_13.Max);
			
			// Capsule SDF
			TVoxelRange<v_flt> Variable_7; // Capsule SDF output 0
			Variable_7 = FVoxelSDFNodeFunctions::Capsule(Variable_8, Variable_9, Variable_10, Variable_0, Variable_1, Variable_2, Variable_3, Variable_4, Variable_5, Variable_6);
			
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
		, Params(FParams
		{
			Object.Noise_Amplitude,
			Object.Seed
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

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVDI_Capsule_Graph::GetTransformableInstance()
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
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
