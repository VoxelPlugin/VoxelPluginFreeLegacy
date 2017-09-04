// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "VoxelLODProfile.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelLODProfile : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		FRuntimeFloatCurve LODCurve;

	UVoxelLODProfile()
	{
		LODCurve.EditorCurveData.AddKey(0, 0);
		LODCurve.EditorCurveData.AddKey(10, 10);
	};

	float GetLODAt(float Distance) const
	{
		return LODCurve.GetRichCurveConst()->Eval(Distance);
	}
};