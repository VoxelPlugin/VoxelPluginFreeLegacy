// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelNoiseNodesEnums.h"

FString FVoxelNoiseNodesEnums::GetFastNoiseName(EInterp Interpolation)
{
	switch (Interpolation)
	{
	case EInterp::Linear:
		return "FVoxelFastNoise::Linear";
	case EInterp::Hermite:
		return "FVoxelFastNoise::Hermite";
	case EInterp::Quintic:
		return "FVoxelFastNoise::Quintic";
	default:
		check(false);
		return "";
	}
}

FString FVoxelNoiseNodesEnums::GetFastNoiseName(EFractalType FractalType)
{
	switch (FractalType)
	{
	case EFractalType::FBM:
		return "FVoxelFastNoise::FBM";
	case EFractalType::Billow:
		return "FVoxelFastNoise::Billow";
	case EFractalType::RigidMulti:
		return "FVoxelFastNoise::RigidMulti";
	default:
		check(false);
		return "";
	}
}

FString FVoxelNoiseNodesEnums::GetFastNoiseName(ECellularDistanceFunction DistanceFunction)
{
	switch (DistanceFunction)
	{
	case ECellularDistanceFunction::Euclidean:
		return "FVoxelFastNoise::Euclidean";
	case ECellularDistanceFunction::Manhattan:
		return "FVoxelFastNoise::Manhattan";
	case ECellularDistanceFunction::Natural:
		return "FVoxelFastNoise::Natural";
	default:
		check(false);
		return "";
	}
}

FString FVoxelNoiseNodesEnums::GetFastNoiseName(ECellularReturnType ReturnType)
{
	switch (ReturnType)
	{
	case ECellularReturnType::CellValue:
		return "FVoxelFastNoise::CellValue";
	case ECellularReturnType::Distance:
		return "FVoxelFastNoise::Distance";
	case ECellularReturnType::Distance2:
		return "FVoxelFastNoise::Distance2";
	case ECellularReturnType::Distance2Add:
		return "FVoxelFastNoise::Distance2Add";
	case ECellularReturnType::Distance2Sub:
		return "FVoxelFastNoise::Distance2Sub";
	case ECellularReturnType::Distance2Mul:
		return "FVoxelFastNoise::Distance2Mul";
	case ECellularReturnType::Distance2Div:
		return "FVoxelFastNoise::Distance2Div";
	default:
		check(false);
		return "";
	}
}

FVoxelFastNoise::Interp FVoxelNoiseNodesEnums::GetFastNoiseEnum(EInterp Interpolation)
{
	switch (Interpolation)
	{
	case EInterp::Linear: 
		return FVoxelFastNoise::Linear;
	case EInterp::Hermite:
		return FVoxelFastNoise::Hermite;
	case EInterp::Quintic:
		return FVoxelFastNoise::Quintic;
	default:;
		check(false);
		return FVoxelFastNoise::Linear;
	}
}

FVoxelFastNoise::FractalType FVoxelNoiseNodesEnums::GetFastNoiseEnum(EFractalType FractalType)
{
	switch (FractalType)
	{
	case EFractalType::FBM:
		return FVoxelFastNoise::FBM;
	case EFractalType::Billow:
		return FVoxelFastNoise::Billow;
	case EFractalType::RigidMulti:
		return FVoxelFastNoise::RigidMulti;
	default:
		check(false);
		return FVoxelFastNoise::FBM;
	}
}

FVoxelFastNoise::CellularDistanceFunction FVoxelNoiseNodesEnums::GetFastNoiseEnum(ECellularDistanceFunction DistanceFunction)
{
	switch (DistanceFunction)
	{
	case ECellularDistanceFunction::Euclidean:
		return FVoxelFastNoise::Euclidean;
	case ECellularDistanceFunction::Manhattan:
		return FVoxelFastNoise::Manhattan;
	case ECellularDistanceFunction::Natural:
		return FVoxelFastNoise::Natural;
	default:
		check(false);
		return FVoxelFastNoise::Euclidean;
	}
}

FVoxelFastNoise::CellularReturnType FVoxelNoiseNodesEnums::GetFastNoiseEnum(ECellularReturnType ReturnType)
{
	switch (ReturnType)
	{
	case ECellularReturnType::CellValue:
		return FVoxelFastNoise::CellValue;
	case ECellularReturnType::Distance:
		return FVoxelFastNoise::Distance;
	case ECellularReturnType::Distance2:
		return FVoxelFastNoise::Distance2;
	case ECellularReturnType::Distance2Add:
		return FVoxelFastNoise::Distance2Add;
	case ECellularReturnType::Distance2Sub:
		return FVoxelFastNoise::Distance2Sub;
	case ECellularReturnType::Distance2Mul:
		return FVoxelFastNoise::Distance2Mul;
	case ECellularReturnType::Distance2Div:
		return FVoxelFastNoise::Distance2Div;
	default:
		check(false);
		return FVoxelFastNoise::CellValue;
	}
}