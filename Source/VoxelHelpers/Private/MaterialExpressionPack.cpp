// Copyright 2020 Phyronnaz

#include "MaterialExpressionPack.h"
#include "MaterialCompiler.h"
#include "EdGraph/EdGraphNode.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionReroute.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"

UMaterialExpressionPack::UMaterialExpressionPack()
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(INVTEXT("Packing"));
#endif
}

#if WITH_EDITOR
void UMaterialExpressionPack::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (PropertyChangedEvent.MemberProperty) // Else crash when calling reconstruct node
	{
		// There's a bug when copy pasting arrays with default values at the end
		for (auto& Input : Inputs)
		{
			if (Input.InputName == "DEFAULT_DO_NOT_USE")
			{
				Input.InputName = {};
			}
		}
		
		if (GraphNode)
		{
			GraphNode->ReconstructNode();
		}

		OnPostEditChangeProperty.Broadcast();
	}
}

int32 UMaterialExpressionPack::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (!FMaterialExpressionUnpackScope::GetQueuedElement())
	{
		Compiler->Error(TEXT("Pack is not followed by an Unpack node!"));
		return -1;
	}
	
	const auto Element = FMaterialExpressionUnpackScope::Pop();
	if (!Element.Unpack->MatchesPack(this))
	{
		Compiler->Error(TEXT("Unpack is out of sync! Click it and press Refresh"));
		return -1;
	}
	
	if (!ensure(Inputs.IsValidIndex(Element.OutputIndex)))
	{
		Compiler->Error(TEXT("Pack: Invalid OutputIndex! (INTERNAL ERROR)"));
		return -1;
	}

	return Inputs[Element.OutputIndex].Input.Compile(Compiler);
}

void UMaterialExpressionPack::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Pack"));
}

const TArray<FExpressionInput*> UMaterialExpressionPack::GetInputs()
{
	TArray<FExpressionInput*> Result;
	for (auto& Input : Inputs)
	{
		Result.Add(&Input.Input);
	}
	return Result;
}

FExpressionInput* UMaterialExpressionPack::GetInput(int32 InputIndex)
{
	if (Inputs.IsValidIndex(InputIndex))
	{
		return &Inputs[InputIndex].Input;
	}
	return nullptr;
}

FName UMaterialExpressionPack::GetInputName(int32 InputIndex) const
{
	if (Inputs.IsValidIndex(InputIndex))
	{
		return Inputs[InputIndex].InputName;
	}
	return {};
}
#endif // WITH_EDITOR

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialExpressionUnpack::UMaterialExpressionUnpack()
{
#if WITH_EDITORONLY_DATA
	MenuCategories.Add(INVTEXT("Packing"));
	Outputs.Reset();

	bShowOutputNameOnPin = true;
#endif
}

#if WITH_EDITOR
void UMaterialExpressionUnpack::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty) // Else crash when calling reconstruct node
	{
		RefreshPack();
	}
}

int32 UMaterialExpressionUnpack::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (FMaterialExpressionUnpackScope::GetQueuedElement())
	{
		Compiler->Error(TEXT("Multiple unpacks are used with no pack in-between! This is not supported"));
		return -1;
	}

	static TMap<int32, FGuid> OutputIndicesGUIDs;
	FGuid& OutputGUID = OutputIndicesGUIDs.FindOrAdd(OutputIndex);
	if (!OutputGUID.IsValid())
	{
		OutputGUID = FGuid::NewGuid();
	}

	// If we don't do that, the output will be cached & the same thing will be used for all output indices
	FScopedMaterialCompilerAttribute AttributeScope(Compiler, OutputGUID);
	FMaterialExpressionUnpackScope UnpackScope(this, OutputIndex);
	
	const int32 Ret = Input.Compile(Compiler);

	if (Ret != -1)
	{
		if (Compiler->GetType(Ret) == MCT_StaticBool)
		{
			// Messes up the instances, due to the code in FMaterialEditorUtilities::GetStaticSwitchExpressionValue
			Compiler->Error(TEXT("Pack/Unpack nodes cannot be used with static bools"));
			return -1;
		}

		if (Input.Expression && !Input.Expression->IsResultMaterialAttributes(0))
		{
			// If Ret is -1, reroute nodes will return false
			Compiler->Error(TEXT("Pack/Unpack nodes can only be connected to material attributes function input/output"));
		}
	}

	return Ret;
}

void UMaterialExpressionUnpack::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add(TEXT("Unpack"));
}

void UMaterialExpressionUnpack::RefreshPack()
{
	bRefresh = false;

	Outputs.Reset();
	auto* Pack = GetPack_NotForCompile();
	if (Pack)
	{
		for (const auto& PackInput : Pack->Inputs)
		{
			Outputs.Add(FExpressionOutput(PackInput.InputName));
		}

		if (!Pack->OnPostEditChangeProperty.IsBoundToObject(this))
		{
			Pack->OnPostEditChangeProperty.AddUObject(this, &UMaterialExpressionUnpack::RefreshPack);
		}
	}
		
	if (GraphNode)
	{
		GraphNode->ReconstructNode();
	}
}

UMaterialExpressionPack* UMaterialExpressionUnpack::GetPack_NotForCompile() const
{
	UMaterialExpression* Expression = Input.Expression;
	TSet<UMaterialExpression*> VisitedExpressions;
	while (!VisitedExpressions.Contains(Expression))
	{
		VisitedExpressions.Add(Expression);

		if (auto* Pack = Cast<UMaterialExpressionPack>(Expression))
		{
			return Pack;
		}
		if (auto* Reroute = Cast<UMaterialExpressionReroute>(Expression))
		{
			Expression = Reroute->Input.Expression;
		}
		if (auto* FunctionInput = Cast<UMaterialExpressionFunctionInput>(Expression))
		{
			Expression = FunctionInput->Preview.Expression;
		}
		if (auto* FunctionOutput = Cast<UMaterialExpressionFunctionOutput>(Expression))
		{
			Expression = FunctionOutput->A.Expression;
		}
		if (auto* FunctionCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expression))
		{
			if (ensure(FunctionCall->FunctionOutputs.IsValidIndex(Input.OutputIndex)))
			{
				const auto& FunctionOutput = FunctionCall->FunctionOutputs[Input.OutputIndex];
				if (ensure(FunctionOutput.ExpressionOutput)) // If this gets raised, the ExpressionOutput is not cached and needs to be
				{
					Expression = FunctionOutput.ExpressionOutput;
				}
			}
		}
	}
	return nullptr;
}

bool UMaterialExpressionUnpack::MatchesPack(UMaterialExpressionPack* Pack)
{
	check(Pack);

	if (Pack->Inputs.Num() != Outputs.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < Outputs.Num(); Index++)
	{
		if (Pack->Inputs[Index].InputName != Outputs[Index].OutputName)
		{
			return false;
		}
	}
	return true;
}
#endif // WITH_EDITOR

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FMaterialExpressionUnpackScope::FMaterialExpressionUnpackScope(UMaterialExpressionUnpack* Unpack, int32 OutputIndex)
{
	check(Unpack);
	check(!QueuedElement.IsValid());
	
	QueuedElement = MakeUnique<FElement>(FElement{ Unpack, OutputIndex });
}

FMaterialExpressionUnpackScope::~FMaterialExpressionUnpackScope()
{
	// Might already have been reset
	QueuedElement.Reset();
}

TUniquePtr<FMaterialExpressionUnpackScope::FElement> FMaterialExpressionUnpackScope::QueuedElement;