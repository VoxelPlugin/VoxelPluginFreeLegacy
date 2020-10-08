// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelUtilities
{
	template<typename T>
	const TCHAR* GetGeneratorOutputTypeName();
	
	template<>
	inline const TCHAR* GetGeneratorOutputTypeName<v_flt>()
	{
		return TEXT("float");
	}
	template<>
	inline const TCHAR* GetGeneratorOutputTypeName<int32>()
	{
		return TEXT("int");
	}
	template<>
	inline const TCHAR* GetGeneratorOutputTypeName<FColor>()
	{
		return TEXT("color");
	}
	
	template<typename T, typename TGenerator>
	inline FString GetMissingGeneratorOutputErrorString(FName Name, const TGenerator& Generator)
	{
		ensure(!Generator.template GetOutputsPtrMap<T>().Contains(Name));
		FString Types;
		for (auto& It : Generator.template GetOutputsPtrMap<T>())
		{
			if (!Types.IsEmpty()) Types += ", ";
			Types += It.Key.ToString();
		}
		return FString::Printf(
			TEXT("No voxel generator/voxel graph output named %s and with type %s found! Valid names: %s"),
			*Name.ToString(),
			GetGeneratorOutputTypeName<T>(),
			*Types);
	}
}