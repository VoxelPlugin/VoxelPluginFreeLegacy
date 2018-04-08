// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"

/**
 * SetVoxelWorld must be called on childs generators
 */
class FVoxelSphereShapeWorldGenerator : public FVoxelWorldGeneratorInstance
{
public:
	FVoxelSphereShapeWorldGenerator(const TSharedRef<FVoxelWorldGeneratorInstance>& OutsideWorldGenerator, const TSharedRef<FVoxelWorldGeneratorInstance>& InsideWorldGenerator, const FIntVector& Center, float Radius)
		: OutsideWorldGenerator(OutsideWorldGenerator)
		, InsideWorldGenerator(InsideWorldGenerator)
		, Center(Center)
		, Radius(Radius)
	{

	}

	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const override
	{		
		const FIntVector P(FMath::CeilToInt(Radius));
		const FIntBox SphereBox(Center - P, Center + P + FIntVector(1, 1, 1));
		const FIntBox Box(Start, Start + Size * Step);

		bool bAllInside = true;
		bool bAllOutside = true;
		for (auto& Corner : Box.GetCorners())
		{
			bool bInside = IsInside(Corner);
			bAllInside = bAllInside && bInside;
			bAllOutside = bAllOutside && !bInside;
		}
		if (bAllInside)
		{
			InsideWorldGenerator->GetValuesAndMaterialsAndVoxelTypes(Values, Materials, VoxelTypes, Start, StartIndex, Step, Size, ArraySize);
		}
		else if (bAllOutside && !SphereBox.Intersect(Box))
		{
			OutsideWorldGenerator->GetValuesAndMaterialsAndVoxelTypes(Values, Materials, VoxelTypes, Start, StartIndex, Step, Size, ArraySize);
		}
		else
		{
			for (int K = 0; K < Size.Z; K++)
			{
				const int Z = Start.Z + K * Step;

				for (int J = 0; J < Size.Y; J++)
				{
					const int Y = Start.Y + J * Step;

					for (int I = 0; I < Size.X; I++)
					{
						const int X = Start.X + I * Step;

						const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

						const bool bInside = IsInside(FIntVector(X, Y, Z));

						float Value;
						FVoxelMaterial Material;
						FVoxelType Type;

						if (bInside)
						{
							InsideWorldGenerator->GetValueAndMaterialAndVoxelType(X, Y, Z, Value, Material, Type);
						}
						else
						{
							OutsideWorldGenerator->GetValueAndMaterialAndVoxelType(X, Y, Z, Value, Material, Type);
						}

						if (Values)
						{
							Values[Index] = Value;
						}
						if (Materials)
						{
							Materials[Index] = Material;
						}
						if (VoxelTypes)
						{
							VoxelTypes[Index] = Type;
						}
					}
				}
			}
		}
	}

	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override
	{		
		const FIntVector P(FMath::CeilToInt(Radius));
		const FIntBox SphereBox(Center - P, Center + P + FIntVector(1, 1, 1));
		const FIntBox Box(Start, Start + Size * Step);

		bool bAllInside = true;
		bool bAllOutside = true;
		for (auto& Corner : Box.GetCorners())
		{
			bool bInside = IsInside(Corner);
			bAllInside = bAllInside && bInside;
			bAllOutside = bAllOutside && !bInside;
		}
		if (bAllInside)
		{
			return InsideWorldGenerator->IsEmpty(Start, Step, Size);
		}
		else if (bAllOutside && !SphereBox.Intersect(Box))
		{
			return OutsideWorldGenerator->IsEmpty(Start, Step, Size);
		}
		else
		{
			return false;
		}
	}

	FORCEINLINE bool IsInside(const FIntVector& V) const
	{
		return (V - Center).Size() < Radius;
	}

private:
	const TSharedRef<FVoxelWorldGeneratorInstance> OutsideWorldGenerator;
	const TSharedRef<FVoxelWorldGeneratorInstance> InsideWorldGenerator;
	const FIntVector Center;
	const float Radius;
};