// Copyright 2020 Phyronnaz

#include "VoxelExample_IQNoise.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_IQNoiseInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_IQNoiseInstance, UVoxelExample_IQNoise>
{
public:
	struct FParams
	{
		const float Frequency;
		const float Height;
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
			
			v_flt Variable_7; // Frequency = 0.001 output 0
			v_flt Variable_3; // Height = 500.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_5; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_2; // * output 0
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
					FVoxelGraphSeed Variable_8; // Seed = 1443 output 0
					Variable_8 = Params.Seed;
					
					
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
				// Frequency = 0.001
				BufferConstant.Variable_7 = Params.Frequency;
				
				// Height = 500.0
				BufferConstant.Variable_3 = Params.Height;
				
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
		
		FVoxelFastNoise _2D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_0_LODToOctaves;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of Seed = 1443
			FVoxelGraphSeed Variable_8; // Seed = 1443 output 0
			Variable_8 = Params.Seed;
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_8);
			_2D_IQ_Noise_0_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
			_2D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_0_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_0_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
			_2D_IQ_Noise_0_Noise.SetMatrixFromRotation_2D(40.0);
			_2D_IQ_Noise_0_LODToOctaves[0] = 15;
			_2D_IQ_Noise_0_LODToOctaves[1] = 15;
			_2D_IQ_Noise_0_LODToOctaves[2] = 15;
			_2D_IQ_Noise_0_LODToOctaves[3] = 15;
			_2D_IQ_Noise_0_LODToOctaves[4] = 15;
			_2D_IQ_Noise_0_LODToOctaves[5] = 15;
			_2D_IQ_Noise_0_LODToOctaves[6] = 15;
			_2D_IQ_Noise_0_LODToOctaves[7] = 15;
			_2D_IQ_Noise_0_LODToOctaves[8] = 15;
			_2D_IQ_Noise_0_LODToOctaves[9] = 15;
			_2D_IQ_Noise_0_LODToOctaves[10] = 15;
			_2D_IQ_Noise_0_LODToOctaves[11] = 15;
			_2D_IQ_Noise_0_LODToOctaves[12] = 15;
			_2D_IQ_Noise_0_LODToOctaves[13] = 15;
			_2D_IQ_Noise_0_LODToOctaves[14] = 15;
			_2D_IQ_Noise_0_LODToOctaves[15] = 15;
			_2D_IQ_Noise_0_LODToOctaves[16] = 15;
			_2D_IQ_Noise_0_LODToOctaves[17] = 15;
			_2D_IQ_Noise_0_LODToOctaves[18] = 15;
			_2D_IQ_Noise_0_LODToOctaves[19] = 15;
			_2D_IQ_Noise_0_LODToOctaves[20] = 15;
			_2D_IQ_Noise_0_LODToOctaves[21] = 15;
			_2D_IQ_Noise_0_LODToOctaves[22] = 15;
			_2D_IQ_Noise_0_LODToOctaves[23] = 15;
			_2D_IQ_Noise_0_LODToOctaves[24] = 15;
			_2D_IQ_Noise_0_LODToOctaves[25] = 15;
			_2D_IQ_Noise_0_LODToOctaves[26] = 15;
			_2D_IQ_Noise_0_LODToOctaves[27] = 15;
			_2D_IQ_Noise_0_LODToOctaves[28] = 15;
			_2D_IQ_Noise_0_LODToOctaves[29] = 15;
			_2D_IQ_Noise_0_LODToOctaves[30] = 15;
			_2D_IQ_Noise_0_LODToOctaves[31] = 15;
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// X
			BufferX.Variable_5 = Context.GetLocalX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_6; // Y output 0
			Variable_6 = Context.GetLocalY();
			
			// 2D IQ Noise
			v_flt Variable_4; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_4 = FMath::Clamp<v_flt>(Variable_4, -0.722945, 0.798964);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.342896, 1.704131);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.209649, 1.295183);
			
			// *
			BufferXY.Variable_2 = Variable_4 * BufferConstant.Variable_3;
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Y
			v_flt Variable_6; // Y output 0
			Variable_6 = Context.GetLocalY();
			
			// X
			BufferX.Variable_5 = Context.GetLocalX();
			
			// 2D IQ Noise
			v_flt Variable_4; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(BufferX.Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_4 = FMath::Clamp<v_flt>(Variable_4, -0.722945, 0.798964);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.342896, 1.704131);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.209649, 1.295183);
			
			// *
			BufferXY.Variable_2 = Variable_4 * BufferConstant.Variable_3;
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// -
			v_flt Variable_1; // - output 0
			Variable_1 = Variable_0 - BufferXY.Variable_2;
			
			Outputs.Value = Variable_1;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Z
			v_flt Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// Y
			v_flt Variable_6; // Y output 0
			Variable_6 = Context.GetLocalY();
			
			// X
			v_flt Variable_5; // X output 0
			Variable_5 = Context.GetLocalX();
			
			// 2D IQ Noise
			v_flt Variable_4; // 2D IQ Noise output 0
			v_flt _2D_IQ_Noise_0_Temp_1; // 2D IQ Noise output 1
			v_flt _2D_IQ_Noise_0_Temp_2; // 2D IQ Noise output 2
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoise_2D_Deriv(Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			Variable_4 = FMath::Clamp<v_flt>(Variable_4, -0.722945, 0.798964);
			_2D_IQ_Noise_0_Temp_1 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_1, -1.342896, 1.704131);
			_2D_IQ_Noise_0_Temp_2 = FMath::Clamp<v_flt>(_2D_IQ_Noise_0_Temp_2, -1.209649, 1.295183);
			
			// *
			v_flt Variable_2; // * output 0
			Variable_2 = Variable_4 * BufferConstant.Variable_3;
			
			// -
			v_flt Variable_1; // - output 0
			Variable_1 = Variable_0 - Variable_2;
			
			Outputs.Value = Variable_1;
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
			
			TVoxelRange<v_flt> Variable_2; // * output 0
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
					
					// Init of 2D IQ Noise
					_2D_IQ_Noise_1_Noise.SetSeed(FVoxelGraphSeed(1337));
					_2D_IQ_Noise_1_Noise.SetInterpolation(EVoxelNoiseInterpolation::Quintic);
					_2D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.5);
					_2D_IQ_Noise_1_Noise.SetFractalLacunarity(2.0);
					_2D_IQ_Noise_1_Noise.SetFractalType(EVoxelNoiseFractalType::FBM);
					_2D_IQ_Noise_1_Noise.SetMatrixFromRotation_2D(40.0);
					_2D_IQ_Noise_1_LODToOctaves[0] = 15;
					_2D_IQ_Noise_1_LODToOctaves[1] = 15;
					_2D_IQ_Noise_1_LODToOctaves[2] = 15;
					_2D_IQ_Noise_1_LODToOctaves[3] = 15;
					_2D_IQ_Noise_1_LODToOctaves[4] = 15;
					_2D_IQ_Noise_1_LODToOctaves[5] = 15;
					_2D_IQ_Noise_1_LODToOctaves[6] = 15;
					_2D_IQ_Noise_1_LODToOctaves[7] = 15;
					_2D_IQ_Noise_1_LODToOctaves[8] = 15;
					_2D_IQ_Noise_1_LODToOctaves[9] = 15;
					_2D_IQ_Noise_1_LODToOctaves[10] = 15;
					_2D_IQ_Noise_1_LODToOctaves[11] = 15;
					_2D_IQ_Noise_1_LODToOctaves[12] = 15;
					_2D_IQ_Noise_1_LODToOctaves[13] = 15;
					_2D_IQ_Noise_1_LODToOctaves[14] = 15;
					_2D_IQ_Noise_1_LODToOctaves[15] = 15;
					_2D_IQ_Noise_1_LODToOctaves[16] = 15;
					_2D_IQ_Noise_1_LODToOctaves[17] = 15;
					_2D_IQ_Noise_1_LODToOctaves[18] = 15;
					_2D_IQ_Noise_1_LODToOctaves[19] = 15;
					_2D_IQ_Noise_1_LODToOctaves[20] = 15;
					_2D_IQ_Noise_1_LODToOctaves[21] = 15;
					_2D_IQ_Noise_1_LODToOctaves[22] = 15;
					_2D_IQ_Noise_1_LODToOctaves[23] = 15;
					_2D_IQ_Noise_1_LODToOctaves[24] = 15;
					_2D_IQ_Noise_1_LODToOctaves[25] = 15;
					_2D_IQ_Noise_1_LODToOctaves[26] = 15;
					_2D_IQ_Noise_1_LODToOctaves[27] = 15;
					_2D_IQ_Noise_1_LODToOctaves[28] = 15;
					_2D_IQ_Noise_1_LODToOctaves[29] = 15;
					_2D_IQ_Noise_1_LODToOctaves[30] = 15;
					_2D_IQ_Noise_1_LODToOctaves[31] = 15;
					
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
				TVoxelRange<v_flt> Variable_3; // Height = 500.0 output 0
				Variable_3 = Params.Height;
				
				// 2D IQ Noise
				TVoxelRange<v_flt> Variable_4; // 2D IQ Noise output 0
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
				TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
				Variable_4 = { -0.722945f, 0.798964f };
				_2D_IQ_Noise_1_Temp_1 = { -1.342896f, 1.704131f };
				_2D_IQ_Noise_1_Temp_2 = { -1.209649f, 1.295183f };
				
				// *
				BufferConstant.Variable_2 = Variable_4 * Variable_3;
				
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
		
		FVoxelFastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		
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
			TVoxelRange<v_flt> Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// -
			TVoxelRange<v_flt> Variable_1; // - output 0
			Variable_1 = Variable_0 - BufferConstant.Variable_2;
			
			Outputs.Value = Variable_1;
		}
		
	};
	
	FVoxelExample_IQNoiseInstance(UVoxelExample_IQNoise& Object)
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
			Object.Height,
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
inline v_flt FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_IQNoise::UVoxelExample_IQNoise()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_IQNoise::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_IQNoiseInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_IQNoise. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_IQNoise. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_IQNoise. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_IQNoise. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
