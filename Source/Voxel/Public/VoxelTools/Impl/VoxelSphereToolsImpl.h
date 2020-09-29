// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities/VoxelLambdaUtilities.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"

class VOXEL_API FVoxelSphereToolsImpl : public FVoxelToolsBaseImpl
{
public:
	static FVoxelIntBox GetBounds(const FVoxelVector& Position, float Radius);
	
public:
	template<bool bAdd, typename TData>
	static void SphereEdit(
		TData& Data, 
		const FVoxelVector& Position, 
		float Radius);

	template<typename TData, typename T>
	static void SetMaterialSphereImpl(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		T GetStrength = FVoxelLambdaUtilities::ConstantStrength);

public:
	template<typename T, typename TInterpolator, typename TGetInterpolator>
	static void ApplyKernelSphereImpl_GetData(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		TInterpolator* RESTRICT & SrcBuffer,
		bool bForceSingleThread,
		TGetInterpolator GetInterpolator);
	
	template<typename TInterpolator>
	static TInterpolator ApplyKernelSphereImpl_GetNeighborsValue(
		TInterpolator* RESTRICT Buffer,
		float FirstDegreeNeighborMultiplier,
		float SecondDegreeNeighborMultiplier,
		float ThirdDegreeNeighborMultiplier,
		const FIntVector& Size,
		int32 X, int32 Y, int32 Z);

	template<typename TInterpolator, typename TGetStrength>
	static void ApplyKernelSphereImpl_Iterate(
		TInterpolator* RESTRICT & SrcBuffer,
		TInterpolator* RESTRICT & DstBuffer,
		bool bForceSingleThread,
		const FVoxelVector& LocalPosition,
		const FIntVector& Size,
		float SquaredRadius,
		float CenterMultiplier,
		float FirstDegreeNeighborMultiplier,
		float SecondDegreeNeighborMultiplier,
		float ThirdDegreeNeighborMultiplier,
		int32 NumIterations,
		TGetStrength GetStrength);

	template<typename T, typename TInterpolator, typename TData, typename TGetInterpolator, typename TSetInterpolator, typename TGetStrength>
	static void ApplyKernelSphereImpl(
		TData& Data,
		TGetInterpolator GetInterpolator,
		TSetInterpolator SetInterpolator,
		const FVoxelVector& Position,
		float Radius,
		float CenterMultiplier,
		float FirstDegreeNeighborMultiplier,
		float SecondDegreeNeighborMultiplier,
		float ThirdDegreeNeighborMultiplier,
		int32 NumIterations,
		TGetStrength GetStrength);

