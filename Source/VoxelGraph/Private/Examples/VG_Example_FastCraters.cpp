// Copyright 2020 Phyronnaz

#include "VG_Example_FastCraters.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVG_Example_FastCratersInstance : public TVoxelGraphGeneratorInstanceHelper<FVG_Example_FastCratersInstance, UVG_Example_FastCraters>
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
			
			v_flt Variable_15; // XYZ.X output 0
			v_flt Variable_7; // XYZ.X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_16; // XYZ.Y output 0
			v_flt Variable_8; // XYZ.Y output 0
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
		
		FVoxelFastNoise _3D_Crater_Noise_Fractal_0_Noise;
		TStaticArray<uint8, 32> _3D_Crater_Noise_Fractal_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 3D Crater Noise Fractal
			_3D_Crater_Noise_Fractal_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			_3D_Crater_Noise_Fractal_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_3D_Crater_Noise_Fractal_0_Noise.SetFractalOctavesAndGain(5, 0.6);
			_3D_Crater_Noise_Fractal_0_Noise.SetFractalLacunarity(2.0);
			_3D_Crater_Noise_Fractal_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_3D_Crater_Noise_Fractal_0_Noise.SetCellularJitter(0.45);
			_3D_Crater_Noise_Fractal_0_Noise.SetCraterFalloffExponent(4.0);
			_3D_Crater_Noise_Fractal_0_LODToOctaves[0] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[1] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[2] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[3] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[4] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[5] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[6] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[7] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[8] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[9] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[10] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[11] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[12] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[13] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[14] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[15] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[16] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[17] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[18] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[19] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[20] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[21] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[22] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[23] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[24] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[25] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[26] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[27] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[28] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[29] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[30] = 5;
			_3D_Crater_Noise_Fractal_0_LODToOctaves[31] = 5;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// XYZ.X
			BufferX.Variable_15 = Context.GetLocalX();
			
			// XYZ.X
			BufferX.Variable_7 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// XYZ.Y
			BufferXY.Variable_16 = Context.GetLocalY();
			
			// XYZ.Y
			BufferXY.Variable_8 = Context.GetLocalY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// XYZ.Y
			BufferXY.Variable_16 = Context.GetLocalY();
			
			// XYZ.X
			BufferX.Variable_15 = Context.GetLocalX();
			
			// XYZ.X
			BufferX.Variable_7 = Context.GetLocalX();
			
			// XYZ.Y
			BufferXY.Variable_8 = Context.GetLocalY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// XYZ.Z
			v_flt Variable_0; // XYZ.Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// XYZ.Z
			v_flt Variable_4; // XYZ.Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// Normalize.Vector Length
			v_flt Variable_9; // Normalize.Vector Length output 0
			Variable_9 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_7, BufferXY.Variable_8, Variable_0);
			
			// Vector Length
			v_flt Variable_1; // Vector Length output 0
			Variable_1 = FVoxelNodeFunctions::VectorLength(BufferX.Variable_15, BufferXY.Variable_16, Variable_4);
			
			// Normalize./
			v_flt Variable_12; // Normalize./ output 0
			Variable_12 = Variable_0 / Variable_9;
			
			// Normalize./
			v_flt Variable_11; // Normalize./ output 0
			Variable_11 = BufferXY.Variable_8 / Variable_9;
			
			// Normalize./
			v_flt Variable_10; // Normalize./ output 0
			Variable_10 = BufferX.Variable_7 / Variable_9;
			
			// 3D Crater Noise Fractal
			v_flt Variable_2; // 3D Crater Noise Fractal output 0
			Variable_2 = _3D_Crater_Noise_Fractal_0_Noise.GetCraterFractal_3D(Variable_10, Variable_11, Variable_12, v_flt(2.0f), _3D_Crater_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, 0.008314, 0.543512);
			
			// * -1
			v_flt Variable_5; // * -1 output 0
			Variable_5 = Variable_2 * -1;
			
			// 2D Noise SDF.*
			v_flt Variable_14; // 2D Noise SDF.* output 0
			Variable_14 = Variable_5 * v_flt(30.0f);
			
			// 2D Noise SDF.+
			v_flt Variable_3; // 2D Noise SDF.+ output 0
			Variable_3 = Variable_14 + v_flt(500.0f);
			
			// 2D Noise SDF.-
			v_flt Variable_13; // 2D Noise SDF.- output 0
			Variable_13 = Variable_1 - Variable_3;
			
			// Set High Quality Value.*
			v_flt Variable_6; // Set High Quality Value.* output 0
			Variable_6 = Variable_13 * v_flt(0.2f);
			
			Outputs.Value = Variable_6;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// XYZ.Y
			v_flt Variable_16; // XYZ.Y output 0
			Variable_16 = Context.GetLocalY();
			
			// XYZ.Z
			v_flt Variable_0; // XYZ.Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// XYZ.X
			v_flt Variable_15; // XYZ.X output 0
			Variable_15 = Context.GetLocalX();
			
			// XYZ.X
			v_flt Variable_7; // XYZ.X output 0
			Variable_7 = Context.GetLocalX();
			
			// XYZ.Z
			v_flt Variable_4; // XYZ.Z output 0
			Variable_4 = Context.GetLocalZ();
			
			// XYZ.Y
			v_flt Variable_8; // XYZ.Y output 0
			Variable_8 = Context.GetLocalY();
			
			// Normalize.Vector Length
			v_flt Variable_9; // Normalize.Vector Length output 0
			Variable_9 = FVoxelNodeFunctions::VectorLength(Variable_7, Variable_8, Variable_0);
			
			// Vector Length
			v_flt Variable_1; // Vector Length output 0
			Variable_1 = FVoxelNodeFunctions::VectorLength(Variable_15, Variable_16, Variable_4);
			
			// Normalize./
			v_flt Variable_12; // Normalize./ output 0
			Variable_12 = Variable_0 / Variable_9;
			
			// Normalize./
			v_flt Variable_11; // Normalize./ output 0
			Variable_11 = Variable_8 / Variable_9;
			
			// Normalize./
			v_flt Variable_10; // Normalize./ output 0
			Variable_10 = Variable_7 / Variable_9;
			
			// 3D Crater Noise Fractal
			v_flt Variable_2; // 3D Crater Noise Fractal output 0
			Variable_2 = _3D_Crater_Noise_Fractal_0_Noise.GetCraterFractal_3D(Variable_10, Variable_11, Variable_12, v_flt(2.0f), _3D_Crater_Noise_Fractal_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)]);
			Variable_2 = FMath::Clamp<v_flt>(Variable_2, 0.008314, 0.543512);
			
			// * -1
			v_flt Variable_5; // * -1 output 0
			Variable_5 = Variable_2 * -1;
			
			// 2D Noise SDF.*
			v_flt Variable_14; // 2D Noise SDF.* output 0
			Variable_14 = Variable_5 * v_flt(30.0f);
			
			// 2D Noise SDF.+
			v_flt Variable_3; // 2D Noise SDF.+ output 0
			Variable_3 = Variable_14 + v_flt(500.0f);
			
			// 2D Noise SDF.-
			v_flt Variable_13; // 2D Noise SDF.- output 0
			Variable_13 = Variable_1 - Variable_3;
			
			// Set High Quality Value.*
			v_flt Variable_6; // Set High Quality Value.* output 0
			Variable_6 = Variable_13 * v_flt(0.2f);
			
			Outputs.Value = Variable_6;
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
			
			TVoxelRange<v_flt> Variable_2; // 2D Noise SDF.+ output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_8; // XYZ.X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_9; // XYZ.Y output 0
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
					
					// Init of 3D Crater Noise Fractal
					_3D_Crater_Noise_Fractal_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_3D_Crater_Noise_Fractal_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_3D_Crater_Noise_Fractal_1_Noise.SetFractalOctavesAndGain(5, 0.6);
					_3D_Crater_Noise_Fractal_1_Noise.SetFractalLacunarity(2.0);
					_3D_Crater_Noise_Fractal_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_3D_Crater_Noise_Fractal_1_Noise.SetCellularJitter(0.45);
					_3D_Crater_Noise_Fractal_1_Noise.SetCraterFalloffExponent(4.0);
					_3D_Crater_Noise_Fractal_1_LODToOctaves[0] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[1] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[2] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[3] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[4] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[5] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[6] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[7] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[8] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[9] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[10] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[11] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[12] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[13] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[14] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[15] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[16] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[17] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[18] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[19] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[20] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[21] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[22] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[23] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[24] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[25] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[26] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[27] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[28] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[29] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[30] = 5;
					_3D_Crater_Noise_Fractal_1_LODToOctaves[31] = 5;
					
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
				// 3D Crater Noise Fractal
				TVoxelRange<v_flt> Variable_1; // 3D Crater Noise Fractal output 0
				Variable_1 = { 0.008314f, 0.543512f };
				
				// * -1
				TVoxelRange<v_flt> Variable_4; // * -1 output 0
				Variable_4 = Variable_1 * -1;
				
				// 2D Noise SDF.*
				TVoxelRange<v_flt> Variable_7; // 2D Noise SDF.* output 0
				Variable_7 = Variable_4 * TVoxelRange<v_flt>(30.0f);
				
				// 2D Noise SDF.+
				BufferConstant.Variable_2 = Variable_7 + TVoxelRange<v_flt>(500.0f);
				
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
		
		FVoxelFastNoise _3D_Crater_Noise_Fractal_1_Noise;
		TStaticArray<uint8, 32> _3D_Crater_Noise_Fractal_1_LODToOctaves;
		
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
			// XYZ.Y
			TVoxelRange<v_flt> Variable_9; // XYZ.Y output 0
			Variable_9 = Context.GetLocalY();
			
			// XYZ.X
			TVoxelRange<v_flt> Variable_8; // XYZ.X output 0
			Variable_8 = Context.GetLocalX();
			
			// XYZ.Z
			TVoxelRange<v_flt> Variable_3; // XYZ.Z output 0
			Variable_3 = Context.GetLocalZ();
			
			// Vector Length
			TVoxelRange<v_flt> Variable_0; // Vector Length output 0
			Variable_0 = FVoxelNodeFunctions::VectorLength(Variable_8, Variable_9, Variable_3);
			
			// 2D Noise SDF.-
			TVoxelRange<v_flt> Variable_6; // 2D Noise SDF.- output 0
			Variable_6 = Variable_0 - BufferConstant.Variable_2;
			
			// Set High Quality Value.*
			TVoxelRange<v_flt> Variable_5; // Set High Quality Value.* output 0
			Variable_5 = Variable_6 * TVoxelRange<v_flt>(0.2f);
			
			Outputs.Value = Variable_5;
		}
		
	};
	
	FVG_Example_FastCratersInstance(UVG_Example_FastCraters& Object)
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
inline v_flt FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVG_Example_FastCratersInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVG_Example_FastCratersInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVG_Example_FastCratersInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVG_Example_FastCratersInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVG_Example_FastCratersInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVG_Example_FastCraters::UVG_Example_FastCraters()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVG_Example_FastCraters::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVG_Example_FastCratersInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VG_Example_FastCraters. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VG_Example_FastCraters. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_FastCraters. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VG_Example_FastCraters. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
