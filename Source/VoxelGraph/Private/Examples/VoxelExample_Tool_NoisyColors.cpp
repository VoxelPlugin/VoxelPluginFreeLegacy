// Copyright 2020 Phyronnaz

#include "VoxelExample_Tool_NoisyColors.h"

PRAGMA_GENERATED_VOXEL_GRAPH_START

using FVoxelGraphSeed = int32;

#if VOXEL_GRAPH_GENERATED_VERSION == 1
class FVoxelExample_Tool_NoisyColorsInstance : public TVoxelGraphGeneratorInstanceHelper<FVoxelExample_Tool_NoisyColorsInstance, UVoxelExample_Tool_NoisyColors>
{
public:
	struct FParams
	{
		const FColor Color;
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
			
			v_flt Variable_1; // Previous Generator Value Material.Global X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			v_flt Variable_2; // Previous Generator Value Material.Global Y output 0
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
			// Previous Generator Value Material.Global X
			BufferX.Variable_1 = Context.GetWorldX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Previous Generator Value Material.Global Y
			BufferXY.Variable_2 = Context.GetWorldY();
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Previous Generator Value Material.Global X
			BufferX.Variable_1 = Context.GetWorldX();
			
			// Previous Generator Value Material.Global Y
			BufferXY.Variable_2 = Context.GetWorldY();
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			// Previous Generator Value Material.Global Z
			v_flt Variable_3; // Previous Generator Value Material.Global Z output 0
			Variable_3 = Context.GetWorldZ();
			
			// Previous Generator Value Material.Get Previous Generator Value
			v_flt Variable_0; // Previous Generator Value Material.Get Previous Generator Value output 0
			Variable_0 = FVoxelNodeFunctions::GetPreviousGeneratorValue(BufferX.Variable_1, BufferXY.Variable_2, Variable_3, Context, nullptr);
			
			Outputs.Value = Variable_0;
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Previous Generator Value Material.Global X
			v_flt Variable_1; // Previous Generator Value Material.Global X output 0
			Variable_1 = Context.GetWorldX();
			
			// Previous Generator Value Material.Global Y
			v_flt Variable_2; // Previous Generator Value Material.Global Y output 0
			Variable_2 = Context.GetWorldY();
			
			// Previous Generator Value Material.Global Z
			v_flt Variable_3; // Previous Generator Value Material.Global Z output 0
			Variable_3 = Context.GetWorldZ();
			
			// Previous Generator Value Material.Get Previous Generator Value
			v_flt Variable_0; // Previous Generator Value Material.Get Previous Generator Value output 0
			Variable_0 = FVoxelNodeFunctions::GetPreviousGeneratorValue(Variable_1, Variable_2, Variable_3, Context, nullptr);
			
			Outputs.Value = Variable_0;
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
			
			v_flt Variable_2; // RGB to HSV output 0
			v_flt Variable_3; // RGB to HSV output 1
			v_flt Variable_4; // RGB to HSV output 2
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			v_flt Variable_14; // Global X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			FColor Variable_11; // Make Color output 0
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
				// Color
				FColor Variable_1; // Color output 0
				Variable_1 = Params.Color;
				
				// Break Color
				v_flt Variable_5; // Break Color output 0
				v_flt Variable_6; // Break Color output 1
				v_flt Variable_7; // Break Color output 2
				v_flt Break_Color_0_Temp_3; // Break Color output 3
				FVoxelNodeFunctions::BreakColorFloat(Variable_1, Variable_5, Variable_6, Variable_7, Break_Color_0_Temp_3);
				
				// RGB to HSV
				FVoxelNodeFunctions::RGBToHSV(Variable_5, Variable_6, Variable_7, BufferConstant.Variable_2, BufferConstant.Variable_3, BufferConstant.Variable_4);
				
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
		
		FVoxelFastNoise _2D_White_Noise_0_Noise;
		
		///////////////////////////////////////////////////////////////////////
		//////////////////////////// Init functions ///////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_XYZWithoutCache_Init(const FVoxelGeneratorInit& InitStruct)
		{
			// Init of 2D White Noise
			_2D_White_Noise_0_Noise.SetSeed(FVoxelGraphSeed(1337));
			
		}
		
		///////////////////////////////////////////////////////////////////////
		////////////////////////// Compute functions //////////////////////////
		///////////////////////////////////////////////////////////////////////
		
		void Function0_X_Compute(const FVoxelContext& Context, FBufferX& BufferX) const
		{
			// Global X
			BufferX.Variable_14 = Context.GetWorldX();
			
		}
		
		void Function0_XYWithCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Global Y
			v_flt Variable_15; // Global Y output 0
			Variable_15 = Context.GetWorldY();
			
			// 2D White Noise
			v_flt Variable_12; // 2D White Noise output 0
			Variable_12 = _2D_White_Noise_0_Noise.GetWhiteNoise_2D(BufferX.Variable_14, Variable_15);
			
