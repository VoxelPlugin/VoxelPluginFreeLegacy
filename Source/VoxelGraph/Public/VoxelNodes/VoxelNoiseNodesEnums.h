// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "FastNoise.h"
#include "VoxelNoiseNodesEnums.generated.h"

UENUM(BlueprintType)
enum class EFractalType : uint8
{
	FBM,
	Billow,
	RigidMulti
};

UENUM(BlueprintType)
enum class EInterp : uint8
{
	Linear,
	Hermite,
	Quintic
};

UENUM(BlueprintType)
enum class ECellularDistanceFunction : uint8
{
	Euclidean,
	Manhattan,
	Natural
};

UENUM(BlueprintType)
enum class ECellularReturnType : uint8
{
	CellValue,
	Distance,
	Distance2,
	Distance2Add,
	Distance2Sub,
	Distance2Mul,
	Distance2Div
};

namespace FVoxelNoiseNodesEnums
{
	FString GetFastNoiseName(EInterp Interpolation);
	FString GetFastNoiseName(EFractalType FractalType);
	FString GetFastNoiseName(ECellularDistanceFunction DistanceFunction);
	FString GetFastNoiseName(ECellularReturnType ReturnType);
	FastNoise::Interp                   GetFastNoiseEnum(EInterp Interpolation);
	FastNoise::FractalType              GetFastNoiseEnum(EFractalType FractalType);
	FastNoise::CellularDistanceFunction GetFastNoiseEnum(ECellularDistanceFunction DistanceFunction);
	FastNoise::CellularReturnType       GetFastNoiseEnum(ECellularReturnType ReturnType);
}