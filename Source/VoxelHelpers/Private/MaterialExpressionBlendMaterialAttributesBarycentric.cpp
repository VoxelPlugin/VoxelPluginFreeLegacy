// Copyright 2020 Phyronnaz

#include "MaterialExpressionBlendMaterialAttributesBarycentric.h"
#include "MaterialCompiler.h"

UMaterialExpressionBlendMaterialAttributesBarycentric::UMaterialExpressionBlendMaterialAttributesBarycentric()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		FText NAME_MaterialAttributes;
		FConstructorStatics()
			: NAME_MaterialAttributes(NSLOCTEXT("Voxel", "Material Attributes", "Material Attributes"))
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
	int32 ResultAlphaA = AlphaA.Compile(Compiler);
	int32 ResultAlphaB = AlphaB.Compile(Compiler);

	return Compiler->Add
	(
		Compiler->Add
		(
			Compiler->Mul(ResultA, ResultAlphaA),
			Compiler->Mul(ResultB, ResultAlphaB)
		),
		Compiler->Mul
		(
			ResultC, 
			Compiler->Sub
			(
				Compiler->Constant(1.0f),
				Compiler->Add(ResultAlphaA, ResultAlphaB)
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
	Result.Add(&AlphaA);
	Result.Add(&AlphaB);
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
		return &AlphaA;
	}
	else if (InputIndex == 4)
	{
		return &AlphaB;
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