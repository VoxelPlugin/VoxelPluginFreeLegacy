// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h" // Not needed, but that way only need to include this file when declaring a world generator
#include "VoxelWorldGeneratorInstance.h"

/**
 * Inherit from TVoxelWorldGeneratorInstanceHelper<FYourInstance, UYourClass>, and implement:
 *
 * For the values:
 * inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
 * 
 * For the materials:
 * inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
 *
 * And:
 * TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
 * {
 *     return { Min, Max }; // Replace this by the possible values in Bounds
 * }
 */
template<
	typename TWorldInstance, 
	typename UWorldObject, 
	typename TParent = FVoxelWorldGeneratorInstance>
class TVoxelWorldGeneratorInstanceHelper : public TParent
{
public:
	GENERATE_MEMBER_FUNCTION_CHECK(GetValueImpl, v_flt, const, v_flt, v_flt, v_flt, int32, const FVoxelItemStack&);
	GENERATE_MEMBER_FUNCTION_CHECK(GetMaterialImpl, FVoxelMaterial, const, v_flt, v_flt, v_flt, int32, const FVoxelItemStack&);
	GENERATE_MEMBER_FUNCTION_CHECK(GetValueRangeImpl, TVoxelRange<v_flt>, const, const FIntBox&, int32, const FVoxelItemStack&);

	using FVoxelWorldGeneratorInstance::TOutputFunctionPtr;
	using FVoxelWorldGeneratorInstance::TRangeOutputFunctionPtr;

	using FVoxelWorldGeneratorInstance::FBaseFunctionPtrs;
	using FVoxelWorldGeneratorInstance::FCustomFunctionPtrs;

	using UStaticClass = UWorldObject; 

	explicit TVoxelWorldGeneratorInstanceHelper(const FCustomFunctionPtrs& CustomFunctionPtrs = {})
		: TParent(
			UWorldObject::StaticClass(),
			FBaseFunctionPtrs
			{
			   static_cast<TOutputFunctionPtr<v_flt>>(&TWorldInstance::GetValueImpl),
			   static_cast<TOutputFunctionPtr<FVoxelMaterial>>(&TWorldInstance::GetMaterialImpl),
			   static_cast<TRangeOutputFunctionPtr<v_flt>>(&TWorldInstance::GetValueRangeImpl),
			},
			CustomFunctionPtrs)
	{
		// doesn't work with fwd decl static_assert(TIsDerivedFrom<UWorldObject, UVoxelWorldGenerator>::IsDerived, "UWorldObject needs to inherit from UVoxelWorldGenerator");
		static_assert(THasMemberFunction_GetValueImpl   <TWorldInstance>::Value, "Missing 'v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const'");
		static_assert(THasMemberFunction_GetMaterialImpl<TWorldInstance>::Value, "Missing 'FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const'");
		static_assert(THasMemberFunction_GetValueRangeImpl<TWorldInstance>::Value, "Missing 'TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const'");
	}

	virtual void GetValues(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, FVoxelValue(This().GetValueImpl(X, Y, Z, LOD, Items)));
				}
			}
		}
	}
	virtual void GetMaterials(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, This().GetMaterialImpl(X, Y, Z, LOD, Items));
				}
			}
		}
	}
	
private:
	inline const TWorldInstance& This() const
	{
		return static_cast<const TWorldInstance&>(*this);
	}
};

/**
 * If you want your world generator to be placeable as an asset,
 * inherit from TVoxelTransformableWorldGeneratorInstanceHelper<FYourInstance, UYourClass>, and implement:
 *
 * For the values:
 * template<bool bCustomTransform> // bCustomTransform is false if LocalToWorld is identity
 * inline v_flt GetValueImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
 * 
 * For the materials:
 * template<bool bCustomTransform>
 * inline FVoxelMaterial GetMaterialImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
 * 
 * And:
 * template<bool bCustomTransform>
 * TVoxelRange<v_flt> GetValueRangeImpl(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
 * {
 *     return { Min, Max }; // Replace this by the possible values in Bounds
 * }
 */
template<
	typename TWorldInstance,
	typename UWorldObject,
	typename TParent = FVoxelTransformableWorldGeneratorInstance>
