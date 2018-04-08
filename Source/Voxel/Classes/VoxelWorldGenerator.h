// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelType.h"
#include "VoxelMaterial.h"
#include "Templates/SubclassOf.h"
#include "IntBox.h"
#include "VoxelWorldGenerator.generated.h"

class AVoxelWorld;

/**
 * A FVoxelWorldGeneratorInstance is a constant object created by a UVoxelWorldGenerator
 */
class VOXEL_API FVoxelWorldGeneratorInstance
{
public:
	virtual ~FVoxelWorldGeneratorInstance() = default;

	/** Get the value at (X,Y,Z) */
	float GetValue(int X, int Y, int Z) const;
	/** Get the material at (X,Y,Z) */
	FVoxelMaterial GetMaterial(int X, int Y, int Z) const;
	/** Get the voxel type at (X,Y,Z) */
	FVoxelType GetVoxelType(int X, int Y, int Z) const;

	/** Get the value, material and voxel type at (X,Y,Z) */
	void GetValueAndMaterialAndVoxelType(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial, FVoxelType& OutVoxelType);

	/** Get the value and material at (X,Y,Z) */
	void GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial);

	/**
	 * Get the values/materials/voxeltypes
	 * Values/Materials/VoxelTypes can be null
	 * Start.X <= X < Start.X + Step * Size.X
	 * If set, the arrays MUST be initialized
	 *
	 * @param	Values		Values. Can be nullptr
	 * @param	Materials	Materials. Can be nullptr
	 * @param	VoxelTypes	VoxelTypes. Can be nullptr
	 * @param	Start		Start of the processed chunk in voxel position
	 * @param	StartIndex	Start of the processed chunk in the arrays
	 * @param	Step		How far are each values from each other in voxel position
	 * @param	Size		Size of the chunk in the arrays. Voxel Size of the chunk = Size * Step
	 * @param	ArraySize	Size of the arrays
	 */
	virtual void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
	{
		unimplemented();

		for (int K = 0; K < Size.Z; K++)
		{
			const int Z = Start.Z + K * Step;
			// If Value/Material/VoxelType doesn't depend on X and Y, you should compute it here

			for (int J = 0; J < Size.Y; J++)
			{
				const int Y = Start.Y + J * Step;
				// If Value/Material/VoxelType doesn't depend on X, you should compute it here

				for (int I = 0; I < Size.X; I++)
				{
					const int X = Start.X + I * Step;

					const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

					if (Values)
					{
						Values[Index] = 0; // Your value here
					}
					if (Materials)
					{
						Materials[Index] = FVoxelMaterial(); // Your material here
					}
					if (VoxelTypes)
					{
						VoxelTypes[Index] = FVoxelType(); // Your type here
					}
				}
			}
		}
	}

	/**
	 * If you need a reference to Voxel World. Called only once and before GetValuesAndMaterialsAndVoxelTypes
	 */
	virtual void SetVoxelWorld(const AVoxelWorld* VoxelWorld) {}

	/**
	 * World up vector at position (must be normalized). Mainly used for grass
	 */
	virtual FVector GetUpVector(int X, int Y, int Z) const { return FVector::UpVector; }

	/**
	 * If true, GetValuesAndMaterialsAndVoxelTypes(Start, Step, Size) MUST be empty, else it MIGHT be empty
	 */
	virtual bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const { return false; }
};

/**
 * A UVoxelWorldGenerator is used to create a FVoxelWorldGeneratorInstance
 */
UCLASS()
class VOXEL_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelWorldGenerator Interface
	virtual TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() { unimplemented(); return MakeShareable(new FVoxelWorldGeneratorInstance()); }
	//~ End UVoxelWorldGenerator Interface
};

UENUM(BlueprintType)
enum class EVoxelWorldGeneratorClassOrObject : uint8
{
	Class,
	Object
};

/**
 * Used to select a world generator
 */
USTRUCT(BlueprintType)
struct VOXEL_API FVoxelWorldGeneratorPicker
{
	GENERATED_BODY()

public:
	FVoxelWorldGeneratorPicker();

	UPROPERTY(EditAnywhere)
	EVoxelWorldGeneratorClassOrObject UseClassOrObject;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UVoxelWorldGenerator> WorldGeneratorClass;

	UPROPERTY(EditAnywhere)
	UVoxelWorldGenerator* WorldGeneratorObject;

	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() const;
	FString GetName() const;
};