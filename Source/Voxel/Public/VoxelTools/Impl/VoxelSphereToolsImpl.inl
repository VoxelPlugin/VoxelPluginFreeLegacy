// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"
#include "VoxelUtilities/VoxelSDFUtilities.h"

#define VOXEL_SPHERE_TOOL_IMPL() const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(Position, Radius); VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

inline FVoxelIntBox FVoxelSphereToolsImpl::GetBounds(const FVoxelVector& Position, float Radius)
{
	return FVoxelIntBox(Position - Radius - 3, Position + Radius + 3);
}


template<bool bAdd, typename TData>
void FVoxelSphereToolsImpl::SphereEdit(TData& Data, const FVoxelVector& Position, float Radius)
{
	VOXEL_SPHERE_TOOL_IMPL();

	const float SquaredRadiusPlus2 = FMath::Square(Radius + 2);
	const float SquaredRadiusMinus2 = FMath::Square(FMath::Max(Radius - 2, 0.f));

	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance > SquaredRadiusPlus2) return;

		if (SquaredDistance <= SquaredRadiusMinus2)
		{
			Value = bAdd ? FVoxelValue::Full() : FVoxelValue::Empty();
		}
		else
		{
			const float Distance = FMath::Sqrt(SquaredDistance);
			const FVoxelValue NewValue{ FMath::Clamp(Radius - Distance, -2.f, 2.f) / 2 * (bAdd ? -1 : 1) };

			// We want to cover as many surface as possible, so we take the biggest value
			Value = FVoxelUtilities::MergeAsset(Value, NewValue, !bAdd);
		}
	});
}

template<typename TData>
void FVoxelSphereToolsImpl::SetValueSphere(TData& Data, const FVoxelVector& Position, float Radius, FVoxelValue Value)
{
	VOXEL_SPHERE_TOOL_IMPL();

	const float SquaredRadius = FMath::Square(Radius);

	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& OldValue)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			OldValue = Value;
		}
	});
}

template<typename TData>
void FVoxelSphereToolsImpl::SetMaterialSphere(TData& Data, const FVoxelVector& Position, float Radius, const FVoxelPaintMaterial& PaintMaterial)
{
	VOXEL_SPHERE_TOOL_IMPL();

	const float SquaredRadius = FMath::Square(Radius);
	Data.template Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			//const float Alpha = FMath::Sqrt(SquaredDistance / SquaredRadius);
			const float Strength = 1; // TODO use Alpha
			PaintMaterial.ApplyToMaterial(Material, Strength);
		}
	});
}