class TVoxelTransformableWorldGeneratorInstanceHelper : public TParent
{
public:
	// doesn't work with fwd decl static_assert(TIsDerivedFrom<UWorldObject, UVoxelTransformableWorldGenerator>::IsDerived, "UWorldObject needs to inherit from UVoxelTransformableWorldGenerator");
	
	using FVoxelWorldGeneratorInstance::TOutputFunctionPtr;
	using FVoxelWorldGeneratorInstance::TRangeOutputFunctionPtr;
	
	using FVoxelTransformableWorldGeneratorInstance::TOutputFunctionPtr_Transform;
	using FVoxelTransformableWorldGeneratorInstance::TRangeOutputFunctionPtr_Transform;
	
	using FVoxelWorldGeneratorInstance::FBaseFunctionPtrs;
	using FVoxelWorldGeneratorInstance::FCustomFunctionPtrs;
	
	using FVoxelTransformableWorldGeneratorInstance::FBaseFunctionPtrs_Transform;
	using FVoxelTransformableWorldGeneratorInstance::FCustomFunctionPtrs_Transform;

	using UStaticClass = UWorldObject;

	explicit TVoxelTransformableWorldGeneratorInstanceHelper(
		const FCustomFunctionPtrs& CustomFunctionPtrs = {},
		const FCustomFunctionPtrs_Transform& CustomFunctionPtrs_Transform = {})
		: TParent(
			UWorldObject::StaticClass(),
			FBaseFunctionPtrs
			{
				static_cast<TOutputFunctionPtr<v_flt>>(&TWorldInstance::GetValueNoTransformImpl),
				static_cast<TOutputFunctionPtr<FVoxelMaterial>>(&TWorldInstance::GetMaterialNoTransformImpl),
				static_cast<TRangeOutputFunctionPtr<v_flt>>(&TWorldInstance::GetValueRangeNoTransformImpl),
			},
			CustomFunctionPtrs,
			FBaseFunctionPtrs_Transform
			{
				static_cast<TOutputFunctionPtr_Transform<v_flt>>(&TWorldInstance::GetValueWithTransformImpl),
				static_cast<TOutputFunctionPtr_Transform<FVoxelMaterial>>(&TWorldInstance::GetMaterialWithTransformImpl),
				static_cast<TRangeOutputFunctionPtr_Transform<v_flt>>(&TWorldInstance::GetValueRangeWithTransformImpl),
			},
			CustomFunctionPtrs_Transform)
	{
	}

	virtual void GetValues(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, FVoxelValue(This().GetValueNoTransformImpl(X, Y, Z, LOD, Items)));
				}
			}
		}
	}
	virtual void GetMaterials(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, This().GetMaterialNoTransformImpl(X, Y, Z, LOD, Items));
				}
			}
		}
	}
	
	virtual void GetValues_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, FVoxelValue(This().GetValueWithTransformImpl(LocalToWorld, X, Y, Z, LOD, Items)));
				}
			}
		}
	}
	virtual void GetMaterials_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					QueryZone.Set(X, Y, Z, This().GetMaterialWithTransformImpl(LocalToWorld, X, Y, Z, LOD, Items));
				}
			}
		}
	}

	v_flt GetValueNoTransformImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetValueImpl<false>(FTransform(), X, Y, Z, LOD, Items);
	}
	v_flt GetValueWithTransformImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetValueImpl<true>(LocalToWorld, X, Y, Z, LOD, Items);
	}

	FVoxelMaterial GetMaterialNoTransformImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetMaterialImpl<false>(FTransform(), X, Y, Z, LOD, Items);
	}
	FVoxelMaterial GetMaterialWithTransformImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetMaterialImpl<true>(LocalToWorld, X, Y, Z, LOD, Items);
	}
	
	TVoxelRange<v_flt> GetValueRangeNoTransformImpl(const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetValueRangeImpl<false>(FTransform(), WorldBounds, LOD, Items);
	}
	TVoxelRange<v_flt> GetValueRangeWithTransformImpl(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return This().template GetValueRangeImpl<true>(LocalToWorld, WorldBounds, LOD, Items);
	}
	
private:
	inline const TWorldInstance& This() const
	{
		return static_cast<const TWorldInstance&>(*this);
	}
};