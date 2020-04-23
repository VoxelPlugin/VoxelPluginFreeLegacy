// Copyright 2020 Phyronnaz

#include "VoxelShaders/VoxelDistanceFieldShader.h"
#include "VoxelIntVectorUtilities.h"
#include "VoxelThreadingUtilities.h"

#include "ShaderParameterUtils.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FVoxelDistanceFieldParameters, "VoxelDistanceFieldParameters");

FVoxelDistanceFieldBaseCS::FVoxelDistanceFieldBaseCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	Src.Bind(Initializer.ParameterMap, TEXT("Src"));
	Dst.Bind(Initializer.ParameterMap, TEXT("Dst"));
}

void FVoxelDistanceFieldBaseCS::ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	OutEnvironment.SetDefine(TEXT("NUM_THREADS_CS"), VOXEL_DISTANCE_FIELD_NUM_THREADS_CS);
	//OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
}

#if ENGINE_MINOR_VERSION < 25
bool FVoxelDistanceFieldBaseCS::Serialize(FArchive& Ar)
{
	const bool bShaderHasOutdatedParams = FGlobalShader::Serialize(Ar);
	Ar << Src;
	Ar << Dst;
	return bShaderHasOutdatedParams;
}
#endif

void FVoxelDistanceFieldBaseCS::SetBuffers(
		FRHICommandList& RHICmdList,
		const FRWBuffer& SrcBuffer,
		const FRWBuffer& DstBuffer) const
{
	Src.SetBuffer(RHICmdList, UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), SrcBuffer);
	Dst.SetBuffer(RHICmdList, UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), DstBuffer);
}

void FVoxelDistanceFieldBaseCS::SetUniformBuffers(FRHICommandList& RHICmdList, const FVoxelDistanceFieldParameters& Parameters) const
{
	const FVoxelDistanceFieldParametersRef ParametersBuffer = FVoxelDistanceFieldParametersRef::CreateUniformBufferImmediate(Parameters, UniformBuffer_MultiFrame);
	SetUniformBufferParameter(RHICmdList, UE_25_SWITCH(GetComputeShader(), RHICmdList.GetBoundComputeShader()), GetUniformBufferParameter<FVoxelDistanceFieldParameters>(), ParametersBuffer);
}

IMPLEMENT_TYPE_LAYOUT(FVoxelDistanceFieldBaseCS)
IMPLEMENT_SHADER_TYPE(, FVoxelExpandDistanceFieldCS, TEXT("/Plugin/Voxel/Private/DistanceField.usf"), TEXT("ExpandDistanceField"), SF_Compute);
IMPLEMENT_SHADER_TYPE(, FVoxelComputeDistanceFieldFromValuesCS, TEXT("/Plugin/Voxel/Private/DistanceField.usf"), TEXT("ComputeDistanceFieldFromValues"), SF_Compute);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldShaderHelper::StartCompute(
	const FIntVector& Size,
	const TVoxelSharedRef<TArray<FFloat16>>& InOutData,
	int32 NumberOfPasses,
	bool bInputIsDensities)
{
	VOXEL_FUNCTION_COUNTER();

#if VOXEL_DEBUG
	if (!bInputIsDensities)
	{
		for (auto& F : *InOutData) ensureVoxelSlow(F >= 0);
	}
#endif
	
	check(InOutData->Num() == Size.X * Size.Y * Size.Z);
	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);
	
	ensure(Fence.IsFenceComplete());
	
	ENQUEUE_RENDER_COMMAND(VoxelDistanceFieldCompute)(
		FVoxelUtilities::MakeVoxelWeakPtrLambda<FRHICommandListImmediate&>(this, [=](FVoxelDistanceFieldShaderHelper& Helper, FRHICommandListImmediate& RHICmdList)
		{
			Helper.Compute_RenderThread(RHICmdList, Size, InOutData, NumberOfPasses, bInputIsDensities);
		}));
	
	Fence.BeginFence();
}

