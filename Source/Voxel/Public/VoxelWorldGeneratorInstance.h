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

public:
	FVoxelWorldGeneratorInstance(
		TSubclassOf<UVoxelWorldGenerator> Class,
		
		TOutputFunctionPtr<v_flt> GetValuePtr,
		TOutputFunctionPtr<FVoxelMaterial> GetMaterialPtr,
		TRangeOutputFunctionPtr<v_flt> GetValueRangePtr,

		const TMap<FName, TOutputFunctionPtr<v_flt>>& FloatOutputsPtr,
		const TMap<FName, TOutputFunctionPtr<int32>>& Int32OutputsPtr,
		const TMap<FName, TRangeOutputFunctionPtr<v_flt>>& FloatOutputsRangesPtr)
		: Class(Class)

		, GetValuePtr(GetValuePtr)
		, GetMaterialPtr(GetMaterialPtr)
		, GetValueRangePtr(GetValueRangePtr)

		, FloatOutputsPtr(FloatOutputsPtr)
		, Int32OutputsPtr(Int32OutputsPtr)
		, FloatOutputsRangesPtr(FloatOutputsRangesPtr)
	{
		check(Class);
		check(GetValuePtr);
		check(GetMaterialPtr);
		check(GetValueRangePtr);
	}
	virtual ~FVoxelWorldGeneratorInstance() = default;

public:
	const TSubclassOf<UVoxelWorldGenerator> Class;

	const TOutputFunctionPtr<v_flt> GetValuePtr;
	const TOutputFunctionPtr<FVoxelMaterial> GetMaterialPtr;
	const TRangeOutputFunctionPtr<v_flt> GetValueRangePtr;
	
	const TMap<FName, TOutputFunctionPtr<v_flt>> FloatOutputsPtr;
	const TMap<FName, TOutputFunctionPtr<int32>> Int32OutputsPtr;
	const TMap<FName, TRangeOutputFunctionPtr<v_flt>> FloatOutputsRangesPtr;

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

public:
	FVoxelTransformableWorldGeneratorInstance(
		TSubclassOf<UVoxelWorldGenerator> Class,
		
		TOutputFunctionPtr<v_flt> GetValuePtr,
		TOutputFunctionPtr<FVoxelMaterial> GetMaterialPtr,
		TRangeOutputFunctionPtr<v_flt> GetValueRangePtr,

		const TMap<FName, TOutputFunctionPtr<v_flt>>& FloatOutputsPtr,
		const TMap<FName, TOutputFunctionPtr<int32>>& Int32OutputsPtr,
		const TMap<FName, TRangeOutputFunctionPtr<v_flt>>& FloatOutputsRangesPtr,
		
		TOutputFunctionPtr_Transform<v_flt> GetValuePtr_Transform,
		TOutputFunctionPtr_Transform<FVoxelMaterial> GetMaterialPtr_Transform,
		TRangeOutputFunctionPtr_Transform<v_flt> GetValueRangePtr_Transform,
		
		const TMap<FName, TOutputFunctionPtr_Transform<v_flt>>& FloatOutputsPtr_Transform,
		const TMap<FName, TOutputFunctionPtr_Transform<int32>>& Int32OutputsPtr_Transform,
		const TMap<FName, TRangeOutputFunctionPtr_Transform<v_flt>>& FloatOutputsRangesPtr_Transform)

		: FVoxelWorldGeneratorInstance(Class, GetValuePtr, GetMaterialPtr, GetValueRangePtr, FloatOutputsPtr, Int32OutputsPtr, FloatOutputsRangesPtr)

		, GetValuePtr_Transform(GetValuePtr_Transform)
		, GetMaterialPtr_Transform(GetMaterialPtr_Transform)
		, GetValueRangePtr_Transform(GetValueRangePtr_Transform)

		, FloatOutputsPtr_Transform(FloatOutputsPtr_Transform)
		, Int32OutputsPtr_Transform(Int32OutputsPtr_Transform)
		, FloatOutputsRangesPtr_Transform(FloatOutputsRangesPtr_Transform)
	{
		check(GetValuePtr_Transform);
		check(GetMaterialPtr_Transform);
		check(GetValueRangePtr_Transform);
	}
	
public:
	const TOutputFunctionPtr_Transform<v_flt> GetValuePtr_Transform;
	const TOutputFunctionPtr_Transform<FVoxelMaterial> GetMaterialPtr_Transform;
	const TRangeOutputFunctionPtr_Transform<v_flt> GetValueRangePtr_Transform;
	
	const TMap<FName, TOutputFunctionPtr_Transform<v_flt>> FloatOutputsPtr_Transform;
	const TMap<FName, TOutputFunctionPtr_Transform<int32>> Int32OutputsPtr_Transform;
	const TMap<FName, TRangeOutputFunctionPtr_Transform<v_flt>> FloatOutputsRangesPtr_Transform;
	
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