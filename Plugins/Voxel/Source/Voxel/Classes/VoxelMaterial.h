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
		float Alpha;

	FVoxelMaterial() : Index1(0), Index2(0), Alpha(0)
	{

	}

	FVoxelMaterial(uint8 Index1, uint8 Index2, float Alpha) : Index1(Index1), Index2(Index2), Alpha(FMath::Clamp(Alpha, 0.f, 1.f))
	{

	}

	FVoxelMaterial(FColor Color) : Index1(Color.R), Index2(Color.G), Alpha(Color.B / 256.f)
	{

	}

	FColor ToFColor() const
	{
		return FColor(Index1, Index2, 255 * Alpha, 0);
	}

	uint8 GetMax() const
	{
		return Alpha < 0.5 ? Index1 : Index2;
	}

	void ReplaceMin(uint8 NewMin)
	{
		Alpha > 0.5 ? Index1 : Index2 = NewMin;
	}
};