			// Map.-
			v_flt Variable_16; // Map.- output 0
			Variable_16 = Variable_12 - v_flt(-1.0f);
			
			// Map./
			v_flt Variable_17; // Map./ output 0
			Variable_17 = Variable_16 / v_flt(2.0f);
			
			// Map.*
			v_flt Variable_18; // Map.* output 0
			Variable_18 = Variable_17 * v_flt(0.65f);
			
			// Map.+
			v_flt Variable_0; // Map.+ output 0
			Variable_0 = Variable_18 + v_flt(0.35f);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = BufferConstant.Variable_4 * Variable_0;
			
			// HSV to RGB
			v_flt Variable_8; // HSV to RGB output 0
			v_flt Variable_9; // HSV to RGB output 1
			v_flt Variable_10; // HSV to RGB output 2
			FVoxelNodeFunctions::HSVToRGB(BufferConstant.Variable_2, BufferConstant.Variable_3, Variable_13, Variable_8, Variable_9, Variable_10);
			
			// Make Color
			BufferXY.Variable_11 = FVoxelNodeFunctions::MakeColorFloat(Variable_8, Variable_9, Variable_10, v_flt(1.0f));
			
		}
		
		void Function0_XYWithoutCache_Compute(const FVoxelContext& Context, FBufferX& BufferX, FBufferXY& BufferXY) const
		{
			// Global Y
			v_flt Variable_15; // Global Y output 0
			Variable_15 = Context.GetWorldY();
			
			// Global X
			BufferX.Variable_14 = Context.GetWorldX();
			
			// 2D White Noise
			v_flt Variable_12; // 2D White Noise output 0
			Variable_12 = _2D_White_Noise_0_Noise.GetWhiteNoise_2D(BufferX.Variable_14, Variable_15);
			
			// Map.-
			v_flt Variable_16; // Map.- output 0
			Variable_16 = Variable_12 - v_flt(-1.0f);
			
			// Map./
			v_flt Variable_17; // Map./ output 0
			Variable_17 = Variable_16 / v_flt(2.0f);
			
			// Map.*
			v_flt Variable_18; // Map.* output 0
			Variable_18 = Variable_17 * v_flt(0.65f);
			
			// Map.+
			v_flt Variable_0; // Map.+ output 0
			Variable_0 = Variable_18 + v_flt(0.35f);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = BufferConstant.Variable_4 * Variable_0;
			
			// HSV to RGB
			v_flt Variable_8; // HSV to RGB output 0
			v_flt Variable_9; // HSV to RGB output 1
			v_flt Variable_10; // HSV to RGB output 2
			FVoxelNodeFunctions::HSVToRGB(BufferConstant.Variable_2, BufferConstant.Variable_3, Variable_13, Variable_8, Variable_9, Variable_10);
			
			// Make Color
			BufferXY.Variable_11 = FVoxelNodeFunctions::MakeColorFloat(Variable_8, Variable_9, Variable_10, v_flt(1.0f));
			
		}
		
		void Function0_XYZWithCache_Compute(const FVoxelContext& Context, const FBufferX& BufferX, const FBufferXY& BufferXY, FOutputs& Outputs) const
		{
			Outputs.MaterialBuilder.SetColor(BufferXY.Variable_11);
		}
		
		void Function0_XYZWithoutCache_Compute(const FVoxelContext& Context, FOutputs& Outputs) const
		{
			// Global Y
			v_flt Variable_15; // Global Y output 0
			Variable_15 = Context.GetWorldY();
			
			// Global X
			v_flt Variable_14; // Global X output 0
			Variable_14 = Context.GetWorldX();
			
			// 2D White Noise
			v_flt Variable_12; // 2D White Noise output 0
			Variable_12 = _2D_White_Noise_0_Noise.GetWhiteNoise_2D(Variable_14, Variable_15);
			
			// Map.-
			v_flt Variable_16; // Map.- output 0
			Variable_16 = Variable_12 - v_flt(-1.0f);
			
			// Map./
			v_flt Variable_17; // Map./ output 0
			Variable_17 = Variable_16 / v_flt(2.0f);
			
			// Map.*
			v_flt Variable_18; // Map.* output 0
			Variable_18 = Variable_17 * v_flt(0.65f);
			
			// Map.+
			v_flt Variable_0; // Map.+ output 0
			Variable_0 = Variable_18 + v_flt(0.35f);
			
			// *
			v_flt Variable_13; // * output 0
			Variable_13 = BufferConstant.Variable_4 * Variable_0;
			
			// HSV to RGB
			v_flt Variable_8; // HSV to RGB output 0
			v_flt Variable_9; // HSV to RGB output 1
			v_flt Variable_10; // HSV to RGB output 2
			FVoxelNodeFunctions::HSVToRGB(BufferConstant.Variable_2, BufferConstant.Variable_3, Variable_13, Variable_8, Variable_9, Variable_10);
			
			// Make Color
			FColor Variable_11; // Make Color output 0
			Variable_11 = FVoxelNodeFunctions::MakeColorFloat(Variable_8, Variable_9, Variable_10, v_flt(1.0f));
			
			Outputs.MaterialBuilder.SetColor(Variable_11);
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
			
		};
		
