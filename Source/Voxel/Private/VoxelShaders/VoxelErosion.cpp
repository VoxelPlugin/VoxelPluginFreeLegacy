// Copyright 2021 Phyronnaz

#include "VoxelShaders/VoxelErosion.h"
#include "VoxelShaders/VoxelErosionShader.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMessages.h"

#include "Engine/Texture2D.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

void UVoxelErosion::Initialize()
{
	if (bIsInit)
	{
		FVoxelMessages::Error("Erosion is already initialized!");
		return;
	}
	
	RealSize = FMath::Max(32, FMath::CeilToInt(Size / 32.f) * 32);;
	
	ENQUEUE_RENDER_COMMAND(Step)(
		[ThisPtr = this](FRHICommandList& RHICmdList) 
	{
		ThisPtr->Init_RenderThread();
	});

	FlushRenderingCommands();

	if (RainMapInit->GetSizeX() == RealSize &&
		RainMapInit->GetSizeY() == RealSize)
	{
		CopyTextureToRHI(*RainMapInit, RainMap);
	}
	else
	{
		FVoxelMessages::Error(
			FString::Printf(
				TEXT("Voxel Erosion Init: RainMapInit has size (%d, %d), but should have size (%d, %d)"),
				RainMapInit->GetSizeX(),
				RainMapInit->GetSizeY(),
				RealSize,
				RealSize),
			this);
	}
	
	if (HeightmapInit->GetSizeX() == RealSize &&
		HeightmapInit->GetSizeY() == RealSize)
	{
		CopyTextureToRHI(*HeightmapInit, TerrainHeight);
	}
	else
	{
		FVoxelMessages::Error(
			FString::Printf(
				TEXT("Voxel Erosion Init: HeightmapInit has size (%d, %d), but should have size (%d, %d)"),
				HeightmapInit->GetSizeX(),
				HeightmapInit->GetSizeY(),
				RealSize,
				RealSize),
			this);
	}

	bIsInit = true;
}

bool UVoxelErosion::IsInitialized() const
{
	return bIsInit;
}

void UVoxelErosion::Step(int32 Count)
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return;
	}
	
	FVoxelErosionParameters Parameters;
	Parameters.size = RealSize;
	Parameters.dt = DeltaTime;

	Parameters.l = Scale;
	Parameters.g = Gravity;

	Parameters.Kc = SedimentCapacity;
	Parameters.Ks = SedimentDissolving;
	Parameters.Kd = SedimentDeposition;

	Parameters.Kr = RainStrength;
	Parameters.Ke = Evaporation;

	ENQUEUE_RENDER_COMMAND(Step)(
		[Parameters, Count, ThisPtr = this](FRHICommandList& RHICmdList) 
	{
		ThisPtr->Step_RenderThread(Parameters, Count);
	});
}

FVoxelFloatTexture UVoxelErosion::GetTerrainHeightTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(TerrainHeight, Texture);
	return { TVoxelTexture<float>(Texture) };
}


FVoxelFloatTexture UVoxelErosion::GetWaterHeightTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(WaterHeight, Texture);
	return { TVoxelTexture<float>(Texture) };
}


FVoxelFloatTexture UVoxelErosion::GetSedimentTexture()
{
	if (!bIsInit)
	{
		FVoxelMessages::Error("Erosion is not initialized!");
		return {};
	}
	
	auto Texture = MakeVoxelShared<TVoxelTexture<float>::FTextureData>();
	CopyRHIToTexture(Sediment, Texture);
	return { TVoxelTexture<float>(Texture) };
}

template<typename T>
void UVoxelErosion::RunShader(const FVoxelErosionParameters& Parameters)
{
	FRHICommandListImmediate& RHICmdList = GRHICommandList.GetImmediateCommandList();
	
	TShaderMapRef<T> ComputeShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	RHICmdList.SetComputeShader(UE_25_SWITCH(ComputeShader->GetComputeShader(), ComputeShader.GetComputeShader()));

	ComputeShader->SetSurfaces(
		RHICmdList, 
		RainMapUAV, 
		TerrainHeightUAV, 
		TerrainHeight1UAV, 
		WaterHeightUAV, 
		WaterHeight1UAV, 
		WaterHeight2UAV, 
		SedimentUAV, 
		Sediment1UAV, 
		OutflowUAV, 
		VelocityUAV);
	ComputeShader->SetUniformBuffers(RHICmdList, Parameters);
	
	RHICmdList.DispatchComputeShader(RealSize / VOXEL_EROSION_NUM_THREADS_CS, RealSize / VOXEL_EROSION_NUM_THREADS_CS, 1);

	ComputeShader->UnbindBuffers(RHICmdList);
}

