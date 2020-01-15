// Copyright 2020 Phyronnaz

#include "CppTranslation/VoxelVariables.h"
#include "VoxelNodes/VoxelExposedNodes.h"

FString FVoxelVariable::SanitizeName(const FString& InName)
{
	static const FString ValidChars = TEXT("_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	static const FString ValidStartChars = TEXT("_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	FString SanitizedName;

	for (int32 CharIdx = 0; CharIdx < InName.Len(); ++CharIdx)
	{
		FString Char = InName.Mid(CharIdx, 1);

		if (!ValidChars.Contains(*Char))
		{
			SanitizedName += TEXT("_");
		}
		else
		{
			SanitizedName += Char;
		}
	}

	if (SanitizedName == "None")
	{
		return "None1";
	}

	if (SanitizedName.IsEmpty() || SanitizedName == TEXT("_"))
	{
		return TEXT("Name");
	}

	if (ValidStartChars.Contains(*SanitizedName.Mid(0, 1)))
	{
		return SanitizedName;
	}
	else
	{
		return TEXT("_") + SanitizedName;
	}
}

FVoxelVariable::FVoxelVariable(const FString& Type, const FString& Name)
	: Type(Type)
	, Name(SanitizeName(Name))
{

}

FVoxelExposedVariable::FVoxelExposedVariable(
	const UVoxelExposedNode& Node,
	const FString& Type,
	const FString& ExposedType,
	const FString& DefaultValue)
	: FVoxelVariable(Type, Node.UniqueName.ToString())
	, Node(&Node)
	, DefaultValue(DefaultValue)
	, ExposedType(ExposedType.IsEmpty() ? Type : ExposedType)
	, Category(Node.Category.IsEmpty() ? "No Category" : Node.Category)
	, Tooltip(Node.Tooltip)
	, Priority(Node.Priority)
	, CustomMetaData(Node.CustomMetaData)
{

}

inline bool operator==(const TMap<FName, FString>& A, const TMap<FName, FString>& B)
{
	TArray<FName> KeysA;
	TArray<FName> KeysB;
	A.GetKeys(KeysA);
	B.GetKeys(KeysB);
#if ENGINE_MINOR_VERSION < 23
	KeysA.Sort();
	KeysB.Sort();
#else
	KeysA.Sort([](auto& InA, auto& InB) {return InA.FastLess(InB); });
	KeysB.Sort([](auto& InA, auto& InB) {return InA.FastLess(InB); });
#endif

	if (KeysA != KeysB)
	{
		return false;
	}
	for (auto& Key : KeysA)
	{
		if (A[Key] != B[Key])
		{
			return false;
		}
	}
	return true;
}

bool FVoxelExposedVariable::IsSameAs(const FVoxelExposedVariable& Other, bool bCheckNode) const
{
	return
		Type == Other.Type &&
		Name == Other.Name &&
		(!bCheckNode || Node == Other.Node) &&
		Priority == Other.Priority &&
		DefaultValue == Other.DefaultValue &&
		Category == Other.Category &&
		Tooltip == Other.Tooltip &&
		ExposedType == Other.ExposedType &&
		CustomMetaData == Other.CustomMetaData &&
		GetLocalVariableFromExposedOne() == Other.GetLocalVariableFromExposedOne() &&
		GetExposedVariableDefaultMetadata() == Other.GetExposedVariableDefaultMetadata();
}

FString FVoxelExposedVariable::GetMetadataString() const
{
	auto Metadata = GetExposedVariableDefaultMetadata();
	Metadata.Append(CustomMetaData);
	Metadata.KeySort([](auto& A, auto& B)
	{
#if ENGINE_MINOR_VERSION < 23
		return A < B;
#else
		return A.FastLess(B);
#endif
	});

	FString Result;
	for (auto& It : Metadata)
	{
		if (It.Key == STATIC_FNAME("UIMin") ||
			It.Key == STATIC_FNAME("UIMax"))
		{
			if (It.Value.IsEmpty())
			{
				continue;
			}
		}
		if (!Result.IsEmpty())
		{
			Result += ", ";
		}
		Result += It.Key.ToString();
		if (!It.Value.IsEmpty())
		{
			Result += "=";
			Result += "\"";
			Result += It.Value;
			Result += "\"";
		}
	}
	return Result;
}