template<typename TData, typename T>
void FVoxelSphereToolsImpl::ApplyKernelSphere(
	TData& Data, 
	const FVoxelVector& Position, 
	float Radius, 
	float CenterMultiplier, 
	float FirstDegreeNeighborMultiplier, 
	float SecondDegreeNeighborMultiplier, 
	float ThirdDegreeNeighborMultiplier, 
	int32 NumIterations, 
	T GetStrength)
{
	VOXEL_SPHERE_TOOL_IMPL();
	
	if (NumIterations <= 0)
	{
		return;
	}

	const float SquaredRadius = FMath::Square(Radius);
	const FIntVector Size = Bounds.Size();

	const auto GetNewValue = [&](auto& GetValue, float Distance, int32 X, int32 Y, int32 Z)
	{
		const auto GetNeighbor = [&](int32 DX, int32 DY, int32 DZ)
		{
			const int32 Degree = FMath::Abs(DX) + FMath::Abs(DY) + FMath::Abs(DZ);
			const float Multiplier =
				Degree == 1 ? FirstDegreeNeighborMultiplier
				: Degree == 2 ? SecondDegreeNeighborMultiplier
				: ThirdDegreeNeighborMultiplier;

			return GetValue(X + DX, Y + DY, Z + DZ) * Multiplier;
		};

		const float NeighborSum =
			GetNeighbor(-1, -1, -1) +
			GetNeighbor(+0, -1, -1) +
			GetNeighbor(+1, -1, -1) +
			GetNeighbor(-1, +0, -1) +
			GetNeighbor(+0, +0, -1) +
			GetNeighbor(+1, +0, -1) +
			GetNeighbor(-1, +1, -1) +
			GetNeighbor(+0, +1, -1) +
			GetNeighbor(+1, +1, -1) +
			GetNeighbor(-1, -1, +0) +
			GetNeighbor(+0, -1, +0) +
			GetNeighbor(+1, -1, +0) +
			GetNeighbor(-1, +0, +0) +
			GetNeighbor(+1, +0, +0) +
			GetNeighbor(-1, +1, +0) +
			GetNeighbor(+0, +1, +0) +
			GetNeighbor(+1, +1, +0) +
			GetNeighbor(-1, -1, +1) +
			GetNeighbor(+0, -1, +1) +
			GetNeighbor(+1, -1, +1) +
			GetNeighbor(-1, +0, +1) +
			GetNeighbor(+0, +0, +1) +
			GetNeighbor(+1, +0, +1) +
			GetNeighbor(-1, +1, +1) +
			GetNeighbor(+0, +1, +1) +
			GetNeighbor(+1, +1, +1);

		const float OldValue = GetValue(X, Y, Z);

		return FMath::Lerp(OldValue, NeighborSum + OldValue * CenterMultiplier, GetStrength(Distance));
	};
	
	const TArray<FVoxelValue> Values = GetActualData(Data).GetValues(Bounds);
	const auto GetValue = [&](int32 X, int32 Y, int32 Z) { return FVoxelUtilities::Get3D(Values, Size, X, Y, Z, Bounds.Min).ToFloat(); };

	if (NumIterations > 1)
	{
		TArray<float> Buffer;
		Buffer.SetNumUninitialized(Values.Num() * 2);

		// Only make one allocation
		float* RESTRICT const BufferA = Buffer.GetData();
		float* RESTRICT const BufferB = Buffer.GetData() + Values.Num();

		const auto Step = [&](auto Src, auto Dst)
		{
			Bounds.ParallelIterate([&](int32 X, int32 Y, int32 Z)
			{
				const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
				
				if (SquaredDistance <= SquaredRadius)
				{
					Dst(X, Y, Z) = GetNewValue(Src, FMath::Sqrt(SquaredDistance), X, Y, Z);
				}
				else
				{
					Dst(X, Y, Z) = Src(X, Y, Z);
				}
			}, !IsDataMultiThreaded(Data));
		};
		
		const auto GetA = FVoxelUtilities::Create3DGetter(BufferA, Size, Bounds.Min);
		const auto GetB = FVoxelUtilities::Create3DGetter(BufferB, Size, Bounds.Min);

		Step(GetValue, GetA);

		bool bSrcIsA = true;
		// -1: last one is done in the SetValue
		for (int32 Iteration = 1; Iteration < NumIterations - 1; Iteration++)
		{
			if (bSrcIsA)
			{
				Step(GetA, GetB);
			}
			else
			{
				Step(GetB, GetA);
			}
			bSrcIsA = !bSrcIsA;
		}
		
		Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
		{
			const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
			if (SquaredDistance <= SquaredRadius)
			{
				if (bSrcIsA)
				{
					Value = FVoxelValue(GetNewValue(GetA, FMath::Sqrt(SquaredDistance), X, Y, Z));
				}
				else
				{
					Value = FVoxelValue(GetNewValue(GetB, FMath::Sqrt(SquaredDistance), X, Y, Z));
				}
			}
		});
	}
	else
	{
		Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
		{
			const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
			if (SquaredDistance <= SquaredRadius) // Kinda hacky: assume this is false for at least a 1-voxel thick border, making it safe to query neighbors
			{
				Value = FVoxelValue(GetNewValue(GetValue, FMath::Sqrt(SquaredDistance), X, Y, Z));
			}
		});
	}
}

template<typename TData>
void FVoxelSphereToolsImpl::SmoothSphere(TData& Data, const FVoxelVector& Position, float Radius, float Strength, int32 NumIterations, EVoxelFalloff FalloffType, float Falloff)
{
	float CenterStrength = 1;
	float NeighborsStrength = Strength;
	const float Sum = 26 * NeighborsStrength + CenterStrength;
	CenterStrength /= Sum;
	NeighborsStrength /= Sum;

	FVoxelUtilities::DispatchFalloff(FalloffType, Radius, Falloff, [&](auto GetFalloff)
	{
		ApplyKernelSphere(
			Data,
			Position,
			Radius,
			CenterStrength,
			NeighborsStrength,
			NeighborsStrength,
			NeighborsStrength,
			NumIterations,
			GetFalloff);
	});
}

template<typename TData>
void FVoxelSphereToolsImpl::TrimSphere(TData& Data, const FVoxelVector& Position, const FVector& Normal, float Radius, float Falloff, bool bAdditive)
{
	VOXEL_SPHERE_TOOL_IMPL();

	const float RelativeRadius = Radius * (1.f - Falloff);
	const float RelativeFalloff = Radius * Falloff;
	
	const FPlane Plane(Position.ToFloat(), Normal);
	const float SquaredRadiusFalloff = FMath::Square(RelativeRadius + RelativeFalloff + 2);

	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadiusFalloff)
		{
			const float Distance = FMath::Sqrt(SquaredDistance);
			const float PlaneSDF = Plane.PlaneDot(FVector(X, Y, Z));
			const float SphereSDF = Distance - RelativeRadius - RelativeFalloff;
			if (bAdditive)
			{
				const float SDF = FVoxelSDFUtilities::opSmoothIntersection(PlaneSDF, SphereSDF, RelativeFalloff);
				Value = FMath::Min(Value, FVoxelValue(SDF));
			}
			else
			{
				const float SDF = -FVoxelSDFUtilities::opSmoothIntersection(-PlaneSDF, SphereSDF, RelativeFalloff);
				Value = FMath::Max(Value, FVoxelValue(SDF));
			}
		}
	});
}

