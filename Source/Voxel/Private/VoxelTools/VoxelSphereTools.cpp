// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelData.h"
#include "StackArray.h"

void UVoxelSphereTools::SetValueSphereImpl(
	FVoxelData& Data, 
	const FVoxelVector& Position, 
	float Radius, 
	FVoxelValue Value)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const float SquaredRadius = FMath::Square(Radius);
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& OldValue)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			OldValue = Value;
		}
	});
}

template<bool bAdd, bool bComputeModifiedVoxels>
void UVoxelSphereTools::SphereEditImpl(
	FVoxelData& Data, 
	const FVoxelVector& Position,
	float Radius,
	TArray<FModifiedVoxelValue>& ModifiedVoxels)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const float SquaredRadiusPlus2 = FMath::Square(Radius + 2);
	const float SquaredRadiusMinus2 = FMath::Square(FMath::Max(Radius - 2, 0.f));
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance > SquaredRadiusPlus2) return;

		const FVoxelValue OldValue = Value;
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

		if (bComputeModifiedVoxels)
		{
			FModifiedVoxelValue ModifiedVoxel;
			ModifiedVoxel.Position = { X, Y, Z };
			ModifiedVoxel.OldValue = OldValue.ToFloat();
			ModifiedVoxel.NewValue = Value.ToFloat();
			ModifiedVoxels.Add(ModifiedVoxel);
		}
	});
}

template VOXEL_API void UVoxelSphereTools::SphereEditImpl<false, false>(FVoxelData& Data, const FVoxelVector& Position, float Radius, TArray<FModifiedVoxelValue>& ModifiedVoxels);
template VOXEL_API void UVoxelSphereTools::SphereEditImpl<true, false>(FVoxelData& Data, const FVoxelVector& Position, float Radius, TArray<FModifiedVoxelValue>& ModifiedVoxels);
template VOXEL_API void UVoxelSphereTools::SphereEditImpl<false, true>(FVoxelData& Data, const FVoxelVector& Position, float Radius, TArray<FModifiedVoxelValue>& ModifiedVoxels);
template VOXEL_API void UVoxelSphereTools::SphereEditImpl<true, true>(FVoxelData& Data, const FVoxelVector& Position, float Radius, TArray<FModifiedVoxelValue>& ModifiedVoxels);

void UVoxelSphereTools::SetMaterialSphereImpl(
	FVoxelData& Data,
	const FVoxelVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const float SquaredRadius = FMath::Square(Radius);
	Data.Set<FVoxelMaterial>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelMaterial& Material)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			const float Alpha = FMath::Sqrt(SquaredDistance / SquaredRadius);
			const float Strength = 1; // TODO use Alpha
			PaintMaterial.ApplyToMaterial(Material, Strength);
		}
	});
}

void UVoxelSphereTools::ApplyKernelSphereImpl(
	FVoxelData& Data, 
	const FVoxelVector& Position, 
	const float Radius, 
	const float Center,
	const float FirstDegreeNeighbor, 
	const float SecondDegreeNeighbor,
	const float ThirdDegreeNeighbor)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const float SquaredRadius = FMath::Square(Radius);
	const FIntVector Size = Bounds.Size();
	
	const TArray<FVoxelValue> Values = Data.GetValues(Bounds);

	const auto GetValue = [&](int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(0 <= X && X < Size.X);
		checkVoxelSlow(0 <= Y && Y < Size.Y);
		checkVoxelSlow(0 <= Z && Z < Size.Z);
		const int32 Index = X + Y * Size.X + Z * Size.X * Size.Y;
#if VOXEL_DEBUG
		return Values[Index];
#else
		auto* RESTRICT ValuesPtr = Values.GetData();
		return ValuesPtr[Index];
#endif
	};
	
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			const auto GetNeighbor = [&](int32 DX, int32 DY, int32 DZ)
			{
				if (DX == 0 && DY == 0 && DZ == 0)
				{
					return Value.ToFloat() * Center;
				}

				const int32 Degree = FMath::Abs(DX) + FMath::Abs(DY) + FMath::Abs(DZ);
				const float Multiplier = Degree == 1 ? FirstDegreeNeighbor : Degree == 2 ? SecondDegreeNeighbor : ThirdDegreeNeighbor;

				FIntVector P(X + DX, Y + DY, Z + DZ);
				checkVoxelSlow(Bounds.Contains(P));
				P -= Bounds.Min;
				return GetValue(P.X, P.Y, P.Z).ToFloat() * Multiplier;
			};

			Value = FVoxelValue(
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
				GetNeighbor(+0, +0, +0) +
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
				GetNeighbor(+1, +1, +1));
		}
	});
}

