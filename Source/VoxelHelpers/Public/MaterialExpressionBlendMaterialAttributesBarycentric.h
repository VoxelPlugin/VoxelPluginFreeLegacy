// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionBlendMaterialAttributesBarycentric.generated.h"

// Returns A * AlphaA + B * AlphaB + C * (1 - AlphaA - AlphaB)
UCLASS(CollapseCategories, HideCategories = Object, MinimalAPI)
class UMaterialExpressionBlendMaterialAttributesBarycentric : public UMaterialExpression
{
	GENERATED_BODY()
		
public:
	UMaterialExpressionBlendMaterialAttributesBarycentric();

	UPROPERTY()
 	FMaterialAttributesInput A;

	UPROPERTY()
 	FMaterialAttributesInput B;
	
	UPROPERTY()
 	FMaterialAttributesInput C;

	UPROPERTY()
	FExpressionInput AlphaA;

	UPROPERTY()
	FExpressionInput AlphaB;

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual int32 Compile(class FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual const TArray<FExpressionInput*> GetInputs()override;
	virtual FExpressionInput* GetInput(int32 InputIndex)override;
	virtual FName GetInputName(int32 InputIndex) const override;
	virtual bool IsInputConnectionRequired(int32 InputIndex) const override { return true; }
	virtual bool IsResultMaterialAttributes(int32 OutputIndex) override { return true; }
	virtual uint32 GetInputType(int32 InputIndex) override { return InputIndex > 2 ? MCT_Float1 : MCT_MaterialAttributes; }
#endif
	//~ End UMaterialExpression Interface
};
