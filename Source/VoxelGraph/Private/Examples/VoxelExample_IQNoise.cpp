// Copyright 2020 Phyronnaz

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnull-dereference"
#else
#pragma warning(push)
#pragma warning(disable : 4101 4701)
#endif

#include "VoxelExample_IQNoise.h"

using Seed = int32;

class FVoxelExample_IQNoiseInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_IQNoiseInstance, UVoxelExample_IQNoise>
{
public:
	class FLocalComputeStruct_LocalValue
	{
	public:
		struct FOutputs
		{
			FOutputs() {}
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
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
		
		FLocalComputeStruct_LocalValue(const float& InHeight, const float& InFrequency)
			: Height(InHeight)
			, Frequency(InFrequency)
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
					
					// Init of Seed
					Seed Variable_8; // Seed output 0
					{
						static FName StaticName = "Seed";
						Variable_8 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 1443;
					}
					
					
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
				BufferConstant.Variable_7 = Frequency;
				
				// Height = 500.0
				BufferConstant.Variable_3 = Height;
				
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
		FastNoise _2D_IQ_Noise_0_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_0_LODToOctaves;
		
		const float& Height;
		const float& Frequency;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of Seed
			Seed Variable_8; // Seed output 0
			{
				static FName StaticName = "Seed";
				Variable_8 = InitStruct.Seeds.Contains(StaticName) ? InitStruct.Seeds[StaticName] : 1443;
			}
			
			// Init of 2D IQ Noise
			_2D_IQ_Noise_0_Noise.SetSeed(Variable_8);
			_2D_IQ_Noise_0_Noise.SetInterp(FastNoise::Quintic);
			_2D_IQ_Noise_0_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_0_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_0_Noise.SetFractalType(FastNoise::FBM);
			_2D_IQ_Noise_0_Noise.SetMatrix(FMatrix2x2(FQuat2D(FMath::DegreesToRadians(40.000000))));
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
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(BufferX.Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
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
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(BufferX.Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
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
			Variable_4 = _2D_IQ_Noise_0_Noise.IQNoiseDeriv_2D(Variable_5, Variable_6, BufferConstant.Variable_7, _2D_IQ_Noise_0_LODToOctaves[FMath::Clamp(Context.LOD, 0, 31)],_2D_IQ_Noise_0_Temp_1,_2D_IQ_Noise_0_Temp_2);
			
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
			FVoxelMaterial Material;
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
		
		FLocalComputeStruct_LocalMaterial(const float& InHeight, const float& InFrequency)
			: Height(InHeight)
			, Frequency(InFrequency)
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
		
		const float& Height;
		const float& Frequency;
		
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(T*)nullptr;
			}
			
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
		
		FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ(const float& InHeight, const float& InFrequency)
			: Height(InHeight)
			, Frequency(InFrequency)
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
		
		const float& Height;
		const float& Frequency;
		
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
			
			template<typename T, uint32 Index>
			inline auto& GetRef()
			{
				unimplemented();
				return *(TVoxelRange<T>*)nullptr;
			}
			
			TVoxelRange<v_flt> Value;
		};
		struct FBufferConstant
		{
			FBufferConstant() {}
			
			TVoxelRange<v_flt> Variable_7; // Frequency = 0.001 output 0
			TVoxelRange<v_flt> Variable_3; // Height = 500.0 output 0
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_5; // X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_2; // * output 0
		};
		
		FLocalComputeStruct_LocalValueRangeAnalysis(const float& InHeight, const float& InFrequency)
			: Height(InHeight)
			, Frequency(InFrequency)
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
				// Frequency = 0.001
				BufferConstant.Variable_7 = Frequency;
				
				// Height = 500.0
				BufferConstant.Variable_3 = Height;
				
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
		FastNoise _2D_IQ_Noise_1_Noise;
		TStaticArray<uint8, 32> _2D_IQ_Noise_1_LODToOctaves;
		
