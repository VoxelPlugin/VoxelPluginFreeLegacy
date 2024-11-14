// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "ShaderParameterMacros.h"
#include "RenderCommandFence.h"
#include "GlobalShader.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelDistanceFieldParameters,)

	SHADER_PARAMETER(uint32, SizeX)
	SHADER_PARAMETER(uint32, SizeY)
	SHADER_PARAMETER(uint32, SizeZ)
	SHADER_PARAMETER(uint32, Step)

END_GLOBAL_SHADER_PARAMETER_STRUCT()

#define VOXEL_DISTANCE_FIELD_NUM_THREADS_CS 8

typedef TUniformBufferRef<FVoxelDistanceFieldParameters> FVoxelDistanceFieldParametersRef;

class FVoxelDistanceFieldBaseCS : public FGlobalShader
{
	DECLARE_TYPE_LAYOUT(FVoxelDistanceFieldBaseCS, NonVirtual);
public:
	FVoxelDistanceFieldBaseCS() = default;
	FVoxelDistanceFieldBaseCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	
	void SetBuffers(
		FRHICommandList& RHICmdList,
		const FRWBuffer& SrcBuffer,
		const FRWBuffer& DstBuffer) const;
		
	void SetUniformBuffers(
		FRHICommandList& RHICmdList, 
		const FVoxelDistanceFieldParameters& Parameters) const;

private:
#if VOXEL_ENGINE_VERSION >= 503
	LAYOUT_FIELD(FShaderResourceParameter, Src);
	LAYOUT_FIELD(FShaderResourceParameter, Dst);
#else
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	LAYOUT_FIELD(FRWShaderParameter, Src);
	LAYOUT_FIELD(FRWShaderParameter, Dst);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelJumpFloodCS : public FVoxelDistanceFieldBaseCS
{
public:
    DECLARE_SHADER_TYPE(FVoxelJumpFloodCS, Global);

	using FVoxelDistanceFieldBaseCS::FVoxelDistanceFieldBaseCS;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelDistanceFieldShaderHelper : public TVoxelSharedFromThis<FVoxelDistanceFieldShaderHelper>
{
public:
	FVoxelDistanceFieldShaderHelper() = default;
	
	void WaitForCompletion() const;

	void StartCompute(
        const FIntVector& Size,
        const TVoxelSharedRef<TArray<FVector3f>>& InOutData, 
		int32 MaxPasses_Debug = -1);

	void Compute_RenderThread(
        FRHICommandListImmediate& RHICmdList,
        const FIntVector& Size,
        FVector3f* RESTRICT Data,
		int32 Num,
		int32 MaxPasses_Debug = -1);

private:
	FIntVector AllocatedSize = FIntVector::ZeroValue;
	
	FRWBuffer SrcBuffer;
	FRWBuffer DstBuffer;

	FRenderCommandFence Fence;

	template<typename T>
	void ApplyComputeShader(
		FRHICommandListImmediate& RHICmdList, 
		const FIntVector& Size,
		int32 Step);
};