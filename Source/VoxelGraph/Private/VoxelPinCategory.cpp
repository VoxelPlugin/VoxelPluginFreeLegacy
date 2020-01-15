// Copyright 2020 Phyronnaz

#include "VoxelPinCategory.h"
#include "Runtime/VoxelNodeType.h"

static const FName PC_Exec(TEXT("exec"));
static const FName PC_Boolean(TEXT("bool"));
static const FName PC_Int(TEXT("int"));
static const FName PC_Float(TEXT("float"));
static const FName PC_Material(TEXT("FVoxelMaterial"));
static const FName PC_Color(TEXT("Color"));
static const FName PC_Seed(TEXT("Seed"));
static const FName PC_Wildcard(TEXT("Wildcard"));

EVoxelPinCategory FVoxelPinCategory::DataPinToPin(EVoxelDataPinCategory Category)
{
	switch (Category)
	{
	case EVoxelDataPinCategory::Boolean:
		return EVoxelPinCategory::Boolean;
	case EVoxelDataPinCategory::Int:
		return EVoxelPinCategory::Int;
	case EVoxelDataPinCategory::Float:
		return EVoxelPinCategory::Float;
	case EVoxelDataPinCategory::Material:
		return EVoxelPinCategory::Material;
	case EVoxelDataPinCategory::Color:
		return EVoxelPinCategory::Color;
	default:
		check(false);
		return EVoxelPinCategory::Boolean;
	}
}

EVoxelPinCategory FVoxelPinCategory::FromString(const FName& String)
{
	if (String == PC_Exec)
	{
		return EVoxelPinCategory::Exec;
	}
	else if (String == PC_Boolean)
	{
		return EVoxelPinCategory::Boolean;
	}
	else if (String == PC_Int)
	{
		return EVoxelPinCategory::Int;
	}
	else if (String == PC_Float)
	{
		return EVoxelPinCategory::Float;
	}
	else if (String == PC_Material)
	{
		return EVoxelPinCategory::Material;
	}
	else if (String == PC_Color)
	{
		return EVoxelPinCategory::Color;
	}
	else if (String == PC_Seed)
	{
		return EVoxelPinCategory::Seed;
	}
	else if (String == PC_Wildcard)
	{
		return EVoxelPinCategory::Wildcard;
	}
	else
	{
		ensure(false);
		return EVoxelPinCategory::Exec;
	}
}

FName FVoxelPinCategory::GetName(EVoxelPinCategory Category)
{
	switch (Category)
	{
	case EVoxelPinCategory::Exec:
		return PC_Exec;
	case EVoxelPinCategory::Boolean:
		return PC_Boolean;
	case EVoxelPinCategory::Int:
		return PC_Int;
	case EVoxelPinCategory::Float:
		return PC_Float;
	case EVoxelPinCategory::Material:
		return PC_Material;
	case EVoxelPinCategory::Color:
		return PC_Color;
	case EVoxelPinCategory::Seed:
		return PC_Seed;
	case EVoxelPinCategory::Wildcard:
		return PC_Wildcard;
	default:
		check(false);
		return FName();
	}
}

FString FVoxelPinCategory::GetDefaultValue(EVoxelPinCategory Category)
{
	switch (Category)
	{
	case EVoxelPinCategory::Exec:
		return FString();
	case EVoxelPinCategory::Boolean:
		return FString();
	case EVoxelPinCategory::Int:
		return TEXT("0");
	case EVoxelPinCategory::Float:
		return TEXT("0");
	case EVoxelPinCategory::Material:
		return FString();
	case EVoxelPinCategory::Color:
		return FString();
	case EVoxelPinCategory::Seed:
		return TEXT("1337");
	case EVoxelPinCategory::Wildcard:
		return FString();
	default:
		check(false);
		return FString();
	}
}

FString FVoxelPinCategory::GetDefaultValue(EVoxelDataPinCategory Category)
{
	const EVoxelPinCategory PinCategory = DataPinToPin(Category);
	return GetDefaultValue(PinCategory);
}

FString FVoxelPinCategory::GetTypeString(EVoxelPinCategory Category)
{
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		return "bool";
	case EVoxelPinCategory::Int:
		return "int32";
	case EVoxelPinCategory::Float:
		return "v_flt";
	case EVoxelPinCategory::Material:
		return "FVoxelMaterial";
	case EVoxelPinCategory::Color:
		return "FColor";
	case EVoxelPinCategory::Seed:
		return "Seed";
	case EVoxelPinCategory::Wildcard:
	case EVoxelPinCategory::Exec:
	default:
		check(false);
		return "";
	}
}

