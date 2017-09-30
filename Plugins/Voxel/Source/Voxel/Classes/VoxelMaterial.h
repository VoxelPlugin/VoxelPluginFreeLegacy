#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.generated.h"


USTRUCT(BlueprintType)
struct FVoxelMaterial
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint8 Index1;
	UPROPERTY(EditAnywhere)
		uint8 Index2;
	UPROPERTY(EditAnywhere)
		uint8 Alpha;

	FVoxelMaterial() : Index1(0), Index2(0), Alpha(0)
	{

	}

	FVoxelMaterial(uint8 Index1, uint8 Index2, uint8 Alpha) : Index1(Index1), Index2(Index2), Alpha(Alpha)
	{

	}

	FVoxelMaterial(uint8 Index1, uint8 Index2, float Alpha) : Index1(Index1), Index2(Index2), Alpha(255 * FMath::Clamp(Alpha, 0.f, 1.f))
	{

	}

	FVoxelMaterial(FColor Color) : Index1(Color.R), Index2(Color.G), Alpha(Color.B)
	{

	}

	FORCEINLINE FColor ToFColor() const
	{
		return FColor(Index1, Index2, Alpha, 0);
	}

	FORCEINLINE uint8 GetMax() const
	{
		return Alpha < 0.5 ? Index1 : Index2;
	}

	FORCEINLINE void ReplaceMin(uint8 NewMin)
	{
		Alpha > 0.5 ? Index1 : Index2 = NewMin;
	}

	FORCEINLINE bool HasSameIndexesAs(FVoxelMaterial Other) const
	{
		return (Index1 == Other.Index1 && Index2 == Other.Index2)
			|| (Index1 == Other.Index2 && Index2 == Other.Index1);
	}
};