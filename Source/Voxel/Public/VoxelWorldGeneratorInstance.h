// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelRange.h"
#include "IntBox.h"
#include "VoxelQueryZone.h"
#include "VoxelItemStack.h"
#include "VoxelWorldGeneratorInit.h"
#include "Templates/SubclassOf.h"
#include "VoxelWorldGenerator.h"

class UMaterialInstanceDynamic;

/**
 * A FVoxelWorldGeneratorInstance is a constant object created by a UVoxelWorldGenerator
 */
class VOXEL_API FVoxelWorldGeneratorInstance : public TVoxelSharedFromThis<FVoxelWorldGeneratorInstance>
{
public:
	template<typename T>
	using TOutputFunctionPtr = T(FVoxelWorldGeneratorInstance::*)(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	
	template<typename T>
	using TRangeOutputFunctionPtr = TVoxelRange<T>(FVoxelWorldGeneratorInstance::*)(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;
	
	struct FBaseFunctionPtrs
	{
		TOutputFunctionPtr<v_flt> Value;
		TOutputFunctionPtr<FVoxelMaterial> Material;
		TRangeOutputFunctionPtr<v_flt> ValueRange;
	};
	struct FCustomFunctionPtrs
	{
		TMap<FName, TOutputFunctionPtr<v_flt>> Float;
		TMap<FName, TOutputFunctionPtr<int32>> Int;
		TMap<FName, TOutputFunctionPtr<FColor>> Color;
		
		TMap<FName, TRangeOutputFunctionPtr<v_flt>> FloatRange;
	};

public:
	FVoxelWorldGeneratorInstance(
		TSubclassOf<UVoxelWorldGenerator> Class,
		const FBaseFunctionPtrs& BasePtrs,
		const FCustomFunctionPtrs& CustomPtrs)
		: Class(Class)
		, BasePtrs(BasePtrs)
		, CustomPtrs(CustomPtrs)
	{
		check(Class);
		check(BasePtrs.Value);
		check(BasePtrs.Material);
		check(BasePtrs.ValueRange);
	}
	virtual ~FVoxelWorldGeneratorInstance() = default;

public:
	const TSubclassOf<UVoxelWorldGenerator> Class;
	const FBaseFunctionPtrs BasePtrs;
	const FCustomFunctionPtrs CustomPtrs;

	template<typename T>
	const TMap<FName, TOutputFunctionPtr<T>>& GetOutputsPtrMap() const;
	template<typename T>
	const TMap<FName, TRangeOutputFunctionPtr<T>>& GetOutputsRangesPtrMap() const;

public:
	//~ Begin FVoxelWorldGeneratorInstance Interface
	// Initialization
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) {}

	// Called before a chunk is computed
	// Needs to be thread safe!
	virtual void InitArea(const FIntBox& Bounds, int32 LOD) {}

	// Will be called on every chunk material instance
	// Can be used eg to pass a texture per chunk
	virtual void SetupMaterialInstance(int32 ChunkLOD, const FIntBox& ChunkBounds, UMaterialInstanceDynamic* Instance) {}
	
	// You should implement a fast version of that function, called every time a chunk is updated
	virtual void GetValues   (TVoxelQueryZone<FVoxelValue   >& QueryZone, int32 LOD, const FVoxelItemStack& Items) const = 0;
	// This function is only called when a chunk material is edited for the first time. Fine to leave as default
	virtual void GetMaterials(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const = 0;

	// World up vector at position (must be normalized). Used for spawners
	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const = 0;
	//~ End FVoxelWorldGeneratorInstance Interface
	
public:
	v_flt GetValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterial(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	TVoxelRange<v_flt> GetValueRange(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;

	template<typename T>
	T Get(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	void Get(TVoxelQueryZone<T>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	T Get(const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const;

	template<typename TVector>
	FVector GetUpVector(const TVector& P) const;

	template<typename T>
	T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T, typename U>
	T GetCustomOutput(T DefaultValue, FName Name, const U& P, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	TVoxelRange<T> GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;
};

class VOXEL_API FVoxelTransformableWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	template<typename T>
	using TOutputFunctionPtr_Transform = T(FVoxelTransformableWorldGeneratorInstance::*)(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	
	template<typename T>
	using TRangeOutputFunctionPtr_Transform = TVoxelRange<T>(FVoxelTransformableWorldGeneratorInstance::*)(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const;
	
	struct FBaseFunctionPtrs_Transform
	{
		TOutputFunctionPtr_Transform<v_flt> Value;
		TOutputFunctionPtr_Transform<FVoxelMaterial> Material;
		TRangeOutputFunctionPtr_Transform<v_flt> ValueRange;
	};
	struct FCustomFunctionPtrs_Transform
	{
		TMap<FName, TOutputFunctionPtr_Transform<v_flt>> Float;
		TMap<FName, TOutputFunctionPtr_Transform<int32>> Int;
		TMap<FName, TOutputFunctionPtr_Transform<FColor>> Color;
		
		TMap<FName, TRangeOutputFunctionPtr_Transform<v_flt>> FloatRange;
	};
	
public:
	FVoxelTransformableWorldGeneratorInstance(
		TSubclassOf<UVoxelWorldGenerator> Class,
		const FBaseFunctionPtrs& BasePtrs,
		const FCustomFunctionPtrs& CustomPtrs,
		const FBaseFunctionPtrs_Transform& BasePtrs_Transform,
		const FCustomFunctionPtrs_Transform& CustomPtrs_Transform)
		: FVoxelWorldGeneratorInstance(Class, BasePtrs, CustomPtrs)
		, BasePtrs_Transform(BasePtrs_Transform)
		, CustomPtrs_Transform(CustomPtrs_Transform)
	{
		check(BasePtrs_Transform.Value);
		check(BasePtrs_Transform.Material);
		check(BasePtrs_Transform.ValueRange);
	}
	
public:
	const FBaseFunctionPtrs_Transform BasePtrs_Transform;
	const FCustomFunctionPtrs_Transform CustomPtrs_Transform;
	
	template<typename T>
	const TMap<FName, TOutputFunctionPtr_Transform<T>>& GetOutputsPtrMap_Transform() const;
	template<typename T>
	const TMap<FName, TRangeOutputFunctionPtr_Transform<T>>& GetOutputsRangesPtrMap_Transform() const;

	//~ Begin FVoxelWorldGeneratorWithTransformInstance Interface
	virtual void GetValues_Transform   (const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelValue   >& QueryZone, int32 LOD, const FVoxelItemStack& Items) const = 0;
	virtual void GetMaterials_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const = 0;
	//~ End FVoxelWorldGeneratorWithTransformInstance Interface
	
public:
	v_flt GetValue_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	FVoxelMaterial GetMaterial_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	TVoxelRange<v_flt> GetValueRange_Transform(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const;

	template<typename T>
	T Get_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	void Get_Transform(const FTransform& LocalToWorld, TVoxelQueryZone<T>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	T Get_Transform(const FTransform& LocalToWorld, const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const;

	template<typename T>
	T GetCustomOutput_Transform(const FTransform& LocalToWorld, T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const;
	template<typename T>
	TVoxelRange<T> GetCustomOutputRange_Transform(const FTransform& LocalToWorld, TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const;
};