template<typename TData>
void FVoxelSphereToolsImpl::RevertSphere(TData& InData, const FVoxelVector& Position, float Radius, int32 HistoryPosition, bool bRevertValues, bool bRevertMaterials)
{
	VOXEL_SPHERE_TOOL_IMPL();

	auto& Data = GetActualData(InData);
	const float RadiusSquared = FMath::Square(Radius);

	FVoxelOctreeUtilities::IterateTreeInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (!Chunk.IsLeaf())
		{
			return;
		}

		ensureThreadSafe(Chunk.IsLockedForWrite());
		auto& Leaf = Chunk.AsLeaf();
			
		const auto ApplyForType = [&](auto TypeInst, auto SetValue)
		{
			using Type = decltype(TypeInst);
		
			if (!Leaf.GetData<Type>().IsDirty() || !Leaf.UndoRedo.IsValid())
			{
				return;
			}

			TVoxelStaticArray<bool, VOXELS_PER_DATA_CHUNK> IsValueSet;
			IsValueSet.Memzero();

			TVoxelStaticArray<Type, VOXELS_PER_DATA_CHUNK> Values;
			Leaf.GetData<Type>().CopyTo(Values.GetData());

			const auto& Stack = Leaf.UndoRedo->GetFramesStack<EVoxelUndoRedo::Undo>();
			for (int32 Index = Stack.Num() - 1; Index >= 0; --Index)
			{
				if (Stack[Index]->HistoryPosition < HistoryPosition) break;

				for (auto& Value : FVoxelUtilities::TValuesMaterialsSelector<Type>::Get(*Stack[Index]))
				{
					IsValueSet[Value.Index] = true;
					Values[Value.Index] = Value.Value;
				}
			}

			const FIntVector Min = Leaf.GetMin();

			FVoxelDataOctreeSetter::Set<Type>(Data, Leaf, [&](auto Lambda)
			{
				Leaf.GetBounds().Iterate(Lambda);
			},
			[&](int32 X, int32 Y, int32 Z, Type& Value)
			{
				const float DistanceSquared = (FVector(X, Y, Z) - Position).SizeSquared();
				if (DistanceSquared < RadiusSquared)
				{
					const uint32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);
					if (IsValueSet[Index])
					{
						SetValue(DistanceSquared, Value, Values[Index]);
					}
				}
			});
		};
		
		if (bRevertValues)
		{
			ApplyForType(FVoxelValue(), [&](float DistanceSquared, FVoxelValue& Value, const FVoxelValue& NewValue)
			{
				//const float Alpha = FMath::Sqrt(DistanceSquared) / Radius;
				const float Strength = 1; // TODO use Alpha
				Value = FVoxelValue(FMath::Lerp(Value.ToFloat(), NewValue.ToFloat(), Strength));
			});
		}
		if (bRevertMaterials)
		{
			ApplyForType(FVoxelMaterial(), [&](float DistanceSquared, FVoxelMaterial& Value, const FVoxelMaterial& NewValue)
			{
				Value = NewValue;
			});
		}
	});
}

template<typename TData>
void FVoxelSphereToolsImpl::RevertSphereToGenerator(TData& InData, const FVoxelVector& Position, float Radius, bool bRevertValues, bool bRevertMaterials)
{
	VOXEL_SPHERE_TOOL_IMPL();
	
	auto& Data = GetActualData(InData);
	const float RadiusSquared = FMath::Square(Radius);

	FVoxelOctreeUtilities::IterateTreeInBounds(Data.GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (!Chunk.IsLeaf())
		{
			return;
		}

		ensureThreadSafe(Chunk.IsLockedForWrite());
		auto& Leaf = Chunk.AsLeaf();

		const auto ApplyForType = [&](auto TypeInst)
		{
			using Type = decltype(TypeInst);

			auto& DataHolder = Leaf.GetData<Type>();
			if (!DataHolder.IsDirty())
			{
				return;
			}

			TVoxelStaticArray<Type, VOXELS_PER_DATA_CHUNK> Values;
			TVoxelQueryZone<Type> QueryZone(Leaf.GetBounds(), Values);
			Leaf.GetFromGeneratorAndAssets<Type>(*Data.WorldGenerator, QueryZone, 0);

			const FIntVector Min = Leaf.GetMin();

			bool bAllGeneratorValues = true;
			
			FVoxelDataOctreeSetter::Set<Type>(Data, Leaf, [&](auto Lambda)
			{
				Leaf.GetBounds().Iterate(Lambda);
			},
			[&](int32 X, int32 Y, int32 Z, Type& Value)
			{
				const float DistanceSquared = (FVector(X, Y, Z) - Position).SizeSquared();
				const int32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);
				if (DistanceSquared < RadiusSquared)
				{
					Value = Values[Index];
				}
				else
				{
					bAllGeneratorValues &= (Value == Values[Index]);
				}
			});

			if (bAllGeneratorValues)
			{
				DataHolder.ClearData(Data);
			}
		};

		if (bRevertValues)
		{
			ApplyForType(FVoxelValue());
		}
		if (bRevertMaterials)
		{
			ApplyForType(FVoxelMaterial());
		}
	});
}
