// Copyright 2020 Phyronnaz

#include "VG_Example_Erosion.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVG_Example_ErosionInstance : public TVoxelGraphGeneratorInstanceHelper<FVG_Example_ErosionInstance, UVG_Example_Erosion>
{
public:
	struct FParams
	{
		const float Erosion_Material_Offset;
		const float Erosion_Material_Strength;
		const float Erosion_Strength;
		const float Height;
		const FName Rocks;
		const FName Snow;
		const float Valleys_Height;
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
			
			v_flt Variable_10; // Height = 500.0 output 0
			v_flt Variable_13; // Valleys Height = -0.5 output 0
			v_flt Variable_9; // Erosion Strength = 0.008 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_2; // X output 0
			v_flt Variable_11; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_0; // 2D Noise SDF.+ output 0
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
				// Height = 500.0
				BufferConstant.Variable_10 = Params.Height;
				
				// Valleys Height = -0.5
				BufferConstant.Variable_13 = Params.Valleys_Height;
				
				// Erosion Strength = 0.008
				BufferConstant.Variable_9 = Params.Erosion_Strength;
				
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
		FVoxelFastNoise _2D_Erosion_0_Noise;
		TStaticArray<uint8, 32> _2D_Erosion_0_LODToOctaves;
		
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
			
			// Init of 2D Erosion
			_2D_Erosion_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Erosion_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Erosion_0_Noise.SetFractalOctavesAndGain(5, 0.5);
			_2D_Erosion_0_Noise.SetFractalLacunarity(2.0);
			_2D_Erosion_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Erosion_0_Noise.SetCellularJitter(0.5);
			_2D_Erosion_0_LODToOctaves[0] = 5;
			_2D_Erosion_0_LODToOctaves[1] = 5;
			_2D_Erosion_0_LODToOctaves[2] = 5;
			_2D_Erosion_0_LODToOctaves[3] = 5;
			_2D_Erosion_0_LODToOctaves[4] = 5;
			_2D_Erosion_0_LODToOctaves[5] = 5;
			_2D_Erosion_0_LODToOctaves[6] = 5;
			_2D_Erosion_0_LODToOctaves[7] = 5;
			_2D_Erosion_0_LODToOctaves[8] = 5;
			_2D_Erosion_0_LODToOctaves[9] = 5;
			_2D_Erosion_0_LODToOctaves[10] = 5;
			_2D_Erosion_0_LODToOctaves[11] = 5;
			_2D_Erosion_0_LODToOctaves[12] = 5;
			_2D_Erosion_0_LODToOctaves[13] = 5;
			_2D_Erosion_0_LODToOctaves[14] = 5;
			_2D_Erosion_0_LODToOctaves[15] = 5;
			_2D_Erosion_0_LODToOctaves[16] = 5;
			_2D_Erosion_0_LODToOctaves[17] = 5;
			_2D_Erosion_0_LODToOctaves[18] = 5;
			_2D_Erosion_0_LODToOctaves[19] = 5;
			_2D_Erosion_0_LODToOctaves[20] = 5;
			_2D_Erosion_0_LODToOctaves[21] = 5;
			_2D_Erosion_0_LODToOctaves[22] = 5;
			_2D_Erosion_0_LODToOctaves[23] = 5;
			_2D_Erosion_0_LODToOctaves[24] = 5;
			_2D_Erosion_0_LODToOctaves[25] = 5;
			_2D_Erosion_0_LODToOctaves[26] = 5;
			_2D_Erosion_0_LODToOctaves[27] = 5;
			_2D_Erosion_0_LODToOctaves[28] = 5;
			_2D_Erosion_0_LODToOctaves[29] = 5;
			_2D_Erosion_0_LODToOctaves[30] = 5;
			_2D_Erosion_0_LODToOctaves[31] = 5;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_2 = Context.GetLocalX();
			
			// X
			BufferX.Variable_11 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_14; // 2D Perlin Noise Fractal output 0
			v_flt Variable_15; // 2D Perlin Noise Fractal output 1
			v_flt Variable_16; // 2D Perlin Noise Fractal output 2
			Variable_14 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D_Deriv(BufferX.Variable_2, Variable_3, v_flt(0.001f), _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_15,Variable_16);
			Variable_14 = FMath::Clamp<v_flt>(Variable_14, -0.722935, 0.711631);
			Variable_15 = FMath::Clamp<v_flt>(Variable_15, -1.982108, 2.144371);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -2.105316, 1.997740);
			
			// Smooth Step
			v_flt Variable_7; // Smooth Step output 0
			Variable_7 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_13, v_flt(0.0f), Variable_14);
			
			// 2D Erosion
			v_flt Variable_1; // 2D Erosion output 0
			v_flt _2D_Erosion_0_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_0_Temp_2; // 2D Erosion output 2
			Variable_1 = _2D_Erosion_0_Noise.GetErosion_2D(BufferX.Variable_11, Variable_12, v_flt(0.02f), _2D_Erosion_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_15, Variable_16, _2D_Erosion_0_Temp_1, _2D_Erosion_0_Temp_2);
			Variable_1 = FMath::Clamp<v_flt>(Variable_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_2, -1.200000, 1.200000);
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = Variable_1 * Variable_7;
			
			// *
			v_flt Variable_5; // * output 0
			Variable_5 = BufferConstant.Variable_9 * Variable_8;
			
			// +
			v_flt Variable_4; // + output 0
			Variable_4 = Variable_14 + Variable_5;
			
			// 2D Noise SDF.*
			v_flt Variable_19; // 2D Noise SDF.* output 0
			Variable_19 = Variable_4 * BufferConstant.Variable_10;
			
			// 2D Noise SDF.+
			BufferXY.Variable_0 = Variable_19 + v_flt(0.0f);
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_2 = Context.GetLocalX();
			
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// X
			BufferX.Variable_11 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_14; // 2D Perlin Noise Fractal output 0
			v_flt Variable_15; // 2D Perlin Noise Fractal output 1
			v_flt Variable_16; // 2D Perlin Noise Fractal output 2
			Variable_14 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D_Deriv(BufferX.Variable_2, Variable_3, v_flt(0.001f), _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_15,Variable_16);
			Variable_14 = FMath::Clamp<v_flt>(Variable_14, -0.722935, 0.711631);
			Variable_15 = FMath::Clamp<v_flt>(Variable_15, -1.982108, 2.144371);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -2.105316, 1.997740);
			
			// Smooth Step
			v_flt Variable_7; // Smooth Step output 0
			Variable_7 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_13, v_flt(0.0f), Variable_14);
			
			// 2D Erosion
			v_flt Variable_1; // 2D Erosion output 0
			v_flt _2D_Erosion_0_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_0_Temp_2; // 2D Erosion output 2
			Variable_1 = _2D_Erosion_0_Noise.GetErosion_2D(BufferX.Variable_11, Variable_12, v_flt(0.02f), _2D_Erosion_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_15, Variable_16, _2D_Erosion_0_Temp_1, _2D_Erosion_0_Temp_2);
			Variable_1 = FMath::Clamp<v_flt>(Variable_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_2, -1.200000, 1.200000);
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = Variable_1 * Variable_7;
			
			// *
			v_flt Variable_5; // * output 0
			Variable_5 = BufferConstant.Variable_9 * Variable_8;
			
			// +
			v_flt Variable_4; // + output 0
			Variable_4 = Variable_14 + Variable_5;
			
			// 2D Noise SDF.*
			v_flt Variable_19; // 2D Noise SDF.* output 0
			Variable_19 = Variable_4 * BufferConstant.Variable_10;
			
			// 2D Noise SDF.+
			BufferXY.Variable_0 = Variable_19 + v_flt(0.0f);
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// 2D Noise SDF.-
			v_flt Variable_18; // 2D Noise SDF.- output 0
			Variable_18 = Variable_6 - BufferXY.Variable_0;
			
			// Set High Quality Value.*
			v_flt Variable_17; // Set High Quality Value.* output 0
			Variable_17 = Variable_18 * v_flt(0.2f);
			
			Outputs.Value = Variable_17;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_2; // X output 0
			Variable_2 = Context.GetLocalX();
			
			// Y
			v_flt Variable_3; // Y output 0
			Variable_3 = Context.GetLocalY();
			
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_12; // Y output 0
			Variable_12 = Context.GetLocalY();
			
			// X
			v_flt Variable_11; // X output 0
			Variable_11 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_14; // 2D Perlin Noise Fractal output 0
			v_flt Variable_15; // 2D Perlin Noise Fractal output 1
			v_flt Variable_16; // 2D Perlin Noise Fractal output 2
			Variable_14 = _2D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_2D_Deriv(Variable_2, Variable_3, v_flt(0.001f), _2D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_15,Variable_16);
			Variable_14 = FMath::Clamp<v_flt>(Variable_14, -0.722935, 0.711631);
			Variable_15 = FMath::Clamp<v_flt>(Variable_15, -1.982108, 2.144371);
			Variable_16 = FMath::Clamp<v_flt>(Variable_16, -2.105316, 1.997740);
			
			// Smooth Step
			v_flt Variable_7; // Smooth Step output 0
			Variable_7 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_13, v_flt(0.0f), Variable_14);
			
			// 2D Erosion
			v_flt Variable_1; // 2D Erosion output 0
			v_flt _2D_Erosion_0_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_0_Temp_2; // 2D Erosion output 2
			Variable_1 = _2D_Erosion_0_Noise.GetErosion_2D(Variable_11, Variable_12, v_flt(0.02f), _2D_Erosion_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_15, Variable_16, _2D_Erosion_0_Temp_1, _2D_Erosion_0_Temp_2);
			Variable_1 = FMath::Clamp<v_flt>(Variable_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_0_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_0_Temp_2, -1.200000, 1.200000);
			
			// *
			v_flt Variable_8; // * output 0
			Variable_8 = Variable_1 * Variable_7;
			
			// *
			v_flt Variable_5; // * output 0
			Variable_5 = BufferConstant.Variable_9 * Variable_8;
			
			// +
			v_flt Variable_4; // + output 0
			Variable_4 = Variable_14 + Variable_5;
			
			// 2D Noise SDF.*
			v_flt Variable_19; // 2D Noise SDF.* output 0
			Variable_19 = Variable_4 * BufferConstant.Variable_10;
			
			// 2D Noise SDF.+
			v_flt Variable_0; // 2D Noise SDF.+ output 0
			Variable_0 = Variable_19 + v_flt(0.0f);
			
			// 2D Noise SDF.-
			v_flt Variable_18; // 2D Noise SDF.- output 0
			Variable_18 = Variable_6 - Variable_0;
			
			// Set High Quality Value.*
			v_flt Variable_17; // Set High Quality Value.* output 0
			Variable_17 = Variable_18 * v_flt(0.2f);
			
			Outputs.Value = Variable_17;
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
			
			int32 Variable_8; // Get Material Collection Index: Rocks output 0
			v_flt Variable_17; // Valleys Height = -0.5 output 0
			v_flt Variable_16; // Erosion Material Strength = 3.0 output 0
			int32 Variable_4; // Get Material Collection Index: Snow output 0
			v_flt Variable_13; // 1 - X output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_1; // X output 0
			v_flt Variable_14; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_9; // * output 0
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
					
					// Init of Get Material Collection Index: Rocks
					if (InitStruct.MaterialCollection)
					{
						Get_Material_Collection_Index__Rocks_0_Index = InitStruct.MaterialCollection->GetMaterialIndex(Params.Rocks);
					}
					else
					{
						Get_Material_Collection_Index__Rocks_0_Index = -1;
					}
					
					// Init of Get Material Collection Index: Snow
					if (InitStruct.MaterialCollection)
					{
						Get_Material_Collection_Index__Snow_0_Index = InitStruct.MaterialCollection->GetMaterialIndex(Params.Snow);
					}
					else
					{
						Get_Material_Collection_Index__Snow_0_Index = -1;
					}
					
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
				// Get Material Collection Index: Rocks
				BufferConstant.Variable_8 = Get_Material_Collection_Index__Rocks_0_Index;
				
				// Erosion Material Offset = 0.65
				v_flt Variable_11; // Erosion Material Offset = 0.65 output 0
				Variable_11 = Params.Erosion_Material_Offset;
				
				// Valleys Height = -0.5
				BufferConstant.Variable_17 = Params.Valleys_Height;
				
				// Erosion Material Strength = 3.0
				BufferConstant.Variable_16 = Params.Erosion_Material_Strength;
				
				// Get Material Collection Index: Snow
				BufferConstant.Variable_4 = Get_Material_Collection_Index__Snow_0_Index;
				
				// 1 - X
				BufferConstant.Variable_13 = 1 - Variable_11;
				
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
		
		int32 Get_Material_Collection_Index__Rocks_0_Index;
		int32 Get_Material_Collection_Index__Snow_0_Index;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_1_LODToOctaves;
		FVoxelFastNoise _2D_Erosion_1_Noise;
		TStaticArray<uint8, 32> _2D_Erosion_1_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
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
			
			// Init of 2D Erosion
			_2D_Erosion_1_Noise.SetSeed(FVoxelGraphSeed(1337));
			_2D_Erosion_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_Erosion_1_Noise.SetFractalOctavesAndGain(5, 0.5);
			_2D_Erosion_1_Noise.SetFractalLacunarity(2.0);
			_2D_Erosion_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_Erosion_1_Noise.SetCellularJitter(0.5);
			_2D_Erosion_1_LODToOctaves[0] = 5;
			_2D_Erosion_1_LODToOctaves[1] = 5;
			_2D_Erosion_1_LODToOctaves[2] = 5;
			_2D_Erosion_1_LODToOctaves[3] = 5;
			_2D_Erosion_1_LODToOctaves[4] = 5;
			_2D_Erosion_1_LODToOctaves[5] = 5;
			_2D_Erosion_1_LODToOctaves[6] = 5;
			_2D_Erosion_1_LODToOctaves[7] = 5;
			_2D_Erosion_1_LODToOctaves[8] = 5;
			_2D_Erosion_1_LODToOctaves[9] = 5;
			_2D_Erosion_1_LODToOctaves[10] = 5;
			_2D_Erosion_1_LODToOctaves[11] = 5;
			_2D_Erosion_1_LODToOctaves[12] = 5;
			_2D_Erosion_1_LODToOctaves[13] = 5;
			_2D_Erosion_1_LODToOctaves[14] = 5;
			_2D_Erosion_1_LODToOctaves[15] = 5;
			_2D_Erosion_1_LODToOctaves[16] = 5;
			_2D_Erosion_1_LODToOctaves[17] = 5;
			_2D_Erosion_1_LODToOctaves[18] = 5;
			_2D_Erosion_1_LODToOctaves[19] = 5;
			_2D_Erosion_1_LODToOctaves[20] = 5;
			_2D_Erosion_1_LODToOctaves[21] = 5;
			_2D_Erosion_1_LODToOctaves[22] = 5;
			_2D_Erosion_1_LODToOctaves[23] = 5;
			_2D_Erosion_1_LODToOctaves[24] = 5;
			_2D_Erosion_1_LODToOctaves[25] = 5;
			_2D_Erosion_1_LODToOctaves[26] = 5;
			_2D_Erosion_1_LODToOctaves[27] = 5;
			_2D_Erosion_1_LODToOctaves[28] = 5;
			_2D_Erosion_1_LODToOctaves[29] = 5;
			_2D_Erosion_1_LODToOctaves[30] = 5;
			_2D_Erosion_1_LODToOctaves[31] = 5;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_18; // 2D Perlin Noise Fractal output 0
			v_flt Variable_19; // 2D Perlin Noise Fractal output 1
			v_flt Variable_20; // 2D Perlin Noise Fractal output 2
			Variable_18 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D_Deriv(BufferX.Variable_1, Variable_2, v_flt(0.001f), _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_19,Variable_20);
			Variable_18 = FMath::Clamp<v_flt>(Variable_18, -0.722935, 0.711631);
			Variable_19 = FMath::Clamp<v_flt>(Variable_19, -1.982108, 2.144371);
			Variable_20 = FMath::Clamp<v_flt>(Variable_20, -2.105316, 1.997740);
			
			// 2D Erosion
			v_flt Variable_0; // 2D Erosion output 0
			v_flt _2D_Erosion_1_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_1_Temp_2; // 2D Erosion output 2
			Variable_0 = _2D_Erosion_1_Noise.GetErosion_2D(BufferX.Variable_14, Variable_15, v_flt(0.02f), _2D_Erosion_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_19, Variable_20, _2D_Erosion_1_Temp_1, _2D_Erosion_1_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_2, -1.200000, 1.200000);
			
			// Smooth Step
			v_flt Variable_3; // Smooth Step output 0
			Variable_3 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_17, v_flt(0.0f), Variable_18);
			
			// *
			v_flt Variable_10; // * output 0
			Variable_10 = Variable_0 * Variable_3;
			
			// Smooth Step
			v_flt Variable_5; // Smooth Step output 0
			Variable_5 = FVoxelMathNodeFunctions::SmoothStep(v_flt(-1.2f), v_flt(1.2f), Variable_10);
			
			// 1 - X
			v_flt Variable_6; // 1 - X output 0
			Variable_6 = 1 - Variable_5;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_6 - BufferConstant.Variable_13;
			
			// Max (float)
			v_flt Variable_12; // Max (float) output 0
			Variable_12 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, v_flt(0.0f));
			
			// *
			BufferXY.Variable_9 = Variable_12 * BufferConstant.Variable_16;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// X
			BufferX.Variable_1 = Context.GetLocalX();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// X
			BufferX.Variable_14 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_18; // 2D Perlin Noise Fractal output 0
			v_flt Variable_19; // 2D Perlin Noise Fractal output 1
			v_flt Variable_20; // 2D Perlin Noise Fractal output 2
			Variable_18 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D_Deriv(BufferX.Variable_1, Variable_2, v_flt(0.001f), _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_19,Variable_20);
			Variable_18 = FMath::Clamp<v_flt>(Variable_18, -0.722935, 0.711631);
			Variable_19 = FMath::Clamp<v_flt>(Variable_19, -1.982108, 2.144371);
			Variable_20 = FMath::Clamp<v_flt>(Variable_20, -2.105316, 1.997740);
			
			// 2D Erosion
			v_flt Variable_0; // 2D Erosion output 0
			v_flt _2D_Erosion_1_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_1_Temp_2; // 2D Erosion output 2
			Variable_0 = _2D_Erosion_1_Noise.GetErosion_2D(BufferX.Variable_14, Variable_15, v_flt(0.02f), _2D_Erosion_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_19, Variable_20, _2D_Erosion_1_Temp_1, _2D_Erosion_1_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_2, -1.200000, 1.200000);
			
			// Smooth Step
			v_flt Variable_3; // Smooth Step output 0
			Variable_3 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_17, v_flt(0.0f), Variable_18);
			
			// *
			v_flt Variable_10; // * output 0
			Variable_10 = Variable_0 * Variable_3;
			
			// Smooth Step
			v_flt Variable_5; // Smooth Step output 0
			Variable_5 = FVoxelMathNodeFunctions::SmoothStep(v_flt(-1.2f), v_flt(1.2f), Variable_10);
			
			// 1 - X
			v_flt Variable_6; // 1 - X output 0
			Variable_6 = 1 - Variable_5;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_6 - BufferConstant.Variable_13;
			
			// Max (float)
			v_flt Variable_12; // Max (float) output 0
			Variable_12 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, v_flt(0.0f));
			
			// *
			BufferXY.Variable_9 = Variable_12 * BufferConstant.Variable_16;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			Outputs.MaterialBuilder.AddMultiIndex(BufferConstant.Variable_4, v_flt(1.0f), bool(false));
			Outputs.MaterialBuilder.AddMultiIndex(BufferConstant.Variable_8, BufferXY.Variable_9, bool(false));
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Y
			v_flt Variable_2; // Y output 0
			Variable_2 = Context.GetLocalY();
			
			// X
			v_flt Variable_1; // X output 0
			Variable_1 = Context.GetLocalX();
			
			// Y
			v_flt Variable_15; // Y output 0
			Variable_15 = Context.GetLocalY();
			
			// X
			v_flt Variable_14; // X output 0
			Variable_14 = Context.GetLocalX();
			
			// 2D Perlin Noise Fractal
			v_flt Variable_18; // 2D Perlin Noise Fractal output 0
			v_flt Variable_19; // 2D Perlin Noise Fractal output 1
			v_flt Variable_20; // 2D Perlin Noise Fractal output 2
			Variable_18 = _2D_Perlin_Noise_Fractal_1_Noise.GetPerlinFractal_2D_Deriv(Variable_1, Variable_2, v_flt(0.001f), _2D_Perlin_Noise_Fractal_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],Variable_19,Variable_20);
			Variable_18 = FMath::Clamp<v_flt>(Variable_18, -0.722935, 0.711631);
			Variable_19 = FMath::Clamp<v_flt>(Variable_19, -1.982108, 2.144371);
			Variable_20 = FMath::Clamp<v_flt>(Variable_20, -2.105316, 1.997740);
			
			// 2D Erosion
			v_flt Variable_0; // 2D Erosion output 0
			v_flt _2D_Erosion_1_Temp_1; // 2D Erosion output 1
			v_flt _2D_Erosion_1_Temp_2; // 2D Erosion output 2
			Variable_0 = _2D_Erosion_1_Noise.GetErosion_2D(Variable_14, Variable_15, v_flt(0.02f), _2D_Erosion_1_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)], Variable_19, Variable_20, _2D_Erosion_1_Temp_1, _2D_Erosion_1_Temp_2);
			Variable_0 = FMath::Clamp<v_flt>(Variable_0, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_1 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_1, -1.200000, 1.200000);
			_2D_Erosion_1_Temp_2 = FMath::Clamp<v_flt>(_2D_Erosion_1_Temp_2, -1.200000, 1.200000);
			
			// Smooth Step
			v_flt Variable_3; // Smooth Step output 0
			Variable_3 = FVoxelMathNodeFunctions::SmoothStep(BufferConstant.Variable_17, v_flt(0.0f), Variable_18);
			
			// *
			v_flt Variable_10; // * output 0
			Variable_10 = Variable_0 * Variable_3;
			
			// Smooth Step
			v_flt Variable_5; // Smooth Step output 0
			Variable_5 = FVoxelMathNodeFunctions::SmoothStep(v_flt(-1.2f), v_flt(1.2f), Variable_10);
			
			// 1 - X
			v_flt Variable_6; // 1 - X output 0
			Variable_6 = 1 - Variable_5;
			
			// -
			v_flt Variable_7; // - output 0
			Variable_7 = Variable_6 - BufferConstant.Variable_13;
			
			// Max (float)
			v_flt Variable_12; // Max (float) output 0
			Variable_12 = FVoxelNodeFunctions::Max<v_flt>(Variable_7, v_flt(0.0f));
			
			// *
			v_flt Variable_9; // * output 0
			Variable_9 = Variable_12 * BufferConstant.Variable_16;
			
			Outputs.MaterialBuilder.AddMultiIndex(BufferConstant.Variable_4, v_flt(1.0f), bool(false));
			Outputs.MaterialBuilder.AddMultiIndex(BufferConstant.Variable_8, Variable_9, bool(false));
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
			
			TVoxelRange<v_flt> Variable_0; // 2D Noise SDF.+ output 0
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
					
					// Init of 2D Erosion
					_2D_Erosion_2_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Erosion_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Erosion_2_Noise.SetFractalOctavesAndGain(5, 0.5);
					_2D_Erosion_2_Noise.SetFractalLacunarity(2.0);
					_2D_Erosion_2_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Erosion_2_Noise.SetCellularJitter(0.5);
					_2D_Erosion_2_LODToOctaves[0] = 5;
					_2D_Erosion_2_LODToOctaves[1] = 5;
					_2D_Erosion_2_LODToOctaves[2] = 5;
					_2D_Erosion_2_LODToOctaves[3] = 5;
					_2D_Erosion_2_LODToOctaves[4] = 5;
					_2D_Erosion_2_LODToOctaves[5] = 5;
					_2D_Erosion_2_LODToOctaves[6] = 5;
					_2D_Erosion_2_LODToOctaves[7] = 5;
					_2D_Erosion_2_LODToOctaves[8] = 5;
					_2D_Erosion_2_LODToOctaves[9] = 5;
					_2D_Erosion_2_LODToOctaves[10] = 5;
					_2D_Erosion_2_LODToOctaves[11] = 5;
					_2D_Erosion_2_LODToOctaves[12] = 5;
					_2D_Erosion_2_LODToOctaves[13] = 5;
					_2D_Erosion_2_LODToOctaves[14] = 5;
					_2D_Erosion_2_LODToOctaves[15] = 5;
					_2D_Erosion_2_LODToOctaves[16] = 5;
					_2D_Erosion_2_LODToOctaves[17] = 5;
					_2D_Erosion_2_LODToOctaves[18] = 5;
					_2D_Erosion_2_LODToOctaves[19] = 5;
					_2D_Erosion_2_LODToOctaves[20] = 5;
					_2D_Erosion_2_LODToOctaves[21] = 5;
					_2D_Erosion_2_LODToOctaves[22] = 5;
					_2D_Erosion_2_LODToOctaves[23] = 5;
					_2D_Erosion_2_LODToOctaves[24] = 5;
					_2D_Erosion_2_LODToOctaves[25] = 5;
					_2D_Erosion_2_LODToOctaves[26] = 5;
					_2D_Erosion_2_LODToOctaves[27] = 5;
					_2D_Erosion_2_LODToOctaves[28] = 5;
					_2D_Erosion_2_LODToOctaves[29] = 5;
					_2D_Erosion_2_LODToOctaves[30] = 5;
					_2D_Erosion_2_LODToOctaves[31] = 5;
					
					// Init of 2D Perlin Noise Fractal
					_2D_Perlin_Noise_Fractal_2_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_Perlin_Noise_Fractal_2_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalOctavesAndGain(3, 0.5);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalLacunarity(2.0);
					_2D_Perlin_Noise_Fractal_2_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[0] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[1] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[2] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[3] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[4] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[5] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[6] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[7] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[8] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[9] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[10] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[11] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[12] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[13] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[14] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[15] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[16] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[17] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[18] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[19] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[20] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[21] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[22] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[23] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[24] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[25] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[26] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[27] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[28] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[29] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[30] = 3;
					_2D_Perlin_Noise_Fractal_2_LODToOctaves[31] = 3;
					
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
				// 2D Erosion
				TVoxelRange<v_flt> Variable_1; // 2D Erosion output 0
				TVoxelRange<v_flt> _2D_Erosion_2_Temp_1; // 2D Erosion output 1
				TVoxelRange<v_flt> _2D_Erosion_2_Temp_2; // 2D Erosion output 2
				Variable_1 = { -1.200000f, 1.200000f };
				_2D_Erosion_2_Temp_1 = { -1.200000f, 1.200000f };
				_2D_Erosion_2_Temp_2 = { -1.200000f, 1.200000f };
				
				// 2D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_10; // 2D Perlin Noise Fractal output 0
				TVoxelRange<v_flt> _2D_Perlin_Noise_Fractal_2_Temp_1; // 2D Perlin Noise Fractal output 1
				TVoxelRange<v_flt> _2D_Perlin_Noise_Fractal_2_Temp_2; // 2D Perlin Noise Fractal output 2
				Variable_10 = { -0.722935f, 0.711631f };
				_2D_Perlin_Noise_Fractal_2_Temp_1 = { -1.982108f, 2.144371f };
				_2D_Perlin_Noise_Fractal_2_Temp_2 = { -2.105316f, 1.997740f };
				
				// Valleys Height = -0.5
				TVoxelRange<v_flt> Variable_9; // Valleys Height = -0.5 output 0
				Variable_9 = Params.Valleys_Height;
				
				// Height = 500.0
				TVoxelRange<v_flt> Variable_8; // Height = 500.0 output 0
				Variable_8 = Params.Height;
				
				// Erosion Strength = 0.008
				TVoxelRange<v_flt> Variable_7; // Erosion Strength = 0.008 output 0
				Variable_7 = Params.Erosion_Strength;
				
				// Smooth Step
				TVoxelRange<v_flt> Variable_5; // Smooth Step output 0
				Variable_5 = FVoxelMathNodeFunctions::SmoothStep(Variable_9, TVoxelRange<v_flt>(0.0f), Variable_10);
				
				// *
				TVoxelRange<v_flt> Variable_6; // * output 0
				Variable_6 = Variable_1 * Variable_5;
				
				// *
				TVoxelRange<v_flt> Variable_3; // * output 0
				Variable_3 = Variable_7 * Variable_6;
				
				// +
				TVoxelRange<v_flt> Variable_2; // + output 0
				Variable_2 = Variable_10 + Variable_3;
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_13; // 2D Noise SDF.* output 0
				Variable_13 = Variable_2 * Variable_8;
				
				// 2D Noise SDF.+
				BufferConstant.Variable_0 = Variable_13 + TVoxelRange<v_flt>(0.0f);
				
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
		
		FVoxelFastNoise _2D_Erosion_2_Noise;
		TStaticArray<uint8, 32> _2D_Erosion_2_LODToOctaves;
		FVoxelFastNoise _2D_Perlin_Noise_Fractal_2_Noise;
		TStaticArray<uint8, 32> _2D_Perlin_Noise_Fractal_2_LODToOctaves;
		
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
			TVoxelRange<v_flt> Variable_4; // Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_12; // 2D Noise SDF.- output 0
			Variable_12 = Variable_4 - BufferConstant.Variable_0;
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_11; // Set High Quality Value.* output 0
			Variable_11 = Variable_12 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_11;
		}
		
	};
	
	FVG_Example_ErosionInstance(UVG_Example_Erosion& Object)
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
			Object.Erosion_Material_Offset,
			Object.Erosion_Material_Strength,
			Object.Erosion_Strength,
			Object.Height,
			*Object.Rocks.GetAssetName(),
			*Object.Snow.GetAssetName(),
			Object.Valleys_Height
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
inline v_flt FVG_Example_ErosionInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVG_Example_ErosionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVG_Example_ErosionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_ErosionInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVG_Example_ErosionInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVG_Example_ErosionInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVG_Example_ErosionInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVG_Example_ErosionInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVG_Example_Erosion::UVG_Example_Erosion()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVG_Example_Erosion::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVG_Example_ErosionInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VG_Example_Erosion. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VG_Example_Erosion. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Erosion. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Erosion. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