void UVoxelSphereTools::SmoothSphereImpl(
	FVoxelData& Data,
	const FVoxelVector& Position,
	float Radius,
	float Strength)
{
	VOXEL_FUNCTION_COUNTER();
	
	float CenterStrength = 1;
	float NeighborsStrength = Strength;
	const float Sum = 26 * NeighborsStrength + CenterStrength;
	CenterStrength /= Sum;
	NeighborsStrength /= Sum;

	UVoxelSphereTools::ApplyKernelSphereImpl(
		Data,
		Position,
		Radius,
		CenterStrength,
		NeighborsStrength,
		NeighborsStrength,
		NeighborsStrength);
}

void UVoxelSphereTools::SharpenSphereImpl(
	FVoxelData& Data,
	const FVoxelVector& Position,
	float Radius,
	float Strength)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const float SquaredRadius = FMath::Square(Radius);
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			Value = FVoxelValue(FMath::Lerp(Value.ToFloat(), Value.IsEmpty() ? 1.f : -1.f, Strength));
		}
	});
}

void UVoxelSphereTools::TrimSphereImpl(
	FVoxelData& Data,
	const FVoxelVector& Position,
	const FVector& Normal,
	float Radius,
	float Falloff,
	bool bAdditive)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius + Falloff);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
	const FPlane Plane(Position.ToFloat(), Normal);
	const float SquaredRadiusFalloff = FMath::Square(Radius + Falloff + 2);
	Data.Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector(X - Position.X, Y - Position.Y, Z - Position.Z).SizeSquared();
		if (SquaredDistance <= SquaredRadiusFalloff)
		{
			const float Distance = FMath::Sqrt(SquaredDistance);
			const float PlaneSDF = Plane.PlaneDot(FVector(X, Y, Z));
			const float SphereSDF = Distance - Radius - Falloff;
			if (bAdditive)
			{
				const float SDF = FVoxelUtilities::SmoothIntersection(PlaneSDF, SphereSDF, Falloff);
				Value = FMath::Min(Value, FVoxelValue(SDF));
			}
			else
			{
				const float SDF = -FVoxelUtilities::SmoothIntersection(-PlaneSDF, SphereSDF, Falloff);
				Value = FMath::Max(Value, FVoxelValue(SDF));
			}
		}
	});
}

void UVoxelSphereTools::RevertSphereImpl(
	FVoxelData& Data, 
	const FVoxelVector& Position, 
	const float Radius,
	const int32 HistoryPosition,
	const bool bRevertValues,
	const bool bRevertMaterials)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
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

			TStackArray<bool, VOXELS_PER_DATA_CHUNK> IsValueSet;
			IsValueSet.Memzero();

			TStackArray<Type, VOXELS_PER_DATA_CHUNK> Values;
			if (auto* DataPtr = Leaf.GetData<Type>().GetDataPtr())
			{
				FMemory::Memcpy(Values.GetData(), DataPtr, VOXELS_PER_DATA_CHUNK * sizeof(Type));
			}
			else
			{
				const Type SingleValue = Leaf.GetData<Type>().GetSingleValue();
				for (auto& Value : Values)
				{
					Value = SingleValue;
				}
			}

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
				const float Alpha = FMath::Sqrt(DistanceSquared) / Radius;
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

void UVoxelSphereTools::RevertSphereToGeneratorImpl(
	FVoxelData& Data,
	const FVoxelVector& Position,
	float Radius,
	bool bRevertValues,
	bool bRevertMaterials)
{
	const FIntBox Bounds = GetSphereBounds(Position, Radius);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());
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
			if (!DataHolder.GetDataPtr())
			{
				DataHolder.ExpandSingleValue(Data);
			}

			TStackArray<Type, VOXELS_PER_DATA_CHUNK> Values;
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define SPHERE_TOOL_PREFIX \
	const float Radius = FVoxelToolHelpers::GetRealDistance(World, InRadius, bConvertToVoxelSpace); \
	const FVoxelVector Position = FVoxelToolHelpers::GetRealPosition(World, InPosition, bConvertToVoxelSpace); \
	const auto Bounds = UVoxelSphereTools::GetSphereBounds(Position, Radius);

#define SPHERE_TOOL_WITH_FALLOFF_PREFIX \
	const float Radius = FVoxelToolHelpers::GetRealDistance(World, InRadius, bConvertToVoxelSpace); \
	const float Falloff = FVoxelToolHelpers::GetRealDistance(World, InFalloff, bConvertToVoxelSpace); \
	const FVoxelVector Position = FVoxelToolHelpers::GetRealPosition(World, InPosition, bConvertToVoxelSpace); \
	const auto Bounds = UVoxelSphereTools::GetSphereBounds(Position, Radius + Falloff);

