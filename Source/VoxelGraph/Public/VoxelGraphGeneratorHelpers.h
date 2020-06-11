// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "VoxelMinimal.h"
#include "VoxelContext.h"
#include "VoxelGraphConstants.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.inl"
#include "VoxelGraphGeneratorHelpers.generated.h"

// See https://godbolt.org/z/4IzS-b
#if defined(_MSC_VER) && !defined(INTELLISENSE_PARSER)
#define MSVC_TEMPLATE
#else
#define MSVC_TEMPLATE template
#endif

struct FVoxelGraphOutputsInit
{
	EVoxelMaterialConfig MaterialConfig;
};

template<typename TChild, typename UWorldObject>
class TVoxelGraphGeneratorInstanceHelper : public TVoxelTransformableWorldGeneratorInstanceHelper<TChild, UWorldObject>
{
public:
	using FVoxelWorldGeneratorInstance::TOutputFunctionPtr;
	using FVoxelWorldGeneratorInstance::TRangeOutputFunctionPtr;
	using FVoxelTransformableWorldGeneratorInstance::TOutputFunctionPtr_Transform;
	using FVoxelTransformableWorldGeneratorInstance::TRangeOutputFunctionPtr_Transform;
	
	using FVoxelWorldGeneratorInstance::FCustomFunctionPtrs;
	using FVoxelTransformableWorldGeneratorInstance::FCustomFunctionPtrs_Transform;

	TVoxelGraphGeneratorInstanceHelper(
		const TMap<FName, uint32>& FloatOutputs,
		const TMap<FName, uint32>& Int32Outputs,
		const TMap<FName, uint32>& ColorOutputs,

		const FCustomFunctionPtrs& CustomFunctionPtrs,
		const FCustomFunctionPtrs_Transform& CustomFunctionPtrs_Transform,
		
		bool bEnableRangeAnalysis)
		: TVoxelTransformableWorldGeneratorInstanceHelper<TChild, UWorldObject>(CustomFunctionPtrs, CustomFunctionPtrs_Transform)
		, bEnableRangeAnalysis(bEnableRangeAnalysis)
		, CustomOutputsNames(FName())
	{
		auto& Array = const_cast<TStaticArray<FName, MAX_VOXELGRAPH_OUTPUTS>&>(CustomOutputsNames);
		for (auto& It : FloatOutputs)
		{
			ensure(Array[It.Value] == FName());
			Array[It.Value] = It.Key;
		}
		for (auto& It : Int32Outputs)
		{
			ensure(Array[It.Value] == FName());
			Array[It.Value] = It.Key;
		}
		for (auto& It : ColorOutputs)
		{
			ensure(Array[It.Value] == FName());
			Array[It.Value] = It.Key;
		}
	}

public:
	template<bool bCustomTransform, typename T, uint32 Index, typename F>
	inline T GetOutput(const FTransform& LocalToWorld, T DefaultValue, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items, F CallNextGenerator) const
	{
		ensure(bInit);
		
		auto&& Target = This().template GetTarget<Index>();
		auto Outputs = Target.GetOutputs();

		Outputs.Init(FVoxelGraphOutputsInit{ MaterialConfig });
		Outputs.template Set<T, Index>(DefaultValue);

		const FVoxelContext Context(LOD, Items, LocalToWorld, bCustomTransform, X, Y, Z);
		Target.ComputeXYZWithoutCache(Context, Outputs);

		return Outputs.template Get<T, Index>();
	}

