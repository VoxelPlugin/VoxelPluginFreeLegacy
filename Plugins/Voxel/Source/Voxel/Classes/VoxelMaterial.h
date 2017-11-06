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

	FVoxelMaterial(FColor Color) : Index1(Color.R), Index2(Color.G), Alpha(Color.B)
	{

	}

	FORCEINLINE FColor ToFColor(uint8 AmbientOcclusion = 255) const
	{
		return FColor(Index1, Index2, Alpha, AmbientOcclusion);
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
		return (Index1 == Other.Index1) && (Index2 == Other.Index2);
	}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FVoxelMaterial& Material)
{
	Ar << Material.Index1;
	Ar << Material.Index2;
	Ar << Material.Alpha;

	return Ar;
}