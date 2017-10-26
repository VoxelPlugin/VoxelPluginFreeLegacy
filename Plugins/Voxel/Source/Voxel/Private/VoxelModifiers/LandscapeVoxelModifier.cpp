// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelLandscapeAsset.h"
#include "LandscapeVoxelModifier.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"

ALandscapeVoxelModifier::ALandscapeVoxelModifier()
{
};

void ALandscapeVoxelModifier::InitExportedLandscape(TArray<float>& Heights, TArray<FVoxelMaterial>& Materials, int Size)
{
	ExportedLandscape->Init(Heights, Materials, Size);
}
