// Copyright 2020 Phyronnaz

#include "VoxelExample_LayeredPlanet.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_LayeredPlanetInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_LayeredPlanetInstance, UVoxelExample_LayeredPlanet>
{
public:
	struct FParams
	{
		const float Frequency;
		const FVoxelRichCurve None1;
		const int32 Seed;
		const bool Slice_Mode;
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
			
			bool Variable_51; // Slice Mode = False output 0
			v_flt Variable_43; // Frequency = 0.005 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_22; // X output 0
			v_flt Variable_35; // X output 0
			v_flt Variable_8; // X output 0
			v_flt Variable_26; // X output 0
			v_flt Variable_30; // X output 0
			v_flt Variable_18; // X output 0
			v_flt Variable_14; // X output 0
			v_flt Variable_39; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_9; // Y output 0
			v_flt Variable_31; // Y output 0
			v_flt Variable_19; // Y output 0
			v_flt Variable_27; // Y output 0
			v_flt Variable_40; // Y output 0
			v_flt Variable_23; // Y output 0
			v_flt Variable_15; // Y output 0
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
					FVoxelGraphSeed Variable_49; // Seed = 4761 output 0
					Variable_49 = Params.Seed;
					
					// Init of Make Seeds
					FVoxelGraphSeed Variable_44; // Make Seeds output 0
					FVoxelGraphSeed Variable_45; // Make Seeds output 1
					FVoxelGraphSeed Variable_46; // Make Seeds output 2
					FVoxelGraphSeed Variable_47; // Make Seeds output 3
					FVoxelGraphSeed Variable_48; // Make Seeds output 4
					Variable_44 = FVoxelUtilities::MurmurHash32(Variable_49);
					Variable_45 = FVoxelUtilities::MurmurHash32(Variable_44);
					Variable_46 = FVoxelUtilities::MurmurHash32(Variable_45);
					Variable_47 = FVoxelUtilities::MurmurHash32(Variable_46);
					Variable_48 = FVoxelUtilities::MurmurHash32(Variable_47);
					
					
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
				// Slice Mode = False
				BufferConstant.Variable_51 = Params.Slice_Mode;
				
				// Frequency = 0.005
				BufferConstant.Variable_43 = Params.Frequency;
				
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
		
		FVoxelFastNoise _3D_Perlin_Noise_0_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_1_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_2_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_3_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_4_Noise;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 4761
			FVoxelGraphSeed Variable_49; // Seed = 4761 output 0
			Variable_49 = Params.Seed;
			
			// Init of Make Seeds
			FVoxelGraphSeed Variable_44; // Make Seeds output 0
			FVoxelGraphSeed Variable_45; // Make Seeds output 1
			FVoxelGraphSeed Variable_46; // Make Seeds output 2
			FVoxelGraphSeed Variable_47; // Make Seeds output 3
			FVoxelGraphSeed Variable_48; // Make Seeds output 4
			Variable_44 = FVoxelUtilities::MurmurHash32(Variable_49);
			Variable_45 = FVoxelUtilities::MurmurHash32(Variable_44);
			Variable_46 = FVoxelUtilities::MurmurHash32(Variable_45);
			Variable_47 = FVoxelUtilities::MurmurHash32(Variable_46);
			Variable_48 = FVoxelUtilities::MurmurHash32(Variable_47);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_0_Noise.SetSeed(Variable_45);
			_3D_Perlin_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_1_Noise.SetSeed(Variable_48);
			_3D_Perlin_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_2_Noise.SetSeed(Variable_47);
			_3D_Perlin_Noise_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_3_Noise.SetSeed(Variable_46);
			_3D_Perlin_Noise_3_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
			// Init of 3D Perlin Noise
			_3D_Perlin_Noise_4_Noise.SetSeed(Variable_44);
			_3D_Perlin_Noise_4_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_22 = Context.GetLocalX();
			
			// X
			BufferX.Variable_35 = Context.GetLocalX();
			
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// X
			BufferX.Variable_26 = Context.GetLocalX();
			
			// X
			BufferX.Variable_30 = Context.GetLocalX();
			
			// X
			BufferX.Variable_18 = Context.GetLocalX();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
			// X
			BufferX.Variable_39 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_31 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_19 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_27 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_40 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_23 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_15 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_22 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
			// X
			BufferX.Variable_35 = Context.GetLocalX();
			
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_31 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_19 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_27 = Context.GetLocalY();
			
			// X
			BufferX.Variable_26 = Context.GetLocalX();
			
			// X
			BufferX.Variable_30 = Context.GetLocalX();
			
			// X
			BufferX.Variable_18 = Context.GetLocalX();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_40 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_23 = Context.GetLocalY();
			
			// X
			BufferX.Variable_39 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_15 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_20; // Z output 0
			Variable_20 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_32; // Z output 0
			Variable_32 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_28; // Z output 0
			Variable_28 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_41; // Z output 0
			Variable_41 = Context.GetLocalZ();
			
			// Data Item Sample
			v_flt Variable_13; // Data Item Sample output 0
			Variable_13 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, BufferX.Variable_8, BufferXY.Variable_9, Variable_10, v_flt(5.0f), v_flt(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// Normalize.Vector Length
			v_flt Variable_74; // Normalize.Vector Length output 0
			Variable_74 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_18, BufferXY.Variable_19, Variable_20);
			
			// Vector Length
			v_flt Variable_16; // Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_14, BufferXY.Variable_15, Variable_0);
			
			// Normalize./
			v_flt Variable_75; // Normalize./ output 0
			Variable_75 = BufferX.Variable_18 / Variable_74;
			
			// 3D Perlin Noise
			v_flt Variable_21; // 3D Perlin Noise output 0
			Variable_21 = _3D_Perlin_Noise_0_Noise.GetPerlin_3D(BufferX.Variable_22, BufferXY.Variable_23, Variable_24, BufferConstant.Variable_43);
			Variable_21 = FMath::Clamp<v_flt>(Variable_21, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_38; // 3D Perlin Noise output 0
			Variable_38 = _3D_Perlin_Noise_1_Noise.GetPerlin_3D(BufferX.Variable_39, BufferXY.Variable_40, Variable_41, BufferConstant.Variable_43);
			Variable_38 = FMath::Clamp<v_flt>(Variable_38, -0.790554, 0.781474);
			
			// -
			v_flt Variable_33; // - output 0
			Variable_33 = Variable_16 - v_flt(500.0f);
			
			// Normalize./
			v_flt Variable_76; // Normalize./ output 0
			Variable_76 = BufferXY.Variable_19 / Variable_74;
			
			// Normalize./
			v_flt Variable_77; // Normalize./ output 0
			Variable_77 = Variable_20 / Variable_74;
			
			// * -1
			v_flt Variable_12; // * -1 output 0
			Variable_12 = Variable_13 * -1;
			
			// 3D Perlin Noise
			v_flt Variable_29; // 3D Perlin Noise output 0
			Variable_29 = _3D_Perlin_Noise_2_Noise.GetPerlin_3D(BufferX.Variable_30, BufferXY.Variable_31, Variable_32, BufferConstant.Variable_43);
			Variable_29 = FMath::Clamp<v_flt>(Variable_29, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_25; // 3D Perlin Noise output 0
			Variable_25 = _3D_Perlin_Noise_3_Noise.GetPerlin_3D(BufferX.Variable_26, BufferXY.Variable_27, Variable_28, BufferConstant.Variable_43);
			Variable_25 = FMath::Clamp<v_flt>(Variable_25, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_17; // 3D Perlin Noise output 0
			Variable_17 = _3D_Perlin_Noise_4_Noise.GetPerlin_3D(Variable_75, Variable_76, Variable_77, v_flt(5.0f));
			Variable_17 = FMath::Clamp<v_flt>(Variable_17, -0.911908, 0.945800);
			
			// Cave Layer.*
			v_flt Variable_56; // Cave Layer.* output 0
			Variable_56 = Variable_25 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_63; // Cave Layer.* output 0
			Variable_63 = Variable_21 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_71; // Cave Layer.* output 0
			Variable_71 = Variable_29 * v_flt(50.0f);
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_33 - v_flt(75.0f);
			
			// +
			v_flt Variable_37; // + output 0
			Variable_37 = v_flt(300.0f) + Variable_33;
			
			// 2D Noise SDF.*
			v_flt Variable_79; // 2D Noise SDF.* output 0
			Variable_79 = Variable_38 * v_flt(50.0f);
			
			// +
			v_flt Variable_1; // + output 0
			Variable_1 = v_flt(100.0f) + Variable_33;
			
			// +
			v_flt Variable_3; // + output 0
			Variable_3 = v_flt(200.0f) + Variable_33;
			
			// Cave Layer.1 - X
			v_flt Variable_60; // Cave Layer.1 - X output 0
			Variable_60 = 1 - Variable_63;
			
			// Cave Layer.-
			v_flt Variable_55; // Cave Layer.- output 0
			Variable_55 = Variable_56 - Variable_1;
			
			// Float Curve: 
			v_flt Variable_7; // Float Curve:  output 0
			Variable_7 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_17);
			
			// Cave Layer.1 - X
			v_flt Variable_53; // Cave Layer.1 - X output 0
			Variable_53 = 1 - Variable_56;
			
			// Cave Layer.-
			v_flt Variable_62; // Cave Layer.- output 0
			Variable_62 = Variable_63 - Variable_33;
			
			// 2D Noise SDF.+
			v_flt Variable_34; // 2D Noise SDF.+ output 0
			Variable_34 = Variable_79 + v_flt(0.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_68; // Cave Layer.1 - X output 0
			Variable_68 = 1 - Variable_71;
			
			// Cave Layer.-
			v_flt Variable_70; // Cave Layer.- output 0
			Variable_70 = Variable_71 - Variable_3;
			
			// Cave Layer.*
			v_flt Variable_73; // Cave Layer.* output 0
			Variable_73 = v_flt(0.2f) * Variable_68;
			
			// Cave Layer.*
			v_flt Variable_58; // Cave Layer.* output 0
			Variable_58 = v_flt(0.2f) * Variable_53;
			
			// 2D Noise SDF.-
			v_flt Variable_78; // 2D Noise SDF.- output 0
			Variable_78 = Variable_37 - Variable_34;
			
			// Cave Layer.*
			v_flt Variable_65; // Cave Layer.* output 0
			Variable_65 = v_flt(0.2f) * Variable_60;
			
			// *
			v_flt Variable_5; // * output 0
			Variable_5 = Variable_7 * v_flt(100.0f);
			
			// Cave Layer.-
			v_flt Variable_69; // Cave Layer.- output 0
			Variable_69 = Variable_3 - Variable_73;
			
			// Cave Layer.-
			v_flt Variable_54; // Cave Layer.- output 0
			Variable_54 = Variable_1 - Variable_58;
			
			// * -1
			v_flt Variable_42; // * -1 output 0
			Variable_42 = Variable_78 * -1;
			
			// Cave Layer.-
			v_flt Variable_61; // Cave Layer.- output 0
			Variable_61 = Variable_33 - Variable_65;
			
			// -
			v_flt Variable_6; // - output 0
			Variable_6 = Variable_4 - Variable_5;
			
			// Cave Layer.Smooth Union
			v_flt Variable_67; // Cave Layer.Smooth Union output 0
			Variable_67 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_69, Variable_70, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_59; // Cave Layer.Smooth Union output 0
			Variable_59 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_61, Variable_62, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_52; // Cave Layer.Smooth Union output 0
			Variable_52 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_54, Variable_55, v_flt(100.0f));
			
			// Cave Layer.+
			v_flt Variable_64; // Cave Layer.+ output 0
			Variable_64 = Variable_59 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_57; // Cave Layer.+ output 0
			Variable_57 = Variable_52 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_72; // Cave Layer.+ output 0
			Variable_72 = Variable_67 + v_flt(25.0f);
			
			// Max (float)
			v_flt Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_6, FVoxelNodeFunctions::Max<v_flt>(Variable_64, FVoxelNodeFunctions::Max<v_flt>(Variable_57, FVoxelNodeFunctions::Max<v_flt>(Variable_72, Variable_42))));
			
			// Smooth Intersection
			v_flt Variable_11; // Smooth Intersection output 0
			Variable_11 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_12, v_flt(5.0f));
			
			// Max (float)
			v_flt Variable_36; // Max (float) output 0
			Variable_36 = FVoxelNodeFunctions::Max<v_flt>(BufferX.Variable_35, Variable_11);
			
			// Switch (float)
			v_flt Variable_50; // Switch (float) output 0
			Variable_50 = FVoxelNodeFunctions::Switch(Variable_36, Variable_11, BufferConstant.Variable_51);
			
			// Set High Quality Value.*
			v_flt Variable_66; // Set High Quality Value.* output 0
			Variable_66 = Variable_50 * v_flt(0.2f);
			
			Outputs.Value = Variable_66;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_22; // X output 0
			Variable_22 = Context.GetLocalX();
			
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
			// X
			v_flt Variable_35; // X output 0
			Variable_35 = Context.GetLocalX();
			
			// X
			v_flt Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// Y
			v_flt Variable_31; // Y output 0
			Variable_31 = Context.GetLocalY();
			
			// Y
			v_flt Variable_19; // Y output 0
			Variable_19 = Context.GetLocalY();
			
			// Y
			v_flt Variable_27; // Y output 0
			Variable_27 = Context.GetLocalY();
			
			// Z
			v_flt Variable_20; // Z output 0
			Variable_20 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_32; // Z output 0
			Variable_32 = Context.GetLocalZ();
			
			// X
			v_flt Variable_26; // X output 0
			Variable_26 = Context.GetLocalX();
			
			// X
			v_flt Variable_30; // X output 0
			Variable_30 = Context.GetLocalX();
			
			// X
			v_flt Variable_18; // X output 0
			Variable_18 = Context.GetLocalX();
			
			// Z
			v_flt Variable_24; // Z output 0
			Variable_24 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_28; // Z output 0
			Variable_28 = Context.GetLocalZ();
			
			// X
			v_flt Variable_14; // X output 0
			Variable_14 = Context.GetLocalX();
			
			// Y
			v_flt Variable_40; // Y output 0
			Variable_40 = Context.GetLocalY();
			
			// Y
			v_flt Variable_23; // Y output 0
			Variable_23 = Context.GetLocalY();
			
			// X
			v_flt Variable_39; // X output 0
			Variable_39 = Context.GetLocalX();
			
			// Z
			v_flt Variable_41; // Z output 0
			Variable_41 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// Data Item Sample
			v_flt Variable_13; // Data Item Sample output 0
			Variable_13 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_8, Variable_9, Variable_10, v_flt(5.0f), v_flt(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// Normalize.Vector Length
			v_flt Variable_74; // Normalize.Vector Length output 0
			Variable_74 = FVoxelNodeFunctions::VectorLength(Variable_18, Variable_19, Variable_20);
			
			// Vector Length
			v_flt Variable_16; // Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(Variable_14, Variable_15, Variable_0);
			
			// Normalize./
			v_flt Variable_75; // Normalize./ output 0
			Variable_75 = Variable_18 / Variable_74;
			
			// 3D Perlin Noise
			v_flt Variable_21; // 3D Perlin Noise output 0
			Variable_21 = _3D_Perlin_Noise_0_Noise.GetPerlin_3D(Variable_22, Variable_23, Variable_24, BufferConstant.Variable_43);
			Variable_21 = FMath::Clamp<v_flt>(Variable_21, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_38; // 3D Perlin Noise output 0
			Variable_38 = _3D_Perlin_Noise_1_Noise.GetPerlin_3D(Variable_39, Variable_40, Variable_41, BufferConstant.Variable_43);
			Variable_38 = FMath::Clamp<v_flt>(Variable_38, -0.790554, 0.781474);
			
			// -
			v_flt Variable_33; // - output 0
			Variable_33 = Variable_16 - v_flt(500.0f);
			
			// Normalize./
			v_flt Variable_76; // Normalize./ output 0
			Variable_76 = Variable_19 / Variable_74;
			
			// Normalize./
			v_flt Variable_77; // Normalize./ output 0
			Variable_77 = Variable_20 / Variable_74;
			
			// * -1
			v_flt Variable_12; // * -1 output 0
			Variable_12 = Variable_13 * -1;
			
			// 3D Perlin Noise
			v_flt Variable_29; // 3D Perlin Noise output 0
			Variable_29 = _3D_Perlin_Noise_2_Noise.GetPerlin_3D(Variable_30, Variable_31, Variable_32, BufferConstant.Variable_43);
			Variable_29 = FMath::Clamp<v_flt>(Variable_29, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_25; // 3D Perlin Noise output 0
			Variable_25 = _3D_Perlin_Noise_3_Noise.GetPerlin_3D(Variable_26, Variable_27, Variable_28, BufferConstant.Variable_43);
			Variable_25 = FMath::Clamp<v_flt>(Variable_25, -0.790554, 0.781474);
			
			// 3D Perlin Noise
			v_flt Variable_17; // 3D Perlin Noise output 0
			Variable_17 = _3D_Perlin_Noise_4_Noise.GetPerlin_3D(Variable_75, Variable_76, Variable_77, v_flt(5.0f));
			Variable_17 = FMath::Clamp<v_flt>(Variable_17, -0.911908, 0.945800);
			
			// Cave Layer.*
			v_flt Variable_56; // Cave Layer.* output 0
			Variable_56 = Variable_25 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_63; // Cave Layer.* output 0
			Variable_63 = Variable_21 * v_flt(50.0f);
			
			// Cave Layer.*
			v_flt Variable_71; // Cave Layer.* output 0
			Variable_71 = Variable_29 * v_flt(50.0f);
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_33 - v_flt(75.0f);
			
			// +
			v_flt Variable_37; // + output 0
			Variable_37 = v_flt(300.0f) + Variable_33;
			
			// 2D Noise SDF.*
			v_flt Variable_79; // 2D Noise SDF.* output 0
			Variable_79 = Variable_38 * v_flt(50.0f);
			
			// +
			v_flt Variable_1; // + output 0
			Variable_1 = v_flt(100.0f) + Variable_33;
			
			// +
			v_flt Variable_3; // + output 0
			Variable_3 = v_flt(200.0f) + Variable_33;
			
			// Cave Layer.1 - X
			v_flt Variable_60; // Cave Layer.1 - X output 0
			Variable_60 = 1 - Variable_63;
			
			// Cave Layer.-
			v_flt Variable_55; // Cave Layer.- output 0
			Variable_55 = Variable_56 - Variable_1;
			
			// Float Curve: 
			v_flt Variable_7; // Float Curve:  output 0
			Variable_7 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_17);
			
			// Cave Layer.1 - X
			v_flt Variable_53; // Cave Layer.1 - X output 0
			Variable_53 = 1 - Variable_56;
			
			// Cave Layer.-
			v_flt Variable_62; // Cave Layer.- output 0
			Variable_62 = Variable_63 - Variable_33;
			
			// 2D Noise SDF.+
			v_flt Variable_34; // 2D Noise SDF.+ output 0
			Variable_34 = Variable_79 + v_flt(0.0f);
			
			// Cave Layer.1 - X
			v_flt Variable_68; // Cave Layer.1 - X output 0
			Variable_68 = 1 - Variable_71;
			
			// Cave Layer.-
			v_flt Variable_70; // Cave Layer.- output 0
			Variable_70 = Variable_71 - Variable_3;
			
			// Cave Layer.*
			v_flt Variable_73; // Cave Layer.* output 0
			Variable_73 = v_flt(0.2f) * Variable_68;
			
			// Cave Layer.*
			v_flt Variable_58; // Cave Layer.* output 0
			Variable_58 = v_flt(0.2f) * Variable_53;
			
			// 2D Noise SDF.-
			v_flt Variable_78; // 2D Noise SDF.- output 0
			Variable_78 = Variable_37 - Variable_34;
			
			// Cave Layer.*
			v_flt Variable_65; // Cave Layer.* output 0
			Variable_65 = v_flt(0.2f) * Variable_60;
			
			// *
			v_flt Variable_5; // * output 0
			Variable_5 = Variable_7 * v_flt(100.0f);
			
			// Cave Layer.-
			v_flt Variable_69; // Cave Layer.- output 0
			Variable_69 = Variable_3 - Variable_73;
			
			// Cave Layer.-
			v_flt Variable_54; // Cave Layer.- output 0
			Variable_54 = Variable_1 - Variable_58;
			
			// * -1
			v_flt Variable_42; // * -1 output 0
			Variable_42 = Variable_78 * -1;
			
			// Cave Layer.-
			v_flt Variable_61; // Cave Layer.- output 0
			Variable_61 = Variable_33 - Variable_65;
			
			// -
			v_flt Variable_6; // - output 0
			Variable_6 = Variable_4 - Variable_5;
			
			// Cave Layer.Smooth Union
			v_flt Variable_67; // Cave Layer.Smooth Union output 0
			Variable_67 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_69, Variable_70, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_59; // Cave Layer.Smooth Union output 0
			Variable_59 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_61, Variable_62, v_flt(100.0f));
			
			// Cave Layer.Smooth Union
			v_flt Variable_52; // Cave Layer.Smooth Union output 0
			Variable_52 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_54, Variable_55, v_flt(100.0f));
			
			// Cave Layer.+
			v_flt Variable_64; // Cave Layer.+ output 0
			Variable_64 = Variable_59 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_57; // Cave Layer.+ output 0
			Variable_57 = Variable_52 + v_flt(25.0f);
			
			// Cave Layer.+
			v_flt Variable_72; // Cave Layer.+ output 0
			Variable_72 = Variable_67 + v_flt(25.0f);
			
			// Max (float)
			v_flt Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_6, FVoxelNodeFunctions::Max<v_flt>(Variable_64, FVoxelNodeFunctions::Max<v_flt>(Variable_57, FVoxelNodeFunctions::Max<v_flt>(Variable_72, Variable_42))));
			
			// Smooth Intersection
			v_flt Variable_11; // Smooth Intersection output 0
			Variable_11 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_12, v_flt(5.0f));
			
			// Max (float)
			v_flt Variable_36; // Max (float) output 0
			Variable_36 = FVoxelNodeFunctions::Max<v_flt>(Variable_35, Variable_11);
			
			// Switch (float)
			v_flt Variable_50; // Switch (float) output 0
			Variable_50 = FVoxelNodeFunctions::Switch(Variable_36, Variable_11, BufferConstant.Variable_51);
			
			// Set High Quality Value.*
			v_flt Variable_66; // Set High Quality Value.* output 0
			Variable_66 = Variable_50 * v_flt(0.2f);
			
			Outputs.Value = Variable_66;
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
			
			v_flt Variable_1; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_2; // Y output 0
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
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_2 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_2 = Context.GetLocalY();
			
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_1, BufferXY.Variable_2, Variable_0);
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_3 - v_flt(500.0f);
			
			// <=
			bool Variable_7; // <= output 0
			Variable_7 = Variable_4 <= v_flt(50.0f);
			
			// <=
			bool Variable_8; // <= output 0
			Variable_8 = Variable_4 <= v_flt(-40.0f);
			
			// <=
			bool Variable_10; // <= output 0
			Variable_10 = Variable_4 <= v_flt(-250.0f);
			
			// Switch (color)
			FColor Variable_5; // Switch (color) output 0
			Variable_5 = FVoxelNodeFunctions::Switch(FColor(FColor(14, 71, 255, 255)), FColor(FColor(229, 0, 255, 255)), Variable_7);
			
			// Switch (color)
			FColor Variable_6; // Switch (color) output 0
			Variable_6 = FVoxelNodeFunctions::Switch(FColor(FColor(255, 142, 44, 255)), Variable_5, Variable_8);
			
			// Switch (color)
			FColor Variable_9; // Switch (color) output 0
			Variable_9 = FVoxelNodeFunctions::Switch(FColor(FColor(3, 143, 0, 255)), Variable_6, Variable_10);
			
			Outputs.MaterialBuilder.SetColor(Variable_9);
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// X
			v_flt Variable_1; // X output 0
			Variable_1 = Context.GetLocalX();
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_1, Variable_2, Variable_0);
			
			// -
			v_flt Variable_4; // - output 0
			Variable_4 = Variable_3 - v_flt(500.0f);
			
			// <=
			bool Variable_7; // <= output 0
			Variable_7 = Variable_4 <= v_flt(50.0f);
			
			// <=
			bool Variable_8; // <= output 0
			Variable_8 = Variable_4 <= v_flt(-40.0f);
			
			// <=
			bool Variable_10; // <= output 0
			Variable_10 = Variable_4 <= v_flt(-250.0f);
			
			// Switch (color)
			FColor Variable_5; // Switch (color) output 0
			Variable_5 = FVoxelNodeFunctions::Switch(FColor(FColor(14, 71, 255, 255)), FColor(FColor(229, 0, 255, 255)), Variable_7);
			
			// Switch (color)
			FColor Variable_6; // Switch (color) output 0
			Variable_6 = FVoxelNodeFunctions::Switch(FColor(FColor(255, 142, 44, 255)), Variable_5, Variable_8);
			
			// Switch (color)
			FColor Variable_9; // Switch (color) output 0
			Variable_9 = FVoxelNodeFunctions::Switch(FColor(FColor(3, 143, 0, 255)), Variable_6, Variable_10);
			
			Outputs.MaterialBuilder.SetColor(Variable_9);
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
			
			FVoxelBoolRange Variable_29; // Slice Mode = False output 0
			TVoxelRange<v_flt> Variable_49; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_34; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_41; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_5; // * output 0
			TVoxelRange<v_flt> Variable_22; // 2D Noise SDF.+ output 0
			TVoxelRange<v_flt> Variable_51; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_36; // Cave Layer.* output 0
			TVoxelRange<v_flt> Variable_43; // Cave Layer.* output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_23; // X output 0
			TVoxelRange<v_flt> Variable_8; // X output 0
			TVoxelRange<v_flt> Variable_14; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_9; // Y output 0
			TVoxelRange<v_flt> Variable_15; // Y output 0
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
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_5_Noise.SetSeed(FVoxelGraphSeed(13339));
					_3D_Perlin_Noise_5_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_6_Noise.SetSeed(FVoxelGraphSeed(1340));
					_3D_Perlin_Noise_6_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_7_Noise.SetSeed(FVoxelGraphSeed(1330));
					_3D_Perlin_Noise_7_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_8_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_Perlin_Noise_8_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
					// Init of 3D Perlin Noise
					_3D_Perlin_Noise_9_Noise.SetSeed(FVoxelGraphSeed(1339));
					_3D_Perlin_Noise_9_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					
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
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_26; // 3D Perlin Noise output 0
				Variable_26 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_17; // 3D Perlin Noise output 0
				Variable_17 = { -0.911908f, 0.945800f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_18; // 3D Perlin Noise output 0
				Variable_18 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_19; // 3D Perlin Noise output 0
				Variable_19 = { -0.790554f, 0.781474f };
				
				// 3D Perlin Noise
				TVoxelRange<v_flt> Variable_20; // 3D Perlin Noise output 0
				Variable_20 = { -0.790554f, 0.781474f };
				
				// Slice Mode = False
				BufferConstant.Variable_29 = Params.Slice_Mode;
				
				// Cave Layer.*
				BufferConstant.Variable_49 = Variable_20 * TVoxelRange<v_flt>(50.0f);
				
				// Float Curve: 
				TVoxelRange<v_flt> Variable_7; // Float Curve:  output 0
				Variable_7 = FVoxelNodeFunctions::GetCurveValue(Params.None1, Variable_17);
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_53; // 2D Noise SDF.* output 0
				Variable_53 = Variable_26 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.*
				BufferConstant.Variable_34 = Variable_19 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.*
				BufferConstant.Variable_41 = Variable_18 * TVoxelRange<v_flt>(50.0f);
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_38; // Cave Layer.1 - X output 0
				Variable_38 = 1 - BufferConstant.Variable_41;
				
				// *
				BufferConstant.Variable_5 = Variable_7 * TVoxelRange<v_flt>(100.0f);
				
				// 2D Noise SDF.+
				BufferConstant.Variable_22 = Variable_53 + TVoxelRange<v_flt>(0.0f);
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_31; // Cave Layer.1 - X output 0
				Variable_31 = 1 - BufferConstant.Variable_34;
				
				// Cave Layer.1 - X
				TVoxelRange<v_flt> Variable_46; // Cave Layer.1 - X output 0
				Variable_46 = 1 - BufferConstant.Variable_49;
				
				// Cave Layer.*
				BufferConstant.Variable_51 = TVoxelRange<v_flt>(0.2f) * Variable_46;
				
				// Cave Layer.*
				BufferConstant.Variable_36 = TVoxelRange<v_flt>(0.2f) * Variable_31;
				
				// Cave Layer.*
				BufferConstant.Variable_43 = TVoxelRange<v_flt>(0.2f) * Variable_38;
				
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
		
		FVoxelFastNoise _3D_Perlin_Noise_5_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_6_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_7_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_8_Noise;
		FVoxelFastNoise _3D_Perlin_Noise_9_Noise;
		
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
			TVoxelRange<v_flt> Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
			// X
			TVoxelRange<v_flt> Variable_23; // X output 0
			Variable_23 = Context.GetLocalX();
			
			// X
			TVoxelRange<v_flt> Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_14; // X output 0
			Variable_14 = Context.GetLocalX();
			
			// Data Item Sample
			TVoxelRange<v_flt> Variable_13; // Data Item Sample output 0
			Variable_13 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_8, Variable_9, Variable_10, TVoxelRange<v_flt>(5.0f), TVoxelRange<v_flt>(1000.0f), 1u, EVoxelDataItemCombineMode::Min);
			
			// Vector Length
			TVoxelRange<v_flt> Variable_16; // Vector Length output 0
			Variable_16 = FVoxelNodeFunctions::VectorLength(Variable_14, Variable_15, Variable_0);
			
			// -
			TVoxelRange<v_flt> Variable_21; // - output 0
			Variable_21 = Variable_16 - TVoxelRange<v_flt>(500.0f);
			
			// * -1
			TVoxelRange<v_flt> Variable_12; // * -1 output 0
			Variable_12 = Variable_13 * -1;
			
			// -
			TVoxelRange<v_flt> Variable_4; // - output 0
			Variable_4 = Variable_21 - TVoxelRange<v_flt>(75.0f);
			
			// +
			TVoxelRange<v_flt> Variable_3; // + output 0
			Variable_3 = TVoxelRange<v_flt>(200.0f) + Variable_21;
			
			// +
			TVoxelRange<v_flt> Variable_1; // + output 0
			Variable_1 = TVoxelRange<v_flt>(100.0f) + Variable_21;
			
			// +
			TVoxelRange<v_flt> Variable_25; // + output 0
			Variable_25 = TVoxelRange<v_flt>(300.0f) + Variable_21;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_39; // Cave Layer.- output 0
			Variable_39 = Variable_21 - BufferConstant.Variable_43;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_40; // Cave Layer.- output 0
			Variable_40 = BufferConstant.Variable_41 - Variable_21;
			
			// -
			TVoxelRange<v_flt> Variable_6; // - output 0
			Variable_6 = Variable_4 - BufferConstant.Variable_5;
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_37; // Cave Layer.Smooth Union output 0
			Variable_37 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_39, Variable_40, TVoxelRange<v_flt>(100.0f));
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_32; // Cave Layer.- output 0
			Variable_32 = Variable_1 - BufferConstant.Variable_36;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_47; // Cave Layer.- output 0
			Variable_47 = Variable_3 - BufferConstant.Variable_51;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_33; // Cave Layer.- output 0
			Variable_33 = BufferConstant.Variable_34 - Variable_1;
			
			// Cave Layer.-
			TVoxelRange<v_flt> Variable_48; // Cave Layer.- output 0
			Variable_48 = BufferConstant.Variable_49 - Variable_3;
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_52; // 2D Noise SDF.- output 0
			Variable_52 = Variable_25 - BufferConstant.Variable_22;
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_42; // Cave Layer.+ output 0
			Variable_42 = Variable_37 + TVoxelRange<v_flt>(25.0f);
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_30; // Cave Layer.Smooth Union output 0
			Variable_30 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_32, Variable_33, TVoxelRange<v_flt>(100.0f));
			
			// * -1
			TVoxelRange<v_flt> Variable_27; // * -1 output 0
			Variable_27 = Variable_52 * -1;
			
			// Cave Layer.Smooth Union
			TVoxelRange<v_flt> Variable_45; // Cave Layer.Smooth Union output 0
			Variable_45 = FVoxelSDFNodeFunctions::SmoothUnion(Variable_47, Variable_48, TVoxelRange<v_flt>(100.0f));
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_50; // Cave Layer.+ output 0
			Variable_50 = Variable_45 + TVoxelRange<v_flt>(25.0f);
			
			// Cave Layer.+
			TVoxelRange<v_flt> Variable_35; // Cave Layer.+ output 0
			Variable_35 = Variable_30 + TVoxelRange<v_flt>(25.0f);
			
			// Max (float)
			TVoxelRange<v_flt> Variable_2; // Max (float) output 0
			Variable_2 = FVoxelNodeFunctions::Max<v_flt>(Variable_6, FVoxelNodeFunctions::Max<v_flt>(Variable_42, FVoxelNodeFunctions::Max<v_flt>(Variable_35, FVoxelNodeFunctions::Max<v_flt>(Variable_50, Variable_27))));
			
			// Smooth Intersection
			TVoxelRange<v_flt> Variable_11; // Smooth Intersection output 0
			Variable_11 = FVoxelSDFNodeFunctions::SmoothIntersection(Variable_2, Variable_12, TVoxelRange<v_flt>(5.0f));
			
			// Max (float)
			TVoxelRange<v_flt> Variable_24; // Max (float) output 0
			Variable_24 = FVoxelNodeFunctions::Max<v_flt>(Variable_23, Variable_11);
			
			// Switch (float)
			TVoxelRange<v_flt> Variable_28; // Switch (float) output 0
			Variable_28 = FVoxelNodeFunctions::Switch(Variable_24, Variable_11, BufferConstant.Variable_29);
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_44; // Set High Quality Value.* output 0
			Variable_44 = Variable_28 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_44;
		}
		
	};
	
	FVoxelExample_LayeredPlanetInstance(UVoxelExample_LayeredPlanet& Object)
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
			Object.Seed,
			Object.Slice_Mode
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
inline v_flt FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_LayeredPlanetInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_LayeredPlanetInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_LayeredPlanetInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_LayeredPlanetInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_LayeredPlanetInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_LayeredPlanet::UVoxelExample_LayeredPlanet()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_LayeredPlanet::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_LayeredPlanetInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_LayeredPlanet. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_LayeredPlanet. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_LayeredPlanet. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_LayeredPlanet. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
