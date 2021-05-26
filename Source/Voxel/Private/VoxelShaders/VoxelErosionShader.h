// Copyright 2021 Phyronnaz

#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "VoxelMinimal.h"
#include "ShaderParameterMacros.h"

#define VOXEL_EROSION_NUM_THREADS_CS 32

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelErosionParameters, )

// Texture size
SHADER_PARAMETER(uint32, size)
SHADER_PARAMETER(float, dt)

// Size of a pipe
SHADER_PARAMETER(float, l)
// Gravity
SHADER_PARAMETER(float, g)

// Sediment capacity
SHADER_PARAMETER(float, Kc)
// Sediment dissolving
SHADER_PARAMETER(float, Ks)
// Sediment deposition
SHADER_PARAMETER(float, Kd)

// Rain strength
SHADER_PARAMETER(float, Kr)
// Evaporation
SHADER_PARAMETER(float, Ke)

END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FVoxelErosionParameters> FVoxelErosionParametersRef;

class FVoxelErosionCS : public FGlobalShader
{
	DECLARE_TYPE_LAYOUT(FVoxelErosionCS, NonVirtual);
public:
	FVoxelErosionCS() = default;
	FVoxelErosionCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

#if VOXEL_ENGINE_VERSION  < 425
	virtual bool Serialize(FArchive& Ar) override;
#endif

	void SetSurfaces(
		FRHICommandList& RHICmdList,
		FUnorderedAccessViewRHIRef RainMapUAV,
		FUnorderedAccessViewRHIRef TerrainHeightUAV,
		FUnorderedAccessViewRHIRef TerrainHeight1UAV,
		FUnorderedAccessViewRHIRef WaterHeightUAV,
		FUnorderedAccessViewRHIRef WaterHeight1UAV,
		FUnorderedAccessViewRHIRef WaterHeight2UAV,
		FUnorderedAccessViewRHIRef SedimentUAV,
		FUnorderedAccessViewRHIRef Sediment1UAV,
		FUnorderedAccessViewRHIRef OutflowUAV,
		FUnorderedAccessViewRHIRef VelocityUAV);
		
	void SetUniformBuffers(FRHICommandList& RHICmdList, const FVoxelErosionParameters& Parameters);
	void UnbindBuffers(FRHICommandList& RHICmdList);

private:
	LAYOUT_FIELD(FShaderResourceParameter, RainMap);
	LAYOUT_FIELD(FShaderResourceParameter, TerrainHeight);
	LAYOUT_FIELD(FShaderResourceParameter, TerrainHeight1);
	LAYOUT_FIELD(FShaderResourceParameter, WaterHeight);
	LAYOUT_FIELD(FShaderResourceParameter, WaterHeight1);
	LAYOUT_FIELD(FShaderResourceParameter, WaterHeight2);
	LAYOUT_FIELD(FShaderResourceParameter, Sediment);
	LAYOUT_FIELD(FShaderResourceParameter, Sediment1);
	LAYOUT_FIELD(FShaderResourceParameter, Outflow);
	LAYOUT_FIELD(FShaderResourceParameter, Velocity);
};

class FVoxelErosionWaterIncrementCS : public FVoxelErosionCS
{
	DECLARE_SHADER_TYPE(FVoxelErosionWaterIncrementCS, Global);

	using FVoxelErosionCS::FVoxelErosionCS;
};

class FVoxelErosionFlowSimulationCS : public FVoxelErosionCS
{
	DECLARE_SHADER_TYPE(FVoxelErosionFlowSimulationCS, Global);

	using FVoxelErosionCS::FVoxelErosionCS;
};

class FVoxelErosionErosionDepositionCS : public FVoxelErosionCS
{
	DECLARE_SHADER_TYPE(FVoxelErosionErosionDepositionCS, Global);

	using FVoxelErosionCS::FVoxelErosionCS;
};

class FVoxelErosionSedimentTransportationCS : public FVoxelErosionCS
{
	DECLARE_SHADER_TYPE(FVoxelErosionSedimentTransportationCS, Global);

	using FVoxelErosionCS::FVoxelErosionCS;
};

class FVoxelErosionEvaporationCS : public FVoxelErosionCS
{
	DECLARE_SHADER_TYPE(FVoxelErosionEvaporationCS, Global);

	using FVoxelErosionCS::FVoxelErosionCS;
};