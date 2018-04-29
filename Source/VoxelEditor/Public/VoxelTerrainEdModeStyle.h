// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class FVoxelTerrainEdModeStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static TSharedPtr< class ISlateStyle > Get();
	static const FName GetStyleSetName();

private:
	static TSharedPtr< class FSlateStyleSet > StyleSet;

	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
};