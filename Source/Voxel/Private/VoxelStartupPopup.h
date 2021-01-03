// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class FVoxelStartupPopup
{
public:
	static void OnModuleStartup();

private:
	void ShowPopup();
};