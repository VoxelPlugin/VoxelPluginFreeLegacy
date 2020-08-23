// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"

struct FVoxelHardnessHandler;
struct FVoxelSurfaceEditsVoxel;
struct FVoxelSurfaceEditsProcessedVoxels;

/**
 * @dependency VoxelTools/VoxelSurfaceEdits.h
 */
class VOXEL_API FVoxelSurfaceEditToolsImpl : public FVoxelToolsBaseImpl
{
public:
	static FVoxelIntBox GetBounds(const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels);
	static bool ShouldCompute(const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels);
	
public:
	// Bounds need to encompass Bounds(Voxels).Extend(0, 0, MaxStrength + DistanceDivisor + 2)!
	template<typename TData>
	static void EditVoxelValues2D(
		TData& Data,
		const FVoxelHardnessHandler& HardnessHandler,
		const FVoxelIntBox& Bounds,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		float DistanceDivisor);

public:
	template<typename T, typename TLambda, typename TData>
	static void Edit(
		TData& Data,
		const FVoxelIntBox& Bounds,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		TLambda Lambda);
	
	template<typename TData>
	static void EditVoxelValues(
		TData& Data,
		const FVoxelHardnessHandler& HardnessHandler,
		const FVoxelIntBox& Bounds,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
		float DistanceDivisor,
		bool bHasValues);
	
	template<typename TData>
	static void EditVoxelMaterials(
		TData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels);
	
	template<typename TData>
	static void PropagateVoxelMaterials(
		TData& Data,
		const TArray<FVoxelSurfaceEditsVoxel>& Voxels);

public:
	/**
	 * Apply processed voxels strengths to the voxel values
	 * @param	HardnessHandler		Hardness handler @Internal
	 * @param	Bounds				Bounds. See EditVoxelValues2D comment. @Internal
	 * @param	ProcessedVoxels		The processed voxels, usually obtained by applying a surface edit stack @GetBounds @ShouldCompute
	 * @param	DistanceDivisor		Distance divisor: the new value will be divided by this. Useful if normals are bad because of clamped values @Advanced
	 * @see ApplyStack, AddToStack
	 * @ExportSetValue
	 */
	template<typename TData>
	static void EditVoxelValues(
		TData& Data,
		const FVoxelHardnessHandler& HardnessHandler,
		const FVoxelIntBox& Bounds,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
		float DistanceDivisor = 1.f);
	
	/**
	 * Apply paint material to the processed voxels, using the processed strengths
	 * @param	Bounds				Bounds. See EditVoxelValues2D comment. @Internal
	 * @param	PaintMaterial		The paint material to apply
	 * @param	ProcessedVoxels		The processed voxels, usually obtained by applying a surface edit stack @GetBounds @ShouldCompute
	 * @see ApplyStack, AddToStack
	 * @ExportSetMaterial
	 */
	template<typename TData>
	static void EditVoxelMaterials(
		TData& Data,
		const FVoxelIntBox& Bounds,
		const FVoxelPaintMaterial& PaintMaterial,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels);
	
	/**
	 * Propagate the materials of the voxels, so that the new surface is painted correctly
	 * Must be called BEFORE EditVoxelValues!
	 * @param	ProcessedVoxels		The processed voxels, usually obtained by applying a surface edit stack @GetBounds @ShouldCompute
	 * @see ApplyStack, AddToStack
	 * @check	ProcessedVoxels.Info.bHasSurfacePositions	PropagateVoxelMaterials needs surface positions! Use FindSurfaceVoxelsFromDistanceField
	 * @ExportSetMaterial
	 */
	template<typename TData>
	static void PropagateVoxelMaterials(
		TData& Data,
		const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels);
};