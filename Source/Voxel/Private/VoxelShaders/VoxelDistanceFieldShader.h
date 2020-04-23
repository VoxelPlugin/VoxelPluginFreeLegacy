// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "ShaderParameterMacros.h"
#include "RenderCommandFence.h"
#include "GlobalShader.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelDistanceFieldParameters,)

	SHADER_PARAMETER(uint32, SizeX)
	SHADER_PARAMETER(uint32, SizeY)
	SHADER_PARAMETER(uint32, SizeZ)

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

#if ENGINE_MINOR_VERSION < 25
	virtual bool Serialize(FArchive& Ar) override;
#endif
	
	void SetBuffers(
		FRHICommandList& RHICmdList,
		const FRWBuffer& SrcBuffer,
		const FRWBuffer& DstBuffer) const;
		
	void SetUniformBuffers(
		FRHICommandList& RHICmdList, 
		const FVoxelDistanceFieldParameters& Parameters) const;

private:
	LAYOUT_FIELD(FRWShaderParameter, Src);
	LAYOUT_FIELD(FRWShaderParameter, Dst);
};

class FVoxelExpandDistanceFieldCS : public FVoxelDistanceFieldBaseCS
{
public:
	DECLARE_SHADER_TYPE(FVoxelExpandDistanceFieldCS, Global);

	using FVoxelDistanceFieldBaseCS::FVoxelDistanceFieldBaseCS;
};

class FVoxelComputeDistanceFieldFromValuesCS : public FVoxelDistanceFieldBaseCS
{
public:
	DECLARE_SHADER_TYPE(FVoxelComputeDistanceFieldFromValuesCS, Global);

	using FVoxelDistanceFieldBaseCS::FVoxelDistanceFieldBaseCS;
};

// Input: voxel values
// Output: distance field, correct up to NumberOfPasses
class VOXEL_API FVoxelDistanceFieldShaderHelper : public TVoxelSharedFromThis<FVoxelDistanceFieldShaderHelper>
{
public:
	FVoxelDistanceFieldShaderHelper() = default;

	void StartCompute(
		const FIntVector& Size,
		const TVoxelSharedRef<TArray<FFloat16>>& InOutData,
		int32 NumberOfPasses,
		bool bInputIsDensities);
	void WaitForCompletion() const;

	void Compute_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		const FIntVector& Size,
		const TVoxelSharedRef<TArray<FFloat16>>& InOutData,
		int32 NumberOfPasses,
		bool bInputIsDensities);

private:
	FIntVector AllocatedSize = FIntVector::ZeroValue;
	
	FRWBuffer SrcBuffer;
	FRWBuffer DstBuffer;

	FRenderCommandFence Fence;

	template<typename T>
	void ApplyComputeShader(
		FRHICommandListImmediate& RHICmdList, 
		const FIntVector& Size,
		int32 NumberOfPasses);
};