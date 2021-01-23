// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelEditorUtilities
{
	VOXELEDITOR_API UObject* CreateAsset(FString AssetName, FString FolderPath, UClass* Class, FString Suffix = "");

	template<typename T>
	T* CreateAsset(FString AssetName, FString FolderPath, UClass* Class = nullptr, FString Suffix = "")
	{
		if (!Class)
		{
			Class = T::StaticClass();
		}
		return CastChecked<T>(CreateAsset(AssetName, FolderPath, Class, Suffix), ECastCheckedType::NullAllowed);
	}
}