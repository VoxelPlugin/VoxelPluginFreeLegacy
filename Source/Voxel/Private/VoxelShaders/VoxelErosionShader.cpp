// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelShaders/VoxelErosionShader.h"
#include "ShaderParameterUtils.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelErosionParameters, "VoxelErosionParameters");

FVoxelErosionCS::FVoxelErosionCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
#define PROCESS_SURFACE(Name) Name.Bind(Initializer.ParameterMap, TEXT(#Name), EShaderParameterFlags::SPF_Optional);
	PROCESS_SURFACE(RainMap);
	PROCESS_SURFACE(TerrainHeight);
	PROCESS_SURFACE(TerrainHeight1);
	PROCESS_SURFACE(WaterHeight);
	PROCESS_SURFACE(WaterHeight1);
	PROCESS_SURFACE(WaterHeight2);
	PROCESS_SURFACE(Sediment);
	PROCESS_SURFACE(Sediment1);
	PROCESS_SURFACE(Outflow);
	PROCESS_SURFACE(Velocity);
#undef PROCESS_SURFACE
}

void FVoxelErosionCS::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.SetDefine(TEXT("NUM_THREADS_CS"), VOXEL_EROSION_NUM_THREADS_CS);
	//OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
}

void FVoxelErosionCS::SetSurfaces(
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
	FUnorderedAccessViewRHIRef VelocityUAV)
{
#if VOXEL_ENGINE_VERSION >= 503
#define PROCESS_SURFACE(Name) SetUAVParameter(RHICmdList.GetScratchShaderParameters(), Name, Name##UAV);
#else
#define PROCESS_SURFACE(Name) SetUAVParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), Name, Name##UAV);
#endif
	PROCESS_SURFACE(RainMap);
	PROCESS_SURFACE(TerrainHeight);
	PROCESS_SURFACE(TerrainHeight1);
	PROCESS_SURFACE(WaterHeight);
	PROCESS_SURFACE(WaterHeight1);
	PROCESS_SURFACE(WaterHeight2);
	PROCESS_SURFACE(Sediment);
	PROCESS_SURFACE(Sediment1);
	PROCESS_SURFACE(Outflow);
	PROCESS_SURFACE(Velocity);
#undef PROCESS_SURFACE
}

void FVoxelErosionCS::SetUniformBuffers(FRHICommandList& RHICmdList, const FVoxelErosionParameters& Parameters)
{
	const FVoxelErosionParametersRef ParametersBuffer = FVoxelErosionParametersRef::CreateUniformBufferImmediate(Parameters, UniformBuffer_MultiFrame);
#if VOXEL_ENGINE_VERSION >= 503
	SetUniformBufferParameter(RHICmdList.GetScratchShaderParameters(), GetUniformBufferParameter<FVoxelErosionParameters>(), ParametersBuffer);
#else
	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundComputeShader(), GetUniformBufferParameter<FVoxelErosionParameters>(), ParametersBuffer);
#endif
}

/* Unbinds buffers that will be used elsewhere */
void FVoxelErosionCS::UnbindBuffers(FRHICommandList& RHICmdList)
{
#if VOXEL_ENGINE_VERSION >= 503
#define PROCESS_SURFACE(Name) SetUAVParameter(RHICmdList.GetScratchShaderParameters(), Name, FUnorderedAccessViewRHIRef());
#else
#define PROCESS_SURFACE(Name) RHICmdList.SetUAVParameter(RHICmdList.GetBoundComputeShader(), Name.GetBaseIndex(), FUnorderedAccessViewRHIRef());
#endif
	PROCESS_SURFACE(RainMap);
	PROCESS_SURFACE(TerrainHeight);
	PROCESS_SURFACE(TerrainHeight1);
	PROCESS_SURFACE(WaterHeight);
	PROCESS_SURFACE(WaterHeight1);
	PROCESS_SURFACE(WaterHeight2);
	PROCESS_SURFACE(Sediment);
	PROCESS_SURFACE(Sediment1);
	PROCESS_SURFACE(Outflow);
	PROCESS_SURFACE(Velocity);
#undef PROCESS_SURFACE
}

IMPLEMENT_TYPE_LAYOUT(FVoxelErosionCS);
IMPLEMENT_SHADER_TYPE(, FVoxelErosionWaterIncrementCS        , TEXT("/Plugin/Voxel/Private/Erosion.usf"), TEXT("WaterIncrement")        , SF_Compute);
IMPLEMENT_SHADER_TYPE(, FVoxelErosionFlowSimulationCS        , TEXT("/Plugin/Voxel/Private/Erosion.usf"), TEXT("FlowSimulation")        , SF_Compute);
IMPLEMENT_SHADER_TYPE(, FVoxelErosionErosionDepositionCS     , TEXT("/Plugin/Voxel/Private/Erosion.usf"), TEXT("ErosionDeposition")     , SF_Compute);
IMPLEMENT_SHADER_TYPE(, FVoxelErosionSedimentTransportationCS, TEXT("/Plugin/Voxel/Private/Erosion.usf"), TEXT("SedimentTransportation"), SF_Compute);
IMPLEMENT_SHADER_TYPE(, FVoxelErosionEvaporationCS           , TEXT("/Plugin/Voxel/Private/Erosion.usf"), TEXT("Evaporation")           , SF_Compute);