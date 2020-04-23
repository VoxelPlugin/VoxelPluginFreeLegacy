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

