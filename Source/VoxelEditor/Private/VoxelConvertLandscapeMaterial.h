// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UMaterial;
class UMaterialFunction;
class UMaterialExpression;

class FVoxelConvertLandscapeMaterial
{
public:
	static void Init();

	static void ConvertMaterial(UMaterial* Material);
	
	static int32 ConvertExpressions(UObject* Owner, const TArray<UMaterialExpression*>& Expressions, TSet<UMaterialFunction*>& VisitedFunctions);
	static void ConvertExpression(UObject* Owner, UMaterialExpression* Expression, UClass* NewClass);
};