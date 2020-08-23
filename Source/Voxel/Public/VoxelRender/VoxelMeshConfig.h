// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMeshConfig.generated.h"

enum class ERendererStencilMask : uint8;

USTRUCT(BlueprintType)
struct FVoxelMeshConfig
{
	GENERATED_BODY()

	FVoxelMeshConfig() = default;

	/** Whether the primitive receives decals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	bool bReceivesDecals = true;
	
	/** If true, this component will be rendered in the CustomDepth pass (usually used for outlines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel", meta=(DisplayName = "Render CustomDepth Pass"))
	bool bRenderCustomDepth = false;
	
	/** Mask used for stencil buffer writes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel", meta=(EditCondition="bRenderCustomDepth"))
	ERendererStencilMask CustomDepthStencilWriteMask = ERendererStencilMask(0);

	/** Optionally write this 0-255 value to the stencil buffer in CustomDepth pass (Requires project setting or r.CustomDepth == 3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel",  meta=(UIMin = "0", UIMax = "255", EditCondition="bRenderCustomDepth", DisplayName = "CustomDepth Stencil Value"))
	int32 CustomDepthStencilValue = 0;

	template<typename T>
	inline auto& ApplyTo(T& Mesh) const
	{
		Mesh.bReceivesDecals = bReceivesDecals;
		Mesh.bRenderCustomDepth = bRenderCustomDepth;
		Mesh.CustomDepthStencilWriteMask = CustomDepthStencilWriteMask;
		Mesh.CustomDepthStencilValue = CustomDepthStencilValue;
		return *this;
	}
	template<typename T>
	inline auto& CopyFrom(T& Mesh)
	{
		bReceivesDecals = Mesh.bReceivesDecals;
		bRenderCustomDepth = Mesh.bRenderCustomDepth;
		CustomDepthStencilWriteMask = Mesh.CustomDepthStencilWriteMask;
		CustomDepthStencilValue = Mesh.CustomDepthStencilValue;
		return *this;
	}
};

inline uint32 GetTypeHash(const FVoxelMeshConfig& Config)
{
	return
		Config.bReceivesDecals * 131 +
		Config.bRenderCustomDepth * 5413 +
		uint8(Config.CustomDepthStencilWriteMask) * 56453 +
		Config.CustomDepthStencilValue * 26737;
}

inline bool operator==(const FVoxelMeshConfig& A, const FVoxelMeshConfig& B)
{
	return
		A.bReceivesDecals == B.bReceivesDecals &&
		A.bRenderCustomDepth == B.bRenderCustomDepth &&
		A.CustomDepthStencilWriteMask == B.CustomDepthStencilWriteMask &&
		A.CustomDepthStencilValue == B.CustomDepthStencilValue;
}