		struct FBufferX
		{
			FBufferX() {}
			
			TVoxelRange<v_flt> Variable_1; // Previous Generator Value Material.Global X output 0
		};
		
		struct FBufferXY
		{
			FBufferXY() {}
			
			TVoxelRange<v_flt> Variable_2; // Previous Generator Value Material.Global Y output 0
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
			// Previous Generator Value Material.Global X
			TVoxelRange<v_flt> Variable_1; // Previous Generator Value Material.Global X output 0
			Variable_1 = Context.GetWorldX();
			
			// Previous Generator Value Material.Global Y
			TVoxelRange<v_flt> Variable_2; // Previous Generator Value Material.Global Y output 0
			Variable_2 = Context.GetWorldY();
			
			// Previous Generator Value Material.Global Z
			TVoxelRange<v_flt> Variable_3; // Previous Generator Value Material.Global Z output 0
			Variable_3 = Context.GetWorldZ();
			
			// Previous Generator Value Material.Get Previous Generator Value
			TVoxelRange<v_flt> Variable_0; // Previous Generator Value Material.Get Previous Generator Value output 0
			Variable_0 = FVoxelNodeFunctions::GetPreviousGeneratorValue(Variable_1, Variable_2, Variable_3, Context, nullptr);
			
			Outputs.Value = Variable_0;
		}
		
	};
	
	FVoxelExample_Tool_NoisyColorsInstance(UVoxelExample_Tool_NoisyColors& Object)
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
			Object.Color.ToFColor(false)
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
inline v_flt FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalValue::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalValue::FOutputs::Set<v_flt, 1>(v_flt InValue)
{
	Value = InValue;
}
template<>
inline FVoxelMaterial FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Get<FVoxelMaterial, 2>() const
{
	return MaterialBuilder.Build();
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalMaterial::FOutputs::Set<FVoxelMaterial, 2>(FVoxelMaterial Material)
{
}
template<>
inline v_flt FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 3>() const
{
	return UpVectorX;
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 3>(v_flt InValue)
{
	UpVectorX = InValue;
}
template<>
inline v_flt FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 4>() const
{
	return UpVectorY;
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 4>(v_flt InValue)
{
	UpVectorY = InValue;
}
template<>
inline v_flt FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Get<v_flt, 5>() const
{
	return UpVectorZ;
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalUpVectorXUpVectorYUpVectorZ::FOutputs::Set<v_flt, 5>(v_flt InValue)
{
	UpVectorZ = InValue;
}
template<>
inline TVoxelRange<v_flt> FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Get<v_flt, 1>() const
{
	return Value;
}
template<>
inline void FVoxelExample_Tool_NoisyColorsInstance::FLocalComputeStruct_LocalValueRangeAnalysis::FOutputs::Set<v_flt, 1>(TVoxelRange<v_flt> InValue)
{
	Value = InValue;
}
template<>
inline auto& FVoxelExample_Tool_NoisyColorsInstance::GetTarget<1>() const
{
	return LocalValue;
}
template<>
inline auto& FVoxelExample_Tool_NoisyColorsInstance::GetTarget<2>() const
{
	return LocalMaterial;
}
template<>
inline auto& FVoxelExample_Tool_NoisyColorsInstance::GetRangeTarget<0, 1>() const
{
	return LocalValueRangeAnalysis;
}
template<>
inline auto& FVoxelExample_Tool_NoisyColorsInstance::GetTarget<3, 4, 5>() const
{
	return LocalUpVectorXUpVectorYUpVectorZ;
}
#endif

////////////////////////////////////////////////////////////
////////////////////////// UCLASS //////////////////////////
////////////////////////////////////////////////////////////

UVoxelExample_Tool_NoisyColors::UVoxelExample_Tool_NoisyColors()
{
	bEnableRangeAnalysis = true;
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelExample_Tool_NoisyColors::GetTransformableInstance()
{
#if VOXEL_GRAPH_GENERATED_VERSION == 1
	return MakeVoxelShared<FVoxelExample_Tool_NoisyColorsInstance>(*this);
#else
#if VOXEL_GRAPH_GENERATED_VERSION > 1
	EMIT_CUSTOM_WARNING("Outdated generated voxel graph: VoxelExample_Tool_NoisyColors. You need to regenerate it.");
	FVoxelMessages::Warning("Outdated generated voxel graph: VoxelExample_Tool_NoisyColors. You need to regenerate it.");
#else
	EMIT_CUSTOM_WARNING("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Tool_NoisyColors. You need to update the plugin.");
	FVoxelMessages::Warning("Generated voxel graph is more recent than the Voxel Plugin version: VoxelExample_Tool_NoisyColors. You need to update the plugin.");
#endif
	return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
#endif
}

PRAGMA_GENERATED_VOXEL_GRAPH_END
