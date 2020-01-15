// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelNoiseNodesEnums.h"

FString FVoxelNoiseNodesEnums::GetFastNoiseName(EInterp Interpolation)
{
	switch (Interpolation)
	{
	case EInterp::Linear:
		return "FastNoise::Linear";
	case EInterp::Hermite:
		return "FastNoise::Hermite";
	case EInterp::Quintic:
		return "FastNoise::Quintic";
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
		return "FastNoise::FBM";
	case EFractalType::Billow:
		return "FastNoise::Billow";
	case EFractalType::RigidMulti:
		return "FastNoise::RigidMulti";
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
		return "FastNoise::Euclidean";
	case ECellularDistanceFunction::Manhattan:
		return "FastNoise::Manhattan";
	case ECellularDistanceFunction::Natural:
		return "FastNoise::Natural";
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
		return "FastNoise::CellValue";
	case ECellularReturnType::Distance:
		return "FastNoise::Distance";
	case ECellularReturnType::Distance2:
		return "FastNoise::Distance2";
	case ECellularReturnType::Distance2Add:
		return "FastNoise::Distance2Add";
	case ECellularReturnType::Distance2Sub:
		return "FastNoise::Distance2Sub";
	case ECellularReturnType::Distance2Mul:
		return "FastNoise::Distance2Mul";
	case ECellularReturnType::Distance2Div:
		return "FastNoise::Distance2Div";
	default:
		check(false);
		return "";
	}
}

FastNoise::Interp FVoxelNoiseNodesEnums::GetFastNoiseEnum(EInterp Interpolation)
{
	switch (Interpolation)
	{
	case EInterp::Linear: 
		return FastNoise::Linear;
	case EInterp::Hermite:
		return FastNoise::Hermite;
	case EInterp::Quintic:
		return FastNoise::Quintic;
	default:;
		check(false);
		return FastNoise::Linear;
	}
}

FastNoise::FractalType FVoxelNoiseNodesEnums::GetFastNoiseEnum(EFractalType FractalType)
{
	switch (FractalType)
	{
	case EFractalType::FBM:
		return FastNoise::FBM;
	case EFractalType::Billow:
		return FastNoise::Billow;
	case EFractalType::RigidMulti:
		return FastNoise::RigidMulti;
	default:
		check(false);
		return FastNoise::FBM;
	}
}

FastNoise::CellularDistanceFunction FVoxelNoiseNodesEnums::GetFastNoiseEnum(ECellularDistanceFunction DistanceFunction)
{
	switch (DistanceFunction)
	{
	case ECellularDistanceFunction::Euclidean:
		return FastNoise::Euclidean;
	case ECellularDistanceFunction::Manhattan:
		return FastNoise::Manhattan;
	case ECellularDistanceFunction::Natural:
		return FastNoise::Natural;
	default:
		check(false);
		return FastNoise::Euclidean;
	}
}

FastNoise::CellularReturnType FVoxelNoiseNodesEnums::GetFastNoiseEnum(ECellularReturnType ReturnType)
{
	switch (ReturnType)
	{
	case ECellularReturnType::CellValue:
		return FastNoise::CellValue;
	case ECellularReturnType::Distance:
		return FastNoise::Distance;
	case ECellularReturnType::Distance2:
		return FastNoise::Distance2;
	case ECellularReturnType::Distance2Add:
		return FastNoise::Distance2Add;
	case ECellularReturnType::Distance2Sub:
		return FastNoise::Distance2Sub;
	case ECellularReturnType::Distance2Mul:
		return FastNoise::Distance2Mul;
	case ECellularReturnType::Distance2Div:
		return FastNoise::Distance2Div;
	default:
		check(false);
		return FastNoise::CellValue;
	}
}