		const float& Height;
		const float& Frequency;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelWorldGeneratorInit& InitStruct)
		{
			// Init of 2D IQ Noise
			_2D_IQ_Noise_1_Noise.SetSeed(Seed(1337));
			_2D_IQ_Noise_1_Noise.SetInterp(FastNoise::Quintic);
			_2D_IQ_Noise_1_Noise.SetFractalOctavesAndGain(15, 0.5);
			_2D_IQ_Noise_1_Noise.SetFractalLacunarity(2.0);
			_2D_IQ_Noise_1_Noise.SetFractalType(FastNoise::FBM);
			_2D_IQ_Noise_1_Noise.SetMatrix(FMatrix2x2(FQuat2D(FMath::DegreesToRadians(40.000000))));
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
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContextRange& Context, FOutputs& Outputs) const
		{
			// Z
			TVoxelRange<v_flt> Variable_0; // Z output 0
			Variable_0 = Context.GetLocalZ();
			
			// X
			TVoxelRange<v_flt> Variable_5; // X output 0
			Variable_5 = Context.GetLocalX();
			
			// Y
			TVoxelRange<v_flt> Variable_6; // Y output 0
			Variable_6 = Context.GetLocalY();
			
			// 2D IQ Noise
			TVoxelRange<v_flt> Variable_4; // 2D IQ Noise output 0
			TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_1; // 2D IQ Noise output 1
			TVoxelRange<v_flt> _2D_IQ_Noise_1_Temp_2; // 2D IQ Noise output 2
			Variable_4 = { -0.722945f, 0.798964f };
			_2D_IQ_Noise_1_Temp_1 = { -1.342896f, 1.704131f };
			_2D_IQ_Noise_1_Temp_2 = { -1.209649f, 1.295183f };
			
			// *
			TVoxelRange<v_flt> Variable_2; // * output 0
			Variable_2 = Variable_4 * BufferConstant.Variable_3;
			
			// -
			TVoxelRange<v_flt> Variable_1; // - output 0
			Variable_1 = Variable_0 - Variable_2;
			
			Outputs.Value = Variable_1;
		}
		
	};
	
	FVoxelExample_IQNoiseInstance(const float& InHeight, const float& InFrequency, bool bEnableRangeAnalysis)
		: TVoxelGraphGeneratorInstanceHelper(
		{
			{"Value", 1}
		},
		{
		},
		{
			{"Value", NoTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr<v_flt>>()}
		},
		{
		},
		{
			{"Value", NoTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr<v_flt>>()}
		},
		{
			{"Value", WithTransformAccessor<v_flt>::Get<1, TOutputFunctionPtr_Transform<v_flt>>()}
		},
		{
		},
		{
			{"Value", WithTransformRangeAccessor<v_flt>::Get<1, TRangeOutputFunctionPtr_Transform<v_flt>>()}
		},
		bEnableRangeAnalysis)
		, Height(InHeight)
		, Frequency(InFrequency)
		, LocalValue(Height, Frequency)
		, LocalMaterial(Height, Frequency)
		, LocalUpVectorXUpVectorYUpVectorZ(Height, Frequency)
		, LocalValueRangeAnalysis(Height, Frequency)
	{
	}
	
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) override final
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
	
	inline void ReportRangeAnalysisFailure() const {}
	
private:
	const float Height;
	const float Frequency;
	FLocalComputeStruct_LocalValue LocalValue;
	FLocalComputeStruct_LocalMaterial LocalMaterial;
	FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ LocalUpVectorXUpVectorYUpVectorZ;
	FLocalComputeStruct_LocalValueRangeAnalysis LocalValueRangeAnalysis;
	
};

template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValue::FOutputs::GetRef<v_flt, 1>()
{
	return Value;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalMaterial::FOutputs::GetRef<FVoxelMaterial, 2>()
{
	return Material;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 3>()
{
	return UpVectorX;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 4>()
{
	return UpVectorY;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::GetRef<v_flt, 5>()
{
	return UpVectorZ;
}
template<>
inline auto& FVoxelExample_IQNoiseInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::GetRef<v_flt, 1>()
{
	return Value;
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

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_IQNoise::UVoxelExample_IQNoise()
{
	bEnableRangeAnalysis = true;
}

TMap<FName, int32> UVoxelExample_IQNoise::GetDefaultSeeds() const
{
	return {
		{ "Seed", 1443 },
		};
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelExample_IQNoise::GetTransformableInstance()
{
	return MakeVoxelShared<FVoxelExample_IQNoiseInstance>(
		Height,
		Frequency,
		bEnableRangeAnalysis);
}

#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma warning(pop)
#endif

