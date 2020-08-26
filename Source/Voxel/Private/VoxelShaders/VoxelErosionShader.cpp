// Copyright 2020 Phyronnaz

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

#if ENGINE_MINOR_VERSION < 25
bool FVoxelErosionCS::Serialize(FArchive& Ar)
{
	const bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);
	Ar << RainMap;
	Ar << TerrainHeight;
	Ar << TerrainHeight1;
	Ar << WaterHeight;
	Ar << WaterHeight1;
	Ar << WaterHeight2;
	Ar << Sediment;
	Ar << Sediment1;
	Ar << Outflow;
	Ar << Velocity;
	return bShaderHasOutdatedParams;
}
#endif

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
#define PROCESS_SURFACE(Name) SetUAVParameter(RHICmdList, UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), Name, Name##UAV);
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
	SetUniformBufferParameter(RHICmdList, UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), GetUniformBufferParameter<FVoxelErosionParameters>(), ParametersBuffer);
}

/* Unbinds buffers that will be used elsewhere */
void FVoxelErosionCS::UnbindBuffers(FRHICommandList& RHICmdList)
{
#define PROCESS_SURFACE(Name) RHICmdList.SetUAVParameter(UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), Name.GetBaseIndex(), FUnorderedAccessViewRHIRef());
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