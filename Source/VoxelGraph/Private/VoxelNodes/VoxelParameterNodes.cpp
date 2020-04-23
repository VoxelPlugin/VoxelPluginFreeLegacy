// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelParameterNodes.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelGraphGenerator.h"

UVoxelNode_FloatParameter::UVoxelNode_FloatParameter()
{
	SetOutputs(EC::Float);
}


FString UVoxelNode_FloatParameter::GetValueString() const
{
	return FString::SanitizeFloat(Value);
}

FLinearColor UVoxelNode_FloatParameter::GetNotExposedColor() const
{
	return FVoxelNodeColors::FloatNode;
}

#if WITH_EDITOR
bool UVoxelNode_FloatParameter::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if (auto* Prop = UE_25_SWITCH(Cast, CastField)<UFloatProperty>(Property))
	{
		Value = *Prop->ContainerPtrToValuePtr<float>(Object);
		return true;
	}
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_IntParameter::UVoxelNode_IntParameter()
{
	SetOutputs(EC::Int);
}


FString UVoxelNode_IntParameter::GetValueString() const
{
	return FString::FromInt(Value);
}

FLinearColor UVoxelNode_IntParameter::GetNotExposedColor() const
{
	return FVoxelNodeColors::IntNode;
}

#if WITH_EDITOR
bool UVoxelNode_IntParameter::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if (auto* Prop = UE_25_SWITCH(Cast, CastField)<UIntProperty>(Property))
	{
		Value = *Prop->ContainerPtrToValuePtr<int32>(Object);
		return true;
	}
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_ColorParameter::UVoxelNode_ColorParameter()
{
	SetOutputs(EC::Color);
}


FString UVoxelNode_ColorParameter::GetValueString() const
{
	return FString::Printf(TEXT("%.2g,%.2g,%.2g,%.2g"), Color.R, Color.G, Color.B, Color.A);
}

FLinearColor UVoxelNode_ColorParameter::GetNotExposedColor() const
{
	return FVoxelNodeColors::ColorNode;
}

#if WITH_EDITOR
bool UVoxelNode_ColorParameter::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if (auto* Prop = UE_25_SWITCH(Cast, CastField)<UStructProperty>(Property))
	{
		if (Prop->GetCPPType(nullptr, 0) == "FLinearColor")
		{
			Color = *Prop->ContainerPtrToValuePtr<FLinearColor>(Object);
			return true;
		}
	}
	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_BoolParameter::UVoxelNode_BoolParameter()
{
	SetOutputs(EC::Boolean);
}


FString UVoxelNode_BoolParameter::GetValueString() const
{
	return LexToString(Value);
}

FLinearColor UVoxelNode_BoolParameter::GetNotExposedColor() const
{
	return FVoxelNodeColors::BoolNode;
}

#if WITH_EDITOR
bool UVoxelNode_BoolParameter::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	if (auto* Prop = UE_25_SWITCH(Cast, CastField)<UBoolProperty>(Property))
	{
		auto* Data = Prop->ContainerPtrToValuePtr<bool>(Object);
		Value = Prop->GetPropertyValue(Data);
		return true;
	}
	return false;
}
#endif