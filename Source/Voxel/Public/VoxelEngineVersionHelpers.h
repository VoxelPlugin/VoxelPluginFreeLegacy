// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefinitions.h"

#if VOXEL_ENGINE_VERSION >= 503
#include "RHICommandList.h"

FORCEINLINE FTexture2DRHIRef RHICreateTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 NumSamples, ETextureCreateFlags Flags, ERHIAccess ResourceState, const FRHIResourceCreateInfo& CreateInfo)
{
	return RHICreateTexture(
		FRHITextureCreateDesc::Create2D(CreateInfo.DebugName)
			.SetExtent((int32)SizeX, (int32)SizeY)
			.SetFormat((EPixelFormat)Format)
			.SetNumMips((uint8)NumMips)
			.SetNumSamples((uint8)NumSamples)
			.SetFlags(Flags)
			.SetInitialState(ResourceState)
			.SetExtData(CreateInfo.ExtData)
			.SetBulkData(CreateInfo.BulkData)
			.SetGPUMask(CreateInfo.GPUMask)
			.SetClearValue(CreateInfo.ClearValueBinding)
	);
}
FORCEINLINE FTexture2DRHIRef RHICreateTexture2D(uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, uint32 NumSamples, ETextureCreateFlags Flags, const FRHIResourceCreateInfo& CreateInfo)
{
	return RHICreateTexture2D(SizeX, SizeY, Format, NumMips, NumSamples, Flags, ERHIAccess::Unknown, CreateInfo);
}
#endif