void UVoxelSphereTools::SetValueSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Value, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SetValueSphereImpl(Data, Position, Radius, FVoxelValue(Value)));
}

void UVoxelSphereTools::AddSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, AddSphereImpl(Data, Position, Radius));
}

void UVoxelSphereTools::RemoveSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RemoveSphereImpl(Data, Position, Radius));
}

void UVoxelSphereTools::SetMaterialSphere(
	AVoxelWorld* World,
	FVector InPosition,
	float InRadius,
	FVoxelPaintMaterial PaintMaterial,
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SetMaterialSphereImpl(Data, Position, Radius, PaintMaterial));
}

void UVoxelSphereTools::ApplyKernelSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, ApplyKernelSphereImpl(
		Data,
		Position,
		Radius,
		CenterMultiplier,
		FirstDegreeNeighborMultiplier,
		SecondDegreeNeighborMultiplier,
		ThirdDegreeNeighborMultiplier));
}

void UVoxelSphereTools::SmoothSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Strength, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SmoothSphereImpl(Data, Position, Radius, Strength));
}

void UVoxelSphereTools::SharpenSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Strength, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SharpenSphereImpl(Data, Position, Radius, Strength));
}

void UVoxelSphereTools::TrimSphere(
	AVoxelWorld* World,
	FVector InPosition,
	FVector Normal,
	float InRadius,
	float InFalloff,
	bool bAdditive,
	bool bConvertToVoxelSpace)
{
	Normal.Normalize();
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_WITH_FALLOFF_PREFIX, TrimSphereImpl(Data, Position, Normal, Radius, Falloff, bAdditive));
}

void UVoxelSphereTools::RevertSphere(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bConvertToVoxelSpace)
{	
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RevertSphereImpl(Data, Position, Radius, HistoryPosition, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereToGenerator(
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	bool bRevertValues, 
	bool bRevertMaterials, 
	bool bConvertToVoxelSpace)
{
	VOXEL_TOOL_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RevertSphereToGeneratorImpl(Data, Position, Radius, bRevertValues, bRevertMaterials));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::SetValueSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Value, 
	bool bConvertToVoxelSpace, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SetValueSphereImpl(Data, Position, Radius, FVoxelValue(Value)));
}

void UVoxelSphereTools::AddSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVector InPosition,
	float InRadius,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, AddSphereImpl(Data, Position, Radius));
}

void UVoxelSphereTools::RemoveSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVector InPosition,
	float InRadius,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RemoveSphereImpl(Data, Position, Radius));
}

void UVoxelSphereTools::SetMaterialSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVector InPosition,
	float InRadius,
	FVoxelPaintMaterial PaintMaterial,
	bool bConvertToVoxelSpace, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SetMaterialSphereImpl(Data, Position, Radius, PaintMaterial));
}

void UVoxelSphereTools::ApplyKernelSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float CenterMultiplier, 
	float FirstDegreeNeighborMultiplier, 
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, ApplyKernelSphereImpl(
		Data, 
		Position, 
		Radius, 
		CenterMultiplier, 
		FirstDegreeNeighborMultiplier, 
		SecondDegreeNeighborMultiplier, 
		ThirdDegreeNeighborMultiplier));
}

void UVoxelSphereTools::SmoothSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Strength, 
	bool bConvertToVoxelSpace, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SmoothSphereImpl(Data, Position, Radius, Strength));
}

void UVoxelSphereTools::SharpenSphereAsync(
	UObject* WorldContextObject, 
	FLatentActionInfo LatentInfo, 
	AVoxelWorld* World, 
	FVector InPosition, 
	float InRadius, 
	float Strength, 
	bool bConvertToVoxelSpace, 
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, SharpenSphereImpl(Data, Position, Radius, Strength));
}

void UVoxelSphereTools::TrimSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVector InPosition,
	FVector Normal,
	float InRadius,
	float InFalloff,
	bool bAdditive,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	Normal.Normalize();
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_WITH_FALLOFF_PREFIX, TrimSphereImpl(Data, Position, Normal, Radius, Falloff, bAdditive));
}

void UVoxelSphereTools::RevertSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVector InPosition,
	float InRadius,
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RevertSphereImpl(Data, Position, Radius, HistoryPosition, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereToGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	AVoxelWorld* World,
	FVector InPosition,
	float InRadius,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bConvertToVoxelSpace,
	bool bHideLatentWarnings)
{
	VOXEL_TOOL_LATENT_HELPER(Write, UpdateRender, SPHERE_TOOL_PREFIX, RevertSphereToGeneratorImpl(Data, Position, Radius, bRevertValues, bRevertMaterials));
}