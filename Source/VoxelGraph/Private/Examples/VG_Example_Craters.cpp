// Copyright 2020 Phyronnaz

#include "VG_Example_Craters.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVG_Example_CratersInstance : public TVoxelGraphGeneratorInstanceHelper<FVG_Example_CratersInstance, UVG_Example_Craters>
{
public:
	struct FParams
	{
		const float Radius;
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
			
			v_flt Variable_15; // Radius = 200.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_4; // X output 0
			v_flt Variable_8; // X output 0
			v_flt Variable_0; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_5; // Y output 0
			v_flt Variable_1; // Y output 0
			v_flt Variable_9; // Y output 0
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
				// Radius = 200.0
				BufferConstant.Variable_15 = Params.Radius;
				
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
			// Init of 3D Perlin Noise Fractal
			_3D_Perlin_Noise_Fractal_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Perlin_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(3, 0.5);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalLacunarity(4.0);
			_3D_Perlin_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[0] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[1] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[2] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[3] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[4] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[5] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[6] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[7] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[8] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[9] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[10] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[11] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[12] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[13] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[14] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[15] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[16] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[17] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[18] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[19] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[20] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[21] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[22] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[23] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[24] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[25] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[26] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[27] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[28] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[29] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[30] = 3;
			_3D_Perlin_Noise_Fractal_0_LODToOctaves[31] = 3;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_4 = Context.GetLocalX();
			
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			BufferXY.Variable_5 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// X
			BufferX.Variable_4 = Context.GetLocalX();
			
			// X
			BufferX.Variable_8 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_5 = Context.GetLocalY();
			
			// X
			BufferX.Variable_0 = Context.GetLocalX();
			
			// Y
			BufferXY.Variable_1 = Context.GetLocalY();
			
			// Y
			BufferXY.Variable_9 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// Normalize.Vector Length
			v_flt Variable_17; // Normalize.Vector Length output 0
			Variable_17 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_4, BufferXY.Variable_5, Variable_6);
			
			// Data Item Sample
			v_flt Variable_7; // Data Item Sample output 0
			Variable_7 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, BufferX.Variable_8, BufferXY.Variable_9, Variable_10, v_flt(0.0f), v_flt(0.0f), 1u, EVoxelDataItemCombineMode::Sum);
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_0, BufferXY.Variable_1, Variable_2);
			
			// Normalize./
			v_flt Variable_18; // Normalize./ output 0
			Variable_18 = BufferX.Variable_4 / Variable_17;
			
			// Normalize./
			v_flt Variable_19; // Normalize./ output 0
			Variable_19 = BufferXY.Variable_5 / Variable_17;
			
			// Normalize./
			v_flt Variable_20; // Normalize./ output 0
			Variable_20 = Variable_6 / Variable_17;
			
			// 3D Perlin Noise Fractal
			v_flt Variable_11; // 3D Perlin Noise Fractal output 0
			Variable_11 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(Variable_18, Variable_19, Variable_20, v_flt(1.0f), _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_11 = FMath::Clamp<v_flt>(Variable_11, -0.686521, 0.684919);
			
			// *
			v_flt Variable_12; // * output 0
			Variable_12 = Variable_11 * v_flt(20.0f);
			
			// +
			v_flt Variable_14; // + output 0
			Variable_14 = BufferConstant.Variable_15 + Variable_12 + Variable_7;
			
			// -
			v_flt Variable_13; // - output 0
			Variable_13 = Variable_3 - Variable_14;
			
			// Set High Quality Value.*
			v_flt Variable_16; // Set High Quality Value.* output 0
			Variable_16 = Variable_13 * v_flt(0.2f);
			
			Outputs.Value = Variable_16;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// X
			v_flt Variable_4; // X output 0
			Variable_4 = Context.GetLocalX();
			
			// X
			v_flt Variable_8; // X output 0
			Variable_8 = Context.GetLocalX();
			
			// Z
			v_flt Variable_10; // Z output 0
			Variable_10 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_5; // Y output 0
			Variable_5 = Context.GetLocalY();
			
			// Z
			v_flt Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// X
			v_flt Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Y
			v_flt Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			v_flt Variable_6; // Z output 0
			Variable_6 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_9; // Y output 0
			Variable_9 = Context.GetLocalY();
			
			// Normalize.Vector Length
			v_flt Variable_17; // Normalize.Vector Length output 0
			Variable_17 = FVoxelNodeFunctions::VectorLength(Variable_4, Variable_5, Variable_6);
			
			// Data Item Sample
			v_flt Variable_7; // Data Item Sample output 0
			Variable_7 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_8, Variable_9, Variable_10, v_flt(0.0f), v_flt(0.0f), 1u, EVoxelDataItemCombineMode::Sum);
			
			// Vector Length
			v_flt Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// Normalize./
			v_flt Variable_18; // Normalize./ output 0
			Variable_18 = Variable_4 / Variable_17;
			
			// Normalize./
			v_flt Variable_19; // Normalize./ output 0
			Variable_19 = Variable_5 / Variable_17;
			
			// Normalize./
			v_flt Variable_20; // Normalize./ output 0
			Variable_20 = Variable_6 / Variable_17;
			
			// 3D Perlin Noise Fractal
			v_flt Variable_11; // 3D Perlin Noise Fractal output 0
			Variable_11 = _3D_Perlin_Noise_Fractal_0_Noise.GetPerlinFractal_3D(Variable_18, Variable_19, Variable_20, v_flt(1.0f), _3D_Perlin_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_11 = FMath::Clamp<v_flt>(Variable_11, -0.686521, 0.684919);
			
			// *
			v_flt Variable_12; // * output 0
			Variable_12 = Variable_11 * v_flt(20.0f);
			
			// +
			v_flt Variable_14; // + output 0
			Variable_14 = BufferConstant.Variable_15 + Variable_12 + Variable_7;
			
			// -
			v_flt Variable_13; // - output 0
			Variable_13 = Variable_3 - Variable_14;
			
			// Set High Quality Value.*
			v_flt Variable_16; // Set High Quality Value.* output 0
			Variable_16 = Variable_13 * v_flt(0.2f);
			
			Outputs.Value = Variable_16;
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
			
			TVoxelRange<v_flt> Variable_12; // Radius = 200.0 output 0
			TVoxelRange<v_flt> Variable_9; // * output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_0; // X output 0
			TVoxelRange<v_flt> Variable_5; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_6; // Y output 0
			TVoxelRange<v_flt> Variable_1; // Y output 0
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
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(3, 0.5);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalLacunarity(4.0);
					_3D_Perlin_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[0] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[1] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[2] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[3] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[4] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[5] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[6] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[7] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[8] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[9] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[10] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[11] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[12] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[13] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[14] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[15] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[16] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[17] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[18] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[19] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[20] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[21] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[22] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[23] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[24] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[25] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[26] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[27] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[28] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[29] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[30] = 3;
					_3D_Perlin_Noise_Fractal_1_LODToOctaves[31] = 3;
					
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
				// Radius = 200.0
				BufferConstant.Variable_12 = Params.Radius;
				
				// 3D Perlin Noise Fractal
				TVoxelRange<v_flt> Variable_8; // 3D Perlin Noise Fractal output 0
				Variable_8 = { -0.686521f, 0.684919f };
				
				// *
				BufferConstant.Variable_9 = Variable_8 * TVoxelRange<v_flt>(20.0f);
				
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
			TVoxelRange<v_flt> Variable_2; // Z output 0
			Variable_2 = Context.GetLocalZ();
			
			// Y
			TVoxelRange<v_flt> Variable_6; // Y output 0
			Variable_6 = Context.GetLocalY();
			
			// X
			TVoxelRange<v_flt> Variable_0; // X output 0
			Variable_0 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_1; // Y output 0
			Variable_1 = Context.GetLocalY();
			
			// Z
			TVoxelRange<v_flt> Variable_7; // Z output 0
			Variable_7 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_5; // X output 0
			Variable_5 = Context.GetLocalX();
			
			// Data Item Sample
			TVoxelRange<v_flt> Variable_4; // Data Item Sample output 0
			Variable_4 = FVoxelNodeFunctions::GetDataItemDistance(Context.Items.ItemHolder, Variable_5, Variable_6, Variable_7, TVoxelRange<v_flt>(0.0f), TVoxelRange<v_flt>(0.0f), 1u, EVoxelDataItemCombineMode::Sum);
			
			// Vector Length
			TVoxelRange<v_flt> Variable_3; // Vector Length output 0
			Variable_3 = FVoxelNodeFunctions::VectorLength(Variable_0, Variable_1, Variable_2);
			
			// +
			TVoxelRange<v_flt> Variable_11; // + output 0
			Variable_11 = BufferConstant.Variable_12 + BufferConstant.Variable_9 + Variable_4;
			
			// -
			TVoxelRange<v_flt> Variable_10; // - output 0
			Variable_10 = Variable_3 - Variable_11;
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_13; // Set High Quality Value.* output 0
			Variable_13 = Variable_10 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_13;
		}
		
	};
	
	FVG_Example_CratersInstance(UVG_Example_Craters& Object)
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
			Object.Radius
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
inline v_flt FVG_Example_CratersInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVG_Example_CratersInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVG_Example_CratersInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_CratersInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVG_Example_CratersInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVG_Example_CratersInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVG_Example_CratersInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVG_Example_CratersInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVG_Example_Craters::UVG_Example_Craters()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVG_Example_Craters::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVG_Example_CratersInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VG_Example_Craters. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VG_Example_Craters. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Craters. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_Craters. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
