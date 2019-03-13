// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelUtilitiesLibrary.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelToolsHelpers.h"

#define LOCTEXT_NAMESPACE "Voxel"

DECLARE_CYCLE_STAT(TEXT("UVoxelUtilitiesLibrary::SetBoxAsDirty"), STAT_VoxelUtilitiesLibrary_SetBoxAsDirty, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelUtilitiesLibrary::RoundVoxels"), STAT_VoxelUtilitiesLibrary_RoundVoxels, STATGROUP_Voxel);

void UVoxelUtilitiesLibrary::AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet)
{
	OutSet.Reset();
	for (auto& P : InSet)
	{
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z - 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z - 1));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 0));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 0));
		
		OutSet.Add(FIntVector(P.X - 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y - 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 0, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 1, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X - 0, P.Y + 1, P.Z + 1));
		OutSet.Add(FIntVector(P.X + 1, P.Y + 1, P.Z + 1));
	}
}

void UVoxelUtilitiesLibrary::SetBoxAsDirty(AVoxelWorld* World, const FIntBox& Bounds, bool bSetValuesAsDirty /*= true*/, bool bSetMaterialsAsDirty /*= true*/)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelUtilitiesLibrary_SetBoxAsDirty);
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	FVoxelData& Data = World->GetData();
	{
		FVoxelReadWriteScopeLock Lock(Data, Bounds, "SetBoxAsDirty");

		if (bSetValuesAsDirty)
		{
			Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value)
			{

			});
		}
		if (bSetMaterialsAsDirty)
		{
			Data.SetValueOrMaterialLambda<FVoxelMaterial>(Bounds, [&](int X, int Y, int Z, FVoxelMaterial& Material)
			{

			});
		}
	}
}

void UVoxelUtilitiesLibrary::RoundVoxels(AVoxelWorld* World, const FIntBox& Bounds)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelUtilitiesLibrary_RoundVoxels);
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	const FIntVector Size = Bounds.Size();

	TArray<FVoxelValue> Values;
	Values.SetNumUninitialized(Size.X * Size.Y * Size.Z);

	FVoxelData& Data = World->GetData();
	FVoxelReadWriteScopeLock Lock(Data, Bounds, "RoundVoxels");

	Data.SetValueOrMaterialLambda<FVoxelValue>(Bounds, [&](int X, int Y, int Z, FVoxelValue& Value) {}); // Make sure values are cached for faster access
	Data.GetValuesAndMaterials(Values.GetData(), nullptr, FVoxelWorldGeneratorQueryZone(Bounds, Size, 0), 0);
	
	FVoxelDataUtilities::LastOctreeAccelerator OctreeAccelerator(Data);

#define VOXELINDEX(A, B, C) (A - Bounds.Min.X) + Size.X * (B - Bounds.Min.Y) + Size.X * Size.Y * (C - Bounds.Min.Z)
#define CHECKVOXEL(A, B, C) if (!Bounds.IsInside(A, B, C) || Values[VOXELINDEX(A, B, C)].IsEmpty() != bEmpty) continue;
	for (int Z = Bounds.Min.Z; Z < Bounds.Max.Z; Z++)
	{
		for (int Y = Bounds.Min.Y; Y < Bounds.Max.Y; Y++)
		{
			for (int X = Bounds.Min.X; X < Bounds.Max.X; X++)
			{
				auto& Value = Values[VOXELINDEX(X, Y, Z)];
				if(Value.IsTotallyEmpty() || Value.IsTotallyFull()) continue;
				bool bEmpty = Value.IsEmpty();
				CHECKVOXEL(X - 1, Y, Z);
				CHECKVOXEL(X + 1, Y, Z);
				CHECKVOXEL(X, Y - 1, Z);
				CHECKVOXEL(X, Y + 1, Z);
				CHECKVOXEL(X, Y, Z - 1);
				CHECKVOXEL(X, Y, Z + 1);
				OctreeAccelerator.SetValue(X, Y, Z, bEmpty ? FVoxelValue::Empty : FVoxelValue::Full);
			}
		}
	}
#undef CHECKVOXEL
#undef VOXELINDEX
}

#undef LOCTEXT_NAMESPACE