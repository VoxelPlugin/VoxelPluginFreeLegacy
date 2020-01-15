// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelWhiteNoiseNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelGraphGenerator.h"
#include "FastNoise.h"


UVoxelNode_2DWhiteNoise::UVoxelNode_2DWhiteNoise()
{
	SetInputs(
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"},
		{"Seed", EC::Seed, "Seed"});
	SetOutputs(EC::Float);
}


UVoxelNode_3DWhiteNoise::UVoxelNode_3DWhiteNoise()
{
	SetInputs(
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"},
		{"Z", EC::Float, "Z"},
		{"Seed", EC::Seed, "Seed"});
	SetOutputs(EC::Float);
}