	template<bool bCustomTransform, typename T, uint32 Index>
	inline TVoxelRange<T> GetOutputRange(const FTransform& LocalToWorld, TVoxelRange<T> DefaultValue, const FVoxelIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		ensure(bInit);
		
		if (!bEnableRangeAnalysis)
		{
			return TVoxelRange<T>::Infinite();
		}

		auto&& Target = This().template GetRangeTarget<FVoxelGraphOutputsIndices::RangeAnalysisIndex, Index>();
		auto Outputs = Target.GetOutputs();
		
		Outputs.Init(FVoxelGraphOutputsInit{ MaterialConfig });
		Outputs.template Set<T, Index>(DefaultValue);

		auto& RangeFailStatus = FVoxelRangeFailStatus::Get();
		
		ensure(!RangeFailStatus.HasFailed());
		RangeFailStatus.Reset();

		const FVoxelContextRange Context(LOD, Items, LocalToWorld, bCustomTransform, WorldBounds);

		Target.ComputeXYZWithoutCache(Context, Outputs);
		
		if (RangeFailStatus.NeedReport())
		{
			RangeFailStatus.ResetNeedReport();
			This().ReportRangeAnalysisFailure();
		}
		
		if (RangeFailStatus.HasFailed())
		{
			RangeFailStatus.Reset();
			return TVoxelRange<T>::Infinite();
		}

		return Outputs.template Get<T, Index>();
	}

	template<bool bCustomTransform, typename T, typename QueryZoneType, uint32 Index, typename F>
	inline void GetOutput(const FTransform& LocalToWorld, T DefaultValue, TVoxelQueryZone<QueryZoneType>& QueryZone, int32 LOD, const FVoxelItemStack& Items, F CallNextGenerator) const
	{
		ensure(bInit);
		
		auto&& Target = This().template GetTarget<Index>();
		FVoxelContext Context(LOD, Items, LocalToWorld, bCustomTransform);
		if (!bCustomTransform || LocalToWorld.GetRotation() == FQuat::Identity)
		{
			// We can only use the dependencies analysis if we don't have a transform, or if it's only translation + scale
			// (and thus not changing the axis)
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
			{
				Context.SetWorldX(X);
				auto BufferX = Target.GetBufferX();
				Target.ComputeX(Context, BufferX);

				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
				{
					Context.SetWorldY(Y);
					auto BufferXY = Target.GetBufferXY();
					Target.ComputeXYWithCache(Context, BufferX, BufferXY);

					for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
					{
						Context.SetWorldZ(Z);

						auto Outputs = Target.GetOutputs();
						Outputs.Init(FVoxelGraphOutputsInit{ MaterialConfig });
						Outputs.template Set<T, Index>(DefaultValue);
						Target.ComputeXYZWithCache(Context, static_cast<const decltype(BufferX)&>(BufferX), static_cast<const decltype(BufferXY)&>(BufferXY), Outputs);
						QueryZone.Set(X, Y, Z, QueryZoneType(Outputs.template Get<T, Index>()));
					}
				}
			}
		}
		else
		{
			// Have to query all the voxels individually
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
			{
				Context.SetWorldX(X);
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
				{
					Context.SetWorldY(Y);
					for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
					{
						Context.SetWorldZ(Z);

						auto Outputs = Target.GetOutputs();
						Outputs.Init(FVoxelGraphOutputsInit{ MaterialConfig });
						Outputs.template Set<T, Index>(DefaultValue);
						Target.ComputeXYZWithoutCache(Context, Outputs);
						QueryZone.Set(X, Y, Z, QueryZoneType(Outputs.template Get<T, Index>()));
					}
				}
			}
		}
	}

	template<bool bCustomTransform, typename T, uint32 Index>
	inline T GetDataImpl(const FTransform& LocalToWorld, T DefaultValue, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetOutput<bCustomTransform, T, Index>(LocalToWorld, DefaultValue, X, Y, Z, LOD, Items, [&](auto& NextStack)
		{
			return NextStack.template Get<T>(X, Y, Z, LOD);
		});
	}

