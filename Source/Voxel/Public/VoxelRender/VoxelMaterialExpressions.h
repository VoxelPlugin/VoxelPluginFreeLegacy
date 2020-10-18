// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionLandscapeLayerSample.h"
#include "Materials/MaterialExpressionLandscapeLayerSwitch.h"
#include "Materials/MaterialExpressionLandscapeLayerWeight.h"
#include "Materials/MaterialExpressionLandscapeVisibilityMask.h"
#include "VoxelMaterialExpressions.generated.h"

#if WITH_EDITOR
#define FORWARD_CLASS() \
	public: \
		virtual void GetCaption(TArray<FString>& OutCaptions) const override; \
		virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
#else
#define FORWARD_CLASS()
#endif

UCLASS()
class VOXEL_API UMaterialExpressionVoxelLandscapeLayerBlend : public UMaterialExpressionLandscapeLayerBlend
{
	GENERATED_BODY()
	FORWARD_CLASS()
};

UCLASS()
class VOXEL_API UMaterialExpressionVoxelLandscapeLayerSwitch : public UMaterialExpressionLandscapeLayerSwitch
{
	GENERATED_BODY()
	FORWARD_CLASS()
};

UCLASS()
class VOXEL_API UMaterialExpressionVoxelLandscapeLayerWeight : public UMaterialExpressionLandscapeLayerWeight
{
	GENERATED_BODY()
	FORWARD_CLASS()
};

UCLASS()
class VOXEL_API UMaterialExpressionVoxelLandscapeLayerSample : public UMaterialExpressionLandscapeLayerSample
{
	GENERATED_BODY()
	FORWARD_CLASS()
};

UCLASS()
class VOXEL_API UMaterialExpressionVoxelLandscapeVisibilityMask : public UMaterialExpressionLandscapeVisibilityMask
{
	GENERATED_BODY()
	FORWARD_CLASS()
};

#undef FORWARD_CLASS

#if WITH_EDITOR
struct VOXEL_API FVoxelMaterialExpressionUtilities
{
	static UClass* GetVoxelExpression(UClass* LandscapeExpression)
	{
		static TMap<UClass*, UClass*> Map =
		{
	#define MAPPING(Name) \
		{ \
			UMaterialExpression ## Name::StaticClass(), \
			UMaterialExpressionVoxel ## Name::StaticClass(), \
		}
		MAPPING(LandscapeLayerBlend),
		MAPPING(LandscapeLayerSwitch),
		MAPPING(LandscapeLayerWeight),
		MAPPING(LandscapeLayerSample),
		MAPPING(LandscapeVisibilityMask)
	#undef MAPPING
		};

		return Map.FindRef(LandscapeExpression);
	}
	static bool NeedsToBeConvertedToVoxel(const TArray<UMaterialExpression*>& Expressions);
};
#endif