void UVoxelErosion::CopyTextureToRHI(const TVoxelTexture<float>& Texture, const FTexture2DRHIRef& RHITexture)
{
	ENQUEUE_RENDER_COMMAND(CopyTextureToRHI)([Texture, RHITexture, ThisPtr = this](FRHICommandList& RHICmdList)
	{
		ThisPtr->CopyTextureToRHI_RenderThread(Texture, RHITexture);
	});

	FlushRenderingCommands(UE_5_SWITCH(false,));
}

void UVoxelErosion::CopyRHIToTexture(const FTexture2DRHIRef& RHITexture, TVoxelSharedRef<TVoxelTexture<float>::FTextureData>& Texture)
{
	ENQUEUE_RENDER_COMMAND(CopyRHIToTexture)(
		[RHITexture, Texture, ThisPtr = this](FRHICommandList& RHICmdList)
	{
		ThisPtr->CopyRHIToTexture_RenderThread(RHITexture, *Texture);
	});

	FlushRenderingCommands(UE_5_SWITCH(false,));
}

void UVoxelErosion::CopyTextureToRHI_RenderThread(const TVoxelTexture<float>& Texture, const FTexture2DRHIRef& RHITexture)
{
	check(IsInRenderingThread());

	const int32 Size = RHITexture->GetSizeX();
	if (!ensureAlways(RHITexture->GetSizeY() == Size)) return;

	if (!ensureAlways(Texture.GetSizeX() == Size)) return;
	if (!ensureAlways(Texture.GetSizeY() == Size)) return;

	uint32 MappedStride = 0;
	float* const RHIData = static_cast<float*>(RHILockTexture2D(RHITexture, 0, RLM_WriteOnly, MappedStride, false));
	if (!ensureAlways(RHIData)) return;

	check(Texture.GetTextureData().Num() == Size * Size);
	FMemory::Memcpy(RHIData, Texture.GetTextureData().GetData(), Size * Size * sizeof(float));

	RHIUnlockTexture2D(RHITexture, 0, false);
}


void UVoxelErosion::CopyRHIToTexture_RenderThread(const FTexture2DRHIRef& RHITexture, TVoxelTexture<float>::FTextureData& Texture)
{
	check(IsInRenderingThread());

	const int32 Size = RHITexture->GetSizeX();
	if (!ensureAlways(RHITexture->GetSizeY() == Size)) return;

	uint32 MappedStride = 0;
	const float* RESTRICT const RHIData = static_cast<float*>(RHILockTexture2D(RHITexture, 0, RLM_ReadOnly, MappedStride, false));
	if (!ensureAlways(RHIData)) return;

	Texture.SetSize(Size, Size);
	
	for (int32 Index = 0; Index < Size * Size; Index++)
	{
		Texture.SetValue(Index, RHIData[Index]);
	}

	RHIUnlockTexture2D(RHITexture, 0, false);
}

void UVoxelErosion::Init_RenderThread()
{
	check(IsInRenderingThread());

	FRHIResourceCreateInfo CreateInfo UE_5_ONLY((TEXT("CreateInfo")));
	const UE_26_SWITCH(uint32, ETextureCreateFlags) Flags = TexCreate_ShaderResource | TexCreate_UAV;

#define CREATE_TEXTURE(Name, SizeX) \
	Name = RHICreateTexture2D(SizeX * RealSize, RealSize, PF_R32_FLOAT, 1, 1, Flags, CreateInfo); \
	Name##UAV = RHICreateUnorderedAccessView(Name); \

	CREATE_TEXTURE(RainMap, 1);
	CREATE_TEXTURE(TerrainHeight, 1);
	CREATE_TEXTURE(TerrainHeight1, 1);
	CREATE_TEXTURE(WaterHeight, 1);
	CREATE_TEXTURE(WaterHeight1, 1);
	CREATE_TEXTURE(WaterHeight2, 1);
	CREATE_TEXTURE(Sediment, 1);
	CREATE_TEXTURE(Sediment1, 1);
	CREATE_TEXTURE(Outflow, 4);
	CREATE_TEXTURE(Velocity, 2);

#undef CREATE_TEXTURE
}


void UVoxelErosion::Step_RenderThread(const FVoxelErosionParameters& Parameters, int32 Count)
{
	check(IsInRenderingThread());
	for (int32 Index = 0; Index < Count; Index++)
	{
		RunShader<FVoxelErosionWaterIncrementCS>(Parameters);
		RunShader<FVoxelErosionFlowSimulationCS>(Parameters);
		RunShader<FVoxelErosionErosionDepositionCS>(Parameters);
		RunShader<FVoxelErosionSedimentTransportationCS>(Parameters);
		RunShader<FVoxelErosionEvaporationCS>(Parameters);
	}
}