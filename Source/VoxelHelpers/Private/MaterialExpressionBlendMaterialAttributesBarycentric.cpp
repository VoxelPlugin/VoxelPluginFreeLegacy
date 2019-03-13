// Copyright 2019 Phyronnaz

#include "MaterialExpressionBlendMaterialAttributesBarycentric.h"
#include "MaterialCompiler.h"

#define LOCTEXT_NAMESPACE "Voxel"

UMaterialExpressionBlendMaterialAttributesBarycentric::UMaterialExpressionBlendMaterialAttributesBarycentric()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		FText NAME_MaterialAttributes;
		FConstructorStatics()
			: NAME_MaterialAttributes(LOCTEXT( "MaterialAttributes", "Material Attributes" ))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(ConstructorStatics.NAME_MaterialAttributes);

	Outputs.Reset();
	Outputs.Add(FExpressionOutput(TEXT(""), 0, 0, 0, 0, 0));
#endif
}

#if WITH_EDITOR
int32 UMaterialExpressionBlendMaterialAttributesBarycentric::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	const FGuid AttributeID = Compiler->GetMaterialAttribute();
	
	int32 ResultA = A.CompileWithDefault(Compiler, AttributeID);
	int32 ResultB = B.CompileWithDefault(Compiler, AttributeID);
	int32 ResultC = C.CompileWithDefault(Compiler, AttributeID);
	int32 ResultAlpha0 = Alpha0.Compile(Compiler);
	int32 ResultAlpha1 = Alpha1.Compile(Compiler);

	return Compiler->Add
	(
		Compiler->Add
		(
			Compiler->Mul(ResultA, ResultAlpha0),
			Compiler->Mul(ResultB, ResultAlpha1)
		),
		Compiler->Mul
		(
			ResultC, 
			Compiler->Sub
			(
				Compiler->Constant(1.0f),
				Compiler->Add(ResultAlpha0, ResultAlpha1)
			)
		)
	);
}

void UMaterialExpressionBlendMaterialAttributesBarycentric::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("BlendMaterialAttributesBarycentric"));
}

const TArray<FExpressionInput*> UMaterialExpressionBlendMaterialAttributesBarycentric::GetInputs()
{
	TArray<FExpressionInput*> Result;
	Result.Add(&A);
	Result.Add(&B);
	Result.Add(&C);
	Result.Add(&Alpha0);
	Result.Add(&Alpha1);
	return Result;
}

FExpressionInput* UMaterialExpressionBlendMaterialAttributesBarycentric::GetInput(int32 InputIndex)
{
	if (InputIndex == 0)
	{
		return &A;
	}
	else if (InputIndex == 1)
	{
		return &B;
	}
	else if (InputIndex == 2)
	{
		return &C;
	}
	else if (InputIndex == 4)
	{
		return &Alpha0;
	}
	else if (InputIndex == 4)
	{
		return &Alpha1;
	}

	return nullptr;
}

FName UMaterialExpressionBlendMaterialAttributesBarycentric::GetInputName(int32 InputIndex) const
{
	FName Name;

	switch (InputIndex)
	{
	case 0: Name = TEXT("A"); break;
	case 1: Name = TEXT("B"); break;
	case 2: Name = TEXT("C"); break;
	case 3: Name = TEXT("Alpha0"); break;
	case 4: Name = TEXT("Alpha1"); break;
	};

	return Name;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE