// Copyright 2020 Phyronnaz

#include "VoxelExample_LayeredWorld.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_LayeredWorldInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_LayeredWorldInstance, UVoxelExample_LayeredWorld>
{
public:
	struct FParams
	{
		const float Frequency;
		const FVoxelRichCurve None1;
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
			
			v_flt Variable_35; // Frequency = 0.005 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_9; // X output 0
			v_flt Variable_31; // X output 0
			v_flt Variable_22; // X output 0
			v_flt Variable_15; // X output 0
			v_flt Variable_18; // X output 0
			v_flt Variable_26; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_32; // Y output 0
			v_flt Variable_10; // Y output 0
			v_flt Variable_27; // Y output 0
			v_flt Variable_19; // Y output 0
			v_flt Variable_23; // Y output 0
			v_flt Variable_6; // * output 0
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
					
					// Init of Seed = 4761
					FVoxelGraphSeed Variable_42; // Seed = 4761 output 0
					Variable_42 = Params.Seed;
					
					// Init of Make Seeds
					FVoxelGraphSeed Variable_37; // Make Seeds output 0
					FVoxelGraphSeed Variable_38; // Make Seeds output 1
					FVoxelGraphSeed Variable_39; // Make Seeds output 2
					FVoxelGraphSeed Variable_40; // Make Seeds output 3
					FVoxelGraphSeed Variable_41; // Make Seeds output 4
					Variable_37 = FVoxelUtilities::MurmurHash32(Variable_42);
					Variable_38 = FVoxelUtilities::MurmurHash32(Variable_37);
					Variable_39 = FVoxelUtilities::MurmurHash32(Variable_38);
					Variable_40 = FVoxelUtilities::MurmurHash32(Variable_39);
					Variable_41 = FVoxelUtilities::MurmurHash32(Variable_40);
					
					
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
				// Frequency = 0.005
				BufferConstant.Variable_35 = Params.Frequency;
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_0_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_0_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_1_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_2_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_3_Noise;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 4761
			FVoxelGraphSeed Variable_42; // Seed = 4761 output 0
			Variable_42 = Params.Seed;
			
			// Init of Make Seeds
			FVoxelGraphSeed Variable_37; // Make Seeds output 0
			FVoxelGraphSeed Variable_38; // Make Seeds output 1
			FVoxelGraphSeed Variable_39; // Make Seeds output 2
			FVoxelGraphSeed Variable_40; // Make Seeds output 3
			FVoxelGraphSeed Variable_41; // Make Seeds output 4
			Variable_37 = FVoxelUtilities::MurmurHash32(Variable_42);
			Variable_38 = FVoxelUtilities::MurmurHash32(Variable_37);
			Variable_39 = FVoxelUtilities::MurmurHash32(Variable_38);
			Variable_40 = FVoxelUtilities::MurmurHash32(Variable_39);
			Variable_41 = FVoxelUtilities::MurmurHash32(Variable_40);
			
			// Init of 2D Perlin Noise
			_2D_Perlin_Noise_0_Noise.SetSeed(Variable_37);
			_2D_Perlin_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_0_Noise.SetSeed(Variable_38);
			_3D_Perlin_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_1_Noise.SetSeed(Variable_39);
			_3D_Perlin_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_2_Noise.SetSeed(Variable_41);
			_3D_Perlin_Noise_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_3_Noise.SetSeed(Variable_40);
			_3D_Perlin_Noise_3_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_9 = Context.GetLocalX();
			
			// X
			BufferX.Variable_31 = Context.GetLocalX();
			
			// X
			BufferX.Variable_22 = Context.GetLocalX();
			
			// X
			BufferX.Variable_15 = Context.GetLocalX();
			
			// X
			BufferX.Variable_18 = Context.GetLocalX();
			
			// X
			BufferX.Variable_26 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_32 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_10 = Context.GetLocalY();
			
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_27 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_19 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_23 = Context.GetLocalY();
			
			// 2D Perlin Noise
			v_flt Variable_36; // 2D Perlin Noise output 0
			Variable_36 = _2D_Perlin_Noise_0_Noise.GetPerlin_2D(BufferX.Variable_15, Variable_16, v_flt(0.01f));
			Variable_36 = FMath::Clamp<v_flt>(Variable_36, -0.767493, 0.745772);
			
			// Float Curve: 
			v_flt Variable_8; // Float Curve:  output 0
			Variable_8 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_36);
			
			// *
			BufferXY.Variable_6 = Variable_8 * v_flt(100.0f);
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_32 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_10 = Context.GetLocalY();
			
			// X
			BufferX.Variable_9 = Context.GetLocalX();
			
			// X
			BufferX.Variable_31 = Context.GetLocalX();
			
			// X
			BufferX.Variable_22 = Context.GetLocalX();
			
			// X
			BufferX.Variable_15 = Context.GetLocalX();
			
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_27 = Context.GetLocalY();
			
			// X
			BufferX.Variable_18 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_19 = Context.GetLocalY();
			
			// X
			BufferX.Variable_26 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_23 = Context.GetLocalY();
			
			// 2D Perlin Noise
			v_flt Variable_36; // 2D Perlin Noise output 0
			Variable_36 = _2D_Perlin_Noise_0_Noise.GetPerlin_2D(BufferX.Variable_15, Variable_16, v_flt(0.01f));
			Variable_36 = FMath::Clamp<v_flt>(Variable_36, -0.767493, 0.745772);
			
			// Float Curve: 
			v_flt Variable_8; // Float Curve:  output 0
			Variable_8 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_36);
			
			// *
			BufferXY.Variable_6 = Variable_8 * v_flt(100.0f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_33; // Z output 0
			Variable_33 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_28; // Z output 0
			Variable_28 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_20; // Z output 0
			Variable_20 = Context.GetLocalZ();
			
			// +
			v_flt Variable_1; // + output 0
			Variable_1 = v_flt(100.0f) + Variable_0;
			
			// Data Item Sample
			v_flt Variable_14; // Data Item Sample output 0
			Variable_14 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, BufferX.Variable_9, BufferXY.Variable_10, Variable_11, v_flt(5.0f), v_flt(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// -
			v_flt Variable_5; // - output 0
			Variable_5 = Variable_0 - v_flt(75.0f);
			
			// +
			v_flt Variable_4; // + output 0
			Variable_4 = v_flt(200.0f) + Variable_0;
			
			// +
			v_flt Variable_29; // + output 0
			Variable_29 = v_flt(300.0f) + Variable_0;
			
			// 3D Perlin Noise
			v_flt Variable_17; // 3D Perlin Noise output 0
			Variable_17 = _3D_Perlin_Noise_0_Noise.GetPerlin_3D(BufferX.Variable_18, BufferXY.Variable_19, Variable_20, BufferConstant.Variable_35);
			Variable_17 = FMath::Clamp<v_flt>(Variable_17, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_21; // 3D Perlin Noise output 0
			Variable_21 = _3D_Perlin_Noise_1_Noise.GetPerlin_3D(BufferX.Variable_22, BufferXY.Variable_23, Variable_24, BufferConstant.Variable_35);
			Variable_21 = FMath::Clamp<v_flt>(Variable_21, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_30; // 3D Perlin Noise output 0
			Variable_30 = _3D_Perlin_Noise_2_Noise.GetPerlin_3D(BufferX.Variable_31, BufferXY.Variable_32, Variable_33, BufferConstant.Variable_35);
			Variable_30 = FMath::Clamp<v_flt>(Variable_30, -0.790554, 0.781474);
			
			// * -1
			v_flt Variable_13; // * -1 output 0
			Variable_13 = Variable_14 * -1;
			
			// 3D Perlin Noise
			v_flt Variable_25; // 3D Perlin Noise output 0
			Variable_25 = _3D_Perlin_Noise_3_Noise.GetPerlin_3D(BufferX.Variable_26, BufferXY.Variable_27, Variable_28, BufferConstant.Variable_35);
			Variable_25 = FMath::Clamp<v_flt>(Variable_25, -0.790554, 0.781474);
			
			// Cave Layer.*
			v_flt Variable_47; // Cave Layer.* output 0
			Variable_47 = Variable_21 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_62; // Cave Layer.* output 0
			Variable_62 = Variable_25 * v_flt(50.0f);
			
			// 2D Noise SDF.*
			v_flt Variable_66; // 2D Noise SDF.* output 0
			Variable_66 = Variable_30 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_54; // Cave Layer.* output 0
			Variable_54 = Variable_17 * v_flt(50.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_44; // Cave Layer.1 - X output 0
			Variable_44 = 1 - Variable_47;
			
			// Cave Layer.-
			v_flt Variable_46; // Cave Layer.- output 0
			Variable_46 = Variable_47 - Variable_1;
			
			// Cave Layer.1 - X
			v_flt Variable_51; // Cave Layer.1 - X output 0
			Variable_51 = 1 - Variable_54;
			
			// Cave Layer.-
			v_flt Variable_53; // Cave Layer.- output 0
			Variable_53 = Variable_54 - Variable_0;
			
			// 2D Noise SDF.+
			v_flt Variable_3; // 2D Noise SDF.+ output 0
			Variable_3 = Variable_66 + v_flt(0.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_59; // Cave Layer.1 - X output 0
			Variable_59 = 1 - Variable_62;
			
			// Cave Layer.-
			v_flt Variable_61; // Cave Layer.- output 0
			Variable_61 = Variable_62 - Variable_4;
			
			// Cave Layer.*
			v_flt Variable_49; // Cave Layer.* output 0
			Variable_49 = v_flt(0.2f) * Variable_44;
			
			// Cave Layer.*
			v_flt Variable_56; // Cave Layer.* output 0
			Variable_56 = v_flt(0.2f) * Variable_51;
			
			// Cave Layer.*
			v_flt Variable_64; // Cave Layer.* output 0
			Variable_64 = v_flt(0.2f) * Variable_59;
			
			// 2D Noise SDF.-
			v_flt Variable_65; // 2D Noise SDF.- output 0
			Variable_65 = Variable_29 - Variable_3;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_5 - BufferXY.Variable_6;
			
			// * -1
			v_flt Variable_34; // * -1 output 0
			Variable_34 = Variable_65 * -1;
			
			// Cave Layer.-
			v_flt Variable_52; // Cave Layer.- output 0
			Variable_52 = Variable_0 - Variable_56;
			
			// Cave Layer.-
			v_flt Variable_60; // Cave Layer.- output 0
			Variable_60 = Variable_4 - Variable_64;
			
			// Cave Layer.-
			v_flt Variable_45; // Cave Layer.- output 0
			Variable_45 = Variable_1 - Variable_49;
			
			// Cave Layer.Smooth Union
			v_flt Variable_50; // Cave Layer.Smooth Union output 0
			Variable_50 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_52, Variable_53, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_43; // Cave Layer.Smooth Union output 0
			Variable_43 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_45, Variable_46, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_58; // Cave Layer.Smooth Union output 0
			Variable_58 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_60, Variable_61, v_flt(100.0f));
			
			// Cave Layer.+
			v_flt Variable_55; // Cave Layer.+ output 0
			Variable_55 = Variable_50 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_63; // Cave Layer.+ output 0
			Variable_63 = Variable_58 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_48; // Cave Layer.+ output 0
			Variable_48 = Variable_43 + v_flt(25.0f);
			
			// Max (float)
			v_flt Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, FVoxelNodeFunctions::Max<v_flt>(Variable_55, FVoxelNodeFunctions::Max<v_flt>(Variable_48, FVoxelNodeFunctions::Max<v_flt>(Variable_63, Variable_34))));
			
			// Smooth Intersection
			v_flt Variable_12; // Smooth Intersection output 0
			Variable_12 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_13, v_flt(5.0f));
			
			// Set High Quality Value.*
			v_flt Variable_57; // Set High Quality Value.* output 0
			Variable_57 = Variable_12 * v_flt(0.2f);
			
			Outputs.Value = Variable_57;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_32; // Y output 0
			Variable_32 = Context.GetLocalY();
			
			// Z
			v_flt Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_10; // Y output 0
			Variable_10 = Context.GetLocalY();
			
			// X
			v_flt Variable_9; // X output 0
			Variable_9 = Context.GetLocalX();
			
			// X
			v_flt Variable_31; // X output 0
			Variable_31 = Context.GetLocalX();
			
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_33; // Z output 0
			Variable_33 = Context.GetLocalZ();
			
			// X
			v_flt Variable_22; // X output 0
			Variable_22 = Context.GetLocalX();
			
			// X
			v_flt Variable_15; // X output 0
			Variable_15 = Context.GetLocalX();
			
			// Y
			v_flt Variable_16; // Y output 0
			Variable_16 = Context.GetLocalY();
			
			// Z
			v_flt Variable_28; // Z output 0
			Variable_28 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_27; // Y output 0
			Variable_27 = Context.GetLocalY();
			
			// Z
			v_flt Variable_20; // Z output 0
			Variable_20 = Context.GetLocalZ();
			
			// X
			v_flt Variable_18; // X output 0
			Variable_18 = Context.GetLocalX();
			
			// Y
			v_flt Variable_19; // Y output 0
			Variable_19 = Context.GetLocalY();
			
			// X
			v_flt Variable_26; // X output 0
			Variable_26 = Context.GetLocalX();
			
			// Y
			v_flt Variable_23; // Y output 0
			Variable_23 = Context.GetLocalY();
			
			// +
			v_flt Variable_1; // + output 0
			Variable_1 = v_flt(100.0f) + Variable_0;
			
			// Data Item Sample
			v_flt Variable_14; // Data Item Sample output 0
			Variable_14 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_9, Variable_10, Variable_11, v_flt(5.0f), v_flt(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// -
			v_flt Variable_5; // - output 0
			Variable_5 = Variable_0 - v_flt(75.0f);
			
			// +
			v_flt Variable_4; // + output 0
			Variable_4 = v_flt(200.0f) + Variable_0;
			
			// +
			v_flt Variable_29; // + output 0
			Variable_29 = v_flt(300.0f) + Variable_0;
			
			// 2D Perlin Noise
			v_flt Variable_36; // 2D Perlin Noise output 0
			Variable_36 = _2D_Perlin_Noise_0_Noise.GetPerlin_2D(Variable_15, Variable_16, v_flt(0.01f));
			Variable_36 = FMath::Clamp<v_flt>(Variable_36, -0.767493, 0.745772);
			
			// 3D Perlin Noise
			v_flt Variable_17; // 3D Perlin Noise output 0
			Variable_17 = _3D_Perlin_Noise_0_Noise.GetPerlin_3D(Variable_18, Variable_19, Variable_20, BufferConstant.Variable_35);
			Variable_17 = FMath::Clamp<v_flt>(Variable_17, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_21; // 3D Perlin Noise output 0
			Variable_21 = _3D_Perlin_Noise_1_Noise.GetPerlin_3D(Variable_22, Variable_23, Variable_24, BufferConstant.Variable_35);
			Variable_21 = FMath::Clamp<v_flt>(Variable_21, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_30; // 3D Perlin Noise output 0
			Variable_30 = _3D_Perlin_Noise_2_Noise.GetPerlin_3D(Variable_31, Variable_32, Variable_33, BufferConstant.Variable_35);
			Variable_30 = FMath::Clamp<v_flt>(Variable_30, -0.790554, 0.781474);
			
			// * -1
			v_flt Variable_13; // * -1 output 0
			Variable_13 = Variable_14 * -1;
			
			// 3D Perlin Noise
			v_flt Variable_25; // 3D Perlin Noise output 0
			Variable_25 = _3D_Perlin_Noise_3_Noise.GetPerlin_3D(Variable_26, Variable_27, Variable_28, BufferConstant.Variable_35);
			Variable_25 = FMath::Clamp<v_flt>(Variable_25, -0.790554, 0.781474);
			
			// Float Curve: 
			v_flt Variable_8; // Float Curve:  output 0
			Variable_8 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_36);
			
			// Cave Layer.*
			v_flt Variable_47; // Cave Layer.* output 0
			Variable_47 = Variable_21 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_62; // Cave Layer.* output 0
			Variable_62 = Variable_25 * v_flt(50.0f);
			
			// 2D Noise SDF.*
			v_flt Variable_66; // 2D Noise SDF.* output 0
			Variable_66 = Variable_30 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_54; // Cave Layer.* output 0
			Variable_54 = Variable_17 * v_flt(50.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_44; // Cave Layer.1 - X output 0
			Variable_44 = 1 - Variable_47;
			
			// Cave Layer.-
			v_flt Variable_46; // Cave Layer.- output 0
			Variable_46 = Variable_47 - Variable_1;
			
			// Cave Layer.1 - X
			v_flt Variable_51; // Cave Layer.1 - X output 0
			Variable_51 = 1 - Variable_54;
			
			// Cave Layer.-
			v_flt Variable_53; // Cave Layer.- output 0
			Variable_53 = Variable_54 - Variable_0;
			
			// 2D Noise SDF.+
			v_flt Variable_3; // 2D Noise SDF.+ output 0
			Variable_3 = Variable_66 + v_flt(0.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_59; // Cave Layer.1 - X output 0
			Variable_59 = 1 - Variable_62;
			
			// Cave Layer.-
			v_flt Variable_61; // Cave Layer.- output 0
			Variable_61 = Variable_62 - Variable_4;
			
			// *
			v_flt Variable_6; // * output 0
			Variable_6 = Variable_8 * v_flt(100.0f);
			
			// Cave Layer.*
			v_flt Variable_49; // Cave Layer.* output 0
			Variable_49 = v_flt(0.2f) * Variable_44;
			
			// Cave Layer.*
			v_flt Variable_56; // Cave Layer.* output 0
			Variable_56 = v_flt(0.2f) * Variable_51;
			
			// Cave Layer.*
			v_flt Variable_64; // Cave Layer.* output 0
			Variable_64 = v_flt(0.2f) * Variable_59;
			
			// 2D Noise SDF.-
			v_flt Variable_65; // 2D Noise SDF.- output 0
			Variable_65 = Variable_29 - Variable_3;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_5 - Variable_6;
			
			// * -1
			v_flt Variable_34; // * -1 output 0
			Variable_34 = Variable_65 * -1;
			
			// Cave Layer.-
			v_flt Variable_52; // Cave Layer.- output 0
			Variable_52 = Variable_0 - Variable_56;
			
			// Cave Layer.-
			v_flt Variable_60; // Cave Layer.- output 0
			Variable_60 = Variable_4 - Variable_64;
			
			// Cave Layer.-
			v_flt Variable_45; // Cave Layer.- output 0
			Variable_45 = Variable_1 - Variable_49;
			
			// Cave Layer.Smooth Union
			v_flt Variable_50; // Cave Layer.Smooth Union output 0
			Variable_50 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_52, Variable_53, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_43; // Cave Layer.Smooth Union output 0
			Variable_43 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_45, Variable_46, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_58; // Cave Layer.Smooth Union output 0
			Variable_58 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_60, Variable_61, v_flt(100.0f));
			
			// Cave Layer.+
			v_flt Variable_55; // Cave Layer.+ output 0
			Variable_55 = Variable_50 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_63; // Cave Layer.+ output 0
			Variable_63 = Variable_58 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_48; // Cave Layer.+ output 0
			Variable_48 = Variable_43 + v_flt(25.0f);
			
			// Max (float)
			v_flt Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, FVoxelNodeFunctions::Max<v_flt>(Variable_55, FVoxelNodeFunctions::Max<v_flt>(Variable_48, FVoxelNodeFunctions::Max<v_flt>(Variable_63, Variable_34))));
			
			// Smooth Intersection
			v_flt Variable_12; // Smooth Intersection output 0
			Variable_12 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_13, v_flt(5.0f));
			
			// Set High Quality Value.*
			v_flt Variable_57; // Set High Quality Value.* output 0
			Variable_57 = Variable_12 * v_flt(0.2f);
			
			Outputs.Value = Variable_57;
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
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// <=
			bool Variable_4; // <= output 0
			Variable_4 = Variable_0 <= v_flt(-40.0f);
			
			// <=
			bool Variable_3; // <= output 0
			Variable_3 = Variable_0 <= v_flt(50.0f);
			
			// <=
			bool Variable_6; // <= output 0
			Variable_6 = Variable_0 <= v_flt(-250.0f);
			
			// Switch (color)
			FColor Variable_1; // Switch (color) output 0
			Variable_1 = FVoxelNodeFunctions::Switch(FColor(FColor(14, 71, 255, 255)), FColor(FColor(229, 0, 255, 255)), Variable_3);
			
			// Switch (color)
			FColor Variable_2; // Switch (color) output 0
			Variable_2 = FVoxelNodeFunctions::Switch(FColor(FColor(255, 142, 44, 255)), Variable_1, Variable_4);
			
			// Switch (color)
			FColor Variable_5; // Switch (color) output 0
			Variable_5 = FVoxelNodeFunctions::Switch(FColor(FColor(3, 143, 0, 255)), Variable_2, Variable_6);
			
			Outputs.MaterialBuilder.SetColor(Variable_5);
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// <=
			bool Variable_4; // <= output 0
			Variable_4 = Variable_0 <= v_flt(-40.0f);
			
			// <=
			bool Variable_3; // <= output 0
			Variable_3 = Variable_0 <= v_flt(50.0f);
			
			// <=
			bool Variable_6; // <= output 0
			Variable_6 = Variable_0 <= v_flt(-250.0f);
			
			// Switch (color)
			FColor Variable_1; // Switch (color) output 0
			Variable_1 = FVoxelNodeFunctions::Switch(FColor(FColor(14, 71, 255, 255)), FColor(FColor(229, 0, 255, 255)), Variable_3);
			
			// Switch (color)
			FColor Variable_2; // Switch (color) output 0
			Variable_2 = FVoxelNodeFunctions::Switch(FColor(FColor(255, 142, 44, 255)), Variable_1, Variable_4);
			
			// Switch (color)
			FColor Variable_5; // Switch (color) output 0
			Variable_5 = FVoxelNodeFunctions::Switch(FColor(FColor(3, 143, 0, 255)), Variable_2, Variable_6);
			
			Outputs.MaterialBuilder.SetColor(Variable_5);
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
			
			TVoxelRange<v_flt> Variable_26; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_33; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_41; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_3; // 2D Noise SDF.+ output 0
			TVoxelRange<v_flt> Variable_6; // * output 0
			TVoxelRange<v_flt> Variable_43; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_28; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_35; // Cave Layer.* output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_9; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_10; // Y output 0
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
					
					// Init of 2D Perlin Noise
					_2D_Perlin_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Perlin_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_4_Noise.SetSeed(FVoxelGraphSeed(1339));
					_3D_Perlin_Noise_4_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_5_Noise.SetSeed(FVoxelGraphSeed(1330));
					_3D_Perlin_Noise_5_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_6_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_Perlin_Noise_6_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_7_Noise.SetSeed(FVoxelGraphSeed(13339));
					_3D_Perlin_Noise_7_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
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
				// 2D Perlin Noise
				TVoxelRange<v_flt> Variable_21; // 2D Perlin Noise output 0
				Variable_21 = { -0.767493f, 0.745772f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_17; // 3D Perlin Noise output 0
				Variable_17 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_15; // 3D Perlin Noise output 0
				Variable_15 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_16; // 3D Perlin Noise output 0
				Variable_16 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_19; // 3D Perlin Noise output 0
				Variable_19 = { -0.790554f, 0.781474f };
				
				// Float Curve: 
				TVoxelRange<v_flt> Variable_8; // Float Curve:  output 0
				Variable_8 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_21);
				
				// Cave Layer.*
				BufferConstant.Variable_26 = Variable_16 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.*
				BufferConstant.Variable_33 = Variable_15 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.*
				BufferConstant.Variable_41 = Variable_17 * TVoxelRange<v_flt>(50.0f);
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_45; // 2D Noise SDF.* output 0
				Variable_45 = Variable_19 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_30; // Cave Layer.1 - X output 0
				Variable_30 = 1 - BufferConstant.Variable_33;
				
				// 2D Noise SDF.+
				BufferConstant.Variable_3 = Variable_45 + TVoxelRange<v_flt>(0.0f);
				
				// *
				BufferConstant.Variable_6 = Variable_8 * TVoxelRange<v_flt>(100.0f);
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_23; // Cave Layer.1 - X output 0
				Variable_23 = 1 - BufferConstant.Variable_26;
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_38; // Cave Layer.1 - X output 0
				Variable_38 = 1 - BufferConstant.Variable_41;
				
				// Cave Layer.*
				BufferConstant.Variable_43 = TVoxelRange<v_flt>(0.2f) * Variable_38;
				
				// Cave Layer.*
				BufferConstant.Variable_28 = TVoxelRange<v_flt>(0.2f) * Variable_23;
				
				// Cave Layer.*
				BufferConstant.Variable_35 = TVoxelRange<v_flt>(0.2f) * Variable_30;
				
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
		
		FVoxelFastNoise _2D_Perlin_Noise_1_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_4_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_5_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_6_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_7_Noise;
		
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
			// X
			TVoxelRange<v_flt> Variable_9; // X output 0
			Variable_9 = Context.GetLocalX();
			
			// Z
			TVoxelRange<v_flt> Variable_11; // Z output 0
			Variable_11 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_10; // Y output 0
			Variable_10 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// -
			TVoxelRange<v_flt> Variable_5; // - output 0
			Variable_5 = Variable_0 - TVoxelRange<v_flt>(75.0f);
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_31; // Cave Layer.- output 0
			Variable_31 = Variable_0 - BufferConstant.Variable_35;
			
			// +
			TVoxelRange<v_flt> Variable_1; // + output 0
			Variable_1 = TVoxelRange<v_flt>(100.0f) + Variable_0;
			
			// +
			TVoxelRange<v_flt> Variable_4; // + output 0
			Variable_4 = TVoxelRange<v_flt>(200.0f) + Variable_0;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_32; // Cave Layer.- output 0
			Variable_32 = BufferConstant.Variable_33 - Variable_0;
			
			// Data Item Sample
			TVoxelRange<v_flt> Variable_14; // Data Item Sample output 0
			Variable_14 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_9, Variable_10, Variable_11, TVoxelRange<v_flt>(5.0f), TVoxelRange<v_flt>(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// +
			TVoxelRange<v_flt> Variable_18; // + output 0
			Variable_18 = TVoxelRange<v_flt>(300.0f) + Variable_0;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_24; // Cave Layer.- output 0
			Variable_24 = Variable_1 - BufferConstant.Variable_28;
			
			// -
			TVoxelRange<v_flt> Variable_7; // - output 0
			Variable_7 = Variable_5 - BufferConstant.Variable_6;
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_29; // Cave Layer.Smooth Union output 0
			Variable_29 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_31, Variable_32, TVoxelRange<v_flt>(100.0f));
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_25; // Cave Layer.- output 0
			Variable_25 = BufferConstant.Variable_26 - Variable_1;
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_44; // 2D Noise SDF.- output 0
			Variable_44 = Variable_18 - BufferConstant.Variable_3;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_39; // Cave Layer.- output 0
			Variable_39 = Variable_4 - BufferConstant.Variable_43;
			
			// * -1
			TVoxelRange<v_flt> Variable_13; // * -1 output 0
			Variable_13 = Variable_14 * -1;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_40; // Cave Layer.- output 0
			Variable_40 = BufferConstant.Variable_41 - Variable_4;
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_22; // Cave Layer.Smooth Union output 0
			Variable_22 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_24, Variable_25, TVoxelRange<v_flt>(100.0f));
			
			// * -1
			TVoxelRange<v_flt> Variable_20; // * -1 output 0
			Variable_20 = Variable_44 * -1;
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_37; // Cave Layer.Smooth Union output 0
			Variable_37 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_39, Variable_40, TVoxelRange<v_flt>(100.0f));
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_34; // Cave Layer.+ output 0
			Variable_34 = Variable_29 + TVoxelRange<v_flt>(25.0f);
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_27; // Cave Layer.+ output 0
			Variable_27 = Variable_22 + TVoxelRange<v_flt>(25.0f);
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_42; // Cave Layer.+ output 0
			Variable_42 = Variable_37 + TVoxelRange<v_flt>(25.0f);
			
			// Max (float)
			TVoxelRange<v_flt> Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, FVoxelNodeFunctions::Max<v_flt>(Variable_34, FVoxelNodeFunctions::Max<v_flt>(Variable_27, FVoxelNodeFunctions::Max<v_flt>(Variable_42, Variable_20))));
			
			// Smooth Intersection
			TVoxelRange<v_flt> Variable_12; // Smooth Intersection output 0
			Variable_12 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_13, TVoxelRange<v_flt>(5.0f));
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_36; // Set High Quality Value.* output 0
			Variable_36 = Variable_12 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_36;
		}
		
	};
	
	FVoxelExample_LayeredWorldInstance(UVoxelExample_LayeredWorld& Object)
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
			FVoxelRichCurve(Object.None1.LoadSynchronous()),
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
inline v_flt FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_LayeredWorldInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_LayeredWorldInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_LayeredWorldInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_LayeredWorldInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_LayeredWorldInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_LayeredWorld::UVoxelExample_LayeredWorld()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_LayeredWorld::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_LayeredWorldInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_LayeredWorld. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_LayeredWorld. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_LayeredWorld. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_LayeredWorld. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