FString FVoxelPinCategory::GetRangeTypeString(EVoxelPinCategory Category)
{
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		return "FVoxelBoolRange";
	case EVoxelPinCategory::Int:
		return "TVoxelRange<int32>";
	case EVoxelPinCategory::Float:
		return "TVoxelRange<v_flt>";
	case EVoxelPinCategory::Material:
		return "FVoxelMaterialRange";
	case EVoxelPinCategory::Color:
		return "FVoxelColorRange";
	case EVoxelPinCategory::Seed:
		return "Seed";
	case EVoxelPinCategory::Wildcard:
	case EVoxelPinCategory::Exec:
	default:
		check(false);
		return "";
	}
}

FVoxelNodeType FVoxelPinCategory::ConvertDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue)
{
	FVoxelNodeType Value;
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		Value.Get<bool>() = DefaultValue.ToBool();
		break;
	case EVoxelPinCategory::Int:
		Value.Get<int32>() = FCString::Atoi(*DefaultValue);
		break;
	case EVoxelPinCategory::Float:
		Value.Get<v_flt>() = FCString::Atof(*DefaultValue);
		break;
	case EVoxelPinCategory::Material:
		Value.Get<FVoxelMaterial>() = FVoxelMaterial::Default();
		break;
	case EVoxelPinCategory::Color:
	{
		FLinearColor Color;
		if (!Color.InitFromString(DefaultValue))
		{
			Color = FColor();
		}
		Value.Get<FColor>() = Color.ToFColor(false);
		break;
	}
	case EVoxelPinCategory::Seed:
		Value.Get<int32>() = FCString::Atoi(*DefaultValue);
		break;
	case EVoxelPinCategory::Exec:
	default:
		check(false);
		Value.Get<v_flt>() = 0;
	}
	return Value;
}

FVoxelNodeRangeType FVoxelPinCategory::ConvertRangeDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue)
{
	FVoxelNodeRangeType RangeValue;
	const FVoxelNodeType Value = ConvertDefaultValue(Category, DefaultValue);
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		RangeValue.Get<bool>() = Value.Get<bool>();
		break;
	case EVoxelPinCategory::Int:
		RangeValue.Get<int32>() = Value.Get<int32>();
		break;
	case EVoxelPinCategory::Float:
		RangeValue.Get<v_flt>() = Value.Get<v_flt>();
		break;
	case EVoxelPinCategory::Material:
		RangeValue.Get<FVoxelMaterial>() = Value.Get<FVoxelMaterial>();
		break;
	case EVoxelPinCategory::Color:
		RangeValue.Get<FColor>() = Value.Get<FColor>();
		break;
	case EVoxelPinCategory::Seed:
		RangeValue.Get<int32>() = Value.Get<int32>();
		break;
	case EVoxelPinCategory::Exec:
	default:
		RangeValue.Get<bool>() = false;
		check(false);
	}
	return RangeValue;
}

FString FVoxelPinCategory::ConvertStringDefaultValue(EVoxelPinCategory Category, const FString& DefaultValue)
{
	const FVoxelNodeType Value = ConvertDefaultValue(Category, DefaultValue);
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		return Value.Get<bool>() ? TEXT("true") : TEXT("false");
	case EVoxelPinCategory::Int:
		return FString::FromInt(Value.Get<int32>());
	case EVoxelPinCategory::Float:
		return FString::SanitizeFloat(Value.Get<v_flt>()) + "f";
	case EVoxelPinCategory::Material:
		return TEXT("FVoxelMaterial(ForceInit)");
	case EVoxelPinCategory::Color:
		return FString::Printf(TEXT("FColor(%d, %d, %d, %d)"),
			Value.Get<FColor>().R,
			Value.Get<FColor>().G,
			Value.Get<FColor>().B,
			Value.Get<FColor>().A);
	case EVoxelPinCategory::Seed:
		return FString::FromInt(Value.Get<int32>());
	case EVoxelPinCategory::Exec:
	default:
		check(false);
		return "";
	}
}

FString FVoxelPinCategory::ToString(EVoxelPinCategory Category, FVoxelNodeType Value)
{
	switch (Category)
	{
	case EVoxelPinCategory::Boolean:
		return LexToString(Value.Get<bool>());
	case EVoxelPinCategory::Int:
		return LexToString(Value.Get<int32>());
	case EVoxelPinCategory::Float:
		return LexToString(Value.Get<v_flt>());
	case EVoxelPinCategory::Seed:
		return LexToString(Value.Get<Seed>());
	case EVoxelPinCategory::Color:
		return FLinearColor(
			Value.Get<FColor>().R / 255.999f,
			Value.Get<FColor>().G / 255.999f,
			Value.Get<FColor>().B / 255.999f,
			Value.Get<FColor>().A / 255.999f).ToString();
	case EVoxelPinCategory::Exec:
	case EVoxelPinCategory::Material:
	default:
		check(false);
		return "";
	}
}