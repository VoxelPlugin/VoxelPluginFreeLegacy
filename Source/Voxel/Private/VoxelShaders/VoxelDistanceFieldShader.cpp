// Copyright 2021 Phyronnaz

#include "VoxelShaders/VoxelDistanceFieldShader.h"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"

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
}

#if VOXEL_ENGINE_VERSION  < 425
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_TYPE_LAYOUT(FVoxelDistanceFieldBaseCS)
IMPLEMENT_SHADER_TYPE(, FVoxelJumpFloodCS, TEXT("/Plugin/Voxel/Private/DistanceField.usf"), TEXT("ExpandDistanceField"), SF_Compute);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldShaderHelper::WaitForCompletion() const
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	Fence.Wait();
}

void FVoxelDistanceFieldShaderHelper::StartCompute(const FIntVector& Size, const TVoxelSharedRef<TArray<FVector>>& InOutData, int32 MaxPasses_Debug)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	check(InOutData->Num() == Size.X * Size.Y * Size.Z);
	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);
	
	ensure(Fence.IsFenceComplete());
	
	ENQUEUE_RENDER_COMMAND(VoxelDistanceFieldCompute)(
		MakeVoxelWeakPtrLambda(this, [=](FRHICommandListImmediate& RHICmdList)
		{
			Compute_RenderThread(RHICmdList, Size, GetData(*InOutData), GetNum(*InOutData), MaxPasses_Debug);
		}));
	
	Fence.BeginFence();
}

void FVoxelDistanceFieldShaderHelper::Compute_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	const FIntVector& Size,
	FVector* RESTRICT const Data,
	const int32 Num,
	int32 MaxPasses_Debug)
{
	VOXEL_RENDER_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	check(Size.X > 0 && Size.Y > 0 && Size.Z > 0);
	check(Num == Size.X * Size.Y * Size.Z);
	
	if (AllocatedSize != Size)
	{
		VOXEL_RENDER_SCOPE_COUNTER("Create Buffers");
		
		AllocatedSize = Size;
		
		SrcBuffer.Initialize(UE_5_ONLY(TEXT("SrcBuffer"),) sizeof(float), 3 * Num, PF_R32_FLOAT);
		DstBuffer.Initialize(UE_5_ONLY(TEXT("DstBuffer"),) sizeof(float), 3 * Num, PF_R32_FLOAT);
	}
	
	{
		VOXEL_RENDER_SCOPE_COUNTER("Copy Data To Buffers");
		void* BufferData = RHICmdList.LockVertexBuffer(SrcBuffer.Buffer, 0, SrcBuffer.NumBytes, EResourceLockMode::RLM_WriteOnly);
		FMemory::Memcpy(BufferData, Data, SrcBuffer.NumBytes);
		RHICmdList.UnlockVertexBuffer(SrcBuffer.Buffer);
	}

	const int32 PowerOfTwo = FMath::CeilLogTwo(Size.GetMax());
	for (int32 Pass = 0; Pass < PowerOfTwo; Pass++)
	{
		if (MaxPasses_Debug == Pass)
		{
			break;
		}
	
		// -1: we want to start with half the size
		const int32 Step = 1 << (PowerOfTwo - 1 - Pass);
		ApplyComputeShader<FVoxelJumpFloodCS>(RHICmdList, Size, Step);
	}

	// To copy data
#if VOXEL_ENGINE_VERSION < 426
	RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, DstBuffer.UAV);
#else
	RHICmdList.Transition(FRHITransitionInfo(DstBuffer.UAV, ERHIAccess::Unknown, ERHIAccess::UAVCompute)); // TODO not unknown?
#endif
	
	{
		VOXEL_RENDER_SCOPE_COUNTER("Copy Data From Buffers");
		void* BufferData = RHICmdList.LockVertexBuffer(SrcBuffer.Buffer, 0, SrcBuffer.NumBytes, EResourceLockMode::RLM_ReadOnly);
		FMemory::Memcpy(Data, BufferData, SrcBuffer.NumBytes);
		RHICmdList.UnlockVertexBuffer(SrcBuffer.Buffer);
	}

	// Make sure to release the buffers, else will crash on DX12!
	SrcBuffer.Release();
	DstBuffer.Release();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void FVoxelDistanceFieldShaderHelper::ApplyComputeShader(
	FRHICommandListImmediate& RHICmdList,
	const FIntVector& Size,
	int32 Step)
{
	check(IsInRenderingThread());
	
	const TShaderMapRef<T> ComputeShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(UE_25_SWITCH(ComputeShader->GetComputeShader(), ComputeShader.GetComputeShader()));

	FVoxelDistanceFieldParameters Parameters;
	Parameters.SizeX = Size.X;
	Parameters.SizeY = Size.Y;
	Parameters.SizeZ = Size.Z;
	Parameters.Step = Step;
	ComputeShader->SetUniformBuffers(RHICmdList, Parameters);
	
	const FIntVector NumThreads = FVoxelUtilities::DivideCeil(Size, VOXEL_DISTANCE_FIELD_NUM_THREADS_CS);
	check(NumThreads.X > 0 && NumThreads.Y > 0 && NumThreads.Z > 0);
	
#if VOXEL_ENGINE_VERSION < 426
	RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, SrcBuffer.UAV);
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, DstBuffer.UAV);
#else
	RHICmdList.Transition(FRHITransitionInfo(SrcBuffer.UAV, ERHIAccess::UAVCompute, ERHIAccess::UAVCompute));
	RHICmdList.Transition(FRHITransitionInfo(DstBuffer.UAV, ERHIAccess::UAVCompute, ERHIAccess::UAVCompute));
#endif
	
	ComputeShader->SetBuffers(RHICmdList, SrcBuffer, DstBuffer);
	RHICmdList.DispatchComputeShader(NumThreads.X, NumThreads.Y, NumThreads.Z);
	Swap(SrcBuffer, DstBuffer);
}