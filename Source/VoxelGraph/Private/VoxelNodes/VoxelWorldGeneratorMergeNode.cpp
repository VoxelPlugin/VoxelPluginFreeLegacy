// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelWorldGeneratorMergeNode.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelGraphOutputsConfig.h"
#include "VoxelNodeFunctions.h"

constexpr int32 NumDefaultInputPins_WGMN = 3 + 2 * 4;

inline TArray<FName> GetFloatOutputs(UVoxelGraphOutputsConfig* Config)
{
	TArray<FName> Result;
	if (Config)
	{
		for (auto& Output : Config->Outputs)
		{
			if (Output.Category == EVoxelDataPinCategory::Float)
			{
				Result.Add(Output.Name);
			}
		}
	}
	return Result;
}


UVoxelNode_WorldGeneratorMerge::UVoxelNode_WorldGeneratorMerge()
{
	SetInputs({
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Index 0", EC::Int, "First generator index" },
		{ "Alpha 0", EC::Float, "First generator alpha" },
		{ "Index 1", EC::Int, "Second generator index" },
		{ "Alpha 1", EC::Float, "Second generator alpha" },
		{ "Index 2", EC::Int, "Third generator index" },
		{ "Alpha 2", EC::Float, "Third generator alpha" },
		{ "Index 3", EC::Int, "Fourth generator index" },
		{ "Alpha 3", EC::Float, "Fourth generator alpha" } });
	check(UVoxelNodeHelper::GetMinInputPins() == NumDefaultInputPins_WGMN);
}

FText UVoxelNode_WorldGeneratorMerge::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("World Generator Merge: {0}"), Super::GetTitle());
}

int32 UVoxelNode_WorldGeneratorMerge::GetOutputPinsCount() const
{
	return 2 + GetFloatOutputs(Outputs).Num() + 1;
}

FName UVoxelNode_WorldGeneratorMerge::GetOutputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "Value";
	}
	if (PinIndex == 1)
	{
		return "Material";
	}
	if (PinIndex == GetOutputPinsCount() - 1)
	{
		return "Num Queried Generators";
	}
	PinIndex -= 2;
	auto FloatOutputs = GetFloatOutputs(Outputs);
	if (FloatOutputs.IsValidIndex(PinIndex))
	{
		return FloatOutputs[PinIndex];
	}
	return "Error";
}

EVoxelPinCategory UVoxelNode_WorldGeneratorMerge::GetOutputPinCategory(int32 PinIndex) const
{
	if (PinIndex == 1)
	{
		return EC::Material;
	}
	else if (PinIndex == GetOutputPinsCount() - 1)
	{
		return EC::Int;
	}
	else
	{
		return EC::Float;
	}
}

void UVoxelNode_WorldGeneratorMerge::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	for (auto& WorldGenerator : WorldGenerators)
	{
		if (!WorldGenerator.IsValid())
		{
			ErrorReporter.AddMessageToNode(this, "invalid world generator", EVoxelGraphNodeMessageType::FatalError);
		}
	}
}

#if WITH_EDITOR
bool UVoxelNode_WorldGeneratorMerge::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if(auto* ArrayProperty = UE_25_SWITCH(Cast, CastField)<UArrayProperty>(Property))
	{
		if (ArrayProperty->Inner->GetCPPType(nullptr, 0) == "FVoxelWorldGeneratorPicker")
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Object));
			WorldGenerators.SetNum(ArrayHelper.Num());
			for (int32 Index = 0; Index < ArrayHelper.Num(); Index++)
			{
				WorldGenerators[Index] = *reinterpret_cast<FVoxelWorldGeneratorPicker*>(ArrayHelper.GetRawPtr(Index));
			}
			return true;
		}
	}
	return false;
}
#endif