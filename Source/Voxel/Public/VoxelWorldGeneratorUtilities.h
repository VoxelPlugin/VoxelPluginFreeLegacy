// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelUtilities
{
	template<typename T>
	const TCHAR* GetWorldGeneratorOutputTypeName();
	
	template<>
	inline const TCHAR* GetWorldGeneratorOutputTypeName<v_flt>()
	{
		return TEXT("float");
	}
	template<>
	inline const TCHAR* GetWorldGeneratorOutputTypeName<int32>()
	{
		return TEXT("int");
	}
	template<>
	inline const TCHAR* GetWorldGeneratorOutputTypeName<FColor>()
	{
		return TEXT("color");
	}
	
	template<typename T, typename TWorldGenerator>
	inline FString GetMissingWorldGeneratorOutputErrorString(FName Name, const TWorldGenerator& WorldGenerator)
	{
		ensure(!WorldGenerator.template GetOutputsPtrMap<T>().Contains(Name));
		FString Types;
		for (auto& It : WorldGenerator.template GetOutputsPtrMap<T>())
		{
			if (!Types.IsEmpty()) Types += ", ";
			Types += It.Key.ToString();
		}
		return FString::Printf(
			TEXT("No world generator/voxel graph output named %s and with type %s found! Valid names: %s"),
			*Name.ToString(),
			GetWorldGeneratorOutputTypeName<T>(),
			*Types);
	}
}