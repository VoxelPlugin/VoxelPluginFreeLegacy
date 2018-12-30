// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "IntBox.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelWorldGenerator.generated.h"

struct FVoxelWorldGeneratorInit
{
	TMap<FString, int> Seeds;
	float VoxelSize = 100;

	FVoxelWorldGeneratorInit() {};

	FVoxelWorldGeneratorInit(const TMap<FString, int>& Seeds, float VoxelSize)
		: Seeds(Seeds)
		, VoxelSize(VoxelSize)
	{
	}
};

struct FVoxelWorldGeneratorQueryZone
{
	const FIntBox Bounds;
	const FIntVector Offset;
	const FIntVector ArraySize;
	const int LOD;
	const int Step;

	FVoxelWorldGeneratorQueryZone(const FIntBox& Bounds, const FIntVector& Offset, const FIntVector& ArraySize, int LOD)
		: Bounds(Bounds)
		, Offset(Offset)
		, ArraySize(ArraySize)
		, LOD(LOD)
		, Step(1 << LOD)
	{
		check(Bounds.IsMultipleOf(Step));
	}
	FVoxelWorldGeneratorQueryZone(const FIntVector& Position)
		: FVoxelWorldGeneratorQueryZone(FIntBox(Position), Position, FIntVector(1, 1, 1), 0)
	{
	}
	FVoxelWorldGeneratorQueryZone(int X, int Y, int Z)
		: FVoxelWorldGeneratorQueryZone(FIntVector(X, Y, Z))
	{
	}
	FVoxelWorldGeneratorQueryZone(const FIntBox& Bounds, const FIntVector& ArraySize, int LOD)
		: FVoxelWorldGeneratorQueryZone(Bounds, Bounds.Min, ArraySize, LOD)
	{
		check(Bounds.Size() / Step == ArraySize);
	}

	inline int GetIndex(int X, int Y, int Z) const
	{
		int LX = (uint32)(X - Offset.X) >> LOD;
		int LY = (uint32)(Y - Offset.Y) >> LOD;
		int LZ = (uint32)(Z - Offset.Z) >> LOD;

		checkVoxelSlow(LX == (X - Offset.X) / Step);
		checkVoxelSlow(LY == (Y - Offset.Y) / Step);
		checkVoxelSlow(LZ == (Z - Offset.Z) / Step);

		checkVoxelSlow(0 <= LX && LX < ArraySize.X);
		checkVoxelSlow(0 <= LY && LY < ArraySize.Y);
		checkVoxelSlow(0 <= LZ && LZ < ArraySize.Z);

		return LX + ArraySize.X * LY + ArraySize.X * ArraySize.Y * LZ;
	}

	inline FVoxelWorldGeneratorQueryZone ShrinkTo(const FIntBox& InBounds) const
	{
		FIntBox LocalBounds = Bounds.Overlap(InBounds);
		LocalBounds.MakeMultipleOfExclusive(Step);
		return FVoxelWorldGeneratorQueryZone(LocalBounds, Offset, ArraySize, LOD);
	}

public:
	struct FIteratorElement
	{
		int Value;
		const int Step;

		FIteratorElement(int Value, int Step = -1) : Value(Value), Step(Step) {}

		inline int operator*() const
		{
			return Value;
		}
		inline void operator++()
		{
			Value += Step;
		}
		inline bool operator!=(const FIteratorElement& Other) const
		{
			checkVoxelSlow(Other.Step == -1);
			return Value < Other.Value;
		}
	};

	struct FIterator
	{
		const int Min;
		const int Max;
		const int Step;

		FIterator(int Min, int Max, int Step) : Min(Min), Max(Max), Step(Step) {}

		FIteratorElement begin() { return FIteratorElement(Min, Step); }
		FIteratorElement end() { return FIteratorElement(Max); }
	};

	inline FIterator XIt() const { return FIterator(Bounds.Min.X, Bounds.Max.X, Step); }
	inline FIterator YIt() const { return FIterator(Bounds.Min.Y, Bounds.Max.Y, Step); }
	inline FIterator ZIt() const { return FIterator(Bounds.Min.Z, Bounds.Max.Z, Step); }
};

enum class EVoxelEmptyState : uint8
{
	Unknown,
	AllFull,
	AllEmpty
};

/**
 * A FVoxelWorldGeneratorInstance is a constant object created by a UVoxelWorldGenerator
 */
class VOXEL_API FVoxelWorldGeneratorInstance
{
public:
	inline static void SetValueAndMaterial(FVoxelValue* Values, FVoxelMaterial* Materials, const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder, int X, int Y, int Z, const FVoxelValue& Value, const FVoxelMaterial& Material)
	{
		int Index = QueryZone.GetIndex(X, Y, Z);
		if (Values)
		{
			Values[Index] = Value;
		}
		if (Materials)
		{
			Materials[Index] = Material;
		}
	}
	
public:
	/** Get the value and material (X, Y, Z) */
	inline void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const
	{
		GetValueAndMaterialInternal(X, Y, Z, OutValue, OutMaterial, QueryLOD, ItemHolder);

	}
	inline FVector GetUpVector(const FIntVector& V) const { return GetUpVector(V.X, V.Y, V.Z); }
	
public:
	//~ Begin FVoxelWorldGeneratorInstance Interface
	virtual ~FVoxelWorldGeneratorInstance() = default;

	virtual void GetValueAndMaterialInternal(int X, int Y, int Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const
	{
		// Defaults to GetValuesAndMaterials, should override with more optimized stuff if you can
		GetValuesAndMaterials(OutValue, OutMaterial, FVoxelWorldGeneratorQueryZone(X, Y, Z), QueryLOD, ItemHolder);
	}
	virtual void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD, const FVoxelPlaceableItemHolder& ItemHolder) const
	{
		unimplemented();

		FVoxelValue Value;
		FVoxelMaterial Material;

		for (int X : QueryZone.XIt())
		{
			// If Value/Material doesn't depend on Y and Z, you should compute it here
			for (int Y : QueryZone.YIt())
			{
				// If Value/Material doesn't depend on Z, you should compute it here
				for (int Z : QueryZone.ZIt())
				{
					SetValueAndMaterial(Values, Materials, QueryZone, QueryLOD, ItemHolder, X, Y, Z, Value, Material);
				}
			}
		}
	}
	// Initialization
	virtual void Init(const FVoxelWorldGeneratorInit& InitStruct) {}
	// World up vector at position (must be normalized). Mainly used for grass
	virtual FVector GetUpVector(int X, int Y, int Z) const { return FVector::UpVector; }
	// If true, GetValuesAndMaterials(Start, Step, Size) MUST be empty, else it MIGHT be empty
	virtual EVoxelEmptyState IsEmpty(const FIntBox& Bounds, int LOD) const { return EVoxelEmptyState::Unknown; }
	// Custom outputs
	virtual float GetFloatOutput(int X, int Y, int Z, uint8 Index) const { return 0; }
	virtual int GetOutputIndex(FName Name) const { return -1; }
	//~ End FVoxelWorldGeneratorInstance Interface

private:
};

/**
 * A UVoxelWorldGenerator is used to create a FVoxelWorldGeneratorInstance
 */
UCLASS(Blueprintable, abstract)
class VOXEL_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGenerator Interface
	virtual TSharedRef<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe> GetWorldGenerator()
	{
		unimplemented();
		return MakeShared<FVoxelWorldGeneratorInstance, ESPMode::ThreadSafe>();
	}
	//~ End UVoxelWorldGenerator Interface
};