// Copyright 2020 Phyronnaz

#include "VoxelGraphShortcuts.h"
#include "UnrealEdMisc.h"
#include "VoxelNodes/VoxelMathNodes.h"
#include "VoxelNodes/VoxelCoordinatesNodes.h"
#include "VoxelNodes/VoxelParameterNodes.h"

UVoxelGraphShortcuts::UVoxelGraphShortcuts()
{
	Shortcuts = {
	{EKeys::X, UVoxelNode_XF::StaticClass()},
	{EKeys::Y, UVoxelNode_YF::StaticClass()},
	{EKeys::Z, UVoxelNode_ZF::StaticClass()},
	{EKeys::One, UVoxelNode_FloatParameter::StaticClass()},

	{EKeys::Multiply, UVoxelNode_FMultiply::StaticClass()},
	{EKeys::Add, UVoxelNode_FAdd::StaticClass()},
	{EKeys::Subtract, UVoxelNode_FSubstract::StaticClass()},
	{EKeys::Divide, UVoxelNode_FDivide::StaticClass()},

	{EKeys::Asterix, UVoxelNode_FMultiply::StaticClass()},
	{EKeys::Slash, UVoxelNode_FDivide::StaticClass()},
	{EKeys::Hyphen, UVoxelNode_FSubstract::StaticClass()}
	};
}

void UVoxelGraphShortcuts::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!FUnrealEdMisc::Get().IsDeletePreferences())
	{
		SaveConfig();
	}
}