	template<int32 NumChannels, typename TData, typename T>
	static void ApplyMaterialKernelSphereImpl(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float CenterMultiplier,
		float FirstDegreeNeighborMultiplier,
		float SecondDegreeNeighborMultiplier,
		float ThirdDegreeNeighborMultiplier,
		int32 NumIterations,
		uint32 Mask,
		T GetStrength);
	
public:
	/**
	 * Set the density in a sphere
	 * @param	Position	The position of the center @VoxelPosition @GetBounds
	 * @param	Radius		The radius @VoxelDistance @GetBounds
	 * @param	Value		The density to set @VoxelValue
	 * @ExportSetValue
	 */
	template<typename TData>
	static void SetValueSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		FVoxelValue Value);
	
	/**
	 * Remove a sphere
	 * @param	Position	The position of the center @VoxelPosition @GetBounds
	 * @param	Radius		The radius @VoxelDistance @GetBounds
	 * @ExportSetValue
	 */
	template<typename TData>
	static void RemoveSphere(
		TData& Data,
		const FVoxelVector& Position, 
		float Radius)
	{
		SphereEdit<false>(Data, Position, Radius);
	}
	
	/**
	 * Add a sphere
	 * @param	Position	The position of the center @VoxelPosition @GetBounds
	 * @param	Radius		The radius @VoxelDistance @GetBounds
	 * @ExportSetValue
	 */
	template<typename TData>
	static void AddSphere(
		TData& Data,
		const FVoxelVector& Position, 
		float Radius)
	{
		SphereEdit<true>(Data, Position, Radius);
	}
	
	/**
	 * Paint material in a sphere shape
	 * @param	Position		The position of the center @VoxelPosition @GetBounds
	 * @param	Radius			The radius @VoxelDistance @GetBounds
	 * @param	PaintMaterial	The material to paint
	 * @param	Strength		The strength of the painting (preferably between 0 and 1)
	 * @param	FalloffType		The type of falloff
	 * @param	Falloff			The falloff, between 0 and 1. Set to 0 to disable.
	 * @ExportSetMaterial
	 */
	template<typename TData>
	static void SetMaterialSphere(
		TData& Data,
		const FVoxelVector& Position, 
		float Radius, 
		const FVoxelPaintMaterial& PaintMaterial,
		float Strength = 1.f,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f);
	
	/**
	 * Apply a 3x3x3 kernel
	 * @param	Position						The position of the center @VoxelPosition @GetBounds
	 * @param	Radius							The radius @VoxelDistance @GetBounds
	 * @param	CenterMultiplier				Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations					The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	GetStrength						Use this to apply a falloff. @UseDefault @Internal
	 * @ExportSetValue
	 */
	template<typename TData, typename T>
	static void ApplyKernelSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		T GetStrength = FVoxelLambdaUtilities::ConstantStrength);
	
	/**
	 * Apply a 3x3x3 kernel to the materials
	 * @param	Position						The position of the center @VoxelPosition @GetBounds
	 * @param	Radius							The radius @VoxelDistance @GetBounds
	 * @param	CenterMultiplier				Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param	SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param	ThirdDegreeNeighborMultiplier	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	NumIterations					The number of times the kernel is going to be applied. Increase to make the effect faster, but more expensive.
	 * @param	Mask							The material channels to affect @BitMask=EVoxelMaterialMask_BP @UFunctionDefault=4095
	 * @param	GetStrength						Use this to apply a falloff. @UseDefault @Internal
	 * @ExportSetMaterial
	 */
	template<typename TData, typename T>
	static void ApplyMaterialKernelSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		T GetStrength = FVoxelLambdaUtilities::ConstantStrength);
	
	/**
	 * Smooth a sphere
	 * @param	Position		The position of the center @VoxelPosition @GetBounds
	 * @param	Radius			The radius @VoxelDistance @GetBounds
	 * @param	Strength		The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	FalloffType		The type of falloff
	 * @param	Falloff			The falloff, between 0 and 1. Set to 0 to disable.
	 * @ExportSetValue
	 */
	template<typename TData>
	static void SmoothSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f);
	
	/**
	 * Smooth materials in a sphere
	 * @param	Position		The position of the center @VoxelPosition @GetBounds
	 * @param	Radius			The radius @VoxelDistance @GetBounds
	 * @param	Strength		The strength of the smoothing (preferably between 0 and 1)
	 * @param	NumIterations	The number of times the smooth is going to be applied. Increase to make smoothing faster, but more expensive.
	 * @param	Mask			The material channels to affect @BitMask=EVoxelMaterialMask_BP @UFunctionDefault=4095
	 * @param	FalloffType		The type of falloff
	 * @param	Falloff			The falloff, between 0 and 1. Set to 0 to disable.
	 * @ExportSetMaterial
	 */
	template<typename TData>
	static void SmoothMaterialSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Strength,
		int32 NumIterations = 1,
		uint32 Mask = EVoxelMaterialMask::All,
		EVoxelFalloff FalloffType = EVoxelFalloff::Linear,
		float Falloff = 0.5f);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * 
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * @param	Position				The position of the center @VoxelPosition @GetBounds
	 * @param	Normal					The normal
	 * @param	Radius					The radius @VoxelDistance @GetBounds
	 * @param	Falloff					The falloff, between 0 and 1
	 * @param	bAdditive				Whether to add or remove voxels
	 * @ImplNote Radius + Falloff must be locked
	 * @ExportSetValue
	 */
	template<typename TData>
	static void TrimSphere(
		TData& Data,
		const FVoxelVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @param	Position				The position of the center @VoxelPosition @GetBounds
	 * @param	Radius					The radius @VoxelDistance @GetBounds
	 * @param	HistoryPosition			The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @ExportSetValue
	 */
	template<typename TData>
	static void RevertSphere(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * NOTE: Does not work with bMultiThreaded and does not fill ModifiedValues!
	 * @param	Position				The position of the center @VoxelPosition @GetBounds
	 * @param	Radius					The radius @VoxelDistance @GetBounds
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @ExportSetValue
	 */
	template<typename TData>
	static void RevertSphereToGenerator(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials);
};