// Copyright 2020 Phyronnaz

#include "VoxelExample_HeightmapComposition.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_HeightmapCompositionInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_HeightmapCompositionInstance, UVoxelExample_HeightmapComposition>
{
public:
	struct FParams
	{
		const float Depth;
		const bool Flip_X;
		const bool Flip_Y;
		const TVoxelHeightmapAssetSamplerWrapper<uint16> heightmap_x0_y0;
		const TVoxelHeightmapAssetSamplerWrapper<uint16> heightmap_x0_y1;
		const TVoxelHeightmapAssetSamplerWrapper<uint16> heightmap_x1_y0;
		const TVoxelHeightmapAssetSamplerWrapper<uint16> heightmap_x1_y1;
		const float Size_X;
		const float Size_Y;
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
			
			v_flt Variable_6; // Size Y = 512.0 output 0
			bool Variable_11; // Flip X = False output 0
			bool Variable_12; // Flip Y = True output 0
			v_flt Variable_5; // Size X = 512.0 output 0
			v_flt Variable_26; // Depth = 300.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_13; // Switch (float) output 0
			bool Variable_7; // < output 0
			v_flt Variable_9; // + output 0
			v_flt Variable_21; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			bool Variable_8; // < output 0
			v_flt Variable_0; // Heightmap: heightmap x0 y0 output 0
			v_flt Variable_1; // Heightmap: heightmap x0 y1 output 0
			v_flt Variable_2; // Heightmap: heightmap x1 y0 output 0
			v_flt Variable_3; // Heightmap: heightmap x1 y1 output 0
			v_flt Variable_20; // Box SDF output 0
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
				Function1_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Size Y = 512.0
				BufferConstant.Variable_6 = Params.Size_Y;
				
				// Flip X = False
				BufferConstant.Variable_11 = Params.Flip_X;
				
				// Flip Y = True
				BufferConstant.Variable_12 = Params.Flip_Y;
				
				// Size X = 512.0
				BufferConstant.Variable_5 = Params.Size_X;
				
				// Depth = 300.0
				BufferConstant.Variable_26 = Params.Depth;
				
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
		
		void Function1_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// Switch (float)
			BufferX.Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// <
			BufferX.Variable_7 = BufferX.Variable_13 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				// +
				BufferX.Variable_9 = BufferX.Variable_13 + BufferConstant.Variable_5;
				
			}
			else
			{
			}
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			BufferXY.Variable_8 = Variable_15 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					FVoxelMaterial Heightmap__heightmap_x0_y0_0_Temp_1; // Heightmap: heightmap x0 y0 output 1
					v_flt Heightmap__heightmap_x0_y0_0_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_0_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_0_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_0_Temp_5; // Heightmap: heightmap x0 y0 output 5
					BufferXY.Variable_0 = Params.heightmap_x0_y0.GetHeight(BufferX.Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_0, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					FVoxelMaterial Heightmap__heightmap_x0_y1_0_Temp_1; // Heightmap: heightmap x0 y1 output 1
					v_flt Heightmap__heightmap_x0_y1_0_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_0_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_0_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_0_Temp_5; // Heightmap: heightmap x0 y1 output 5
					BufferXY.Variable_1 = Params.heightmap_x0_y1.GetHeight(BufferX.Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_1, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					FVoxelMaterial Heightmap__heightmap_x1_y0_0_Temp_1; // Heightmap: heightmap x1 y0 output 1
					v_flt Heightmap__heightmap_x1_y0_0_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_0_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_0_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_0_Temp_5; // Heightmap: heightmap x1 y0 output 5
					BufferXY.Variable_2 = Params.heightmap_x1_y0.GetHeight(BufferX.Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_2, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					FVoxelMaterial Heightmap__heightmap_x1_y1_0_Temp_1; // Heightmap: heightmap x1 y1 output 1
					v_flt Heightmap__heightmap_x1_y1_0_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_0_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_0_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_0_Temp_5; // Heightmap: heightmap x1 y1 output 5
					BufferXY.Variable_3 = Params.heightmap_x1_y1.GetHeight(BufferX.Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_3, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			BufferX.Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			BufferXY.Variable_8 = Variable_15 < v_flt(0.0f);
			
			// <
			BufferX.Variable_7 = BufferX.Variable_13 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				// +
				BufferX.Variable_9 = BufferX.Variable_13 + BufferConstant.Variable_5;
				
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					FVoxelMaterial Heightmap__heightmap_x0_y0_0_Temp_1; // Heightmap: heightmap x0 y0 output 1
					v_flt Heightmap__heightmap_x0_y0_0_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_0_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_0_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_0_Temp_5; // Heightmap: heightmap x0 y0 output 5
					BufferXY.Variable_0 = Params.heightmap_x0_y0.GetHeight(BufferX.Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_0, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					FVoxelMaterial Heightmap__heightmap_x0_y1_0_Temp_1; // Heightmap: heightmap x0 y1 output 1
					v_flt Heightmap__heightmap_x0_y1_0_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_0_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_0_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_0_Temp_5; // Heightmap: heightmap x0 y1 output 5
					BufferXY.Variable_1 = Params.heightmap_x0_y1.GetHeight(BufferX.Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_1, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					FVoxelMaterial Heightmap__heightmap_x1_y0_0_Temp_1; // Heightmap: heightmap x1 y0 output 1
					v_flt Heightmap__heightmap_x1_y0_0_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_0_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_0_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_0_Temp_5; // Heightmap: heightmap x1 y0 output 5
					BufferXY.Variable_2 = Params.heightmap_x1_y0.GetHeight(BufferX.Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_2, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					FVoxelMaterial Heightmap__heightmap_x1_y1_0_Temp_1; // Heightmap: heightmap x1 y1 output 1
					v_flt Heightmap__heightmap_x1_y1_0_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_0_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_0_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_0_Temp_5; // Heightmap: heightmap x1 y1 output 5
					BufferXY.Variable_3 = Params.heightmap_x1_y1.GetHeight(BufferX.Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_3, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			if (BufferX.Variable_7)
			{
				if (BufferXY.Variable_8)
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_0, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_1, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_2, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_3, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			v_flt Variable_13; // Switch (float) output 0
			Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			bool Variable_8; // < output 0
			Variable_8 = Variable_15 < v_flt(0.0f);
			
			// <
			bool Variable_7; // < output 0
			Variable_7 = Variable_13 < v_flt(0.0f);
			
			if (Variable_7)
			{
				// +
				v_flt Variable_9; // + output 0
				Variable_9 = Variable_13 + BufferConstant.Variable_5;
				
				if (Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					v_flt Variable_0; // Heightmap: heightmap x0 y0 output 0
					FVoxelMaterial Heightmap__heightmap_x0_y0_0_Temp_1; // Heightmap: heightmap x0 y0 output 1
					v_flt Heightmap__heightmap_x0_y0_0_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_0_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_0_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_0_Temp_5; // Heightmap: heightmap x0 y0 output 5
					Variable_0 = Params.heightmap_x0_y0.GetHeight(Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_0, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					v_flt Variable_1; // Heightmap: heightmap x0 y1 output 0
					FVoxelMaterial Heightmap__heightmap_x0_y1_0_Temp_1; // Heightmap: heightmap x0 y1 output 1
					v_flt Heightmap__heightmap_x0_y1_0_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_0_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_0_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_0_Temp_5; // Heightmap: heightmap x0 y1 output 5
					Variable_1 = Params.heightmap_x0_y1.GetHeight(Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_1, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
			else
			{
				if (Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					v_flt Variable_2; // Heightmap: heightmap x1 y0 output 0
					FVoxelMaterial Heightmap__heightmap_x1_y0_0_Temp_1; // Heightmap: heightmap x1 y0 output 1
					v_flt Heightmap__heightmap_x1_y0_0_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_0_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_0_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_0_Temp_5; // Heightmap: heightmap x1 y0 output 5
					Variable_2 = Params.heightmap_x1_y0.GetHeight(Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_2, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					v_flt Variable_3; // Heightmap: heightmap x1 y1 output 0
					FVoxelMaterial Heightmap__heightmap_x1_y1_0_Temp_1; // Heightmap: heightmap x1 y1 output 1
					v_flt Heightmap__heightmap_x1_y1_0_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_0_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_0_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_0_Temp_5; // Heightmap: heightmap x1 y1 output 5
					Variable_3 = Params.heightmap_x1_y1.GetHeight(Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_3, BufferConstant.Variable_5, BufferConstant.Variable_6);
				}
			}
		}
		
		void Function1_X_Compute(const FVoxelContext& Context, FBufferX& BufferX, v_flt Variable_29, v_flt Variable_30, v_flt Variable_31) const
		{
			// X
			BufferX.Variable_21 = Context.GetLocalX();
			
		}
		
		void Function1_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY, v_flt Variable_29, v_flt Variable_30, v_flt Variable_31) const
		{
			// Y
			v_flt Variable_22; // Y output 0
			Variable_22 = Context.GetLocalY();
			
			// Box SDF
			BufferXY.Variable_20 = FVoxelSDFNodeFunctions::Box(BufferX.Variable_21, Variable_22, v_flt(0.0f), Variable_30, Variable_31, v_flt(1000000000.0f));
			
		}
		
		void Function1_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY, v_flt Variable_29, v_flt Variable_30, v_flt Variable_31) const
		{
			// X
			BufferX.Variable_21 = Context.GetLocalX();
			
			// Y
			v_flt Variable_22; // Y output 0
			Variable_22 = Context.GetLocalY();
			
			// Box SDF
			BufferXY.Variable_20 = FVoxelSDFNodeFunctions::Box(BufferX.Variable_21, Variable_22, v_flt(0.0f), Variable_30, Variable_31, v_flt(1000000000.0f));
			
		}
		
		void Function1_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs, v_flt Variable_29, v_flt Variable_30, v_flt Variable_31) const
		{
			// Z
			v_flt Variable_18; // Z output 0
			Variable_18 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// +
			v_flt Variable_25; // + output 0
			Variable_25 = Variable_24 + BufferConstant.Variable_26;
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_18 - Variable_29;
			
			// * -1
			v_flt Variable_27; // * -1 output 0
			Variable_27 = Variable_25 * -1;
			
			// Max (float)
			v_flt Variable_23; // Max (float) output 0
			Variable_23 = FVoxelNodeFunctions::Max<v_flt>(Variable_19, FVoxelNodeFunctions::Max<v_flt>(BufferXY.Variable_20, Variable_27));
			
			// Set High Quality Value.*
			v_flt Variable_28; // Set High Quality Value.* output 0
			Variable_28 = Variable_23 * v_flt(0.2f);
			
			Outputs.Value = Variable_28;
		}
		
		void Function1_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs, v_flt Variable_29, v_flt Variable_30, v_flt Variable_31) const
		{
			// X
			v_flt Variable_21; // X output 0
			Variable_21 = Context.GetLocalX();
			
			// Y
			v_flt Variable_22; // Y output 0
			Variable_22 = Context.GetLocalY();
			
			// Z
			v_flt Variable_18; // Z output 0
			Variable_18 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// +
			v_flt Variable_25; // + output 0
			Variable_25 = Variable_24 + BufferConstant.Variable_26;
			
			// Box SDF
			v_flt Variable_20; // Box SDF output 0
			Variable_20 = FVoxelSDFNodeFunctions::Box(Variable_21, Variable_22, v_flt(0.0f), Variable_30, Variable_31, v_flt(1000000000.0f));
			
			// -
			v_flt Variable_19; // - output 0
			Variable_19 = Variable_18 - Variable_29;
			
			// * -1
			v_flt Variable_27; // * -1 output 0
			Variable_27 = Variable_25 * -1;
			
			// Max (float)
			v_flt Variable_23; // Max (float) output 0
			Variable_23 = FVoxelNodeFunctions::Max<v_flt>(Variable_19, FVoxelNodeFunctions::Max<v_flt>(Variable_20, Variable_27));
			
			// Set High Quality Value.*
			v_flt Variable_28; // Set High Quality Value.* output 0
			Variable_28 = Variable_23 * v_flt(0.2f);
			
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
			
			v_flt Variable_6; // Size Y = 512.0 output 0
			bool Variable_11; // Flip X = False output 0
			bool Variable_12; // Flip Y = True output 0
			v_flt Variable_5; // Size X = 512.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_13; // Switch (float) output 0
			bool Variable_7; // < output 0
			v_flt Variable_9; // + output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			bool Variable_8; // < output 0
			FVoxelMaterial Variable_0; // Heightmap: heightmap x0 y0 output 1
			FVoxelMaterial Variable_1; // Heightmap: heightmap x0 y1 output 1
			FVoxelMaterial Variable_2; // Heightmap: heightmap x1 y0 output 1
			FVoxelMaterial Variable_3; // Heightmap: heightmap x1 y1 output 1
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
				Function1_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Size Y = 512.0
				BufferConstant.Variable_6 = Params.Size_Y;
				
				// Flip X = False
				BufferConstant.Variable_11 = Params.Flip_X;
				
				// Flip Y = True
				BufferConstant.Variable_12 = Params.Flip_Y;
				
				// Size X = 512.0
				BufferConstant.Variable_5 = Params.Size_X;
				
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
		
		void Function1_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// Switch (float)
			BufferX.Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// <
			BufferX.Variable_7 = BufferX.Variable_13 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				// +
				BufferX.Variable_9 = BufferX.Variable_13 + BufferConstant.Variable_5;
				
			}
			else
			{
			}
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			BufferXY.Variable_8 = Variable_15 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					v_flt Heightmap__heightmap_x0_y0_1_Temp_0; // Heightmap: heightmap x0 y0 output 0
					v_flt Heightmap__heightmap_x0_y0_1_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_1_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_1_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_1_Temp_5; // Heightmap: heightmap x0 y0 output 5
					BufferXY.Variable_0 = Params.heightmap_x0_y0.GetMaterial(BufferX.Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_0);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					v_flt Heightmap__heightmap_x0_y1_1_Temp_0; // Heightmap: heightmap x0 y1 output 0
					v_flt Heightmap__heightmap_x0_y1_1_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_1_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_1_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_1_Temp_5; // Heightmap: heightmap x0 y1 output 5
					BufferXY.Variable_1 = Params.heightmap_x0_y1.GetMaterial(BufferX.Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_1);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					v_flt Heightmap__heightmap_x1_y0_1_Temp_0; // Heightmap: heightmap x1 y0 output 0
					v_flt Heightmap__heightmap_x1_y0_1_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_1_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_1_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_1_Temp_5; // Heightmap: heightmap x1 y0 output 5
					BufferXY.Variable_2 = Params.heightmap_x1_y0.GetMaterial(BufferX.Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_2);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					v_flt Heightmap__heightmap_x1_y1_1_Temp_0; // Heightmap: heightmap x1 y1 output 0
					v_flt Heightmap__heightmap_x1_y1_1_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_1_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_1_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_1_Temp_5; // Heightmap: heightmap x1 y1 output 5
					BufferXY.Variable_3 = Params.heightmap_x1_y1.GetMaterial(BufferX.Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_3);
				}
			}
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			BufferX.Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			BufferXY.Variable_8 = Variable_15 < v_flt(0.0f);
			
			// <
			BufferX.Variable_7 = BufferX.Variable_13 < v_flt(0.0f);
			
			if (BufferX.Variable_7)
			{
				// +
				BufferX.Variable_9 = BufferX.Variable_13 + BufferConstant.Variable_5;
				
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					v_flt Heightmap__heightmap_x0_y0_1_Temp_0; // Heightmap: heightmap x0 y0 output 0
					v_flt Heightmap__heightmap_x0_y0_1_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_1_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_1_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_1_Temp_5; // Heightmap: heightmap x0 y0 output 5
					BufferXY.Variable_0 = Params.heightmap_x0_y0.GetMaterial(BufferX.Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_0);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					v_flt Heightmap__heightmap_x0_y1_1_Temp_0; // Heightmap: heightmap x0 y1 output 0
					v_flt Heightmap__heightmap_x0_y1_1_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_1_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_1_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_1_Temp_5; // Heightmap: heightmap x0 y1 output 5
					BufferXY.Variable_1 = Params.heightmap_x0_y1.GetMaterial(BufferX.Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_1);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					v_flt Heightmap__heightmap_x1_y0_1_Temp_0; // Heightmap: heightmap x1 y0 output 0
					v_flt Heightmap__heightmap_x1_y0_1_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_1_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_1_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_1_Temp_5; // Heightmap: heightmap x1 y0 output 5
					BufferXY.Variable_2 = Params.heightmap_x1_y0.GetMaterial(BufferX.Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_2);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					v_flt Heightmap__heightmap_x1_y1_1_Temp_0; // Heightmap: heightmap x1 y1 output 0
					v_flt Heightmap__heightmap_x1_y1_1_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_1_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_1_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_1_Temp_5; // Heightmap: heightmap x1 y1 output 5
					BufferXY.Variable_3 = Params.heightmap_x1_y1.GetMaterial(BufferX.Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYWithoutCache_Compute(Context, BufferX, BufferXY, BufferXY.Variable_3);
				}
			}
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			if (BufferX.Variable_7)
			{
				if (BufferXY.Variable_8)
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_0);
				}
				else
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_1);
				}
			}
			else
			{
				if (BufferXY.Variable_8)
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_2);
				}
				else
				{
					Function1_XYZWithCache_Compute(Context, BufferX, BufferXY, Outputs, BufferXY.Variable_3);
				}
			}
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			v_flt Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// * -1
			v_flt Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			v_flt Variable_13; // Switch (float) output 0
			Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// Switch (float)
			v_flt Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			bool Variable_8; // < output 0
			Variable_8 = Variable_15 < v_flt(0.0f);
			
			// <
			bool Variable_7; // < output 0
			Variable_7 = Variable_13 < v_flt(0.0f);
			
			if (Variable_7)
			{
				// +
				v_flt Variable_9; // + output 0
				Variable_9 = Variable_13 + BufferConstant.Variable_5;
				
				if (Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x0 y0
					v_flt Heightmap__heightmap_x0_y0_1_Temp_0; // Heightmap: heightmap x0 y0 output 0
					FVoxelMaterial Variable_0; // Heightmap: heightmap x0 y0 output 1
					v_flt Heightmap__heightmap_x0_y0_1_Temp_2; // Heightmap: heightmap x0 y0 output 2
					v_flt Heightmap__heightmap_x0_y0_1_Temp_3; // Heightmap: heightmap x0 y0 output 3
					v_flt Heightmap__heightmap_x0_y0_1_Temp_4; // Heightmap: heightmap x0 y0 output 4
					v_flt Heightmap__heightmap_x0_y0_1_Temp_5; // Heightmap: heightmap x0 y0 output 5
					Variable_0 = Params.heightmap_x0_y0.GetMaterial(Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_0);
				}
				else
				{
					// Heightmap: heightmap x0 y1
					v_flt Heightmap__heightmap_x0_y1_1_Temp_0; // Heightmap: heightmap x0 y1 output 0
					FVoxelMaterial Variable_1; // Heightmap: heightmap x0 y1 output 1
					v_flt Heightmap__heightmap_x0_y1_1_Temp_2; // Heightmap: heightmap x0 y1 output 2
					v_flt Heightmap__heightmap_x0_y1_1_Temp_3; // Heightmap: heightmap x0 y1 output 3
					v_flt Heightmap__heightmap_x0_y1_1_Temp_4; // Heightmap: heightmap x0 y1 output 4
					v_flt Heightmap__heightmap_x0_y1_1_Temp_5; // Heightmap: heightmap x0 y1 output 5
					Variable_1 = Params.heightmap_x0_y1.GetMaterial(Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_1);
				}
			}
			else
			{
				if (Variable_8)
				{
					// +
					v_flt Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					// Heightmap: heightmap x1 y0
					v_flt Heightmap__heightmap_x1_y0_1_Temp_0; // Heightmap: heightmap x1 y0 output 0
					FVoxelMaterial Variable_2; // Heightmap: heightmap x1 y0 output 1
					v_flt Heightmap__heightmap_x1_y0_1_Temp_2; // Heightmap: heightmap x1 y0 output 2
					v_flt Heightmap__heightmap_x1_y0_1_Temp_3; // Heightmap: heightmap x1 y0 output 3
					v_flt Heightmap__heightmap_x1_y0_1_Temp_4; // Heightmap: heightmap x1 y0 output 4
					v_flt Heightmap__heightmap_x1_y0_1_Temp_5; // Heightmap: heightmap x1 y0 output 5
					Variable_2 = Params.heightmap_x1_y0.GetMaterial(Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_2);
				}
				else
				{
					// Heightmap: heightmap x1 y1
					v_flt Heightmap__heightmap_x1_y1_1_Temp_0; // Heightmap: heightmap x1 y1 output 0
					FVoxelMaterial Variable_3; // Heightmap: heightmap x1 y1 output 1
					v_flt Heightmap__heightmap_x1_y1_1_Temp_2; // Heightmap: heightmap x1 y1 output 2
					v_flt Heightmap__heightmap_x1_y1_1_Temp_3; // Heightmap: heightmap x1 y1 output 3
					v_flt Heightmap__heightmap_x1_y1_1_Temp_4; // Heightmap: heightmap x1 y1 output 4
					v_flt Heightmap__heightmap_x1_y1_1_Temp_5; // Heightmap: heightmap x1 y1 output 5
					Variable_3 = Params.heightmap_x1_y1.GetMaterial(Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
					
					Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_3);
				}
			}
		}
		
		void Function1_X_Compute(const FVoxelContext& Context, FBufferX& BufferX, FVoxelMaterial Variable_18) const
		{
		}
		
		void Function1_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY, FVoxelMaterial Variable_18) const
		{
		}
		
		void Function1_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY, FVoxelMaterial Variable_18) const
		{
		}
		
		void Function1_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs, FVoxelMaterial Variable_18) const
		{
			Outputs.MaterialBuilder = Variable_18;
		}
		
		void Function1_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs, FVoxelMaterial Variable_18) const
		{
			Outputs.MaterialBuilder = Variable_18;
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
			
			TVoxelRange<v_flt> Variable_6; // Size Y = 512.0 output 0
			FVoxelBoolRange Variable_11; // Flip X = False output 0
			FVoxelBoolRange Variable_12; // Flip Y = True output 0
			TVoxelRange<v_flt> Variable_5; // Size X = 512.0 output 0
			TVoxelRange<v_flt> Variable_31; // Depth = 300.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_13; // Switch (float) output 0
			FVoxelBoolRange Variable_7; // < output 0
			FVoxelBoolRange Variable_18; // 4x Flow Merge.Is Single bool output 0
			TVoxelRange<v_flt> Variable_26; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			FVoxelBoolRange Variable_8; // < output 0
			FVoxelBoolRange Variable_19; // 4x Flow Merge.Is Single bool output 0
			TVoxelRange<v_flt> Variable_0; // Heightmap: heightmap x0 y0 output 0
			TVoxelRange<v_flt> Variable_1; // Heightmap: heightmap x0 y1 output 0
			TVoxelRange<v_flt> Variable_2; // Heightmap: heightmap x1 y0 output 0
			TVoxelRange<v_flt> Variable_3; // Heightmap: heightmap x1 y1 output 0
			TVoxelRange<v_flt> Variable_21; // 4x Flow Merge.Range Union output 0
			TVoxelRange<v_flt> Variable_22; // 4x Flow Merge.Range Union output 0
			TVoxelRange<v_flt> Variable_20; // 4x Flow Merge.Range Union output 0
			TVoxelRange<v_flt> Variable_25; // Box SDF output 0
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
				Function1_XYZWithoutCache_Init(InitStruct);
			}
			
			////////////////////////////////////////////////////
			//////////////// Compute constants /////////////////
			////////////////////////////////////////////////////
			{
				// Size Y = 512.0
				BufferConstant.Variable_6 = Params.Size_Y;
				
				// Flip X = False
				BufferConstant.Variable_11 = Params.Flip_X;
				
				// Flip Y = True
				BufferConstant.Variable_12 = Params.Flip_Y;
				
				// Size X = 512.0
				BufferConstant.Variable_5 = Params.Size_X;
				
				// Depth = 300.0
				BufferConstant.Variable_31 = Params.Depth;
				
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
		
		void Function1_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Y
			TVoxelRange<v_flt> Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// X
			TVoxelRange<v_flt> Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// * -1
			TVoxelRange<v_flt> Variable_14; // * -1 output 0
			Variable_14 = Variable_4 * -1;
			
			// * -1
			TVoxelRange<v_flt> Variable_17; // * -1 output 0
			Variable_17 = Variable_16 * -1;
			
			// Switch (float)
			TVoxelRange<v_flt> Variable_13; // Switch (float) output 0
			Variable_13 = FVoxelNodeFunctions::Switch(Variable_14, Variable_4, BufferConstant.Variable_11);
			
			// Switch (float)
			TVoxelRange<v_flt> Variable_15; // Switch (float) output 0
			Variable_15 = FVoxelNodeFunctions::Switch(Variable_17, Variable_16, BufferConstant.Variable_12);
			
			// <
			FVoxelBoolRange Variable_7; // < output 0
			Variable_7 = Variable_13 < TVoxelRange<v_flt>(0.0f);
			
			// 4x Flow Merge.Is Single bool
			FVoxelBoolRange Variable_18; // 4x Flow Merge.Is Single bool output 0
			Variable_18 = FVoxelNodeFunctions::IsSingleBool(Variable_7);
			
			if (Variable_18)
			{
				// <
				FVoxelBoolRange Variable_8; // < output 0
				Variable_8 = Variable_15 < TVoxelRange<v_flt>(0.0f);
				
				// 4x Flow Merge.Is Single bool
				FVoxelBoolRange Variable_19; // 4x Flow Merge.Is Single bool output 0
				Variable_19 = FVoxelNodeFunctions::IsSingleBool(Variable_8);
				
				if (Variable_19)
				{
					if (Variable_7)
					{
						// +
						TVoxelRange<v_flt> Variable_9; // + output 0
						Variable_9 = Variable_13 + BufferConstant.Variable_5;
						
						if (Variable_8)
						{
							// +
							TVoxelRange<v_flt> Variable_10; // + output 0
							Variable_10 = Variable_15 + BufferConstant.Variable_6;
							
							// Heightmap: heightmap x0 y0
							TVoxelRange<v_flt> Variable_0; // Heightmap: heightmap x0 y0 output 0
							FVoxelMaterialRange Heightmap__heightmap_x0_y0_2_Temp_1; // Heightmap: heightmap x0 y0 output 1
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_2; // Heightmap: heightmap x0 y0 output 2
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_3; // Heightmap: heightmap x0 y0 output 3
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_4; // Heightmap: heightmap x0 y0 output 4
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_5; // Heightmap: heightmap x0 y0 output 5
							Variable_0 = Params.heightmap_x0_y0.GetHeightRange(Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
							
							Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_0, BufferConstant.Variable_5, BufferConstant.Variable_6);
						}
						else
						{
							// Heightmap: heightmap x0 y1
							TVoxelRange<v_flt> Variable_1; // Heightmap: heightmap x0 y1 output 0
							FVoxelMaterialRange Heightmap__heightmap_x0_y1_2_Temp_1; // Heightmap: heightmap x0 y1 output 1
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_2; // Heightmap: heightmap x0 y1 output 2
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_3; // Heightmap: heightmap x0 y1 output 3
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_4; // Heightmap: heightmap x0 y1 output 4
							TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_5; // Heightmap: heightmap x0 y1 output 5
							Variable_1 = Params.heightmap_x0_y1.GetHeightRange(Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
							
							Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_1, BufferConstant.Variable_5, BufferConstant.Variable_6);
						}
					}
					else
					{
						if (Variable_8)
						{
							// +
							TVoxelRange<v_flt> Variable_10; // + output 0
							Variable_10 = Variable_15 + BufferConstant.Variable_6;
							
							// Heightmap: heightmap x1 y0
							TVoxelRange<v_flt> Variable_2; // Heightmap: heightmap x1 y0 output 0
							FVoxelMaterialRange Heightmap__heightmap_x1_y0_2_Temp_1; // Heightmap: heightmap x1 y0 output 1
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_2; // Heightmap: heightmap x1 y0 output 2
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_3; // Heightmap: heightmap x1 y0 output 3
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_4; // Heightmap: heightmap x1 y0 output 4
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_5; // Heightmap: heightmap x1 y0 output 5
							Variable_2 = Params.heightmap_x1_y0.GetHeightRange(Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
							
							Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_2, BufferConstant.Variable_5, BufferConstant.Variable_6);
						}
						else
						{
							// Heightmap: heightmap x1 y1
							TVoxelRange<v_flt> Variable_3; // Heightmap: heightmap x1 y1 output 0
							FVoxelMaterialRange Heightmap__heightmap_x1_y1_2_Temp_1; // Heightmap: heightmap x1 y1 output 1
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_2; // Heightmap: heightmap x1 y1 output 2
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_3; // Heightmap: heightmap x1 y1 output 3
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_4; // Heightmap: heightmap x1 y1 output 4
							TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_5; // Heightmap: heightmap x1 y1 output 5
							Variable_3 = Params.heightmap_x1_y1.GetHeightRange(Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
							
							Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_3, BufferConstant.Variable_5, BufferConstant.Variable_6);
						}
					}
				}
				else
				{
					// +
					TVoxelRange<v_flt> Variable_10; // + output 0
					Variable_10 = Variable_15 + BufferConstant.Variable_6;
					
					if (Variable_7)
					{
						// +
						TVoxelRange<v_flt> Variable_9; // + output 0
						Variable_9 = Variable_13 + BufferConstant.Variable_5;
						
						// Heightmap: heightmap x0 y0
						TVoxelRange<v_flt> Variable_0; // Heightmap: heightmap x0 y0 output 0
						FVoxelMaterialRange Heightmap__heightmap_x0_y0_2_Temp_1; // Heightmap: heightmap x0 y0 output 1
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_2; // Heightmap: heightmap x0 y0 output 2
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_3; // Heightmap: heightmap x0 y0 output 3
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_4; // Heightmap: heightmap x0 y0 output 4
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_5; // Heightmap: heightmap x0 y0 output 5
						Variable_0 = Params.heightmap_x0_y0.GetHeightRange(Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
						
						// Heightmap: heightmap x0 y1
						TVoxelRange<v_flt> Variable_1; // Heightmap: heightmap x0 y1 output 0
						FVoxelMaterialRange Heightmap__heightmap_x0_y1_2_Temp_1; // Heightmap: heightmap x0 y1 output 1
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_2; // Heightmap: heightmap x0 y1 output 2
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_3; // Heightmap: heightmap x0 y1 output 3
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_4; // Heightmap: heightmap x0 y1 output 4
						TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_5; // Heightmap: heightmap x0 y1 output 5
						Variable_1 = Params.heightmap_x0_y1.GetHeightRange(Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
						
						// 4x Flow Merge.Range Union
						TVoxelRange<v_flt> Variable_21; // 4x Flow Merge.Range Union output 0
						Variable_21 = FVoxelNodeFunctions::Union(Variable_0, Variable_1);
						
						Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_21, BufferConstant.Variable_5, BufferConstant.Variable_6);
					}
					else
					{
						// Heightmap: heightmap x1 y0
						TVoxelRange<v_flt> Variable_2; // Heightmap: heightmap x1 y0 output 0
						FVoxelMaterialRange Heightmap__heightmap_x1_y0_2_Temp_1; // Heightmap: heightmap x1 y0 output 1
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_2; // Heightmap: heightmap x1 y0 output 2
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_3; // Heightmap: heightmap x1 y0 output 3
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_4; // Heightmap: heightmap x1 y0 output 4
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_5; // Heightmap: heightmap x1 y0 output 5
						Variable_2 = Params.heightmap_x1_y0.GetHeightRange(Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
						
						// Heightmap: heightmap x1 y1
						TVoxelRange<v_flt> Variable_3; // Heightmap: heightmap x1 y1 output 0
						FVoxelMaterialRange Heightmap__heightmap_x1_y1_2_Temp_1; // Heightmap: heightmap x1 y1 output 1
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_2; // Heightmap: heightmap x1 y1 output 2
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_3; // Heightmap: heightmap x1 y1 output 3
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_4; // Heightmap: heightmap x1 y1 output 4
						TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_5; // Heightmap: heightmap x1 y1 output 5
						Variable_3 = Params.heightmap_x1_y1.GetHeightRange(Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
						
						// 4x Flow Merge.Range Union
						TVoxelRange<v_flt> Variable_22; // 4x Flow Merge.Range Union output 0
						Variable_22 = FVoxelNodeFunctions::Union(Variable_2, Variable_3);
						
						Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_22, BufferConstant.Variable_5, BufferConstant.Variable_6);
					}
				}
			}
			else
			{
				// +
				TVoxelRange<v_flt> Variable_10; // + output 0
				Variable_10 = Variable_15 + BufferConstant.Variable_6;
				
				// +
				TVoxelRange<v_flt> Variable_9; // + output 0
				Variable_9 = Variable_13 + BufferConstant.Variable_5;
				
				// Heightmap: heightmap x1 y1
				TVoxelRange<v_flt> Variable_3; // Heightmap: heightmap x1 y1 output 0
				FVoxelMaterialRange Heightmap__heightmap_x1_y1_2_Temp_1; // Heightmap: heightmap x1 y1 output 1
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_2; // Heightmap: heightmap x1 y1 output 2
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_3; // Heightmap: heightmap x1 y1 output 3
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_4; // Heightmap: heightmap x1 y1 output 4
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y1_2_Temp_5; // Heightmap: heightmap x1 y1 output 5
				Variable_3 = Params.heightmap_x1_y1.GetHeightRange(Variable_13, Variable_15, EVoxelSamplerMode::Clamp);
				
				// Heightmap: heightmap x0 y1
				TVoxelRange<v_flt> Variable_1; // Heightmap: heightmap x0 y1 output 0
				FVoxelMaterialRange Heightmap__heightmap_x0_y1_2_Temp_1; // Heightmap: heightmap x0 y1 output 1
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_2; // Heightmap: heightmap x0 y1 output 2
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_3; // Heightmap: heightmap x0 y1 output 3
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_4; // Heightmap: heightmap x0 y1 output 4
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y1_2_Temp_5; // Heightmap: heightmap x0 y1 output 5
				Variable_1 = Params.heightmap_x0_y1.GetHeightRange(Variable_9, Variable_15, EVoxelSamplerMode::Clamp);
				
				// Heightmap: heightmap x1 y0
				TVoxelRange<v_flt> Variable_2; // Heightmap: heightmap x1 y0 output 0
				FVoxelMaterialRange Heightmap__heightmap_x1_y0_2_Temp_1; // Heightmap: heightmap x1 y0 output 1
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_2; // Heightmap: heightmap x1 y0 output 2
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_3; // Heightmap: heightmap x1 y0 output 3
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_4; // Heightmap: heightmap x1 y0 output 4
				TVoxelRange<v_flt> Heightmap__heightmap_x1_y0_2_Temp_5; // Heightmap: heightmap x1 y0 output 5
				Variable_2 = Params.heightmap_x1_y0.GetHeightRange(Variable_13, Variable_10, EVoxelSamplerMode::Clamp);
				
				// Heightmap: heightmap x0 y0
				TVoxelRange<v_flt> Variable_0; // Heightmap: heightmap x0 y0 output 0
				FVoxelMaterialRange Heightmap__heightmap_x0_y0_2_Temp_1; // Heightmap: heightmap x0 y0 output 1
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_2; // Heightmap: heightmap x0 y0 output 2
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_3; // Heightmap: heightmap x0 y0 output 3
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_4; // Heightmap: heightmap x0 y0 output 4
				TVoxelRange<v_flt> Heightmap__heightmap_x0_y0_2_Temp_5; // Heightmap: heightmap x0 y0 output 5
				Variable_0 = Params.heightmap_x0_y0.GetHeightRange(Variable_9, Variable_10, EVoxelSamplerMode::Clamp);
				
				// 4x Flow Merge.Range Union
				TVoxelRange<v_flt> Variable_20; // 4x Flow Merge.Range Union output 0
				Variable_20 = FVoxelNodeFunctions::Union(Variable_0, FVoxelNodeFunctions::Union(Variable_1, FVoxelNodeFunctions::Union(Variable_2, Variable_3)));
				
				Function1_XYZWithoutCache_Compute(Context, Outputs, Variable_20, BufferConstant.Variable_5, BufferConstant.Variable_6);
			}
		}
		
		void Function1_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs, TVoxelRange<v_flt> Variable_34, TVoxelRange<v_flt> Variable_35, TVoxelRange<v_flt> Variable_36) const
		{
			// X
			TVoxelRange<v_flt> Variable_26; // X output 0
			Variable_26 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_27; // Y output 0
			Variable_27 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_23; // Z output 0
			Variable_23 = Context.GetLocalZ();
			
			// Z
			TVoxelRange<v_flt> Variable_29; // Z output 0
			Variable_29 = Context.GetLocalZ();
			
			// +
			TVoxelRange<v_flt> Variable_30; // + output 0
			Variable_30 = Variable_29 + BufferConstant.Variable_31;
			
			// Box SDF
			TVoxelRange<v_flt> Variable_25; // Box SDF output 0
			Variable_25 = FVoxelSDFNodeFunctions::Box(Variable_26, Variable_27, TVoxelRange<v_flt>(0.0f), Variable_35, Variable_36, TVoxelRange<v_flt>(1000000000.0f));
			
			// -
			TVoxelRange<v_flt> Variable_24; // - output 0
			Variable_24 = Variable_23 - Variable_34;
			
			// * -1
			TVoxelRange<v_flt> Variable_32; // * -1 output 0
			Variable_32 = Variable_30 * -1;
			
			// Max (float)
			TVoxelRange<v_flt> Variable_28; // Max (float) output 0
			Variable_28 = FVoxelNodeFunctions::Max<v_flt>(Variable_24, FVoxelNodeFunctions::Max<v_flt>(Variable_25, Variable_32));
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_33; // Set High Quality Value.* output 0
			Variable_33 = Variable_28 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_33;
		}
		
	};
	
	FVoxelExample_HeightmapCompositionInstance(UVoxelExample_HeightmapComposition& Object)
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
			Object.Depth,
			Object.Flip_X,
			Object.Flip_Y,
			TVoxelHeightmapAssetSamplerWrapper<uint16>(Object.heightmap_x0_y0.LoadSynchronous()),
			TVoxelHeightmapAssetSamplerWrapper<uint16>(Object.heightmap_x0_y1.LoadSynchronous()),
			TVoxelHeightmapAssetSamplerWrapper<uint16>(Object.heightmap_x1_y0.LoadSynchronous()),
			TVoxelHeightmapAssetSamplerWrapper<uint16>(Object.heightmap_x1_y1.LoadSynchronous()),
			Object.Size_X,
			Object.Size_Y
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
inline v_flt FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_HeightmapCompositionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_HeightmapCompositionInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_HeightmapCompositionInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_HeightmapCompositionInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_HeightmapCompositionInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_HeightmapComposition::UVoxelExample_HeightmapComposition()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_HeightmapComposition::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_HeightmapCompositionInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_HeightmapComposition. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_HeightmapComposition. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_HeightmapComposition. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_HeightmapComposition. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
