// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"

class VOXEL_API FVoxelBoxToolsImpl : public FVoxelToolsBaseImpl
{
public:
	template<bool bAdd, typename TData>
	static void BoxEdit(
		TData& Data, 
		const FVoxelIntBox& Bounds);
	
public:
	/**
	 * Set the density in a box
	 * @param	Bounds		The bounds of the box
	 * @param	Value		The density to set @VoxelValue
	 * @ExportSetValue
	 */
	template<typename TData>
	static void SetValueBox(
		TData& Data,
		const FVoxelIntBox& Bounds, 
		FVoxelValue Value);

	/**
	 * Add a box shape
	 * @param	Bounds		The bounds of the box
	 * @ExportSetValue
	 */
	template<typename TData>
	static void AddBox(
		TData& Data,
		const FVoxelIntBox& Bounds)
	{
		BoxEdit<true>(Data, Bounds);
	}

	/**
	 * Remove a box shape
	 * @param	Bounds		The bounds of the box
	 * @ExportSetValue
	 */
	template<typename TData>
	static void RemoveBox(
		TData& Data,
		const FVoxelIntBox& Bounds)
	{
		BoxEdit<false>(Data, Bounds);
	}

	/**
	 * Paint a box shape
	 * @param	Bounds		The bounds of the box
	 * @param	PaintMaterial	The material to set
	 * @ExportSetMaterial
	 */
	template<typename TData>
	static void SetMaterialBox(
		TData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial);
};