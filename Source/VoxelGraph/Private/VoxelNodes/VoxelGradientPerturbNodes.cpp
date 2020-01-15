// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelGradientPerturbNodes.h"
#include "VoxelContext.h"

UVoxelNode_2DGradientPerturb::UVoxelNode_2DGradientPerturb()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Frequency", EC::Float, "The frequency of the noise" },
		{ "Amplitude", EC::Float, "The amplitude of the perturbation, in the same unit as the input" },
		{ "Seed", EC::Seed, "Seed" });
	SetOutputs(
		{ "X", EC::Float, "X with perturbation" },
		{ "Y", EC::Float, "Y with perturbation" });
}

UVoxelNode_2DGradientPerturbFractal::UVoxelNode_2DGradientPerturbFractal()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Frequency", EC::Float, "The frequency of the noise" },
		{ "Amplitude", EC::Float, "The amplitude of the perturbation, in the same unit as the input" },
		{ "Seed", EC::Seed, "Seed" });
	SetOutputs(
		{ "X", EC::Float, "X with perturbation" },
		{ "Y", EC::Float, "Y with perturbation" });
}

UVoxelNode_3DGradientPerturb::UVoxelNode_3DGradientPerturb()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Frequency", EC::Float, "The frequency of the noise" },
		{ "Amplitude", EC::Float, "The amplitude of the perturbation, in the same unit as the input" },
		{ "Seed", EC::Seed, "Seed" });
	SetOutputs(
		{ "X", EC::Float, "X with perturbation" },
		{ "Y", EC::Float, "Y with perturbation" },
		{ "Z", EC::Float, "Z with perturbation" });
}

UVoxelNode_3DGradientPerturbFractal::UVoxelNode_3DGradientPerturbFractal()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Frequency", EC::Float, "The frequency of the noise" },
		{ "Amplitude", EC::Float, "The amplitude of the perturbation, in the same unit as the input" },
		{ "Seed", EC::Seed, "Seed" });
	SetOutputs(
		{ "X", EC::Float, "X with perturbation" },
		{ "Y", EC::Float, "Y with perturbation" },
		{ "Z", EC::Float, "Z with perturbation" });
}