	template<bool bCustomTransform, typename T, typename QueryZoneType, uint32 Index>
	inline void GetData(const FTransform& LocalToWorld, T DefaultValue, TVoxelQueryZone<QueryZoneType>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetOutput<bCustomTransform, T, QueryZoneType, Index>(LocalToWorld, DefaultValue, QueryZone, LOD, Items, [&](auto& NextStack, int32 X, int32 Y, int32 Z)
		{
			return NextStack.template Get<T>(X, Y, Z, LOD);
		});
	}

	template<bool bCustomTransform, typename T, uint32 Index>
	T GetCustomOutputImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		static_assert(Index < MAX_VOXELGRAPH_OUTPUTS, "");
		return GetOutput<bCustomTransform, T, Index>(LocalToWorld, T{}, X, Y, Z, LOD, Items, [&](auto& NextStack)
		{
			return NextStack.MSVC_TEMPLATE GetCustomOutput<T>(T{}, CustomOutputsNames[Index], X, Y, Z, LOD);
		});
	}

	template<bool bCustomTransform, typename T, uint32 Index>
	inline TVoxelRange<T> GetCustomOutputRangeImpl(const FTransform& LocalToWorld, const FVoxelIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		static_assert(Index < MAX_VOXELGRAPH_OUTPUTS, "");
		return GetOutputRange<bCustomTransform, T, Index>(LocalToWorld, T{}, WorldBounds, LOD, Items);
	}
	
	template<typename T, uint32 Index>
	inline T GetCustomOutputWithTransform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetCustomOutputImpl<true, T, Index>(LocalToWorld, X, Y, Z, LOD, Items);
	}
	template<typename T, uint32 Index>
	inline TVoxelRange<T> GetCustomOutputRangeWithTransform(const FTransform& LocalToWorld, const FVoxelIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetCustomOutputRangeImpl<true, T, Index>(LocalToWorld, WorldBounds, LOD, Items);
	}
	template<typename T, uint32 Index>
	inline T GetCustomOutputNoTransform(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetCustomOutputImpl<false, T, Index>(FTransform(), X, Y, Z, LOD, Items);
	}
	template<typename T, uint32 Index>
	inline TVoxelRange<T> GetCustomOutputRangeNoTransform(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetCustomOutputRangeImpl<false, T, Index>(FTransform(), Bounds, LOD, Items);
	}

public:
	//~ Begin FVoxelWorldGeneratorInstance Interface
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) override final
	{
		bInit = true;
		MaterialConfig = InitStruct.MaterialConfig;
		InitGraph(InitStruct);
	}
	
	template<bool bCustomTransform>
	v_flt GetValueImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetDataImpl<bCustomTransform, v_flt, FVoxelGraphOutputsIndices::ValueIndex>(LocalToWorld, 1, X, Y, Z, LOD, Items);
	}
	template<bool bCustomTransform>
	FVoxelMaterial GetMaterialImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetDataImpl<bCustomTransform, FVoxelMaterial, FVoxelGraphOutputsIndices::MaterialIndex>(LocalToWorld, FVoxelMaterial::Default(), X, Y, Z, LOD, Items);
	}
	template<bool bCustomTransform>
	TVoxelRange<v_flt> GetValueRangeImpl(const FTransform& LocalToWorld, const FVoxelIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return GetOutputRange<bCustomTransform, v_flt, FVoxelGraphOutputsIndices::ValueIndex>(LocalToWorld, 1, WorldBounds, LOD, Items);
	}

	virtual void GetValues(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		GetData<false, v_flt, FVoxelValue, FVoxelGraphOutputsIndices::ValueIndex>(FTransform(), 1, QueryZone, LOD, Items);
	}
	virtual void GetMaterials(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		GetData<false, FVoxelMaterial, FVoxelMaterial, FVoxelGraphOutputsIndices::MaterialIndex>(FTransform(), FVoxelMaterial::Default(), QueryZone, LOD, Items);
	}

	virtual void GetValues_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		GetData<true, v_flt, FVoxelValue, FVoxelGraphOutputsIndices::ValueIndex>(LocalToWorld, 1, QueryZone, LOD, Items);
	}
	virtual void GetMaterials_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		GetData<true, FVoxelMaterial, FVoxelMaterial, FVoxelGraphOutputsIndices::MaterialIndex>(LocalToWorld, FVoxelMaterial::Default(), QueryZone, LOD, Items);
	}

	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		auto&& Target = This().template GetTarget<
			FVoxelGraphOutputsIndices::UpVectorXIndex,
			FVoxelGraphOutputsIndices::UpVectorYIndex,
			FVoxelGraphOutputsIndices::UpVectorZIndex>();

		auto Outputs = Target.GetOutputs();
		Outputs.Init(FVoxelGraphOutputsInit{ MaterialConfig });

		Outputs.template Set<v_flt, FVoxelGraphOutputsIndices::UpVectorXIndex>(0);
		Outputs.template Set<v_flt, FVoxelGraphOutputsIndices::UpVectorYIndex>(0);
		Outputs.template Set<v_flt, FVoxelGraphOutputsIndices::UpVectorZIndex>(1);

		const FVoxelContext Context(0, FVoxelItemStack::Empty, FTransform::Identity, false, X, Y, Z);

		Target.ComputeXYZWithoutCache(Context, Outputs);

		return FVector(
			Outputs.template Get<v_flt, FVoxelGraphOutputsIndices::UpVectorXIndex>(),
			Outputs.template Get<v_flt, FVoxelGraphOutputsIndices::UpVectorYIndex>(),
			Outputs.template Get<v_flt, FVoxelGraphOutputsIndices::UpVectorZIndex>()).GetSafeNormal();
	}
	//~ End FVoxelWorldGeneratorInstance Interface

