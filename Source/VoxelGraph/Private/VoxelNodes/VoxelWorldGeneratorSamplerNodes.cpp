// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelWorldGeneratorSamplerNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelWorldGenerators/VoxelFlatWorldGenerator.h"
#include "VoxelGraphGenerator.h"
#include "VoxelWorldGeneratorInstance.inl"
#include "VoxelNodeFunctions.h"

EVoxelPinCategory UVoxelNode_WorldGeneratorSamplerBase::GetInputPinCategory(int32 PinIndex) const
{
	const int32 NumDefaultInputPins = Super::GetMinInputPins();
	if (PinIndex < NumDefaultInputPins)
	{
		return Super::GetInputPinCategory(PinIndex);
	}
	PinIndex -= NumDefaultInputPins;
	if (CustomData.IsValidIndex(PinIndex))
	{
		return EC::Float;
	}
	PinIndex -= CustomData.Num();
	if (Seeds.IsValidIndex(PinIndex))
	{
		return EC::Seed;
	}
	return EC::Float;
}

FName UVoxelNode_WorldGeneratorSamplerBase::GetInputPinName(int32 PinIndex) const
{
	const int32 NumDefaultInputPins = Super::GetMinInputPins();
	if (PinIndex < NumDefaultInputPins)
	{
		return Super::GetInputPinName(PinIndex);
	}
	PinIndex -= NumDefaultInputPins;
	if (CustomData.IsValidIndex(PinIndex))
	{
		return CustomData[PinIndex];
	}
	PinIndex -= CustomData.Num();
	if (Seeds.IsValidIndex(PinIndex))
	{
		return Seeds[PinIndex];
	}
	return "ERROR";
}

int32 UVoxelNode_WorldGeneratorSamplerBase::GetMinInputPins() const
{
	return Super::GetMinInputPins() + CustomData.Num() + Seeds.Num();
}

int32 UVoxelNode_WorldGeneratorSamplerBase::GetMaxInputPins() const
{
	return GetMinInputPins();
}


#if WITH_EDITOR
void UVoxelNode_WorldGeneratorSamplerBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Graph && GraphNode && PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SingleWorldGeneratorSamplerBase::UVoxelNode_SingleWorldGeneratorSamplerBase()
{
	WorldGenerator = UVoxelFlatWorldGenerator::StaticClass();

	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" });
}

FText UVoxelNode_SingleWorldGeneratorSamplerBase::GetTitle() const
{
	return FText::Format(
		VOXEL_LOCTEXT("World Generator: {0}"),
		FText::FromString(UniqueName.ToString()));
}

void UVoxelNode_SingleWorldGeneratorSamplerBase::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	if (!WorldGenerator.IsValid())
	{
		ErrorReporter.AddMessageToNode(this, "invalid world generator", EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_SingleWorldGeneratorSamplerBase::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if (auto* Prop = UE_25_SWITCH(Cast, CastField)<UStructProperty>(Property))
	{
		if (Prop->GetCPPType(nullptr, 0) == "FVoxelWorldGeneratorPicker")
		{
			WorldGenerator = *Prop->ContainerPtrToValuePtr<FVoxelWorldGeneratorPicker>(Object);
			return true;
		}
	}
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetWorldGeneratorValue::UVoxelNode_GetWorldGeneratorValue()
{
	SetOutputs({"", EC::Float, "Value"});
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetWorldGeneratorMaterial::UVoxelNode_GetWorldGeneratorMaterial()
{
	SetOutputs({ "", EC::Material, "Material" });
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetWorldGeneratorCustomOutput::UVoxelNode_GetWorldGeneratorCustomOutput()
{
	SetOutputs({ "", EC::Float, "Custom Output Value" });
}

FText UVoxelNode_GetWorldGeneratorCustomOutput::GetTitle() const
{
	return FText::FromString("Get World Generator Custom Output: " + OutputName.ToString());
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetCustomData::UVoxelNode_GetCustomData()
{
	SetOutputs(EC::Float);
}

FText UVoxelNode_GetCustomData::GetTitle() const
{
	return FText::FromString("Custom Data: " + Name.ToString());
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_IsCustomDataSet::UVoxelNode_IsCustomDataSet()
{
	SetOutputs(EC::Boolean);
}

FText UVoxelNode_IsCustomDataSet::GetTitle() const
{
	return FText::FromString("Is Custom Data Set: " + Name.ToString());
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

