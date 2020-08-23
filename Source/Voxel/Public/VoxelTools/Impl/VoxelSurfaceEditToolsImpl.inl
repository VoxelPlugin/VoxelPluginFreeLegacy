// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"
#include "VoxelTools/VoxelSurfaceEdits.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

#define VOXEL_SURFACE_TOOL_IMPL() VOXEL_TOOL_FUNCTION_COUNTER(Voxels.Num());

inline FVoxelIntBox FVoxelSurfaceEditToolsImpl::GetBounds(const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels)
{
	return ProcessedVoxels.Bounds;
}

inline bool FVoxelSurfaceEditToolsImpl::ShouldCompute(const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels)
{
	return ProcessedVoxels.Voxels->Num() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename TData>
void FVoxelSurfaceEditToolsImpl::EditVoxelValues2D(
	TData& Data, 
	const FVoxelHardnessHandler& HardnessHandler, 
	const FVoxelIntBox& Bounds, 
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels, 
	float DistanceDivisor)
{
	VOXEL_SURFACE_TOOL_IMPL();
	
	auto* ModifiedValues = GetModifiedValues(Data);
	
	FVoxelMutableDataAccelerator Accelerator(GetActualData(Data), Bounds);

	for (auto& Voxel : Voxels)
	{
		const FVoxelValue Value = Accelerator.GetValue(Voxel.Position, 0);
		if (!ensureVoxelSlow(!Value.IsEmpty())) continue; // Shouldn't be empty if it's a 2D edit position
		const FVoxelValue ValueAbove = Accelerator.GetValue(Voxel.Position + FIntVector(0, 0, 1), 0);
		if (!ensureVoxelSlow(ValueAbove.IsEmpty())) continue;

		const float VoxelHeight = FVoxelUtilities::GetAbsDistanceFromDensities(Value.ToFloat(), ValueAbove.ToFloat());
		
		float Strength = Voxel.Strength;
		if (HardnessHandler.NeedsToCompute())
		{
			Strength /= HardnessHandler.GetHardness(Accelerator.GetMaterial(Voxel.Position, 0));
		}

		// -: we want a negative strength to add
		const float Height = Voxel.Position.Z + VoxelHeight - Strength;

		const bool bIsAdding = Strength < 0;

		const int32 A = FMath::FloorToInt(Voxel.Position.Z - DistanceDivisor);
		const int32 B = FMath::CeilToInt(Voxel.Position.Z + DistanceDivisor);
		const int32 C = FMath::FloorToInt(Height - DistanceDivisor);
		const int32 D = FMath::CeilToInt(Height + DistanceDivisor);
		const int32 Start = FMath::Min(FMath::Min(A, B), FMath::Min(C, D));
		const int32 End = FMath::Max(FMath::Max(A, B), FMath::Max(C, D));

		for (int32 Z = Start; Z <= End; Z++)
		{
			Accelerator.EditValue(Voxel.Position.X, Voxel.Position.Y, Z, [&](FVoxelValue& CurrentValue)
			{
				const float FloatValue = CurrentValue.ToFloat();
				const float WantedValue = (Z - Height) / DistanceDivisor;

				if ((bIsAdding && WantedValue < FloatValue) ||
					(!bIsAdding && WantedValue > FloatValue))
				{
					const FVoxelValue OldValue = CurrentValue;
					CurrentValue = FVoxelValue(WantedValue);
					
					if (ModifiedValues)
					{
						ModifiedValues->Add({ Voxel.Position, OldValue, CurrentValue });
					}
				}
			});
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename TLambda, typename TData>
void FVoxelSurfaceEditToolsImpl::Edit(TData& Data, const FVoxelIntBox& Bounds, const TArray<FVoxelSurfaceEditsVoxel>& Voxels, TLambda Lambda)
{
	auto* ModifiedValues = GetModifiedValues(Data);
	if (ModifiedValues)
	{
		ModifiedValues->SetNumUninitialized(Voxels.Num());
	}

	TArray<int32> ElementsToRemove;

	FVoxelUtilities::ParallelFor_PerThreadData(Voxels.Num(), [&]()
	{
		return MakeUnique<FVoxelMutableDataAccelerator>(GetActualData(Data), Bounds);
	}, 
	[&](const TUniquePtr<FVoxelMutableDataAccelerator>& Accelerator, int32 Index)
	{
		const FVoxelSurfaceEditsVoxel& Voxel = Voxels[Index];
		const bool bSet = Accelerator->Edit<T>(Voxel.Position, [&](T& Value)
		{
			const auto OldValue = Value;
			Lambda(*Accelerator, Voxel, Value);
			
			if (ModifiedValues)
			{
				(*ModifiedValues)[Index] = { Voxel.Position, OldValue, Value };
			}
		});

		if (!bSet && ModifiedValues)
		{
			ElementsToRemove.Add(Index);
		}
	}, !IsDataMultiThreaded(Data) || true); // TODO multithreading doesn't work because a single data chunk might be accessed by different threads

	for (int32 Index = ElementsToRemove.Num() - 1; Index >= 0; --Index)
	{
		ModifiedValues->RemoveAtSwap(ElementsToRemove[Index]);
	}
}

template<typename TData>
void FVoxelSurfaceEditToolsImpl::EditVoxelValues(
	TData& Data,
	const FVoxelHardnessHandler& HardnessHandler,
	const FVoxelIntBox& Bounds,
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels,
	float DistanceDivisor,
	bool bHasValues)
{
	VOXEL_SURFACE_TOOL_IMPL();

	Edit<FVoxelValue>(Data, Bounds, Voxels, [&](FVoxelMutableDataAccelerator& Accelerator, const FVoxelSurfaceEditsVoxel& Voxel, FVoxelValue& Value)
	{
		const float OldValue = bHasValues ? Voxel.Value : Value.ToFloat();
		float Strength = Voxel.Strength;
		if (HardnessHandler.NeedsToCompute())
		{
			Strength /= HardnessHandler.GetHardness(Accelerator.GetMaterial(Voxel.Position, 0));
		}
		const float NewValue = OldValue + Strength;
		Value = FVoxelValue(NewValue / DistanceDivisor);
	});
}

template<typename TData>
void FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(
	TData& Data, 
	const FVoxelIntBox& Bounds, 
	const FVoxelPaintMaterial& PaintMaterial, 
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels)
{
	VOXEL_SURFACE_TOOL_IMPL();

	Edit<FVoxelMaterial>(Data, Bounds, Voxels, [&](FVoxelMutableDataAccelerator& Accelerator, const FVoxelSurfaceEditsVoxel& Voxel, FVoxelMaterial& Material)
	{
		PaintMaterial.ApplyToMaterial(Material, Voxel.Strength);
	});
}

template<typename TData>
void FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(
	TData& Data, 
	const TArray<FVoxelSurfaceEditsVoxel>& Voxels)
{
	VOXEL_SURFACE_TOOL_IMPL();

	// Note: the reads are only safe because surface positions are all in the bounds (since they are computed by flood jump)

	FVoxelMutableDataAccelerator Accelerator(GetActualData(Data));
	for (auto& Voxel : Voxels)
	{
		FIntVector ClosestPosition;
		{
			v_flt Distance = MAX_vflt;
			for (auto& Neighbor : FVoxelUtilities::GetNeighbors(Voxel.SurfacePosition))
			{
				const FVoxelValue Value = Accelerator.GetValue(Neighbor, 0);
				if (!Value.IsEmpty())
				{
					const v_flt PointDistance = (FVoxelVector(Neighbor) - Voxel.SurfacePosition).SizeSquared();
					if (PointDistance < Distance)
					{
						Distance = PointDistance;
						ClosestPosition = Neighbor;
					}
				}
			}
			if (!ensureVoxelSlow(Distance < MAX_vflt))
			{
				continue;
			}
		}

		Accelerator.SetMaterial(Voxel.Position, Accelerator.GetMaterial(ClosestPosition, 0));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename TData>
void FVoxelSurfaceEditToolsImpl::EditVoxelValues(
	TData& Data, 
	const FVoxelHardnessHandler& HardnessHandler, 
	const FVoxelIntBox& Bounds,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels, 
	float DistanceDivisor)
{
	if (ProcessedVoxels.Info.bIs2D)
	{
		EditVoxelValues2D(Data, HardnessHandler, Bounds, *ProcessedVoxels.Voxels, DistanceDivisor);
	}
	else
	{
		EditVoxelValues(Data, HardnessHandler, Bounds, *ProcessedVoxels.Voxels, DistanceDivisor, ProcessedVoxels.Info.bHasValues);
	}
}

template<typename TData>
void FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(
	TData& Data, 
	const FVoxelIntBox& Bounds, 
	const FVoxelPaintMaterial& PaintMaterial, 
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels)
{
	EditVoxelMaterials(Data, Bounds, PaintMaterial, *ProcessedVoxels.Voxels);
}

template<typename TData>
void FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(
	TData& Data,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels)
{
	ensure(ProcessedVoxels.Info.bHasSurfacePositions);
	PropagateVoxelMaterials(Data, *ProcessedVoxels.Voxels);
}