public:
	virtual void InitGraph(const FVoxelWorldGeneratorInit& InitStruct) = 0;

protected:
	template<typename T>
	struct NoTransformAccessor
	{
		template<uint32 Index, typename ReturnType>
		inline static ReturnType Get()
		{
			return static_cast<ReturnType>(&TChild::template GetCustomOutputNoTransform<T, Index>);
		}
	};
	template<typename T>
	struct NoTransformRangeAccessor
	{
		template<uint32 Index, typename ReturnType>
		inline static ReturnType Get()
		{
			return static_cast<ReturnType>(&TChild::template GetCustomOutputRangeNoTransform<T, Index>);
		}
	};
	template<typename T>
	struct WithTransformAccessor
	{
		template<uint32 Index, typename ReturnType>
		inline static ReturnType Get()
		{
			return static_cast<ReturnType>(&TChild::template GetCustomOutputWithTransform<T, Index>);
		}
	};
	template<typename T>
	struct WithTransformRangeAccessor
	{
		template<uint32 Index, typename ReturnType>
		inline static ReturnType Get()
		{
			return static_cast<ReturnType>(&TChild::template GetCustomOutputRangeWithTransform<T, Index>);
		}
	};

private:
	const bool bEnableRangeAnalysis;
	// Used to forward the custom output calls to the generator in the stack
	const TStaticArray<FName, MAX_VOXELGRAPH_OUTPUTS> CustomOutputsNames;

	bool bInit = false;
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig(-1);

	inline const TChild& This() const
	{
		return static_cast<const TChild&>(*this);
	}
	inline TChild& This()
	{
		return static_cast<TChild&>(*this);
	}
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelGraphGeneratorHelper : public UVoxelTransformableWorldGenerator
{
	GENERATED_BODY()

public:
	// Range analysis gives a pretty significant speed-up. You should not disable it
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Misc")
	bool bEnableRangeAnalysis = true;
	
	//~ Begin UVoxelTransformableWorldGenerator Interface
	void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const override
	{
	}
	TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const override
	{
		return GetClass()->GetDefaultObject<UVoxelGraphGeneratorHelper>()->GetTransformableInstance();
	}
	//~ End UVoxelTransformableWorldGenerator Interface
};