void FVoxelDistanceFieldShaderHelper::WaitForCompletion() const
{
	VOXEL_FUNCTION_COUNTER();
	Fence.Wait();
}

void FVoxelDistanceFieldShaderHelper::Compute_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	const FIntVector& Size,
	const TVoxelSharedRef<TArray<FFloat16>>& InOutData,
	int32 NumberOfPasses,
	bool bInputIsDensities)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);
	const int32 Num = Size.X * Size.Y * Size.Z;
	check(InOutData->Num() == Num);
	
	if (AllocatedSize != Size)
	{
		VOXEL_SCOPE_COUNTER("Create Buffers");
		
		AllocatedSize = Size;

		SrcBuffer.Release();
		DstBuffer.Release();
		
		SrcBuffer.Initialize(sizeof(FFloat16), Num, PF_R16F);
		DstBuffer.Initialize(sizeof(FFloat16), Num, PF_R16F);
	}
	
	{
		VOXEL_SCOPE_COUNTER("Copy Data To Buffers");
		void* BufferData = RHICmdList.LockVertexBuffer(SrcBuffer.Buffer, 0, SrcBuffer.NumBytes, EResourceLockMode::RLM_WriteOnly);
		FMemory::Memcpy(BufferData, InOutData->GetData(), SrcBuffer.NumBytes);
		RHICmdList.UnlockVertexBuffer(SrcBuffer.Buffer);
	}

	if (bInputIsDensities)
	{
		ApplyComputeShader<FVoxelComputeDistanceFieldFromValuesCS>(RHICmdList, Size, 1);
	}

	ApplyComputeShader<FVoxelExpandDistanceFieldCS>(RHICmdList, Size, NumberOfPasses);

	// To copy data
	RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, DstBuffer.UAV);
	
	{
		VOXEL_SCOPE_COUNTER("Copy Data From Buffers");
		void* BufferData = RHICmdList.LockVertexBuffer(DstBuffer.Buffer, 0, DstBuffer.NumBytes, EResourceLockMode::RLM_ReadOnly);
		FMemory::Memcpy(InOutData->GetData(), BufferData, DstBuffer.NumBytes);
		RHICmdList.UnlockVertexBuffer(DstBuffer.Buffer);
	}

	// Make sure to release the buffers, else will crash on DX12!
	SrcBuffer.Release();
	DstBuffer.Release();
}

template<typename T>
void FVoxelDistanceFieldShaderHelper::ApplyComputeShader(
	FRHICommandListImmediate& RHICmdList,
	const FIntVector& Size,
	int32 NumberOfPasses)
{
	check(IsInRenderingThread());
	
	const TShaderMapRef<T> ComputeShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(UE_25_SWITCH(ComputeShader->GetComputeShader(), ComputeShader.GetComputeShader()));

	FVoxelDistanceFieldParameters Parameters;
	Parameters.SizeX = Size.X;
	Parameters.SizeY = Size.Y;
	Parameters.SizeZ = Size.Z;
	ComputeShader->SetUniformBuffers(RHICmdList, Parameters);
	
	const FIntVector NumThreads = FVoxelUtilities::DivideCeil(Size, VOXEL_DISTANCE_FIELD_NUM_THREADS_CS);
	check(NumThreads.X > 0 && NumThreads.Y > 0 && NumThreads.Z > 0);

	for (int32 Pass = 0; Pass < FMath::Min<int32>(NumberOfPasses, Size.GetMax()); Pass++)
	{
		RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, SrcBuffer.UAV);
		RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, DstBuffer.UAV);

		ComputeShader->SetBuffers(RHICmdList, SrcBuffer, DstBuffer);
		RHICmdList.DispatchComputeShader(NumThreads.X, NumThreads.Y, NumThreads.Z);
		Swap(SrcBuffer, DstBuffer);
	}
}